#include "self_driving.h"

#include "ultrasonic.h"
#include "motor.h"
#include "imu.h"
#include "speed.h"

#include "cyhal.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

static bool halted_until_restart = false;
static bool halt_message_printed = false;

/* IMU lift detection debounce */
static uint8_t lift_counter = 0;
static uint8_t imu_counter = 0;

/* IMU baseline captured when self-driving starts */
static int16_t base_ax = 0;
static int16_t base_ay = 0;
static int16_t base_az = 0;

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
    lift_counter = 0;
    imu_counter = 0;

    int16_t gx, gy, gz;
    imu_read_all(&base_ax, &base_ay, &base_az, &gx, &gy, &gz);

    snprintf(event_line, sizeof(event_line), "Event: SELF-DRIVING MODE entered");
    printf("\033[2J\033[H");   /* clear screen, cursor home */
    printf("[ Self-driving status ]\n\r");
    printf("%-160s\n\r", "Sensors: --");
    printf("%-160s\n\r", event_line);
    printf("%-160s\n\r", "IMU: --");
    fflush(stdout);
    motor_forward(65);
}

void self_driving_exit(void)
{
    motor_stop();
    halted_until_restart = false;
    halt_message_printed = false;
    lift_counter = 0;
    imu_counter = 0;
    printf("\n\rEXIT SELF-DRIVING\n\r");
    fflush(stdout);
}

void self_driving_step(void)
{
    int16_t ax, ay, az, gx, gy, gz;
    imu_read_all(&ax, &ay, &az, &gx, &gy, &gz);

    float right = get_distance_right_cm();
    float left  = get_distance_left_cm();

    float spd_r = get_speed_right_kmh();
    float spd_l = get_speed_left_kmh();

    char status_line[200];
    char imu_line[200];

    (void)snprintf(status_line, sizeof(status_line),
                   "R:%5.1f L:%5.1f SPD_R:%.1f SPD_L:%.1f CR:%lu CL:%lu AX:%6d AY:%6d AZ:%6d Mot:%d",
                   right, left, spd_r, spd_l,
                   (unsigned long)Count_EncoderRight, (unsigned long)Count_EncoderLeft,
                   ax, ay, az, (int)Motion_Detected);

    (void)snprintf(imu_line, sizeof(imu_line),
                   "IMU AX:%6d AY:%6d AZ:%6d GX:%6d GY:%6d GZ:%6d BAX:%6d BAY:%6d BAZ:%6d",
                   ax, ay, az, gx, gy, gz, base_ax, base_ay, base_az);

    printf("\033[2;1H\033[K%-160s", status_line);
    printf("\033[3;1H\033[K%-160s", event_line);
    printf("\033[4;1H\033[K%-160s", imu_line);
    fflush(stdout);

    if (halted_until_restart)
    {
        motor_stop();
        if (!halt_message_printed)
        {
            printf("\033[3;1H\033[K%-160s", event_line);
            fflush(stdout);
            halt_message_printed = true;
        }
        cyhal_system_delay_ms(50);
        return;
    }

    /* IMU interrupt stop disabled for now - still too noisy on this setup */
    Motion_Detected = false;

    /* Automatic lift/crash detection based on startup baseline */
    {
        int16_t dax = ax - base_ax;
        int16_t day = ay - base_ay;
        int16_t daz = az - base_az;

        if ((abs(dax) > 3000) || (abs(day) > 3000) || (abs(daz) > 2500))
        {
            imu_counter++;
            if (imu_counter >= 3)
            {
                motor_stop();
                halted_until_restart = true;
                halt_message_printed = false;
                snprintf(event_line, sizeof(event_line),
                         "Event: LIFT DETECTED dAX:%d dAY:%d dAZ:%d - STOPPED",
                         dax, day, daz);
                return;
            }
        }
        else
        {
            imu_counter = 0;
        }
    }

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
        motor_turn_right(70);

    }
    else if (left < 25.0f)
    {
        snprintf(event_line, sizeof(event_line), "Event: Obstacle LEFT (%.1f cm) -> turning right", left);
        motor_turn_left(70);
    }
    else
    {
        snprintf(event_line, sizeof(event_line), "Event: Clear path -> forward");
        motor_forward(80);
    }

    cyhal_system_delay_ms(50);
}
