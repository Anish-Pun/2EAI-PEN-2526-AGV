# Tests

Deze map bevat test- en experimentele projecten die gebruikt zijn tijdens de ontwikkeling van het AGV-systeem.

##  Overzicht

```
tests/
├── AutoKit/              # Volledige integratietest van alle subsystemen (PSoC 6)
├── bleuthooth_test/      # BLE Hello Sensor voorbeeld (Bluetooth LE validatie)
├── JoyCon/               # Joy-Con HID-communicatie test (PSoC 6)
└── test_motor/           # Arduino schets voor MD22 motor driver test
```

---

##  Mapbeschrijvingen

### AutoKit
ModusToolbox-project (`CarApp`) voor de Mini RC Car. Test de volledige integratie van alle subsystemen:
- Bluetooth-communicatie (`app_bt_utils`)
- Motorsturing (`motor`, `speed`)
- IMU (MPU6050) uitlezing (`imu`)
- Ultrasone sensoren (`ultrasonic`)
- Self-driving logica (`self_driving`)
- Remote control (`remote_control`, `button`)

### bleuthooth_test
Standaard **Bluetooth LE Hello Sensor** voorbeeld van Infineon/ModusToolbox. Gebruikt om de BLE-functionaliteit van de PSoC 6 te valideren als basis voor de Joy-Con Bluetooth-communicatie.

### JoyCon
ModusToolbox-project (`JoyConApp`) specifiek gericht op het testen van de **Joy-Con controller** via Bluetooth HID:
- HID-classic pairing en datacommunicatie (`joycon_classic`)
- Drive logic vertaling van Joy-con input naar motorcommando's (`joycon_drive_logic`)
- Status LED indicatie (`joycon_status_led`)
- Motorsturing (`motor`)

### test_motor
Eenvoudige **Arduino-schets** (`test_motor.ino`) voor het testen van de MD22 motor driver via analoge PWM-signalen. Test volgende bewegingen:
- Vooruit / Achteruit
- Links / Rechts draaien
- Stoppen
