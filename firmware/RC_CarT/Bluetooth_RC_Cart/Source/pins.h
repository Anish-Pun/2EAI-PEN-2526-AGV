/*
 * pins.h - Volledig gecentraliseerde, conflictvrije pin-configuratie op Poort 9
 * Specifiek voor de CY8CPROTO-062-4343W Prototyping Kit
 */

#ifndef PINS_H
#define PINS_H

#include "cycfg_pins.h"

// ==========================================
// PWM PINNEN (Snelheid)
// Gekozen op Poort 9: Onafhankelijke TCPWM-lijnen (Line 4 en Line 7)
// ==========================================
#define MOTOR_A_PWM  P9_6  // Volledig vrije PWM-pin op de header
#define MOTOR_B_PWM  P9_7  // Volledig vrije PWM-pin (Zit direct naast P9_6)

// ==========================================
// RICHTING PINNEN (Gewone GPIO)
// Ook op Poort 9, zodat je hele kabelboom op één header aansluit
// ==========================================
// Motor A Richting (Links)
#define MOTOR_A_IN1  P9_0  
#define MOTOR_A_IN2  P9_1  

// Motor B Richting (Rechts)
#define MOTOR_B_IN3  P9_2  
#define MOTOR_B_IN4  P9_4  // Veilig weg van P9_3 (zoals gevraagd)

#endif