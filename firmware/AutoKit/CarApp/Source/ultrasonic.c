#include "ultrasonic.h"
#include "pins.h"
#include "cyhal.h"

static float read_ultrasonic_sensor(cyhal_gpio_t trig_pin, cyhal_gpio_t echo_pin) {
    cyhal_gpio_write(trig_pin, 0);
    cyhal_system_delay_us(5);
    cyhal_gpio_write(trig_pin, 1);
    cyhal_system_delay_us(10);
    cyhal_gpio_write(trig_pin, 0);

    const uint32_t timeout = 20000;
    uint32_t pulse_start = 0, pulse_end = 0;

    while (!cyhal_gpio_read(echo_pin)) {
        pulse_start++;
        cyhal_system_delay_us(1);
        if (pulse_start > timeout) return 1000;
    }
    while (cyhal_gpio_read(echo_pin)) {
        pulse_end++;
        cyhal_system_delay_us(1);
        if (pulse_end > timeout) return 1000;
    }

    float duration = (float)pulse_end;
    return (duration * 0.0343f) / 2.0f;
}

void ultrasonic_init(void) {
    cyhal_gpio_init(ULTRASONIC_TRIG_RIGHT, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    cyhal_gpio_init(ULTRASONIC_ECHO_RIGHT, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, false);
    cyhal_gpio_init(ULTRASONIC_TRIG_LEFT, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    cyhal_gpio_init(ULTRASONIC_ECHO_LEFT, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, false);
}

float get_distance_right_cm(void) {
    return read_ultrasonic_sensor(ULTRASONIC_TRIG_RIGHT, ULTRASONIC_ECHO_RIGHT);
}

float get_distance_left_cm(void) {
    return read_ultrasonic_sensor(ULTRASONIC_TRIG_LEFT, ULTRASONIC_ECHO_LEFT);
}
