# AGV 🏎️

![Overview](./docs/images/RC_Car.gif)

## 📖 Project Description

The **AGV_RCCar** project is an advanced embedded system that transforms RC vehicles into Autonomous Guided Vehicles (AGV). The system is designed for dual-mode operation:
- **Manual Mode:** Real-time control via a wireless Bluetooth connection (Joy-con).
- **Autonomous Mode:** Self-contained navigation using ultrasonic sensors for active obstacle avoidance.

The core value of this project is the **cross-platform firmware architecture**. This allows different hardware chassis to be controlled with a uniform logical foundation.

See [`docs/AN_AGV.pdf`](./docs/AN_AGV.pdf) for the full application note.

---

## 📚 Documentation

| Topic | Read more |
|-------|-----------|
| **Hardware** (BOM, pin mapping, schematics) | [`hardware/README.md`](./hardware/README.md) |
| **Firmware — RC Car** (Anish) | [`firmware/Rc_Project/RC_Car_README.md`](./firmware/Rc_Project/RC_Car_README.md) |
| **Firmware — RC Cart** (Kaelig) | [`firmware/RC_CarT/Bluetooth_RC_Cart/`](./firmware/RC_CarT/Bluetooth_RC_Cart/) |
| **Tests** (projects per subsystem) | [`tests/README.md`](./tests/README.md) |
| **Design Document** | [`docs/design.md`](./docs/design.md) |
| **User Manual** | [`docs/user-manual.md`](./docs/user-manual.md) |
| **Joy-Con Subsystem** | [`docs/en/joycon_en.md`](./docs/en/joycon_en.md) |
| **Self-Driving Subsystem** | [`docs/en/self_driving_en.md`](./docs/en/self_driving_en.md) |
| **Bluetooth Explained** | [`docs/en/Bluetooth_explained_en.md`](./docs/en/Bluetooth_explained_en.md) |
| **Bluetooth Protocol** | [`docs/en/Bluetooth_protocol_en.md`](./docs/en/Bluetooth_protocol_en.md) |

> Nederlandse documentatie is beschikbaar in [`README.md`](./README.md).

---

## 🏗️ Hardware Variants

The project runs the same firmware base on three hardware platforms:

| Variant | MCU | Motor Driver | Status |
|---------|-----|-------------|--------|
| **Mini RC Car** | CY8CKIT-062-WIFI-BT | 2x L9110S | ✅ Ready |
| **2-Wheel RC Car** | CY8CPROTO-062-4343W | L298N | ✅ Ready |
| **Mini RC Car (LineTracker)** | CY8CKIT-062-WIFI-BT | 2x L9110S | ✅ Ready |

See [`hardware/README.md`](./hardware/README.md) for full BOM, pin mapping, power specs, and schematics.

---
## 🖼️ Pictures RC Cars

### RC Car

![RC Car](/docs/images/RC_Car.jpg)

### RC Car with linetracker

![RC Car with linetracker](/docs/images/RC_Car_LineTracker.jpg)
### 2 Wheel RC Car

![2 Wheel RC Car](/docs/images/2Wheel_RC_Car.jpg)
### RC CarT

![RC CarT](/docs/images/RC_CarT.png)

---
## 👥 Team

- **Anish Pun** [@Anish-Pun](https://github.com/Anish-Pun)
- **Joel Akem** [@jjoellie](https://github.com/jjoellie)
- **Kaelig Guillemette** [@guikae06](https://github.com/guikae06)
