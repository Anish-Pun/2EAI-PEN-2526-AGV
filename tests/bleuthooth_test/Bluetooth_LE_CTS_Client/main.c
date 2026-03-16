#include <stdio.h>

#include "cybsp_bt_config.h"
#include "wiced_bt_stack.h"
#include "cycfg_bt_settings.h"
#include "cybsp.h"
#include "cyhal.h"
#include "cy_retarget_io.h"

#include <FreeRTOS.h>
#include <task.h>

#include "cts_client.h"

int main()
{
    cy_rslt_t result;

    result = cybsp_init();

    if(result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    __enable_irq();

    cy_retarget_io_init(
        CYBSP_DEBUG_UART_TX,
        CYBSP_DEBUG_UART_RX,
        CY_RETARGET_IO_BAUDRATE);

    printf("PSoC BLE Central Start\n");

    cybt_platform_config_init(&cybsp_bt_platform_cfg);

    wiced_bt_stack_init(
            app_bt_management_callback,
            &wiced_bt_cfg_settings);

    vTaskStartScheduler();

    for(;;)
    {
    }
}