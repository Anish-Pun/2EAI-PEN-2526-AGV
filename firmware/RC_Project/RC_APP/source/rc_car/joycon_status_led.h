#ifndef JOYCON_STATUS_LED_H
#define JOYCON_STATUS_LED_H

void joycon_status_led_init(void);
void joycon_status_led_set_connecting(void);
void joycon_status_led_set_input_ready(void);
void joycon_status_led_process(void);

#endif