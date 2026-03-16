#include "remote_control.h"
#include "motor.h"
#include "cyhal.h"
#include <stdio.h>

static int printed = 0;

void remote_control_enter(void)
{
    motor_stop();
    printed = 0;
    printf("\n\rBLUETOOTH MODE\n\r");
    printf("Press SW2 to switch to self-driving.\n\r");
}

void remote_control_exit(void)
{
    motor_stop();
    printed = 0;
    printf("\n\rEXIT BLUETOOTH MODE\n\r");
}

void remote_control_step(void)
{
    motor_stop();

    if (!printed)
    {
        printf("\rBT: idle ( real BLE is not implemented yet) ", "");
        fflush(stdout);
        printed = 1;
    }

    cyhal_system_delay_ms(50);
}
