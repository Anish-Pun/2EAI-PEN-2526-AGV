#include "motor.h"
#include "pins.h"
#include "cy_retarget_io.h"

cyhal_pwm_t pwm_a, pwm_b, pwm_c, pwm_d;

void motor_init(void) {
    printf("Motors init\r\n");
    cyhal_pwm_init(&pwm_a, MOTOR_A_PIN, NULL); cyhal_pwm_start(&pwm_a);
    cyhal_pwm_init(&pwm_b, MOTOR_B_PIN, NULL); cyhal_pwm_start(&pwm_b);
    cyhal_pwm_init(&pwm_c, MOTOR_C_PIN, NULL); cyhal_pwm_start(&pwm_c);
    cyhal_pwm_init(&pwm_d, MOTOR_D_PIN, NULL); cyhal_pwm_start(&pwm_d);
}

void motor_forward(uint8_t speed) {
    if (speed > 100) speed = 100;
    cyhal_pwm_set_duty_cycle(&pwm_a, speed, 1000);
    cyhal_pwm_set_duty_cycle(&pwm_b, speed, 1000);
    cyhal_pwm_set_duty_cycle(&pwm_c, speed, 1000);
    cyhal_pwm_set_duty_cycle(&pwm_d, speed, 1000);
    //printf("Forward %d%%\r\n", speed);
}

void motor_stop(void) {
    cyhal_pwm_set_duty_cycle(&pwm_a, 0, 1000);
    cyhal_pwm_set_duty_cycle(&pwm_b, 0, 1000);
    cyhal_pwm_set_duty_cycle(&pwm_c, 0, 1000);
    cyhal_pwm_set_duty_cycle(&pwm_d, 0, 1000);
    printf("STOPPED\r\n");
}

// LEFT TURN
void motor_turn_left(uint8_t speed) {
    if (speed > 100) speed = 100;
    cyhal_pwm_set_duty_cycle(&pwm_a, speed, 1000);  // Left side ON
    cyhal_pwm_set_duty_cycle(&pwm_b, 0, 1000);      // Right side OFF
    cyhal_pwm_set_duty_cycle(&pwm_c, speed, 1000);  // Left side ON
    cyhal_pwm_set_duty_cycle(&pwm_d, 0, 1000);      // Right side OFF
    printf("Turn LEFT %d%% \r\n", speed);
}

// RIGHT TURN
void motor_turn_right(uint8_t speed) {
    if (speed > 100) speed = 100;
    cyhal_pwm_set_duty_cycle(&pwm_a, 0, 1000);      // Left side OFF
    cyhal_pwm_set_duty_cycle(&pwm_b, speed, 1000);  // Right side ON
    cyhal_pwm_set_duty_cycle(&pwm_c, 0, 1000);      // Left side OFF
    cyhal_pwm_set_duty_cycle(&pwm_d, speed, 1000);  // Right side ON
    printf("Turn RIGHT %d%% \r\n", speed);
}
