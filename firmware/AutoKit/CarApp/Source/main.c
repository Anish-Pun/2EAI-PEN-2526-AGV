#include "cybsp.h"
#include "cyhal.h"
#include "cy_retarget_io.h"
#include "pins.h"
#include "ultrasonic.h"
#include "motor.h"
#include "button.h"
#include "imu.h"

int main(void) {
  cy_rslt_t result = cybsp_init();
  if (result != CY_RSLT_SUCCESS) CY_ASSERT(0);

  __enable_irq();
  result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
  if (result != CY_RSLT_SUCCESS) CY_ASSERT(0);

  printf("\033[2J\033[\n\r");
  printf("Car App\n\r");
  printf("<20cm = STOP\n\r");

  ultrasonic_init();
  motor_init();
  button_init();
  extern bool Motion_Detected;
  imu_init();

/*
  uint8_t accel[6];
  uint8_t reg = ACCEL_XOUT_H;
  cyhal_i2c_master_write(&i2c, MPU6050_ADDR, &reg, 1, 10, false);
  cyhal_i2c_master_read(&i2c, MPU6050_ADDR, accel, 6, 10, true);
  int16_t ax = (accel[0]<<8) | accel[1];
  printf("Accel X:%d ", ax);
*/

  cyhal_system_delay_ms(1000);
  motor_stop();
  printf("Test OK. Press SW2!\n\r");

  motor_stop();  // Motors 0% from start
  bool car_running = false;

  while(1) {
    if (button_pressed()) {
      car_running = !car_running;
      Motion_Detected = false;  // ← Reset IMU flag
      if (car_running) {
        printf(" START\n\r");
        motor_forward(65);
      } else {
        motor_stop();
        printf(" STOPPED\n\r");
      }
    }

    if (Motion_Detected) {
      motor_stop();
      printf(" EMERGENCY LIFT STOP - Press SW2!\n\r");
      car_running = false;
      Motion_Detected = false;
    }
    if (car_running) {
      float right = get_distance_right_cm();
      float left = get_distance_left_cm();
      printf("R:%.1f L:%.1f ", right, left);

      if (right < 20.0f || left < 20.0f) {
        motor_stop();
        printf(" OBSTACLE STOP\n\r");
        car_running = false;
      } else if (right < 20.0f) {
        motor_turn_left(80);
        printf(" LEFT\n\r");
      } else if (left < 20.0f) {
        motor_turn_right(80);
        printf(" RIGHT\n\r");
      } else {
        motor_forward(65);
      }
      cyhal_system_delay_ms(100);
    }
  }
}
