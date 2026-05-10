# AGV_RCCar 🏎️

![Overzicht](./docs/images/RC_Car.gif)

## 📖 Beschrijving

Het **AGV** project is een geavanceerd embedded systeem dat RC-voertuigen transformeert in autonome geleide voertuigen (AGV). Het systeem is ontworpen voor dual-mode operatie:
- **Manuele Modus:** Real-time besturing via een draadloze Bluetooth-verbinding (Joy-con).
- **Autonome Modus:** Zelfstandige navigatie middels line-tracking en actieve obstakelvermijding via ultrasone sensoren.

De kernwaarde van dit project is de **cross-platform firmware-architectuur**. Hierdoor kunnen verschillende hardware-onderstellen (zoals de Mini en de Large variant) aangestuurd worden met een uniforme logische basis.

---

## 🏗️ Hardware Varianten

Het project is onderverdeeld in twee specifieke hardware-implementaties die gebruikmaken van dezelfde software-omgeving.

### 1. Mini RC Car (Hoofdproject)
Dit is het primaire platform voor de ontwikkeling van de autonome navigatie-algoritmes.
*   **Microcontroller:** PSoC 6 (CY8CKIT-062-WIFI-BT)
*   **Focus:** Precisie, line-tracking en sensordata-verwerking.

### 2. RC_CarT (Large Scale Project)
Een poging om de firmware te schalen naar een groter, krachtiger onderstel.
*   **Motoren:** 2x My1018 24V DC motoren.
*   **Microcontroller:** PSoC 6 (CY8CPROTO-062-4343W) voor consistentie met de ontwikkelomgeving.
*   **Status:** **ON HOLD**. De MD22 motor driver is defect geraakt. Er wordt gekeken naar een industriële upgrade (Sabertooth 2x32A).
*   **Power:** Gebruikt een DC-converter om de microcontroller rechtstreeks uit de hoofdaccu's te voeden voor volledige autonomie.

---

## 🔋 Batterijbeheer & Veiligheid (RC_CarT)

Voor de grote wagen gebruiken we **Turnigy 6s 5800mAh (XT90 connectoren)** batterijen. Het correct opladen is cruciaal voor de levensduur en veiligheid.

### Oplaadinstructies
Gebruik de **Accucell 6** lader met de volgende instellingen:
1.  Kies het programma: `LiPo BATT`.
2.  Selecteer: `LiPo CHARGE`.
3.  **Voltage:** Stel handmatig in op `22.2V (6S)` als de lader dit niet automatisch herkent.
4.  **Stroomsterkte:** Maximaal 5.0A, maar **2.0A of 3.0A** wordt sterk aangeraden voor de veiligheid.

![Discharge](./docs/images/LiPo_discharge_menu.jpg) *(Afbeelding: Programma om het batterij te onladen)*

![Laadscherm](./docs/images/accucell_screen.jpg) *(Afbeelding: Het laad scherm wanner de batterij aan het opladen bent)*

> **💡 Extra Tip:** Als de Accucell de batterij niet herkent omdat het voltage te laag is (deep discharge), laad de accu dan heel even kort op via een DC labvoeding op exact 22.2V totdat het voltage hoog genoeg is voor de lader om de cellen weer te herkennen.

---

## ⚙️ Technische Stack

| Component | Technologie |
|:--- |:--- |
| **Microcontrollers** | PSoC 6 (Proto & WiFi-BT), Arduino Uno (Test) |
| **Motor Drivers** | MD22 (Legacy), Sabertooth 2x32A |
| **Ontwikkelomgeving** | ModusToolbox™ (Infineon) |
| **Programmeertaal** | C (Embedded) |
| **Sensoren** | HC-SR04 Ultrasoon, Line Tracking Array |

---

## 🚀 Sequence of Operation

1.  **Power-up:** Verbind het bord via USB of zet de hoofdschakelaar aan (voor batterijvoeding).
2.  **Systeemstatus:** De groene LED licht op wanneer de firmware actief is.
3.  **Communicatie:** Open de ModusToolbox Terminal voor welkomstberichten en debug-informatie.
4.  **Kalibratie:** De lineaire slider centreert zichzelf automatisch; wacht op het piepsignaal voor gebruik.

---

## 🛠️ Aan de slag

### Prerequisites
*   ModusToolbox™ 3.x geïnstalleerd.


### Project Setup
1.  **Project importeren:** Open ModusToolbox en importeer de gewenste workspace: `AutoKit` (Mini) of `RC_CarT` (Large).
2.  **Build & Flash:** Voer een `Clean` en `Build` uit en gebruik de `Program` tool om de code te uploaden.

---

## 👥 Team

- **Anish Pun** [@Anish-Pun](https://github.com/Anish-Pun) – *Software Architecture*
- **Joel Akem** [@jjoellie](https://github.com/jjoellie) – *Firmware Development*
- **Kaelig Guillemette** [@guikae06](https://github.com/guikae06) – *Hardware Engineering*
