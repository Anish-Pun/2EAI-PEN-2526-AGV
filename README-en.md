# AGV_RCCar 🏎️

![Overview](./docs/images/RC_Car.gif)

## 📖 Description

The **AGV_RCCar** project is an advanced embedded system that transforms RC vehicles into Automated Guided Vehicles (AGV). The system is designed for dual-mode operation:
- **Manual Mode:** Real-time control via a wireless Bluetooth connection (Joy-con).
- **Autonomous Mode:** Self-contained navigation using line-tracking and active obstacle avoidance via ultrasonic sensors.

The core value of this project lies in its **cross-platform firmware architecture**. This allows different hardware chassis (such as the Mini and Large variants) to be controlled using a uniform logical foundation.

---

## 🏗️ Hardware Variants

The project is divided into two specific hardware implementations that utilize the same software environment.

### 1. Mini RC Car (Primary Project)
This is the main platform for developing autonomous navigation algorithms.
*   **Microcontroller:** PSoC 6 (CY8CKIT-062-WIFI-BT)
*   **Focus:** Precision, line-tracking, and sensor data processing.

### 2. RC_CarT (Large Scale Project)
An initiative to scale the firmware to a larger, more powerful chassis.
*   **Motors:** 2x My1018 24V DC motors.
*   **Microcontroller:** PSoC 6 (CY8CPROTO-062-4343W) for consistency with the development environment.
*   **Status:** **ON HOLD**. The MD22 motor driver has failed. An industrial upgrade (Sabertooth 2x32A) is currently being sourced.
*   **Power:** Uses a DC converter to power the microcontroller directly from the main batteries for full autonomy.

---

## 🔋 Battery Management & Safety (RC_CarT)

For the large-scale vehicle, we use **Turnigy 6s 5800mAh (XT90 connectors)** batteries. Proper charging is crucial for battery lifespan and safety.

### Charging Instructions
Use the **Accucell 6** charger with the following settings:
1.  Select program: `LiPo BATT`.
2.  Select: `LiPo CHARGE`.
3.  **Voltage:** Manually set to `22.2V (6S)` if the charger does not recognize it automatically.
4.  **Current:** Maximum 5.0A, but **2.0A or 3.0A** is highly recommended for safety.

![Discharge](./docs/images/LiPo_discharge_menu.jpg) *(Image: Program menu for discharging the battery)*

![Charging Screen](./docs/images/) *(Image: The charging screen display during an active charge cycle)*

> **💡 Extra Tip:** If the Accucell fails to recognize the battery due to a voltage that is too low (deep discharge), briefly charge the battery using a DC lab power supply set to exactly 22.2V until the voltage is high enough for the charger to detect the cells again.

---

## ⚙️ Technical Stack

| Component | Technology |
|:--- |:--- |
| **Microcontrollers** | PSoC 6 (Proto & WiFi-BT), Arduino Uno (Testing) |
| **Motor Drivers** | MD22 (Legacy), Sabertooth 2x32A |
| **Dev Environment** | ModusToolbox™ (Infineon) |
| **Programming Language** | C (Embedded) |
| **Sensors** | HC-SR04 Ultrasonic, Line Tracking Array |

---

## 🚀 Sequence of Operation

1.  **Power-up:** Connect the board via USB or turn on the main switch (for battery power).
2.  **System Status:** The green LED lights up once the firmware is active.
3.  **Communication:** Open the ModusToolbox Terminal to view welcome messages and debug information.
4.  **Calibration:** The linear slider centers itself automatically; wait for the audible beep before operation.

---

## 🛠️ Getting Started

### Prerequisites
*   ModusToolbox™ 3.x installed.
*   Relevant BSPs (Board Support Packages) for the PSoC 6 variants.

### Project Setup
1.  **Import Project:** Open ModusToolbox and import the desired workspace: `AutoKit` (Mini) or `RC_CarT` (Large).
2.  **Build & Flash:** Perform a `Clean` and `Build`, then use the `Program` tool to upload the code to the microcontroller.

---

## 👥 Team

- **Anish Pun** [@Anish-Pun](https://github.com/Anish-Pun) – *Software Architecture*
- **Joel Akem** [@jjoellie](https://github.com/jjoellie) – *Firmware Development*
- **Kaelig Guillemette** [@guikae06](https://github.com/guikae06) – *Hardware Engineering*
