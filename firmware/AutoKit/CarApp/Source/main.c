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
#include <stdio.h>

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

    ultrasonic_init();
    motor_init();
    button_init();
    imu_init();
    speed_init();

    cyhal_system_delay_ms(200);
    motor_stop();

    printf("Self-driving ready. Press SW2 to START/STOP\n\r");

    bool self_driving_enabled = false;

    while(1) {
        if (button_pressed()) {
            if (self_driving_enabled && self_driving_is_halted()) {
                self_driving_enter();
            } else {
                self_driving_enabled = !self_driving_enabled;
                if (self_driving_enabled)
                    self_driving_enter();
                else
                    self_driving_exit();
            }
        }

        if (self_driving_enabled)
            self_driving_step();
        else
            cyhal_system_delay_ms(50);
    }
}
