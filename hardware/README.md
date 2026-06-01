# 🛠️ Hardware Specification & Bill of Materials (BOM)

Dit document bevat de definitieve hardwareconfiguratie, pin-toewijzungen, kritieke voedingsspecificaties en de componentenlijst voor zowel de **Mini RC Car** als de **RC Cart (AGV)**.

---

## 1. Bill of Materials (BOM) & Datasheets

*Status: Componentenlijst geconsolideerd. Alle datasheets zijn lokaal opgeslagen in `/docs/datasheets/` conform projectrichtlijnen.*

| Component | Type / Model | Functie | Lokale Datasheet Link | Aantal | Toegepast in |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Microcontroller** | Infineon PSoC 6 | Hoofdcontroller (Proto Kit base) | [PSoC6_Proto.pdf](/docs/datasheets/infineon-cy8cproto-062-4343w-psoc-6-wi-fi-bt-prototyping-kit-guide-usermanual-en.pdf) | 1 | RC Cart (AGV) |
| **Development Kit** | CY8CKIT-062-WIFI-BT | PSoC 6 WiFi-BT Pioneer Kit base | [Pioneer_Kit_Guide.pdf](/docs/datasheets/infineon-cy8ckit-062-wifi-bt-psoc-6-wifi-bt-pioneer-kit-guide-usermanual-en.pdf) | 1 | Mini RC Car |
| **Ultrasoon Sensor** | HC-SR04 | Afstandsmeting & Obstakeldetectie | [HCSR04.pdf](/docs/datasheets/HCSR04.pdf) | 2 | Mini RC Car |
| **Line Tracker** | KY-033 | Infrarood Lijnvolging | [KY-033.pdf](/docs/datasheets/p587610-KY-033-Joy-IT.pdf) | 2 | Mini RC Car |
| **Motor Driver (Mini)**| L298N | Dual H-Bridge Motor Driver | [L298N_Motor_Driver.pdf](/docs/datasheets/L298N_Motor_Driver.pdf) | 1 | Mini RC Car |
| **Motor Driver (Mini Alt.)** | L9110S | Dual H-Bridge Motor Driver | [L9110S_motor_driver.pdf](/docs/datasheets/L9110S_motor_driver.pdf) | 1 | Mini RC Car |
| **Motor Driver (AGV)** | Devantech MD22 | 24V 50A Dual H-Bridge Driver | [MD22_Doc.pdf](/docs/datasheets/MD22_TechnicalDocumentation.pdf) | 1 | RC Cart (AGV) |
| **DC-DC Converter** | SBC-BUCK01 | Step-down spanningstabilisatie logica AGV | [SBCBUCK01.pdf](/docs/datasheets/SBCBUCK01.pdf) | 1 | RC Cart (AGV) |
| **Accu Lader** | Turnigy Accucell 6 | Balanceer lader voor accupacks | [Accucell_6.pdf](/docs/datasheets/Accucell_6.pdf) | 1 | Ondersteuning |
| **Snelheidssensor** | SEN-SPEED | Hall-effect encoder voor wielsnelheid | [SEN-SPEED.pdf](/docs/datasheets/SEN-SPEED.pdf) | 2 | Mini RC Car |
| **DC Motor (AGV)** | MY1018 | 24V Aandrijving met vertraging | *Geen (zie MD22 spec)* | 2 | RC Cart (AGV) |
| **DC Motor (Mini)** | Standaard DC | 6V Aandrijving wielen | *N.v.t.* | 4 | Mini RC Car |
| **Draadloze Controller**| Joy-Con / DualShock| Handmatige HID-besturing via BT | *N.v.t.* | 1 | Beide |

---

## 2. Microcontroller Pin Mapping (PSoC6)

Alle GPIO-definities zijn hardcoded in `common/pins.h`. Conflict tussen hardware-varianten wordt opgelost via conditionele compilatie (`#ifdef TARGET_BOARD`).

| Perifeer Subsysteem | Signaal | PSoC6 Pin | Richting (I/O) | Voertuig |
| :--- | :--- | :--- | :--- | :--- |
| **UART (Debug/Log)** | TX | **P5_1** | Output | Beide |
| **UART (Debug/Log)** | RX | **P5_0** | Input | Beide |
| **Aandrijving (PWM)** | Motor A PWM | **P10_0** | Output | Mini RC Car |
| **Aandrijving (PWM)** | Motor B PWM | **P10_1** | Output | Mini RC Car |
| **Aandrijving (PWM)** | Motor C PWM | **P10_2** | Output | Mini RC Car |
| **Aandrijving (PWM)** | Motor D PWM | **P10_3** | Output | Mini RC Car |
| **Ultrasoon Rechts** | Trigger | **P9_0** | Output | Mini RC Car |
| **Ultrasoon Rechts** | Echo | **P9_1** | Input (IRQ) | Mini RC Car |
| **Ultrasoon Links** | Trigger | **P9_4** | Output | Mini RC Car |
| **Ultrasoon Links** | Echo | **P9_5** | Input (IRQ) | Mini RC Car |
| **Sensoren (I²C)** | IMU SDA | **P6_1** | I/O | Mini RC Car |
| **Sensoren (I²C)** | IMU SCL | **P6_0** | Output | Mini RC Car |
| **Sensoren (Interrupt)**| IMU IRQ | **P10_5** | Input (IRQ) | Mini RC Car |
| **Encoders (Snelheid)** | Encoder Rechts | **P9_6** | Input (IRQ) | Mini RC Car |
| **Encoders (Snelheid)** | Encoder Links | **P9_7** | Input (IRQ) | Mini RC Car |
| **User Interface** | Start/Stop SW2 | **P0_4** | Input | Beide |
| **User Interface** | Status LED | **P13_7** | Output | Beide |

---

## 3. Elektrische & Voedingsspecificaties

### Mini RC Car (Pioneer Kit Base)
* **Logische Spanning (MCU):** 3.3V DC via USB-Powerbank (gedocumenteerd in de [Pioneer Kit Guide](/docs/datasheets/infineon-cy8ckit-062-wifi-bt-psoc-6-wifi-bt-pioneer-kit-guide-usermanual-en.pdf)).
* **Vermogensspanning (Motoren):** 6.0V DC via 4× AA-batterijen, geschakeld via de **L298N** dual H-bridge driver. Het vermogenscircuit is strikt gescheiden van het logische circuit om spanningsdippen door pieken te elimineren.
* **Communicatie:** Bluetooth BR/EDR (HID-profiel voor controller), I²C (IMU), PWM (Motor control op 20kHz).

### RC Cart / AGV (Proto Kit Base)
* **Logische Spanning (MCU):** 3.3V DC stabiel geleverd door de **SBC-BUCK01** step-down converter, aangesloten op het hoofdaccupack.
* **Vermogensspanning (Motoren):** 24V DC accupack (veilig te laden via de **Accucell 6** balanceer lader) rechtstreeks gekoppeld aan de **Devantech MD22** Motor Driver.
* **Kritiek Executierisico:** Omdat de MD22 tot 50A kan leveren, is een mechanische noodstopschakelaar en inline zekering direct na de accu verplicht om hardware-defecten en oververhitting te voorkomen.

---

## 4. Systeemschema's & Architectuur

### Mini RC Car Elektrisch Schema
Dit schema toont de interfacing tussen de Pioneer Kit, de L298N driver, de HC-SR04 sensoren, de IMU en de 4 low-power DC-motoren.
![RC Car Schematic](../docs/images/Car_schematica.png)

### RC Cart (AGV) Blokdiagram
Dit schema illustreert de hoog-vermogen architectuur: de Proto Kit stuurt via de SBC-BUCK01 gereguleerde spanning aan, en de MD22 driver regelt de zware MY1018 motoren.
![AGV Schematic](../docs/images/AGV_Schema.png)

---
