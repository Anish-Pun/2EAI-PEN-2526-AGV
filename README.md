# AGV 🏎️

![Overzicht](./docs/images/RC_Car.gif)

## 📖 Projectbeschrijving

Het **AGV_RCCar** project is een geavanceerd embedded systeem dat RC-voertuigen transformeert in Autonomous Guided Vehicles (AGV). Het systeem is ontworpen voor dual-mode operatie:
- **Manuele Modus:** Real-time besturing via een draadloze Bluetooth-verbinding (Joy-con).
- **Autonome Modus:** Zelfstandige navigatie met behulp van ultrasone sensoren om actief obstakels te vermijding.

De kernwaarde van dit project is de **cross-platform firmware-architectuur**. Hierdoor kunnen verschillende hardware-onderstellen (zoals de Mini en de Large variant) aangestuurd worden met een uniforme logische basis.

---

## 🏗️ Hardware Varianten

Het project is onderverdeeld in twee specifieke hardware-implementaties die gebruikmaken van dezelfde software-omgeving.

### 1. Mini RC Car (Hoofdproject)
Dit is het primaire platform voor de ontwikkeling van de autonome navigatie-algoritmes en firmware-architectuur.
* **Microcontroller:** PSoC 6 (CY8CKIT-062-WIFI-BT).
* **Besturing:** Gebruikt FreeRTOS-taken om subsystemen zoals de Joy-con (Bluetooth) en Self-Driving parallel te beheren.
* **Focus:** Precisie, geavanceerde sensorverwerking en cross-platform compatibiliteit.

### 2. RC_CarT (Large Scale Project)
Een poging om de firmware te schalen naar een groter, krachtiger onderstel voor zware toepassingen.
* **Motoren:** 2x My1018 24V DC motoren.
* **Microcontroller:** PSoC 6 (CY8CPROTO-062-4343W) voor consistentie met de ontwikkelomgeving.
* **Status:** **ON HOLD**. De MD22 motor driver is defect geraakt. Er wordt gekeken naar een industriële upgrade (Sabertooth 2x32A).
* **Power:** Maakt gebruik van een DC-converter voor volledige autonomie via de hoofdaccu's.

---

## 🤖 Subsystemen & Functionaliteit

### 🎮 Joy-Con Subsystem (Manual Control)
Verzorgt de draadloze HID-communicatie en fungeert als de interface voor de gebruiker.
* **Bluetooth:** Maakt gebruik van de WICED stack onder de naam `PSoC6-JoyConHost`.
* **Interactie:** Verwerkt knoppen voor rijcommando's en het wisselen tussen modi.
* **Indicatie:** Een LED op pin `P13_7` geeft de verbindingsstatus aan.

### 🤖 Self-Driving Subsystem (Autonomous Control)
Een rule-based controller die de auto autonoom laat navigeren.
* **Obstakelvermijding:** Twee ultrasone sensoren detecteren objecten binnen 25 cm.
* **Veiligheid:** Een MPU6050 IMU detecteert botsingen (motion interrupts) of het optillen van de auto (Lift Detection via Z-as versnelling).
* **Feedback:** Wielencoders houden afgelegde afstand en snelheid bij.

---

## 🛠️ Hardware & Pinbezetting (Mini RC Car)

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

## 🔋 Batterijbeheer & Veiligheid (RC_CarT)

Voor de grote wagen gebruiken we **Turnigy 6s 5800mAh (XT90)** batterijen. Correct opladen is cruciaal voor de veiligheid.

### Oplaadinstructies (Accucell 6)
1. Kies programma: `LiPo BATT` -> `LiPo CHARGE`.
2. **Voltage:** Handmatig instellen op `22.2V (6S)`.
3. **Stroomsterkte:** Maximaal 5.0A (advies: **2.0A of 3.0A** voor veiligheid).

> **💡 Extra Tip:** Als de batterij te diep ontladen is (niet herkend), laad deze dan heel kort op via een DC labvoeding op exact 22.2V totdat de Accucell de cellen weer herkent.

---

## ⚙️ Technische Stack

- **Microcontrollers:** PSoC 6 (Proto & WiFi-BT), Arduino Uno (Test).
- **Motor Drivers:** MD22 (Legacy), Sabertooth 2x32A.
- **Ontwikkelomgeving:** ModusToolbox™ (Infineon).
- **Programmeertaal:** C (Embedded).
- **Sensoren:** HC-SR04 Ultrasoon, MPU6050 IMU.

---

## 🚀 Sequence of Operation

1.  **Power-up:** Verbind via USB of zet de hoofdschakelaar aan.
2.  **Systeemstatus:** De groene LED licht op bij actieve firmware.
3.  **Bluetooth:** Zoek en verbind met een Joy-con controller.
4.  **Runtime:** Wissel tussen **Manual mode** (directe input) en **Self-Driving mode** (autonoom).

---

## 🛠️ Aan de slag

### Prerequisites
* ModusToolbox™ 3.x geïnstalleerd.

### Project Setup
1. **Importeer Project:** Open ModusToolbox en importeer de workspace: `AutoKit` (Mini) of `RC_CarT` (Large).
2. **Build & Flash:** Voer een `Clean` en `Build` uit en gebruik de `Program` tool om de code te uploaden.

---

## 👥 Team

- **Anish Pun** [@Anish-Pun](https://github.com/Anish-Pun) – *Software Architecture*
- **Joel Akem** [@jjoellie](https://github.com/jjoellie) – *Firmware Development*
- **Kaelig Guillemette** [@guikae06](https://github.com/guikae06) – *Hardware Engineering*
