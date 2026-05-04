#if defined(CY_USING_HAL)
#include "cyhal.h"
#endif

#include "cybsp.h"
#include "cycfg_pins.h"
#include "cy_retarget_io.h"
#include "joycon_classic.h"
#include "joycon_status_led.h"
#include "motor.h"
#include "rc_car_logic.h"
#include "self_driving.h"
#include "ultrasonic.h"
#include "imu.h"
#include "speed.h"

#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <stdbool.h>

#define APP_TASK_STACK_SIZE    (4096u)
#define APP_TASK_PRIORITY      (5u)

static volatile rc_car_mode_t g_rc_car_mode = RC_CAR_MODE_BLUETOOTH;
static bool g_self_driving_initialized = false;

void rc_car_logic_set_mode(rc_car_mode_t mode)
{
    if (g_rc_car_mode == mode)
    {
        return;
    }

    if (mode == RC_CAR_MODE_SELF_DRIVING)
    {
        if (!g_self_driving_initialized)
        {
            ultrasonic_init();
            imu_init();
            speed_init();
            g_self_driving_initialized = true;
        }

        self_driving_enter();
        g_rc_car_mode = RC_CAR_MODE_SELF_DRIVING;
        printf("\r\nMode -> SELF DRIVING\r\n");
    }
    else
    {
        if (g_rc_car_mode == RC_CAR_MODE_SELF_DRIVING)
        {
            self_driving_exit();
        }

        motor_stop();
        g_rc_car_mode = RC_CAR_MODE_BLUETOOTH;
        printf("\r\nMode -> BLUETOOTH/MANUAL\r\n");
    }
}

rc_car_mode_t rc_car_logic_get_mode(void)
{
    return g_rc_car_mode;
}

bool rc_car_logic_is_self_driving_active(void)
{
    return (g_rc_car_mode == RC_CAR_MODE_SELF_DRIVING);
}

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

        if (g_rc_car_mode == RC_CAR_MODE_SELF_DRIVING)
        {
            self_driving_step();
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
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
