#include "line_tracker.h"
#include "pins.h"
#include "cyhal.h"

void line_tracker_init(void)
{
    cyhal_gpio_init(LINE_LEFT,
                    CYHAL_GPIO_DIR_INPUT,
                    CYHAL_GPIO_DRIVE_NONE,
                    false);

    cyhal_gpio_init(LINE_MIDDLE,
                    CYHAL_GPIO_DIR_INPUT,
                    CYHAL_GPIO_DRIVE_NONE,
                    false);

    cyhal_gpio_init(LINE_RIGHT,
                    CYHAL_GPIO_DIR_INPUT,
                    CYHAL_GPIO_DRIVE_NONE,
                    false);
}

bool line_left(void)
{
    return !cyhal_gpio_read(LINE_LEFT);
}

bool line_middle(void)
{
    return !cyhal_gpio_read(LINE_MIDDLE);
}

bool line_right(void)
{
    return !cyhal_gpio_read(LINE_RIGHT);
}