#include "self_driving.h"

#include "ultrasonic.h"
#include "motor.h"
#include "imu.h"
#include "speed.h"

#include "cyhal.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

static bool halted_until_restart = false;
static bool halt_message_printed = false;

bool self_driving_is_halted(void)
{
    return halted_until_restart;
}

void self_driving_enter(void)
{
    Motion_Detected = false;
    reset_encoders();

    halted_until_restart = false;
    halt_message_printed = false;

    printf("\n\rSELF-DRIVING MODE\n\r");
    motor_forward(65);
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
    if (halted_until_restart)
    {
        motor_stop();
        if (!halt_message_printed)
        {
            printf("\n\rBOTH OBSTACLES - STOPPED. Press SW2 to restart.\n\r");
            halt_message_printed = true;
        }
        cyhal_system_delay_ms(50);
        return;
    }

    int16_t ax, ay, az, gx, gy, gz;
    imu_read_all(&ax, &ay, &az, &gx, &gy, &gz);

    float right = get_distance_right_cm();
    float left  = get_distance_left_cm();

    float spd_r = get_speed_right_kmh();
    float spd_l = get_speed_left_kmh();

    char status_line[200];
    (void)snprintf(status_line, sizeof(status_line),
                   "R:%5.1f L:%5.1f SPD_R:%.1f SPD_L:%.1f CR:%lu CL:%lu AX:%6d AY:%6d AZ:%6d Mot:%d",
                   right, left, spd_r, spd_l,
                   (unsigned long)Count_EncoderRight, (unsigned long)Count_EncoderLeft,
                   ax, ay, az, (int)Motion_Detected);

    printf("\r%-160s", status_line);
    fflush(stdout);

    if (Motion_Detected)
    {
        motor_stop();
        printf("\n\rEMERGENCY LIFT STOP - Press SW2!\n\r");
        Motion_Detected = false;
        return;
    }

    if (az < 8000)
    {
        motor_stop();
        printf("\n\rLIFT DETECTED - STOP\n\r");
        return;
    }

    if ((right < 20.0f) && (left < 20.0f))
    {
        motor_stop();
        halted_until_restart = true;
        halt_message_printed = false;
        return;
    }
    else if (right < 20.0f)
    {
        motor_turn_left(80);
    }
    else if (left < 20.0f)
    {
        motor_turn_right(80);
    }
    else
    {
        motor_forward(65);
    }

    cyhal_system_delay_ms(50);
}
