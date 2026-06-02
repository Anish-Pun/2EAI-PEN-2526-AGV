#include "self_driving.h"
#include "ultrasonic.h"
#include "motor.h"
#include "line_tracker.h"
#include "button.h"
#include "speed.h"
#include "cyhal.h"
#include <stdio.h>

static bool halted_until_restart = false;
static bool halt_message_printed = false;

bool self_driving_is_halted(void)
{
    return halted_until_restart;
}

void self_driving_enter(void)
{
    reset_encoders();

    halted_until_restart = false;
    halt_message_printed = false;

    printf("\n\rSELF-DRIVING MODE\n\r");
    cyhal_pwm_set_duty_cycle(&pwm_a, 100, 1000);
	cyhal_pwm_set_duty_cycle(&pwm_b, 100, 1000);
	cyhal_pwm_set_duty_cycle(&pwm_c, 100, 1000);
	cyhal_pwm_set_duty_cycle(&pwm_d, 100, 1000);
}

void self_driving_exit(void)
{
    motor_stop();
    halted_until_restart = false;
    halt_message_printed = false;
    printf("\n\rEXIT SELF-DRIVING\n\r");
}

void self_driving_step(void)
{
    float right = get_distance_right_cm();
    float left  = get_distance_left_cm();

    float spd_r = get_speed_right_kmh();
    float spd_l = get_speed_left_kmh();

	bool l = line_left();
	bool r = line_right();
	
	printf("\rUS_L:%5.1f US_R:%5.1f SPD_L:%4.1f SPD_R:%4.1f LINE:%d%d",
	       left,
	       right,
	       spd_l,
	       spd_r,
	       l,
	       r);
	
	if(!l && !r)
	{
	    motor_forward(100);
	}
	else if(l)
	{
	    motor_turn_left(100);
	}
	else if(r)
	{
	    motor_turn_right(100);
	}
}
//    if (az < 8000)
//    {
//        motor_stop();
//        printf("\n\rLIFT DETECTED - STOP\n\r");
//        return;
//    }


