/*
 * pins.h - Pins Input and Outputs
 */

#ifndef PINS_H
#define PINS_H

// Ultrasonic
#define ULTRASONIC_TRIG_RIGHT  P9_0
#define ULTRASONIC_ECHO_RIGHT  P9_1
#define ULTRASONIC_TRIG_LEFT   P9_4
#define ULTRASONIC_ECHO_LEFT   P9_5

// Motor
#define MOTOR_A_PIN  P10_0  // Wheel A
#define MOTOR_B_PIN  P10_1  // Wheel B
#define MOTOR_C_PIN  P10_2  // Wheel C
#define MOTOR_D_PIN  P10_3  // Wheel D

// SW2 Start and Stop
#define CAR_START_STOP_PIN  P0_4

// IMU
#define IMU_SDA P6_1
#define IMU_SCL P6_0
#define MPU_INTERRUPT P10_5

// SPEED ENCODERS
#define ENCODER_RIGHT P9_6
#define ENCODER_LEFT  P9_7

#endif
