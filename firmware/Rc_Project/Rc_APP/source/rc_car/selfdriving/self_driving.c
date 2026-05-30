#include "self_driving.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ultrasonic.h"
#include "motor.h"
#include "imu.h"
#include "speed.h"

#include "cyhal.h"

static bool halted_until_restart = false;
static bool halt_message_printed = false;
static bool imu_ready = false;
static uint32_t startup_delay_cycles = 0;

/* IMU lift detection debounce - requires sustained anomaly to avoid false triggers from bumps */
static uint8_t imu_counter = 0;
/* IMU baseline captured when self-driving starts */
static int16_t base_ax = 0;
static int16_t base_ay = 0;
static int16_t base_az = 0;

/* Last event message shown on line 2 of the terminal output */
static float filt_ax = 0.0f;
static float filt_ay = 0.0f;
static float filt_az = 0.0f;

static char event_line[160] = "Event: --";

#define BASELINE_SAMPLES 16
#define BASELINE_DELAY_MS 20
#define FILTER_ALPHA 0.20f
#define LIFT_THRESHOLD_AX_AY 2500
#define LIFT_THRESHOLD_AZ 2000
#define CRASH_THRESHOLD_SUM 4500
#define IMU_DEBOUNCE_COUNT 6
#define STARTUP_IGNORE_CYCLES 20

static void imu_capture_baseline(void)
{
    int32_t sum_ax = 0, sum_ay = 0, sum_az = 0;
    int16_t ax, ay, az, gx, gy, gz;

    for (int i = 0; i < BASELINE_SAMPLES; i++)
    {
        imu_read_all(&ax, &ay, &az, &gx, &gy, &gz);
        sum_ax += ax;
        sum_ay += ay;
        sum_az += az;
        cyhal_system_delay_ms(BASELINE_DELAY_MS);
    }

    base_ax = (int16_t)(sum_ax / BASELINE_SAMPLES);
    base_ay = (int16_t)(sum_ay / BASELINE_SAMPLES);
    base_az = (int16_t)(sum_az / BASELINE_SAMPLES);

    filt_ax = (float)base_ax;
    filt_ay = (float)base_ay;
    filt_az = (float)base_az;
    imu_ready = true;
}

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
    imu_counter = 0;
    startup_delay_cycles = STARTUP_IGNORE_CYCLES;

    imu_capture_baseline();

    snprintf(event_line, sizeof(event_line), "Event: SELF-DRIVING MODE entered");
    printf("\033[2J\033[H");
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
    imu_counter = 0;
    imu_ready = false;
    printf("\n\rEXIT SELF-DRIVING\n\r");
    fflush(stdout);
}

void self_driving_step(void)
{
    int16_t ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0;
    imu_read_all(&ax, &ay, &az, &gx, &gy, &gz);

    float right = get_distance_right_cm();
    float left = get_distance_left_cm();
    float spd_r = get_speed_right_kmh();
    float spd_l = get_speed_left_kmh();

    if (imu_ready)
    {
        filt_ax = (1.0f - FILTER_ALPHA) * filt_ax + FILTER_ALPHA * (float)ax;
        filt_ay = (1.0f - FILTER_ALPHA) * filt_ay + FILTER_ALPHA * (float)ay;
        filt_az = (1.0f - FILTER_ALPHA) * filt_az + FILTER_ALPHA * (float)az;
    }

    int16_t dax = (int16_t)(filt_ax - (float)base_ax);
    int16_t day = (int16_t)(filt_ay - (float)base_ay);
    int16_t daz = (int16_t)(filt_az - (float)base_az);
    int16_t acc_sum = (int16_t)(abs(dax) + abs(day) + abs(daz));

    char status_line[200];
    char imu_line[220];

    snprintf(status_line, sizeof(status_line),
             "R:%5.1f L:%5.1f SPD_R:%.1f SPD_L:%.1f CR:%lu CL:%lu AX:%6d AY:%6d AZ:%6d Mot:%d",
             right, left, spd_r, spd_l,
             (unsigned long)Count_EncoderRight, (unsigned long)Count_EncoderLeft,
             ax, ay, az, (int)Motion_Detected);

    snprintf(imu_line, sizeof(imu_line),
             "F_AX:%6d F_AY:%6d F_AZ:%6d DAX:%6d DAY:%6d DAZ:%6d CNT:%u",
             (int16_t)filt_ax, (int16_t)filt_ay, (int16_t)filt_az, dax, day, daz, (unsigned)imu_counter);

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
    /* Automatic lift/crash detection based on startup baseline */
    if (startup_delay_cycles > 0)
    {
        startup_delay_cycles--;
        Motion_Detected = false;
    }
    else
    {
        if (Motion_Detected ||
            (abs(dax) > LIFT_THRESHOLD_AX_AY) ||
            (abs(day) > LIFT_THRESHOLD_AX_AY) ||
            (abs(daz) > LIFT_THRESHOLD_AZ) ||
            (acc_sum > CRASH_THRESHOLD_SUM))
        {
            imu_counter++;
            if (imu_counter >= IMU_DEBOUNCE_COUNT)
            {
                motor_stop();
                halted_until_restart = true;
                halt_message_printed = false;
                snprintf(event_line, sizeof(event_line),
                         "Event: IMU anomaly dAX:%d dAY:%d dAZ:%d - STOPPED",
                         dax, day, daz);
                Motion_Detected = false;
                return;
            }
        }
        else
        {
            imu_counter = 0;
        }
        Motion_Detected = false;
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
