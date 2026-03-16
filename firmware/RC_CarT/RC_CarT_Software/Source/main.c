#include "cybsp.h"
#include "cyhal.h"
#include "cy_retarget_io.h"
#include "motor.h"
#include "button.h"
#include <stdio.h>

int main(void)
{
    cy_rslt_t result = cybsp_init();
    if (result != CY_RSLT_SUCCESS) CY_ASSERT(0);

    __enable_irq();

    cy_retarget_io_init(CYBSP_DEBUG_UART_TX,
                        CYBSP_DEBUG_UART_RX,
                        CY_RETARGET_IO_BAUDRATE);

    setvbuf(stdout, NULL, _IONBF, 0);

    printf("Motor Test Program\r\n");
    printf("Press button to start\r\n");

    motor_init();
    button_init();

    motor_stop();   // 🔴 Zorgt dat motor zeker stil staat bij opstart

    while(1)
    {
        if(button_pressed())
        {
            printf("Sequence start\r\n");

            motor_forward(70);
            printf("Forward\r\n");
            cyhal_system_delay_ms(2000);

            motor_turn_left(70);
            printf("Left\r\n");
            cyhal_system_delay_ms(2000);

            motor_turn_right(70);
            printf("Right\r\n");
            cyhal_system_delay_ms(2000);

            motor_stop();
            printf("Stop\r\n");

            printf("Press button to run again\r\n");
        }
    }
}