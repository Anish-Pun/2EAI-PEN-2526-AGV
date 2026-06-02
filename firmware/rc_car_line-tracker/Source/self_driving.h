#ifndef SELF_DRIVING_H
#define SELF_DRIVING_H

#include <stdbool.h>

void self_driving_enter(void);
void self_driving_exit(void);
void self_driving_step(void);

bool self_driving_is_halted(void);

#endif
