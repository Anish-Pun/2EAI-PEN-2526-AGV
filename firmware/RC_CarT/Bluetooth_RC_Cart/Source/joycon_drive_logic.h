#ifndef JOYCON_DRIVE_LOGIC_H
#define JOYCON_DRIVE_LOGIC_H

#include <stdint.h>
#include <stdbool.h>

void joycon_drive_logic_reset(void);
void joycon_drive_logic_handle_hat(uint8_t hat);
void joycon_drive_logic_set_boost(bool active); // Nieuwe functie voor de X-knop

#endif