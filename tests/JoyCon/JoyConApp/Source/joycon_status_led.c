#include "joycon_status_led.h"

#include "cyhal.h"

#include <stdbool.h>
#include <stdint.h>

#define JOYCON_STATUS_LED_PIN              (P13_7)
#define JOYCON_STATUS_LED_ACTIVE_LOW       (1u)
#define JOYCON_STATUS_LED_BLINK_TICKS      (25u)

typedef enum
{
    JOYCON_LED_MODE_CONNECTING = 0,
    JOYCON_LED_MODE_INPUT_READY
} joycon_led_mode_t;

static joycon_led_mode_t led_mode = JOYCON_LED_MODE_CONNECTING;
static uint16_t blink_tick_counter = 0u;
static uint8_t blink_led_on = 0u;

static void joycon_status_led_write(uint8_t on)
{
    bool value = false;

    if (JOYCON_STATUS_LED_ACTIVE_LOW != 0u)
    {
        value = (on != 0u) ? false : true;
    }
    else
    {
        value = (on != 0u) ? true : false;
    }

    cyhal_gpio_write(JOYCON_STATUS_LED_PIN, value);
}

void joycon_status_led_init(void)
{
    cyhal_gpio_init(JOYCON_STATUS_LED_PIN,
                    CYHAL_GPIO_DIR_OUTPUT,
                    CYHAL_GPIO_DRIVE_STRONG,
                    true);

    led_mode = JOYCON_LED_MODE_CONNECTING;
    blink_tick_counter = 0u;
    blink_led_on = 0u;
    joycon_status_led_write(0u);
}

void joycon_status_led_set_connecting(void)
{
    led_mode = JOYCON_LED_MODE_CONNECTING;
    blink_tick_counter = 0u;
    blink_led_on = 0u;
    joycon_status_led_write(0u);
}

void joycon_status_led_set_input_ready(void)
{
    led_mode = JOYCON_LED_MODE_INPUT_READY;
    joycon_status_led_write(1u);
}

void joycon_status_led_process(void)
{
    if (led_mode != JOYCON_LED_MODE_CONNECTING)
    {
        return;
    }

    blink_tick_counter++;
    if (blink_tick_counter >= JOYCON_STATUS_LED_BLINK_TICKS)
    {
        blink_tick_counter = 0u;
        blink_led_on = (uint8_t)(blink_led_on == 0u ? 1u : 0u);
        joycon_status_led_write(blink_led_on);
    }
}