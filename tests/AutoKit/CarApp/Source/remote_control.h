#ifndef REMOTE_CONTROL_H
#define REMOTE_CONTROL_H

void remote_control_init_ble(void);
void remote_control_enter(void);
void remote_control_exit(void);
void remote_control_step(void);
void remote_control_set_enabled(int enabled);

#endif
