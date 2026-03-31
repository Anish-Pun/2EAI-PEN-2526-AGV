#include "motor.h"
#include "cybsp.h"
#include "pins.h"
#include "cy_retarget_io.h"

#include <stdio.h>

cyhal_pwm_t pwm_a, pwm_b;

#define MOTOR_PWM_PERIOD_HZ      (1000u)
#define MOTOR_PWM_ACTIVE_LOW     (1u)

static void motor_set_pwm(cyhal_pwm_t *pwm, uint8_t logical_speed)
{
    uint8_t clamped_speed = logical_speed;
    uint8_t applied_speed;

    if (clamped_speed > 100u)
    {
        clamped_speed = 100u;
    }

    if (MOTOR_PWM_ACTIVE_LOW != 0u)
    {
        applied_speed = (uint8_t)(100u - clamped_speed);
    }
    else
    {
        applied_speed = clamped_speed;
    }

    cyhal_pwm_set_duty_cycle(pwm, applied_speed, MOTOR_PWM_PERIOD_HZ);
}

void motor_init(void)
{
    printf("Motors init\r\n");
    cyhal_pwm_init(&pwm_a, MOTOR_A_PIN, NULL);
    cyhal_pwm_start(&pwm_a);
    cyhal_pwm_init(&pwm_b, MOTOR_B_PIN, NULL);
    cyhal_pwm_start(&pwm_b);
    motor_stop();
}

void motor_forward(uint8_t speed)
{
    motor_set_pwm(&pwm_a, speed);
    motor_set_pwm(&pwm_b, speed);
}

void motor_stop(void)
{
    motor_set_pwm(&pwm_a, 0u);
    motor_set_pwm(&pwm_b, 0u);
}

void motor_turn_left(uint8_t speed)
{
    motor_set_pwm(&pwm_a, speed);
    motor_set_pwm(&pwm_b, 0u);
}

void motor_turn_right(uint8_t speed)
{
    motor_set_pwm(&pwm_a, 0u);
    motor_set_pwm(&pwm_b, speed);
}
