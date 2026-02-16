#include "button.h"
#include "pins.h"
#include "cy_retarget_io.h"

static bool last_state = true;
static bool press_edge = false;

void button_init(void) {
    cyhal_gpio_init(CAR_START_STOP_PIN, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLUP, true);
    printf("SW2 button ready\r\n");
}

bool button_pressed(void) {
    bool curr = !cyhal_gpio_read(CAR_START_STOP_PIN);
    press_edge = curr && !last_state;
    last_state = curr;
    return press_edge;
}
