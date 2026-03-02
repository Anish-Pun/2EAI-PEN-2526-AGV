#ifndef IMU_H
#define IMU_H

#include "cyhal.h"
#include "pins.h"
#include "motor.h"
#include "cy_retarget_io.h"
#include <stdint.h>
#include <stdbool.h>

#define MPU6050_ADDR              0x68
#define PWR_MGMT_1                0x6B
#define WHO_AM_I_REGISTER         0x75
#define EXPECTED_WHO_AM_I         0x68

#define INT_PIN_CFG               0x37
#define MOT_THRESHOLD             0x1F
#define MREG_INT_ENABLE           0x38
#define MPU6050_MOTION_DETECT_EN  0x40

#define ACCEL_XOUT_H              0x3B  /
#define GYRO_XOUT_H               0x43

extern cyhal_i2c_t i2c;
extern cyhal_gpio_t MPU_interrupt;
extern bool Motion_Detected;

// Calibration offsets
extern int16_t calib_ax, calib_ay, calib_az;
extern int16_t calib_gx, calib_gy, calib_gz;

void InitI2C(void);
void mpu6050_enable_interrupts(void);
void imu_init(void);

void imu_read_all(int16_t *ax, int16_t *ay, int16_t *az,
                  int16_t *gx, int16_t *gy, int16_t *gz);

#endif
