#ifndef MOTOR_H
#define MOTOR_H

#include "cyhal.h"

void motor_init(void);
void motor_forward(uint8_t speed);
void motor_stop(void);
void motor_turn_left(uint8_t speed);
void motor_turn_right(uint8_t speed);

extern cyhal_pwm_t pwm_a;
extern cyhal_pwm_t pwm_b;

#endif
