# AGV 🏎️

![Overview](./docs/images/RC_Car.gif)

## 📖 Project Description

The **AGV_RCCar** project is an advanced embedded system that transforms RC vehicles into Autonomous Guided Vehicles (AGV). The system is designed for dual-mode operation:
- **Manual Mode:** Real-time control via a wireless Bluetooth connection (Joy-con).
- **Autonomous Mode:** Self-contained navigation using ultrasonic sensors for active obstacle avoidance.

The core value of this project is the **cross-platform firmware architecture**. This allows different hardware chassis (such as the Mini and Large variants) to be controlled with a uniform logical foundation.

---

## Documentation

Detailed documentation can be found in the [docs](./docs/) folder:

### General
- [Design Document](./docs/design.md)
- [User Manual](./docs/user-manual.md)

### English
- [Joy-Con Subsystem](./docs/en/joycon_en.md)
- [Self-Driving Subsystem](./docs/en/self_driving_en.md)
- [Bluetooth Explained](./docs/en/Bluetooth_explained_en.md)
- [Bluetooth Protocol](./docs/en/Bluetooth_protocol_en.md)

### Dutch
- [Joy-Con Subsysteem](./docs/nl/joycon_nl.md)
- [Self-Driving Subsysteem](./docs/nl/self_driving_nl.md)
- [Bluetooth Uitgelegd](./docs/nl/Bluetooth_explained_nl.md)
- [Bluetooth Protocol](./docs/nl/Bluetooth_protocol_nl.md)


## 🏗️ Hardware Variants

The project is divided into two specific hardware implementations utilizing the same software environment.

### 1. Mini RC Car (Main Project)
This is the primary platform for developing autonomous navigation algorithms and firmware architecture.
* **Microcontroller:** PSoC 6 (CY8CKIT-062-WIFI-BT).
* **Control:** Uses FreeRTOS tasks to manage subsystems like Joy-con (Bluetooth) and Self-Driving in parallel.
* **Focus:** Precision, advanced sensor processing, and cross-platform compatibility.

### 2. RC_CarT (Large Scale Project)
An initiative to scale the firmware to a larger, more powerful chassis for heavy-duty applications.
* **Motors:** 2x My1018 24V DC motors.
* **Microcontroller:** PSoC 6 (CY8CPROTO-062-4343W) for consistency with the development environment.
* **Status:** **ON HOLD**. The MD22 motor driver has failed. An industrial upgrade (Sabertooth 2x32A) is being considered.
* **Power:** Uses a DC converter for full autonomy via the main batteries.

---

## 🤖 Subsystems & Functionality

### 🎮 Joy-Con Subsystem (Manual Control)
Handles wireless HID communication and acts as the user interface.
* **Bluetooth:** Uses the WICED stack under the name `PSoC6-JoyConHost`.
* **Interaction:** Processes buttons for driving commands and switching between modes.
* **Indication:** An LED on pin `P13_7` indicates the connection status.

### 🤖 Self-Driving Subsystem (Autonomous Control)
A rule-based controller that allows the car to navigate autonomously.
* **Obstacle Avoidance:** Two ultrasonic sensors detect objects within 25 cm.
* **Safety:** An MPU6050 IMU detects collisions (motion interrupts) or lifting of the car (Lift Detection via Z-axis acceleration).
* **Feedback:** Wheel encoders track distance traveled and speed.

---

## 🛠️ Hardware & Pin Mapping (Mini RC Car)

### Mini_Rc_Car (CY8CKIT-062-WIFI-BT)

| Peripheral | Signal | Pin |
|---|---|---|
| UART TX (debug) | TX | P5_1 |
| UART RX (debug) | RX | P5_0 |
| Motor A | PWM | P10_0 |
| Motor B | PWM | P10_1 |
| Motor C | PWM | P10_2 |
| Motor D | PWM | P10_3 |
| Ultrasonic Right | Trigger | P9_0 |
| Ultrasonic Right | Echo | P9_1 |
| Ultrasonic Left | Trigger | P9_4 |
| Ultrasonic Left | Echo | P9_5 |
| IMU SDA | I²C data | P6_1 |
| IMU SCL | I²C clock | P6_0 |
| IMU Interrupt | GPIO IRQ | P10_5 |
| Encoder Right | GPIO IRQ | P9_6 |
| Encoder Left | GPIO IRQ | P9_7 |
| Start/Stop SW2 | GPIO input | P0_4 |
| Status LED | GPIO output | P13_7 |

### Rc_CarT (CY8CPROTO-062-1313W)

| Peripheral | Signal | Pin |
|---|---|---|
| Motor A | PWM | P5_4 |
| Motor B | PWM | P5_5 |


---

## 🔋 Battery Management & Safety (RC_CarT)

For the large-scale car, we use **Turnigy 6s 5800mAh (XT90)** batteries. Correct charging is crucial for safety.

### Charging Instructions (Accucell 6)
1. Select program: `LiPo BATT` -> `LiPo CHARGE`.
2. **Voltage:** Manually set to `22.2V (6S)`.
3. **Current:** Maximum 5.0A (recommended: **2.0A or 3.0A** for safety).

> **💡 Extra Tip:** If the battery is over-discharged (not recognized), charge it briefly via a DC lab power supply at exactly 22.2V until the Accucell recognizes the cells again.

---

## ⚙️ Technical Stack

- **Microcontrollers:** PSoC 6 (Proto & WiFi-BT), Arduino Uno (Testing).
- **Motor Drivers:** MD22 (Legacy), Sabertooth 2x32A.
- **Development Environment:** ModusToolbox™ (Infineon).
- **Programming Language:** C (Embedded).
- **Sensors:** HC-SR04 Ultrasonic, MPU6050 IMU.

---

## 🚀 Sequence of Operation

1.  **Power-up:** Connect via USB or turn on the main switch.
2.  **System Status:** The green LED lights up when the firmware is active.
3.  **Bluetooth:** Search for and connect to a Joy-con controller.
4.  **Runtime:** Switch between **Manual mode** (direct input) and **Self-Driving mode** (autonomous).

---

## 🛠️ Getting Started

### Prerequisites
* ModusToolbox™ 3.x installed.

### Project Setup
1. **Import Project:** Open ModusToolbox and import the workspace: `AutoKit` (Mini) or `RC_CarT` (Large).
2. **Build & Flash:** Perform a `Clean` and `Build`, then use the `Program` tool to upload the code.

---

## 👥 Team

- **Anish Pun** [@Anish-Pun](https://github.com/Anish-Pun) – *Software Architecture*
- **Joel Akem** [@jjoellie](https://github.com/jjoellie) – *Firmware Development*
- **Kaelig Guillemette** [@guikae06](https://github.com/guikae06) – *Hardware Engineering*
