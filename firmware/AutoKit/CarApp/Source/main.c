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

    // Force immediate prints
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("\033[2J\033[H");
    printf("Car App\n\r");
    printf("<20cm = STOP\n\r");

    ultrasonic_init();
    motor_init();
    button_init();
    imu_init();

    cyhal_system_delay_ms(1000);
    motor_stop();
    printf("Test OK. Press SW2!\n\r");

    bool car_running = false;

    while(1) {
        if (button_pressed()) {
            car_running = !car_running;
            Motion_Detected = false;
            if (car_running)
            {
                printf("\n\rSTART\n\r");
                motor_forward(65);
            } else {
                motor_stop();
                printf("\n\rSTOPPED\n\r");
            }
        }

        if (Motion_Detected) {
            motor_stop();
            printf("\n\rEMERGENCY LIFT STOP - Press SW2!\n\r");
            car_running = false;
            Motion_Detected = false;
        }

        if (car_running)
        {
            int16_t ax, ay, az, gx, gy, gz;
            imu_read_all(&ax, &ay, &az, &gx, &gy, &gz);

            float right = get_distance_right_cm();
            float left  = get_distance_left_cm();

            printf("R:%5.1f L:%5.1f AX:%6d AY:%6d AZ:%6d GX:%6d GY:%6d GZ:%6d Mot:%d\r",
                   right, left, ax, ay, az, gx, gy, gz, (int)Motion_Detected);

            // Lift detect
            if (az < 8000 || Motion_Detected)
            {
                motor_stop();
                printf("\n\rLIFT DETECTED - STOP\n\r");
                car_running = false;
                Motion_Detected = false;  // Reset flag
            }
            else if (right < 20.0f || left < 20.0f)
            {
                // motor_stop();
                // printf("\n\rOBSTACLE STOP\n\r");
                // car_running = false;
            }
            else if (right < 25.0f)
            {
                motor_turn_left(80);
            }
            else if (left < 25.0f)
            {
                motor_turn_right(80);
            }
            else
            {
                motor_forward(65);
            }

            cyhal_system_delay_ms(50);
        }
    }
}
