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

#define PWM_FREQ 1000 // Verlaagd naar 1kHz, stabieler voor L298N

// Bij de L298N is 'stop' altijd 0.0f duty cycle
#define STOP_LINKS  0.0f
#define STOP_RECHTS 0.0f

#define DEADZONE 3.0f

cyhal_pwm_t pwm_a;
cyhal_pwm_t pwm_b;

/* =========================
   STATE & INTERNAL
========================= */

typedef enum {
    MOTOR_STOP = 0,
    MOTOR_FORWARD,
    MOTOR_BACKWARD
} motor_state_t;

// We houden de 'last' waarden bij voor de smoothing (0.0 tot 100.0)
static float last_a = 0.0f;
static float last_b = 0.0f;

/* =========================
   UTIL
========================= */

static float clamp(float v, float min, float max)
{
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

static float apply_deadzone(float speed, float stop)
{
    if (fabs(speed) < DEADZONE)
        return 0.0f;

    return speed;
}

static float smooth(float prev, float input)
{
    // Behoudt de 80/20 smoothing ratio uit je originele code
    return (prev * 0.8f) + (input * 0.2f);
}

/* =========================
   L298N DIRECTION LOGIC
========================= */

static void set_hardware_direction(float speed_a, float speed_b)
{
    // Motor A Richting (Links)
    if (speed_a > 0.1f) {
        cyhal_gpio_write(MOTOR_A_IN1, true);
        cyhal_gpio_write(MOTOR_A_IN2, false);
    } else if (speed_a < -0.1f) {
        cyhal_gpio_write(MOTOR_A_IN1, false);
        cyhal_gpio_write(MOTOR_A_IN2, true);
    } else {
        cyhal_gpio_write(MOTOR_A_IN1, false);
        cyhal_gpio_write(MOTOR_A_IN2, false);
    }

    // Motor B Richting (Rechts)
    if (speed_b > 0.1f) {
        cyhal_gpio_write(MOTOR_B_IN3, true);
        cyhal_gpio_write(MOTOR_B_IN4, false);
    } else if (speed_b < -0.1f) {
        cyhal_gpio_write(MOTOR_B_IN3, false);
        cyhal_gpio_write(MOTOR_B_IN4, true);
    } else {
        cyhal_gpio_write(MOTOR_B_IN3, false);
        cyhal_gpio_write(MOTOR_B_IN4, false);
    }
}

/* =========================
   MAPPING
========================= */

static float map_motor(motor_state_t state, uint8_t speed)
{
    if (speed > 100) speed = 100;

    float output_speed = 0.0f;

    switch (state)
    {
        case MOTOR_FORWARD:
            output_speed = (float)speed;
            break;

        case MOTOR_BACKWARD:
            output_speed = -(float)speed;
            break;

        case MOTOR_STOP:
        default:
            output_speed = 0.0f;
            break;
    }

    return apply_deadzone(output_speed, 0.0f);
}

/* =========================
   INIT
========================= */

void motor_init(void)
{
    printf("L298N PSoC6 motor init met smoothing\r\n");

    // Init PWM (Snelheid)
    cyhal_pwm_init(&pwm_a, MOTOR_A_PWM, NULL);
    cyhal_pwm_init(&pwm_b, MOTOR_B_PWM, NULL);

    // Init GPIO (Richting)
    cyhal_gpio_init(MOTOR_A_IN1, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    cyhal_gpio_init(MOTOR_A_IN2, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    cyhal_gpio_init(MOTOR_B_IN3, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    cyhal_gpio_init(MOTOR_B_IN4, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);

    cyhal_pwm_set_duty_cycle(&pwm_a, 0.0f, PWM_FREQ);
    cyhal_pwm_set_duty_cycle(&pwm_b, 0.0f, PWM_FREQ);

    cyhal_pwm_start(&pwm_a);
    cyhal_pwm_start(&pwm_b);
}

/* =========================
   APPLY
========================= */

static void motor_apply(float target_a, float target_b)
{
    // Pas smoothing toe op de snelheidswaarden (-100 tot 100)
    last_a = smooth(last_a, target_a);
    last_b = smooth(last_b, target_b);

    // Bepaal de hardware pinnen op basis van de gesmoothe waarde
    set_hardware_direction(last_a, last_b);

    // Stuur de absolute waarde naar de PWM (0.0 tot 100.0)
    cyhal_pwm_set_duty_cycle(&pwm_a, clamp(fabs(last_a), 0.0f, 100.0f), PWM_FREQ);
    cyhal_pwm_set_duty_cycle(&pwm_b, clamp(fabs(last_b), 0.0f, 100.0f), PWM_FREQ);
}

/* =========================
   PUBLIC API
========================= */

void motor_stop(void)
{
    motor_apply(0.0f, 0.0f);
}

void motor_forward(uint8_t speed)
{
    float a = map_motor(MOTOR_FORWARD, speed);
    float b = map_motor(MOTOR_FORWARD, speed);
    motor_apply(a, b);
}

void motor_backward(uint8_t speed)
{
    float a = map_motor(MOTOR_BACKWARD, speed);
    float b = map_motor(MOTOR_BACKWARD, speed);
    motor_apply(a, b);
}

void motor_turn_right(uint8_t speed)
{
    float a = map_motor(MOTOR_BACKWARD, speed); // Links achteruit
    float b = map_motor(MOTOR_FORWARD, speed);  // Rechts vooruit
    motor_apply(a, b);
}

void motor_turn_left(uint8_t speed)
{
    float a = map_motor(MOTOR_FORWARD, speed);  // Links vooruit
    float b = map_motor(MOTOR_BACKWARD, speed); // Rechts achteruit
    motor_apply(a, b);
}