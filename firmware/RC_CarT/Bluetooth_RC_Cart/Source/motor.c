#include "motor.h"
#include "cybsp.h"
#include "cyhal.h"
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include "pins.h"
/* =========================
   CONFIG
========================= */

#define PWM_FREQ 20000

// 🔧 CALIBRATE THESE (MUST)
#define STOP_LINKS 60.0f
#define STOP_RECHTS 65.0f

#define DEADZONE 3.0f

cyhal_pwm_t pwm_a;
cyhal_pwm_t pwm_b;

/* =========================
   UTIL
========================= */

static float clamp(float v, float min, float max)
{
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

static float apply_deadzone(float duty, float stop)
{
    if (fabs(duty - stop) < DEADZONE)
        return stop;

    return duty;
}

static float smooth(float prev, float input)
{
    return (prev * 0.8f) + (input * 0.2f);
}

/* =========================
   STATE
========================= */

typedef enum {
    MOTOR_STOP = 0,
    MOTOR_FORWARD,
    MOTOR_BACKWARD
} motor_state_t;

/* =========================
   INTERNAL STATE
========================= */

static float last_a = STOP_LINKS;
static float last_b = STOP_RECHTS;

/* =========================
   MAPPING
========================= */

static float map_motor(float stop, motor_state_t state, uint8_t speed)
{
    if (speed > 100) speed = 100;

    float duty = stop;

    switch (state)
    {
        case MOTOR_FORWARD:
            duty = stop + (speed * 0.45f);
            break;

        case MOTOR_BACKWARD:
            duty = stop - (speed * 0.45f);
            break;

        case MOTOR_STOP:
        default:
            duty = stop;
            break;
    }

    duty = apply_deadzone(duty, stop);
    return clamp(duty, 0.0f, 100.0f);
}

/* =========================
   INIT
========================= */

void motor_init(void)
{
    printf("MD22 analog motor init\r\n");

    cyhal_pwm_init(&pwm_a, MOTOR_A_PIN, NULL);
    cyhal_pwm_init(&pwm_b, MOTOR_B_PIN, NULL);

    cyhal_pwm_set_duty_cycle(&pwm_a, STOP_LINKS, PWM_FREQ);
    cyhal_pwm_set_duty_cycle(&pwm_b, STOP_RECHTS, PWM_FREQ);

    cyhal_pwm_start(&pwm_a);
    cyhal_pwm_start(&pwm_b);
}

/* =========================
   APPLY
========================= */

static void motor_apply(float a, float b)
{
    last_a = smooth(last_a, a);
    last_b = smooth(last_b, b);

    cyhal_pwm_set_duty_cycle(&pwm_a, last_a, PWM_FREQ);
    cyhal_pwm_set_duty_cycle(&pwm_b, last_b, PWM_FREQ);
}

/* =========================
   PUBLIC API
========================= */

void motor_stop(void)
{
    motor_apply(STOP_LINKS, STOP_RECHTS);
}

void motor_forward(uint8_t speed)
{
    float a = map_motor(STOP_LINKS, MOTOR_FORWARD, speed);
    float b = map_motor(STOP_RECHTS, MOTOR_FORWARD, speed);

    motor_apply(a, b);
}

void motor_backward(uint8_t speed)
{
    float a = map_motor(STOP_LINKS, MOTOR_BACKWARD, speed);
    float b = map_motor(STOP_RECHTS, MOTOR_BACKWARD, speed);

    motor_apply(a, b);
}

void motor_turn_left(uint8_t speed)
{
    float a = STOP_LINKS;
    float b = map_motor(STOP_RECHTS, MOTOR_FORWARD, speed);

    motor_apply(a, b);
}

void motor_turn_right(uint8_t speed)
{
    float a = map_motor(STOP_LINKS, MOTOR_FORWARD, speed);
    float b = STOP_RECHTS;

    motor_apply(a, b);
}