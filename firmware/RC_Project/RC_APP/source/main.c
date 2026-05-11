#if defined (CY_USING_HAL)
#include "cyhal.h"
#endif

#include "cybsp.h"
#include "cy_retarget_io.h"
#include "rc_car_logic.h"
#include <stdio.h>

#if defined(CYBSP_DEBUG_UART_TX)
#define APP_UART_TX CYBSP_DEBUG_UART_TX
#elif defined(CYBSP_UART_TX)
#define APP_UART_TX CYBSP_UART_TX
#else
#define APP_UART_TX (P5_1)
#endif

#if defined(CYBSP_DEBUG_UART_RX)
#define APP_UART_RX CYBSP_DEBUG_UART_RX
#elif defined(CYBSP_UART_RX)
#define APP_UART_RX CYBSP_UART_RX
#else
#define APP_UART_RX (P5_0)
#endif

int main(void)
{
    cy_rslt_t result;

#if defined (CY_DEVICE_SECURE) && defined (CY_USING_HAL)
    cyhal_wdt_t wdt_obj;

    result = cyhal_wdt_init(&wdt_obj, cyhal_wdt_get_max_timeout_ms());
    CY_ASSERT(CY_RSLT_SUCCESS == result);
    cyhal_wdt_free(&wdt_obj);
#endif

    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    __enable_irq();

    result = cy_retarget_io_init(APP_UART_TX, APP_UART_RX, CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    setvbuf(stdout, NULL, _IONBF, 0);
    printf("\033[2J\033[H");
    printf("Main started\r\n");

#if defined(TARGET_APP_CY8CKIT_062_WIFI_BT))
    printf("Detected board: CY8CKIT-062-WIFI-BT -> RC CAR\r\n");
    rc_car_logic_init();
    rc_car_logic_run();

#elif defined(TARGET_APP_CY8CPROTO_062_4343W)
    printf("Detected board: CY8CPROTO-062-4343W -> RC CART (not implemented yet)\r\n");

    while (1)
    {
    }

#else
    printf("Unknown board\r\n");

    while (1)
    {
    }
#endif

    CY_ASSERT(0);

    for (;;)
    {
    }
}

/* [] END OF FILE */
