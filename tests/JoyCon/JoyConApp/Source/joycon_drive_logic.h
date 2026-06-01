#ifndef JOYCON_DRIVE_LOGIC_H
#define JOYCON_DRIVE_LOGIC_H

#include <stdint.h>

void joycon_drive_logic_reset(void);
void joycon_drive_logic_handle_hat(uint8_t hat);

#endif