#include "cybsp.h"
#include "cyhal.h"
#include "cy_retarget_io.h"
#include "pins.h"
#include "ultrasonic.h"
#include "motor.h"
#include "button.h"
#include "imu.h"
#include "speed.h"
#include "self_driving.h"
#include "remote_control.h"
#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>

#define APP_TASK_STACK_SIZE    (4096u)
#define APP_TASK_PRIORITY      (5u)

static void app_task(void *arg)
{
    (void)arg;

    ultrasonic_init();
    motor_init();
    button_init();
    imu_init();
    speed_init();

    cyhal_system_delay_ms(200);
    motor_stop();

    printf("Bluetooth mode ready (default). Press SW2 to switch to self driving mode.\n\r");

    remote_control_set_enabled(1);
    remote_control_enter();

    typedef enum {
        MODE_BLUETOOTH = 0,
        MODE_SELF_DRIVING
    } app_mode_t;

    app_mode_t mode = MODE_BLUETOOTH;

    while (1)
    {
        if (button_pressed()) {
            if (mode == MODE_SELF_DRIVING && self_driving_is_halted()) {
                self_driving_enter();
            } else {
                if (mode == MODE_BLUETOOTH) {
                    remote_control_exit();
                    remote_control_set_enabled(0);
                    mode = MODE_SELF_DRIVING;
                    self_driving_enter();
                } else {
                    self_driving_exit();
                    mode = MODE_BLUETOOTH;
                    remote_control_set_enabled(1);
                    remote_control_enter();
                }
            }
        }

        if (mode == MODE_SELF_DRIVING)
            self_driving_step();
        else
            remote_control_step();
    }
}

int main(void) {
    cy_rslt_t result = cybsp_init();
    if (result != CY_RSLT_SUCCESS) CY_ASSERT(0);

    __enable_irq();
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS) CY_ASSERT(0);

    // Force immediate prints
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("\033[2J\033[H");
    printf("Car App\n\r");

    remote_control_init_ble();

    xTaskCreate(app_task, "App Task", APP_TASK_STACK_SIZE, NULL, APP_TASK_PRIORITY, NULL);

    vTaskStartScheduler();
    CY_ASSERT(0);

    while(1) {}
}
