#include "imu.h"

cyhal_i2c_t i2c;
cyhal_gpio_t MPU_interrupt;
bool Motion_Detected = false;
static bool imu_i2c_initialized = false;

// Calibration offsets (tune these for zero when flat)
int16_t calib_ax = -950, calib_ay = -300, calib_az = 0;
int16_t calib_gx = 480,  calib_gy = 170,  calib_gz = 210;

// ISR function (called on motion interrupt)
void ISR_mpu6050(void *callback_arg, cyhal_gpio_event_t event)
{
    // motor_stop();       /* disabled: triggers falsely on startup */
    Motion_Detected = true;
}

static cyhal_gpio_callback_data_t MPU6050_cb_data =
{
    .callback = ISR_mpu6050,
    .callback_arg = NULL
};

cy_rslt_t InitI2C(void)
{
    printf("InitI2C...\n\r");

    cy_rslt_t result;

    imu_i2c_initialized = false;

    cyhal_i2c_cfg_t i2c_cfg =
    {
        .is_slave = false,
        .address = 0,
        .frequencyhal_hz = 400000
    };

    result = cyhal_i2c_init(&i2c, IMU_SDA, IMU_SCL, NULL);
    if (result != CY_RSLT_SUCCESS)
    {
        printf("I2C init FAILED: 0x%08lX\n\r", (unsigned long)result);
        return result;
    }

    result = cyhal_i2c_configure(&i2c, &i2c_cfg);
    if (result != CY_RSLT_SUCCESS)
    {
        printf("I2C configure FAILED: 0x%08lX\n\r", (unsigned long)result);
        return result;
    }

    // Wake up MPU6050
    uint8_t data[2] = { PWR_MGMT_1, 0x00 };
    result = cyhal_i2c_master_write(&i2c, MPU6050_ADDR, data, 2, 10, true);
    if (result != CY_RSLT_SUCCESS)
    {
        printf("MPU wake write FAILED: 0x%08lX\n\r", (unsigned long)result);
        return result;
    }

    // WHO_AM_I check
    uint8_t reg = WHO_AM_I_REGISTER;
    uint8_t whoami = 0xFF;

    result = cyhal_i2c_master_write(&i2c, MPU6050_ADDR, &reg, 1, 10, false);
    if (result != CY_RSLT_SUCCESS)
    {
        printf("WHO_AM_I reg write FAILED: 0x%08lX\n\r", (unsigned long)result);
        return result;
    }

    result = cyhal_i2c_master_read(&i2c, MPU6050_ADDR, &whoami, 1, 10, true);
    if (result != CY_RSLT_SUCCESS)
    {
        printf("WHO_AM_I read FAILED: 0x%08lX\n\r", (unsigned long)result);
        return result;
    }

    printf("IMU WHO_AM_I: 0x%02X (expected 0x%02X)\n\r", whoami, EXPECTED_WHO_AM_I);

    imu_i2c_initialized = true;
    return CY_RSLT_SUCCESS;
}

void mpu6050_enable_interrupts(void)
{
    uint8_t buffer[2];
    uint8_t motion_threshold = 50;

    // Reset signal paths
    buffer[0] = 0x68; buffer[1] = 0x07;
    cyhal_i2c_master_write(&i2c, MPU6050_ADDR, buffer, 2, 10, true);

    // Interrupt pin config
    buffer[0] = INT_PIN_CFG; buffer[1] = 0x10;
    cyhal_i2c_master_write(&i2c, MPU6050_ADDR, buffer, 2, 10, true);

    // Accel full scale config
    buffer[0] = 0x1C; buffer[1] = 0x01;
    cyhal_i2c_master_write(&i2c, MPU6050_ADDR, buffer, 2, 10, true);

    // Motion threshold
    buffer[0] = MOT_THRESHOLD;
    buffer[1] = motion_threshold;  // 8 = very sensitive to slow lifts
    cyhal_i2c_master_write(&i2c, MPU6050_ADDR, buffer, 2, 10, true);

    // Motion duration
    buffer[0] = 0x20; buffer[1] = 10;
    cyhal_i2c_master_write(&i2c, MPU6050_ADDR, buffer, 2, 10, true);

    // Motion detect control
    buffer[0] = 0x69; buffer[1] = 15;
    cyhal_i2c_master_write(&i2c, MPU6050_ADDR, buffer, 2, 10, true);

    // Enable motion interrupt
    buffer[0] = MREG_INT_ENABLE; buffer[1] = MPU6050_MOTION_DETECT_EN;
    cyhal_i2c_master_write(&i2c, MPU6050_ADDR, buffer, 2, 10, true);

    // GPIO interrupt pin
    cyhal_gpio_init(MPU_INTERRUPT, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, false);
    cyhal_gpio_register_callback(MPU_INTERRUPT, &MPU6050_cb_data);
    cyhal_gpio_enable_event(MPU_INTERRUPT, CYHAL_GPIO_IRQ_RISE, 3, true);

    printf("IMU interrupts enabled\n\r");
}

void imu_init(void)
{
    cy_rslt_t result = InitI2C();
    if (result != CY_RSLT_SUCCESS)
    {
        printf("IMU init skipped (I2C not initialized)\n\r");
        return;
    }

    mpu6050_enable_interrupts();
}

void imu_read_all(int16_t *ax, int16_t *ay, int16_t *az,
                  int16_t *gx, int16_t *gy, int16_t *gz)
{
    if (!imu_i2c_initialized)
        return;

    uint8_t data[14];
    uint8_t reg = ACCEL_XOUT_H;

    cy_rslt_t res;

    res = cyhal_i2c_master_write(&i2c, MPU6050_ADDR, &reg, 1, 10, false);
    if (res != CY_RSLT_SUCCESS) return;

    res = cyhal_i2c_master_read(&i2c, MPU6050_ADDR, data, 14, 10, true);
    if (res != CY_RSLT_SUCCESS) return;

    // accel
    *ax = (int16_t)((data[0]  << 8) | data[1])  - calib_ax;
    *ay = (int16_t)((data[2]  << 8) | data[3])  - calib_ay;
    *az = (int16_t)((data[4]  << 8) | data[5])  - calib_az;

    // gyro (bytes 8..13 in the 14-byte burst)
    *gx = (int16_t)((data[8]  << 8) | data[9])  - calib_gx;
    *gy = (int16_t)((data[10] << 8) | data[11]) - calib_gy;
    *gz = (int16_t)((data[12] << 8) | data[13]) - calib_gz;
}
