#if defined (CY_USING_HAL)
#include "cyhal.h"
#endif

#include "cybsp.h"
#include "cycfg_pins.h"
#include "cy_retarget_io.h"
#include "joycon_classic.h"
#include "joycon_status_led.h"
#include "motor.h"
#include "rc_car_logic.h"

#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>

#define APP_TASK_STACK_SIZE    (4096u)
#define APP_TASK_PRIORITY      (5u)

static void app_task(void *arg)
{
    (void)arg;

    motor_init();
    motor_stop();

    joycon_status_led_init();
    joycon_status_led_set_connecting();

    joycon_classic_init();

    for (;;)
    {
        joycon_classic_process();
        joycon_status_led_process();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void rc_car_logic_init(void)
{
    printf("RC car logic init\r\n");
}

void rc_car_logic_run(void)
{
    BaseType_t task_result;

    task_result = xTaskCreate(app_task,
                              "JoyCon Task",
                              APP_TASK_STACK_SIZE,
                              NULL,
                              APP_TASK_PRIORITY,
                              NULL);

    if (task_result != pdPASS)
    {
        printf("Failed to create JoyCon task\r\n");
        CY_ASSERT(0);
    }

    printf("Starting RC car scheduler\r\n");

    vTaskStartScheduler();

    CY_ASSERT(0);

    for (;;)
    {
    }
}

/* [] END OF FILE */
