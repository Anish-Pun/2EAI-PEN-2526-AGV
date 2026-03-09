#include "speed.h"
#include "pins.h"
#include "cy_retarget_io.h"

uint32_t Count_EncoderRight = 0;
uint32_t Count_EncoderLeft  = 0;

static void isr_right(void *arg, cyhal_gpio_event_t event) {
    if (event == CYHAL_GPIO_IRQ_RISE) {
        Count_EncoderRight++;
    }
}

static void isr_left(void *arg, cyhal_gpio_event_t event) {
    if (event == CYHAL_GPIO_IRQ_RISE) {
        Count_EncoderLeft++;
    }
}

static cyhal_gpio_callback_data_t cb_right = {.callback = isr_right, .callback_arg = NULL};
static cyhal_gpio_callback_data_t cb_left  = {.callback = isr_left,  .callback_arg = NULL};

void speed_init(void) {
    // Right encoder
    cyhal_gpio_init(ENCODER_RIGHT, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULL_NONE, false);
    cyhal_gpio_register_callback(ENCODER_RIGHT, &cb_right);
    cyhal_gpio_enable_event(ENCODER_RIGHT, CYHAL_GPIO_IRQ_RISE, 3, true);

    // Left encoder
    cyhal_gpio_init(ENCODER_LEFT, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULL_NONE, false);
    cyhal_gpio_register_callback(ENCODER_LEFT, &cb_left);
    cyhal_gpio_enable_event(ENCODER_LEFT, CYHAL_GPIO_IRQ_RISE, 3, true);

    reset_encoders();
    printf("Speed encoders init OK\n\r");
}

float get_speed_right_kmh(void) {
    float distance_m = (float)Count_EncoderRight / COUNTS_PER_METER;
    return distance_m * 3.6f;
}

float get_speed_left_kmh(void) {
    float distance_m = (float)Count_EncoderLeft / COUNTS_PER_METER;
    return distance_m * 3.6f;
}

void reset_encoders(void) {
    Count_EncoderRight = 0;
    Count_EncoderLeft = 0;
}
