# AGV 🏎️

![Overzicht](./docs/images/RC_Car.gif)

## 📖 Projectbeschrijving

Het **AGV_RCCar** project is een geavanceerd embedded systeem dat RC-voertuigen transformeert in Autonomous Guided Vehicles (AGV). Het systeem is ontworpen voor dual-mode operatie:
- **Manuele Modus:** Real-time besturing via een draadloze Bluetooth-verbinding (Joy-con).
- **Autonome Modus:** Zelfstandige navigatie met behulp van ultrasone sensoren om actief obstakels te vermijding.

De kernwaarde van dit project is de **cross-platform firmware-architectuur**. Hierdoor kunnen verschillende hardware-onderstellen aangestuurd worden met een uniforme logische basis.

---

## 📚 Documentatie

| Onderwerp | Lees meer |
|-----------|-----------|
| **Hardware** (BOM, pinbezetting, schema's) | [`hardware/README.md`](./hardware/README.md) |
| **Firmware — RC Car** (Anish) | [`firmware/Rc_Project/RC_Car_README.md`](./firmware/Rc_Project/RC_Car_README.md) |
| **Firmware — RC Cart** (Kaelig) | [`firmware/RC_CarT/Bluetooth_RC_Cart/`](./firmware/RC_CarT/Bluetooth_RC_Cart/) |
| **Tests** (projecten per subsystem) | [`tests/README.md`](./tests/README.md) |
| **Design Document** | [`docs/design.md`](./docs/design.md) |
| **User Manual** | [`docs/user-manual.md`](./docs/user-manual.md) |
| **Joy-Con Subsysteem** | [`docs/nl/joycon_nl.md`](./docs/nl/joycon_nl.md) |
| **Self-Driving Subsysteem** | [`docs/nl/self_driving_nl.md`](./docs/nl/self_driving_nl.md) |
| **Bluetooth Uitgelegd** | [`docs/nl/Bluetooth_explained_nl.md`](./docs/nl/Bluetooth_explained_nl.md) |
| **Bluetooth Protocol** | [`docs/nl/Bluetooth_protocol_nl.md`](./docs/nl/Bluetooth_protocol_nl.md) |

> English documentation is available in [`README-en.md`](./README-en.md).

---

## 🏗️ Hardware Varianten

Het project gebruikt dezelfde firmware-base op drie hardware-platformen:

| Variant | MCU | Motor Driver | Status |
|---------|-----|-------------|--------|
| **Mini RC Car** | CY8CKIT-062-WIFI-BT | 2x L9110S | ✅ Klaar |
| **2-Wheel RC Car** | CY8CPROTO-062-4343W | L298N | ✅ Klaar |
| **Mini RC Car (LijnTracker)** | CY8CKIT-062-WIFI-BT | 2x L9110S | ✅ Klaar |

Zie [`hardware/README.md`](./hardware/README.md) voor volledige BOM, pinbezetting, voedingsspecificaties en schema's.

---
## 🖼️ Afbeeldingen RC Cars

### RC Car

![RC Car](/docs/images/RC_Car.jpg)

### RC Car met lijntracker

![RC Car met lijntracker](/docs/images/RC_Car_LineTracker.jpg)
### 2 Wielen RC Car

![2 Wielen RC Car](/docs/images/2Wheel_RC_Car.jpg)
### RC CarT

![RC CarT](/docs/images/RC_CarT.png)


## 👥 Team

- **Anish Pun** [@Anish-Pun](https://github.com/Anish-Pun) 
- **Joel Akem** [@jjoellie](https://github.com/jjoellie)
- **Kaelig Guillemette** [@guikae06](https://github.com/guikae06)
