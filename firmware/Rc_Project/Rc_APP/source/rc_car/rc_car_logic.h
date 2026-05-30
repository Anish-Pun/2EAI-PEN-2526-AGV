#ifndef RC_CAR_LOGIC_H
#define RC_CAR_LOGIC_H

#include <stdbool.h>

typedef enum
{
    RC_CAR_MODE_BLUETOOTH = 0,
    RC_CAR_MODE_SELF_DRIVING
} rc_car_mode_t;

void rc_car_logic_init(void);
void rc_car_logic_run(void);

void rc_car_logic_set_mode(rc_car_mode_t mode);
rc_car_mode_t rc_car_logic_get_mode(void);
bool rc_car_logic_is_self_driving_active(void);

#endif
