#include "motor.h"
#include "pins.h"
#include "cy_retarget_io.h"

cyhal_pwm_t pwm_a, pwm_b, pwm_c, pwm_d;

void motor_init(void) {
    printf("Motors init - Forward/Stop ready\r\n");
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
    printf("Forward %d%%\r\n", speed);
}

void motor_stop(void) {
    cyhal_pwm_set_duty_cycle(&pwm_a, 0, 1000);
    cyhal_pwm_set_duty_cycle(&pwm_b, 0, 1000);
    cyhal_pwm_set_duty_cycle(&pwm_c, 0, 1000);
    cyhal_pwm_set_duty_cycle(&pwm_d, 0, 1000);
    printf("STOP - Obstacle detected\r\n");
}
