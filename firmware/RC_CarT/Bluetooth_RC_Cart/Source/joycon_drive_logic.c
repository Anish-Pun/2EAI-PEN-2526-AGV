#include "joycon_drive_logic.h"
#include "motor.h"
#include <stdbool.h>

/* =========================================================================
   CONFIGURATIE & SNELHEDEN
   ========================================================================= */
// Snelheden gedefinieerd tussen 0 en 100 (percentage)
#define RC_CAR_SPEED_NORMAL   (60u)   // Comfortabele, soepele basissnelheid
#define RC_CAR_SPEED_BOOST    (100u)  // Volledige 100% kracht bij boost

/* =========================================================================
   INTERNE STATE
   ========================================================================= */
// Houdt bij of de Boost-knop (X-knop) fysiek is ingedrukt
static bool is_boost_button_pressed = false;

/* =========================================================================
   PUBLIC API IMPLEMENTATIE
   ========================================================================= */

/**
 * Reset de rij-logica naar de veilige beginstand (stilstand).
 */
void joycon_drive_logic_reset(void)
{
    is_boost_button_pressed = false;
    motor_stop();
}

/**
 * Registreert de status van de RC_CAR_MODE_BOOST knop (X-knop).
 * @param active true als de knop is ingedrukt, false als deze is losgelaten.
 */
void joycon_drive_logic_set_boost(bool active)
{
    is_boost_button_pressed = active;
}

/**
 * Verwerkt de binnenkomende Hat (D-pad / Joystick) van de Joy-Con.
 * Bevat de veiligheidsregel: Boost werkt *alleen* als er een richting gekozen is.
 */
void joycon_drive_logic_handle_hat(uint8_t hat)
{
    uint8_t hat_value = (uint8_t)(hat & 0x0Fu);
    uint8_t current_speed = RC_CAR_SPEED_NORMAL;

    // VEILIGHEIDSCHECK: Als we in het midden staan, negeren we de boost-knop volledig
    if (hat_value == 0x08u)
    {
        motor_stop();
        return; // Stop direct met de functie, auto blijft gegarandeerd stilstaan
    }

    // Als we HIER komen, betekent het dat de joystick in een richting staat gedrukt.
    // Nu mag de boost-knop wel invloed hebben op de snelheid!
    if (is_boost_button_pressed)
    {
        current_speed = RC_CAR_SPEED_BOOST;
    }
    else
    {
        current_speed = RC_CAR_SPEED_NORMAL;
    }

    // Stuur de motor aan in de juiste richting met de berekende snelheid
    switch (hat_value)
    {
        case 0x00u: // UP
            motor_forward(current_speed);
            break;

        case 0x04u: // DOWN
            motor_backward(current_speed);
            break;

        case 0x02u: // RIGHT
        case 0x03u: // DOWN-RIGHT
            motor_turn_right(current_speed);
            break;

        case 0x06u: // LEFT
        case 0x07u: // UP-LEFT
            motor_turn_left(current_speed);
            break;

        default:
            motor_stop();
            break;
    }
}