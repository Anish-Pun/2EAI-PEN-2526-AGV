#include "cybsp.h"
#include "cyhal.h"
#include "cy_retarget_io.h"
#include "pins.h"
#include "ultrasonic.h"

int main(void) {
    cy_rslt_t result = cybsp_init();
    if (result != CY_RSLT_SUCCESS) CY_ASSERT(0);

    __enable_irq();

    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS) CY_ASSERT(0);

    printf("\033[2J\033[HUltrasonic Modular\r\n");

    ultrasonic_init();

    for(;;) {
        float right = get_distance_right_cm();
        float left  = get_distance_left_cm();

        if (right > 50) right = 50;
        if (left  > 50) left  = 50;

        printf("Right: %.2f cm | Left: %.2f cm\r\n", right, left);
        cyhal_system_delay_ms(200);
    }
}
