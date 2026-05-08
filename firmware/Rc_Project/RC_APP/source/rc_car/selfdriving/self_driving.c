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

/* Last event message shown on line 2 of the terminal output */
static char event_line[160] = "Event: --";

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

    snprintf(event_line, sizeof(event_line), "Event: SELF-DRIVING MODE entered");
    printf("\033[2J\033[H");   /* clear screen, cursor home */
    printf("[ Self-driving status ]\n\r");
    printf("%-160s\n\r", "Sensors: --");
    printf("%-160s\n\r", event_line);
    fflush(stdout);
    motor_forward(65);
}

void self_driving_exit(void)
{
    motor_stop();
    halted_until_restart = false;
    halt_message_printed = false;
    printf("\n\rEXIT SELF-DRIVING\n\r");
    fflush(stdout);
}

void self_driving_step(void)
{
    if (halted_until_restart)
    {
        motor_stop();
        if (!halt_message_printed)
        {
            snprintf(event_line, sizeof(event_line), "Event: BOTH OBSTACLES - STOPPED. Switch to BT mode to restart.");
            printf("\033[3;1H\033[K%-160s", event_line);  /* line 3, overwrite */
            fflush(stdout);
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

    // sensor values
    printf("\033[2;1H\033[K%-160s", status_line);
    // last event (overwrite in place)
    printf("\033[3;1H\033[K%-160s", event_line);
    fflush(stdout);

    /*  IMU lift / motion checks disabled (triggering false positives)
    if (Motion_Detected)
    {
        motor_stop();
        snprintf(event_line, sizeof(event_line), "Event: EMERGENCY LIFT STOP - switch to BT mode to reset");
        Motion_Detected = false;
        return;
    }

    if (az < 8000)
    {
        motor_stop();
        snprintf(event_line, sizeof(event_line), "Event: LIFT DETECTED (az=%d) - STOP", az);
        return;
    }
     end disabled block */

    if ((right < 18.0f) && (left < 18.0f))
    {
        motor_stop();
        halted_until_restart = true;
        halt_message_printed = false;
        snprintf(event_line, sizeof(event_line), "Event: BOTH OBSTACLES DETECTED - halted");
        return;
    }
    else if (right < 25.0f)
    {
        snprintf(event_line, sizeof(event_line), "Event: Obstacle RIGHT (%.1f cm) -> turning left", right);
        motor_turn_left(70);
    }
    else if (left < 25.0f)
    {
        snprintf(event_line, sizeof(event_line), "Event: Obstacle LEFT (%.1f cm) -> turning right", left);
        motor_turn_right(70);
    }
    else
    {
        snprintf(event_line, sizeof(event_line), "Event: Clear path -> forward");
        motor_forward(80);
    }

    cyhal_system_delay_ms(50);
}
