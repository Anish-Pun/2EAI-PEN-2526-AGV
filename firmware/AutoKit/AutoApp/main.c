#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"


// Pin Definitions
#define TRIG_PIN1   P10_0   //  right
#define ECHO_PIN1   P10_1
#define TRIG_PIN2   P10_2   //  left
#define ECHO_PIN2   P10_3

//  Function Prototypes
float read_ultrasonic_sensor(cyhal_gpio_t trig_pin, cyhal_gpio_t echo_pin);

// Main Function

int main(void)
{
    cy_rslt_t result;

    /* Initialize the board */
    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize UART for printf */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX,
                                 CYBSP_DEBUG_UART_RX,
                                 CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    printf("\033[2J\033[H");

    /* Initialize Ultrasonic Sensor GPIOs */
    cyhal_gpio_init(TRIG_PIN1, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    cyhal_gpio_init(ECHO_PIN1, CYHAL_GPIO_DIR_INPUT,  CYHAL_GPIO_DRIVE_NONE,   false);
    cyhal_gpio_init(TRIG_PIN2, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    cyhal_gpio_init(ECHO_PIN2, CYHAL_GPIO_DIR_INPUT,  CYHAL_GPIO_DRIVE_NONE,   false);

    for (;;)
    {
        float distance1 = read_ultrasonic_sensor(TRIG_PIN1, ECHO_PIN1); // Sensor 1
        float distance2 = read_ultrasonic_sensor(TRIG_PIN2, ECHO_PIN2); // Sensor 2

        // Optional clamping to 50 cm like before
        if (distance1 > 50) distance1 = 50;
        if (distance2 > 50) distance2 = 50;

        printf("Ultrasonic 1: %.2f cm | Ultrasonic 2: %.2f cm\r\n", distance1, distance2);

        cyhal_system_delay_ms(200);
    }
}

// Ultrasonic Sensor Function
float read_ultrasonic_sensor(cyhal_gpio_t trig_pin, cyhal_gpio_t echo_pin)
{
    // Send trigger pulse (per HC-SR04 timing diagram)[web:24][web:21]
    cyhal_gpio_write(trig_pin, 0);
    cyhal_system_delay_us(5);
    cyhal_gpio_write(trig_pin, 1);
    cyhal_system_delay_us(10);
    cyhal_gpio_write(trig_pin, 0);

    const uint32_t timeout = 20000;
    uint32_t pulse_start = 0;
    uint32_t pulse_end = 0;

    // Wait for the echo signal to start
    while (!cyhal_gpio_read(echo_pin))
    {
        pulse_start++;
        cyhal_system_delay_us(1);
        if (pulse_start > timeout) return 1000; // timeout -> no object
    }

    // Measure high time of echo
    pulse_start = 0;
    while (cyhal_gpio_read(echo_pin))
    {
        pulse_end++;
        cyhal_system_delay_us(1);
        if (pulse_end > timeout) return 1000; // timeout -> out of range
    }

    // Distance(cm) = duration(us) * 0.0343 / 2 [web:24][web:27]
    float duration = (float)pulse_end;
    float distance = (duration * 0.0343f) / 2.0f;

    return distance;
}
