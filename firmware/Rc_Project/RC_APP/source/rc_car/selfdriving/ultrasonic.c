#include "ultrasonic.h"
#include "pins.h"
#include "cyhal.h"
#include "cy_retarget_io.h"

static float read_ultrasonic_sensor(cyhal_gpio_t trig_pin, cyhal_gpio_t echo_pin)
{
    cyhal_gpio_write(trig_pin, 0);
    cyhal_system_delay_us(5);
    cyhal_gpio_write(trig_pin, 1);
    cyhal_system_delay_us(10);
    cyhal_gpio_write(trig_pin, 0);

    const uint32_t timeout = 20000;
    uint32_t pulse_start = 0, pulse_end = 0;

    while (!cyhal_gpio_read(echo_pin))
    {
        pulse_start++;
        cyhal_system_delay_us(1);
        if (pulse_start > timeout)
            return -1.0f;   // invalid reading
    }

    while (cyhal_gpio_read(echo_pin))
    {
        pulse_end++;
        cyhal_system_delay_us(1);
        if (pulse_end > timeout)
            return -1.0f;   // invalid reading
    }

    float duration = (float)pulse_end;
    return (duration * 0.0343f) / 2.0f;
}

static void sort3(float *a, float *b, float *c)
{
    float t;

    if (*a > *b) { t = *a; *a = *b; *b = t; }
    if (*b > *c) { t = *b; *b = *c; *c = t; }
    if (*a > *b) { t = *a; *a = *b; *b = t; }
}

static float filtered_distance(cyhal_gpio_t trig_pin, cyhal_gpio_t echo_pin)
{
    float d1 = read_ultrasonic_sensor(trig_pin, echo_pin);
    cyhal_system_delay_ms(20);

    float d2 = read_ultrasonic_sensor(trig_pin, echo_pin);
    cyhal_system_delay_ms(20);

    float d3 = read_ultrasonic_sensor(trig_pin, echo_pin);

    if (d1 < 0.0f && d2 < 0.0f && d3 < 0.0f)
        return 1000.0f;

    if (d1 < 0.0f) d1 = 1000.0f;
    if (d2 < 0.0f) d2 = 1000.0f;
    if (d3 < 0.0f) d3 = 1000.0f;

    sort3(&d1, &d2, &d3);
    return d2;   // median
}

void ultrasonic_init(void)
{
    cyhal_gpio_init(ULTRASONIC_TRIG_RIGHT, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    cyhal_gpio_init(ULTRASONIC_ECHO_RIGHT, CYHAL_GPIO_DIR_INPUT,  CYHAL_GPIO_DRIVE_NONE,   false);
    cyhal_gpio_init(ULTRASONIC_TRIG_LEFT,  CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    cyhal_gpio_init(ULTRASONIC_ECHO_LEFT,  CYHAL_GPIO_DIR_INPUT,  CYHAL_GPIO_DRIVE_NONE,   false);

    cyhal_gpio_write(ULTRASONIC_TRIG_RIGHT, 0);
    cyhal_gpio_write(ULTRASONIC_TRIG_LEFT,  0);
    cyhal_system_delay_ms(100);
}

float get_distance_right_cm(void)
{
    return filtered_distance(ULTRASONIC_TRIG_RIGHT, ULTRASONIC_ECHO_RIGHT);
}

float get_distance_left_cm(void)
{
    return filtered_distance(ULTRASONIC_TRIG_LEFT, ULTRASONIC_ECHO_LEFT);
}
