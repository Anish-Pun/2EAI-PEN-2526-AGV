#ifndef IMU_H
#define IMU_H

#include "cyhal.h"
#include "pins.h"
#include "motor.h"
#include "cy_retarget_io.h"

#define MPU6050_ADDR 0x68
#define PWR_MGMT_1 0x6B
#define WHO_AM_I_REGISTER 0x75
#define EXPECTED_WHO_AM_I 0x68
#define INT_PIN_CFG 0x37
#define MOT_THRESHOLD 0x1F
#define MREG_INT_ENABLE 0x38
#define MPU6050_MOTION_DETECT_EN 0x40
#define ACCEL_XOUT_H 0x3B
#define ACCEL_YOUT_H 0x3D
#define ACCEL_ZOUT_H 0x3F

extern cyhal_i2c_t i2c;
extern cyhal_gpio_t MPU_interrupt;
extern bool Motion_Detected;

void InitI2C(void);
void mpu6050_enable_interrupts(void);
void imu_init(void);

#endif
