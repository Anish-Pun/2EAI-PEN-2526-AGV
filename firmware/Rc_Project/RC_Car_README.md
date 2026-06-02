# RC Car – Firmware (`Rc_Project`)

**Platform:** CY8CKIT-062-WIFI-BT (PSoC 6)  
**Auteur:** Anish  
**Onderdeel van:** AGV Project — 2EAI-PEN-2526 — PXL Hogeschool

---

## Overzicht

Firmware voor de 4-wielige RC Car, gebouwd op een PSoC 6 microcontroller. De auto ondersteunt twee modi:

- **Manuele Modus** — draadloze besturing via een Nintendo Joy-Con over klassieke Bluetooth
- **Self-Driving Modus** — autonome obstakelvermijding met ultrasone sensoren en IMU-gebaseerde veiligheidsdetectie

Gebouwd met ModusToolbox, met FreeRTOS voor gelijktijdige taakuitvoering.

---

## Projectstructuur

```text
Rc_Project/
└── Rc_APP/
    ├── Makefile
    ├── design.cybt
    └── source/
        ├── main.c                        # Entry point, boarddetectie
        ├── common/
        │   ├── motor.c / motor.h         # PWM motorsturing (4 wielen)
        │   ├── pins.h                    # Alle GPIO pin definities
        │   └── app_bt_utils.c / .h       # Bluetooth hulpfuncties
        └── rc_car/
            ├── rc_car_logic.c / .h       # Moduswisseling, FreeRTOS taak
            ├── joycon/
            │   ├── joycon_classic.c / .h        # BT HID verbinding met Joy-Con
            │   ├── joycon_drive_logic.c / .h    # Hat input → motorcommando's
            │   └── joycon_status_led.c / .h     # LED status indicator
            └── selfdriving/
                ├── self_driving.c / .h          # Autonome navigatielogica
                ├── ultrasonic.c / .h            # HC-SR04 afstandsmeting
                ├── imu.c / .h                   # MPU6050 accelerometer/gyro
                └── speed.c / .h                 # Wielencoder snelheidsmeting
```

---

## Bedieningsmodi

### Manuele Modus (Bluetooth / Joy-Con)

De firmware zoekt naar een Joy-Con op naam (`Joy-Con (L)` of `Joy-Con (R)`) via klassieke Bluetooth (BR/EDR HID). Eenmaal verbonden bestuurt de D-pad de auto:

| Hat Input | Actie |
|-----------|-------|
| OMHOOG | Vooruit |
| LINKS / LINKS-OMHOOG | Links draaien |
| RECHTS / RECHTS-OMHOOG | Rechts draaien |
| CENTRUM / andere | Stoppen |

**Knoptoewijzingen:**

| Knop (btn0 bitmask) | Actie |
|---------------------|-------|
| `0x08` | Schakel naar Self-Driving modus |
| `0x04` | Schakel terug naar Manuele modus |
| `0x02` | Herstart self-driving na stop |

**Status LED (P13_7):**

| Toestand | LED |
|----------|-----|
| Zoeken / Verbinden | Knipperend |
| Verbonden + input ontvangen | Continu AAN |

---

### Self-Driving Modus

Een rule-based autonome regelaar die elke 50 ms draait en in real-time reageert op sensorinput.

| Sensorwaarde | Actie |
|--------------|-------|
| Beide sensoren < 18 cm | Stop (volledige halt) |
| Rechter sensor < 25 cm | Links draaien (weg van obstakel) |
| Linker sensor < 25 cm | Rechts draaien (weg van obstakel) |
| Beide vrij | Vooruit rijden op 80% PWM |

**Veiligheidsfuncties:**

- **IMU Lift Detectie** — de MPU6050 detecteert afwijkende versnelling wanneer de auto wordt opgetild of botst. Na 6 opeenvolgende afwijkende cycli stopt de auto en wacht op een handmatige herstart.
- **Opstart venster** — de eerste 20 stap-cycli na het inschakelen van self-driving worden genegeerd om de motoren te laten starten zonder vals alarm.
- **Basiskalibratie** — 16 IMU-metingen worden gemiddeld bij het opstarten om een rustbaseline vast te leggen voor anomaliedetectie.

---

## Pinbezetting

| Peripheral | Signaal | Pin |
|------------|---------|-----|
| UART TX (debug) | TX | P5_1 |
| UART RX (debug) | RX | P5_0 |
| Motor A | PWM | P10_0 |
| Motor B | PWM | P10_1 |
| Motor C | PWM | P10_2 |
| Motor D | PWM | P10_3 |
| Ultrasoon Rechts | Trigger | P9_0 |
| Ultrasoon Rechts | Echo | P9_1 |
| Ultrasoon Links | Trigger | P9_4 |
| Ultrasoon Links | Echo | P9_5 |
| IMU SDA | I²C Data | P6_1 |
| IMU SCL | I²C Klok | P6_0 |
| IMU Interrupt | GPIO IRQ | P10_5 |
| Encoder Rechts | GPIO IRQ | P9_6 |
| Encoder Links | GPIO IRQ | P9_7 |
| Start/Stop (SW2) | GPIO Ingang | P0_4 |
| Status LED | GPIO Uitgang | P13_7 |

> **Opmerking:** De HC-SR04 ECHO pinnen geven 5V uit. Een spanningsdeler (1 kΩ / 2 kΩ) is nodig op elke ECHO-lijn om het signaal naar 3.3V te brengen voor aansluiting op de PSoC GPIO.

---

## Firmware Architectuur

De firmware start in `main.c`, detecteert het bord tijdens compilatie en roept `rc_car_logic_init()` gevolgd door `rc_car_logic_run()` aan.

`rc_car_logic_run()` maakt een enkele FreeRTOS-taak (`app_task`, prioriteit 5, stack 4096) die de hoofdlus uitvoert:

```text
app_task
│
├── motor_init()
├── joycon_status_led_init()
├── joycon_classic_init()
│
└── lus:
    ├── joycon_classic_process()
    ├── joycon_status_led_process()
    └── if SELF_DRIVING:
        └── self_driving_step()
```

Moduswisseling wordt afgehandeld door `rc_car_logic_set_mode()`. Schakelen naar self-driving initialiseert de ultrasone-, IMU- en encoder-subsystemen eenmalig en roept `self_driving_enter()` aan.

---

## Bluetooth Stack

Gebouwd op de WICED BT stack (`wiced_bt_*` API's). Verbindingsverloop:

```text
BT ingeschakeld
  → Inquiry (scan naar Joy-Con op naam of HID UUID)
  → SDP (service discovery)
  → Security (pairing + BR/EDR encryptie)
  → L2CAP HID kanalen (PSM 0x0011 control, PSM 0x0013 interrupt)
  → Subcommand 0x03 verzonden (activeer standaard volledig input report)
  → Input reports ontvangen → rijlogica toegepast
```

Het bord adverteert als `PSoC6-JoyConHost`. Een keepalive subcommand wordt elke 100 ticks verzonden om de HID-verbinding te behouden.

---

## Sensordetails

### HC-SR04 Ultrasoon (×2)

- 3 metingen per cyclus met 20 ms ertussen
- Mediaan van de 3 waarden wordt gebruikt om uitschieters te filteren
- Geeft `1000.0 cm` terug als alle metingen tijdelijk uitvallen (geen obstakel binnen bereik)
- Drempels: 25 cm start een draai, 18 cm start een volledige stop

### MPU6050 IMU

- I²C op 400 kHz op P6_0 / P6_1
- `WHO_AM_I` register geverifieerd bij init (verwacht: `0x68`)
- Bewegingsinterrupt: drempel = 80, duur = 10
- Kalibratie-offsets toegepast bij uitlezing:
  - `ax -= -950`, `ay -= -300`, `az -= 0`
  - `gx -= 480`, `gy -= 170`, `gz -= 210`
- Laagdoorlaatfilter (α = 0.20) toegepast op accelerometerdata tijdens self-driving

### Snelheidsencoders

- Hall-effect encoders op P9_6 (rechts) en P9_7 (links)
- Stijgende flank interrupt verhoogt een teller per wiel
- Snelheidsformule: `(count / COUNTS_PER_METER) × 3.6` → km/h
- Tellers worden gereset bij elke `self_driving_enter()` aanroep

---

## Gerelateerde Documentatie

- [Hoofdproject README](../../README.md)
- [Joy-Con Subsysteem](../../docs/nl/joycon_nl.md)
- [Self-Driving Subsysteem](../../docs/nl/self_driving_nl.md)
- [Bluetooth Uitgelegd](../../docs/nl/Bluetooth_explained_nl.md)