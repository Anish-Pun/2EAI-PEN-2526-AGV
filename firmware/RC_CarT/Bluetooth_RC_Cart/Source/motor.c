#include "motor.h"
#include "cybsp.h"
#include "pins.h"
#include "cy_retarget_io.h"
#include "cyhal.h"

#include <stdio.h>
#include <stdint.h>

cyhal_pwm_t pwm_a;
cyhal_pwm_t pwm_b;

#define PWM_FREQ 20000   // 20kHz beter dan 1kHz

/*
MD22 Analog mode:
0%   = full reverse
50%  = stop
100% = full forward
*/

static float speed_to_duty_forward(uint8_t speed)
{
    if (speed > 100) speed = 100;

    // 50% -> 100%
    return 50.0f + ((float)speed * 0.5f);
}

static float speed_to_duty_backward(uint8_t speed)
{
    if (speed > 100) speed = 100;

    // 50% -> 0%
    return 50.0f - ((float)speed * 0.5f);
}

void motor_init(void)
{
    printf("Motor init (MD22 Analog Mode)\r\n");

    cyhal_pwm_init(&pwm_a, MOTOR_A_PIN, NULL);
    cyhal_pwm_set_duty_cycle(&pwm_a, 50.0f, PWM_FREQ); // STOP
    cyhal_pwm_start(&pwm_a);

    cyhal_pwm_init(&pwm_b, MOTOR_B_PIN, NULL);
    cyhal_pwm_set_duty_cycle(&pwm_b, 50.0f, PWM_FREQ); // STOP
    cyhal_pwm_start(&pwm_b);

    motor_stop();
}

void motor_stop(void)
{
    // 2.5V center stop
    cyhal_pwm_set_duty_cycle(&pwm_a, 40.0f, PWM_FREQ);
    cyhal_pwm_set_duty_cycle(&pwm_b, 60.0f, PWM_FREQ);
}

void motor_forward(uint8_t speed)
{
    float duty = speed_to_duty_forward(speed);

    cyhal_pwm_set_duty_cycle(&pwm_a, duty, PWM_FREQ);
    cyhal_pwm_set_duty_cycle(&pwm_b, duty, PWM_FREQ);
}

void motor_backward(uint8_t speed)
{
    float duty = speed_to_duty_backward(speed);

    cyhal_pwm_set_duty_cycle(&pwm_a, duty, PWM_FREQ);
    cyhal_pwm_set_duty_cycle(&pwm_b, duty, PWM_FREQ);
}

void motor_turn_left(uint8_t speed)
{
    float fast = speed_to_duty_forward(speed);

    // linker motor stop, rechter motor vooruit
    cyhal_pwm_set_duty_cycle(&pwm_a, 50.0f, PWM_FREQ);
    cyhal_pwm_set_duty_cycle(&pwm_b, fast, PWM_FREQ);
}

void motor_turn_right(uint8_t speed)
{
    float fast = speed_to_duty_forward(speed);

    // rechter motor stop, linker motor vooruit
    cyhal_pwm_set_duty_cycle(&pwm_a, fast, PWM_FREQ);
    cyhal_pwm_set_duty_cycle(&pwm_b, 50.0f, PWM_FREQ);
}