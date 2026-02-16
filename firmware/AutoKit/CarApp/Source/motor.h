#ifndef MOTOR_H
#define MOTOR_H
#include "cyhal.h"

void motor_init(void);
void motor_forward(uint8_t speed);
void motor_stop(void);

extern cyhal_pwm_t pwm_a, pwm_b, pwm_c, pwm_d;

#endif
