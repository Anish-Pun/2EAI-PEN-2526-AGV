#include "cybsp.h"
#include "cyhal.h"
#include "cy_retarget_io.h"
#include "pins.h"
#include "ultrasonic.h"
#include "motor.h"
#include "button.h"
#include "speed.h"
#include "self_driving.h"
#include "line_tracker.h"
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
    line_tracker_init();
    speed_init();


    cyhal_system_delay_ms(200);
    motor_stop();

    printf("Press SW2 to start.\n\r");

    bool running = false;

    while(1)
    {
        if(button_pressed())
        {
            running = !running;

            if(running)
            {
                printf("\n\rSTART\n\r");
            }
            else
            {
                printf("\n\rSTOP\n\r");
                motor_stop();
            }
        }

        if(running)
        {
            self_driving_step();
        }

        cyhal_system_delay_ms(50);
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



    xTaskCreate(app_task, "App Task", APP_TASK_STACK_SIZE, NULL, APP_TASK_PRIORITY, NULL);

    vTaskStartScheduler();
    CY_ASSERT(0);

    while(1) {}
}
