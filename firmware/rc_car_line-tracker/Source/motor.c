#include "motor.h"
#include "pins.h"
#include "cy_retarget_io.h"

cyhal_pwm_t pwm_a, pwm_b, pwm_c, pwm_d;

void motor_init(void)
{
    cy_rslt_t result;

    result = cyhal_pwm_init(&pwm_a, MOTOR_A_PIN, NULL);
    printf("PWM A init = 0x%08lX\r\n", (unsigned long)result);
    if(result == CY_RSLT_SUCCESS) cyhal_pwm_start(&pwm_a);

    result = cyhal_pwm_init(&pwm_b, MOTOR_B_PIN, NULL);
    printf("PWM B init = 0x%08lX\r\n", (unsigned long)result);
    if(result == CY_RSLT_SUCCESS) cyhal_pwm_start(&pwm_b);

    result = cyhal_pwm_init(&pwm_c, MOTOR_C_PIN, NULL);
    printf("PWM C init = 0x%08lX\r\n", (unsigned long)result);
    if(result == CY_RSLT_SUCCESS) cyhal_pwm_start(&pwm_c);

    result = cyhal_pwm_init(&pwm_d, MOTOR_D_PIN, NULL);
    printf("PWM D init = 0x%08lX\r\n", (unsigned long)result);
    if(result == CY_RSLT_SUCCESS) cyhal_pwm_start(&pwm_d);

    motor_stop();
}
void motor_forward(uint8_t speed) {
    if (speed > 100) speed = 100;
    cyhal_pwm_set_duty_cycle(&pwm_a, speed, 1000);
    cyhal_pwm_set_duty_cycle(&pwm_b, speed, 1000);
    cyhal_pwm_set_duty_cycle(&pwm_c, speed, 1000);
    cyhal_pwm_set_duty_cycle(&pwm_d, speed, 1000);
}

void motor_stop(void) {
    cyhal_pwm_set_duty_cycle(&pwm_a, 0, 1000);
    cyhal_pwm_set_duty_cycle(&pwm_b, 0, 1000);
    cyhal_pwm_set_duty_cycle(&pwm_c, 0, 1000);
    cyhal_pwm_set_duty_cycle(&pwm_d, 0, 1000);
}

// LEFT TURN
void motor_turn_left(uint8_t speed) {
    if (speed > 100) speed = 100;
    cyhal_pwm_set_duty_cycle(&pwm_a, 0, 1000);  // Left side ON
    cyhal_pwm_set_duty_cycle(&pwm_b, 0, 1000);      // Right side OFF
    cyhal_pwm_set_duty_cycle(&pwm_c, speed, 1000);  // Left side ON
    cyhal_pwm_set_duty_cycle(&pwm_d, 0, 1000);      // Right side OFF
}

// RIGHT TURN
void motor_turn_right(uint8_t speed) {
    if (speed > 100) speed = 100;
    cyhal_pwm_set_duty_cycle(&pwm_a, 0, 1000);      // Left side OFF
    cyhal_pwm_set_duty_cycle(&pwm_b, speed, 1000);  // Right side ON
    cyhal_pwm_set_duty_cycle(&pwm_c, 0, 1000);      // Left side OFF
    cyhal_pwm_set_duty_cycle(&pwm_d, speed, 1000);  // Right side ON
}
