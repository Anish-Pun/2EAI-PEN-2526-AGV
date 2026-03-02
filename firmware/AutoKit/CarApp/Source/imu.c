#include "imu.h"

cyhal_i2c_t i2c;
cyhal_gpio_t MPU_interrupt;
bool Motion_Detected = false;

// ISR functie VOOR callback
void ISR_mpu6050(void *callback_arg, cyhal_gpio_event_t event) {
  motor_stop();
  Motion_Detected = true;
  // printf("IMU LIFT DETECTED - EMERGENCY STOP\n\r");
}

cyhal_gpio_callback_data_t MPU6050_cb_data = {
  .callback = ISR_mpu6050,
  .callback_arg = NULL
};

void InitI2C(void)
{
    printf("InitI2C...\n\r");

    cy_rslt_t result;

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
        return;
    }

    result = cyhal_i2c_configure(&i2c, &i2c_cfg);
    if (result != CY_RSLT_SUCCESS)
    {
        printf("I2C configure FAILED: 0x%08lX\n\r", (unsigned long)result);
        return;
    }

    // Wake up MPU6050
    uint8_t data[2] = { PWR_MGMT_1, 0x00 };
    result = cyhal_i2c_master_write(&i2c, MPU6050_ADDR, data, 2, 10, true);
    if (result != CY_RSLT_SUCCESS)
    {
        printf("MPU wake write FAILED: 0x%08lX\n\r", (unsigned long)result);
        return;
    }

    // WHO_AM_I read: write register address, then read 1 byte
    uint8_t reg = WHO_AM_I_REGISTER;
    uint8_t whoami = 0xFF;

    result = cyhal_i2c_master_write(&i2c, MPU6050_ADDR, &reg, 1, 10, false);
    if (result != CY_RSLT_SUCCESS)
    {
        printf("WHO_AM_I reg write FAILED: 0x%08lX\n\r", (unsigned long)result);
        return;
    }

    result = cyhal_i2c_master_read(&i2c, MPU6050_ADDR, &whoami, 1, 10, true);
    if (result != CY_RSLT_SUCCESS)
    {
        printf("WHO_AM_I read FAILED: 0x%08lX\n\r", (unsigned long)result);
        return;
    }

    printf("IMU WHO_AM_I: 0x%02X (expected 0x68)\n\r", whoami);
}

void mpu6050_enable_interrupts(void) {
  uint8_t buffer[2];
  uint8_t motion_threshold = 20;

  buffer[0] = 0x68; buffer[1] = 0x07; cyhal_i2c_master_write(&i2c, MPU6050_ADDR, buffer, 2, 10, true);
  buffer[0] = INT_PIN_CFG; buffer[1] = 0x10; cyhal_i2c_master_write(&i2c, MPU6050_ADDR, buffer, 2, 10, true);
  buffer[0] = 0x1C; buffer[1] = 0x01; cyhal_i2c_master_write(&i2c, MPU6050_ADDR, buffer, 2, 10, true);
  buffer[0] = MOT_THRESHOLD; buffer[1] = motion_threshold; cyhal_i2c_master_write(&i2c, MPU6050_ADDR, buffer, 2, 10, true);
  buffer[0] = 0x20; buffer[1] = 10; cyhal_i2c_master_write(&i2c, MPU6050_ADDR, buffer, 2, 10, true);
  buffer[0] = 0x69; buffer[1] = 15; cyhal_i2c_master_write(&i2c, MPU6050_ADDR, buffer, 2, 10, true);
  buffer[0] = 0x38; buffer[1] = 0x40; cyhal_i2c_master_write(&i2c, MPU6050_ADDR, buffer, 2, 10, true);

  cyhal_gpio_init(MPU_INTERRUPT, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, false);
  cyhal_gpio_register_callback(MPU_INTERRUPT, &MPU6050_cb_data);
  cyhal_gpio_enable_event(MPU_INTERRUPT, CYHAL_GPIO_IRQ_RISE, 3, true);

  printf("IMU interrupts enabled\n\r");

  uint8_t int_status;
  cyhal_i2c_master_read(&i2c, MPU6050_ADDR, &int_status, 1, 10, true);
  printf("INT status reg: 0x%02X\n\r", int_status);
}

void imu_init(void) {
  InitI2C();
  mpu6050_enable_interrupts();
}
