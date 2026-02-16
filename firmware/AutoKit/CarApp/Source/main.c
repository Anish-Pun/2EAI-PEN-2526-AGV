#include "cybsp.h"
#include "cyhal.h"
#include "cy_retarget_io.h"
#include "pins.h"
#include "ultrasonic.h"
#include "motor.h"

int main(void) {
    cy_rslt_t result = cybsp_init();
    if (result != CY_RSLT_SUCCESS) CY_ASSERT(0);

    __enable_irq();

    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS) CY_ASSERT(0);

    printf("\033[2J\033[Car App\r\n");
    printf("<20cm = STOP \r\n");

    ultrasonic_init();
    motor_init();

    while(1) {
        float right = get_distance_right_cm();
        float left  = get_distance_left_cm();

        if (right > 50) right = 50;
        if (left  > 50) left  = 50;

        printf("R:%.1f L:%.1f ", right, left);

        if (right < 20.0f || left < 20.0f) {
            motor_stop();
        } else {
            motor_forward(60);  // Forward speed
        }
        cyhal_system_delay_ms(200);
    }
}
