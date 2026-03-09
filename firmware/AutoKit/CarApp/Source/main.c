#include "cybsp.h"
#include "cyhal.h"
#include "cy_retarget_io.h"
#include "pins.h"
#include "ultrasonic.h"
#include "motor.h"
#include "button.h"
#include "imu.h"
#include <stdio.h>

int main(void) {
    cy_rslt_t result = cybsp_init();
    if (result != CY_RSLT_SUCCESS) CY_ASSERT(0);

    __enable_irq();
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS) CY_ASSERT(0);

    printf("\033[2J\033 r\n");
    printf(" Car App \r\n");
    printf("<20cm = STOP \r\n");

    ultrasonic_init();
    motor_init();
    button_init();

    cyhal_system_delay_ms(1000);
    motor_stop();
    printf("Test OK. Press SW2!\r\n");

    motor_stop();   // Motors 0% from start
    bool car_running = false;

    while(1) {
        // Check button press
        if (button_pressed()) {
            car_running = !car_running;
            if (car_running) {
                printf(" START \r\n");
                motor_forward(65);
            } else {
                motor_stop();
                printf("\n\rSTOPPED\n\r");
            }
        }

        // Auto drive
        if (car_running) {
            float right = get_distance_right_cm();
            float left = get_distance_left_cm();
            printf("R:%.1f L:%.1f ", right, left);

            if (right < 20.0f && left < 20.0f) {
                motor_stop();
                printf("\n\rLIFT DETECTED - STOP\n\r");
                car_running = false;
            } else if (right < 20.0f) {
                motor_turn_right(80);      // Right blocked → sharp LEFT
                printf(" LEFT \r\n");
            } else if (left < 20.0f) {
            	motor_turn_left(80);     // Left blocked → sharp RIGHT
                printf(" RIGHT \r\n");
            } else {
                motor_forward(65);
            }
            cyhal_system_delay_ms(50);
        }
    }
}
