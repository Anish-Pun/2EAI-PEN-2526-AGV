#include "joycon_drive_logic.h"

#include "motor.h"

#define JOYCON_DRIVE_PWM    (100u)

void joycon_drive_logic_reset(void)
{
    motor_stop();
}

void joycon_drive_logic_handle_hat(uint8_t hat)
{
    uint8_t hat_value = (uint8_t)(hat & 0x0Fu);

    if (hat_value == 0x00u)
    {
        motor_forward(JOYCON_DRIVE_PWM);
    }
    else if ((hat_value == 0x06u) || (hat_value == 0x07u))
    {
        motor_turn_right(JOYCON_DRIVE_PWM);
    }
    else if ((hat_value == 0x02u) || (hat_value == 0x01u))
    {
        motor_turn_left(JOYCON_DRIVE_PWM);
    }
        else if ((hat_value == 0x04u) || (hat_value == 0x01u))
    {
        motor_backward(JOYCON_DRIVE_PWM);
    }
    else
    {
        motor_stop();
    }
}