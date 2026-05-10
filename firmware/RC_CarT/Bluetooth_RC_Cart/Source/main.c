#if defined (CY_USING_HAL)
#include "cyhal.h"
#endif
#include "cybsp.h"
#include "cycfg_pins.h"
#include "cy_retarget_io.h"
#include "joycon_classic.h"
#include "joycon_status_led.h"
#include "motor.h"
#include <FreeRTOS.h>
#include <task.h>
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

#define APP_TASK_STACK_SIZE    (4096u)
#define APP_TASK_PRIORITY      (5u)

static void app_task(void *arg)
{
    (void)arg;

    motor_init();
    motor_stop();
    
    joycon_classic_init();

    for (;;)
    {
        joycon_classic_process();
        joycon_status_led_process();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

int main(void)
{
    cy_rslt_t result;

#if defined (CY_DEVICE_SECURE) && defined (CY_USING_HAL)
    cyhal_wdt_t wdt_obj;

    /* Clear watchdog timer so that it doesn't trigger a reset */
    result = cyhal_wdt_init(&wdt_obj, cyhal_wdt_get_max_timeout_ms());
    CY_ASSERT(CY_RSLT_SUCCESS == result);
    cyhal_wdt_free(&wdt_obj);
#endif

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    result = cy_retarget_io_init(APP_UART_TX, APP_UART_RX, CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    setvbuf(stdout, NULL, _IONBF, 0);
    printf("\033[2J\033[H");
    printf("JoyCon Classic host started\r\n");

    xTaskCreate(app_task, "JoyCon Task", APP_TASK_STACK_SIZE, NULL, APP_TASK_PRIORITY, NULL);

    vTaskStartScheduler();

    CY_ASSERT(0);

    for (;;) {}
}

/* [] END OF FILE */