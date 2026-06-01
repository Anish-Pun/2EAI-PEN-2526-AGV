# RC Car – Firmware (`Rc_Project`)

**Platform:** CY8CKIT-062-WIFI-BT (PSoC 6)  
**Author:** Anish  
**Part of:** AGV Project — 2EAI-PEN-2526 — PXL Hogeschool

---

## Overview

Firmware for the 4-wheel RC Car, built on a PSoC 6 microcontroller. The car supports two operating modes:

- **Manual Mode** — wireless control via a Nintendo Joy-Con over classic Bluetooth
- **Self-Driving Mode** — autonomous obstacle avoidance using ultrasonic sensors, with IMU-based safety detection

Built with ModusToolbox, using FreeRTOS for concurrent task management.

---

## Project Structure

```text
Rc_Project/
└── Rc_APP/
    ├── Makefile
    ├── design.cybt
    └── source/
        ├── main.c                        # Entry point, board detection
        ├── common/
        │   ├── motor.c / motor.h         # PWM motor control (4 wheels)
        │   ├── pins.h                    # All GPIO pin definitions
        │   └── app_bt_utils.c / .h       # Bluetooth utility helpers
        └── rc_car/
            ├── rc_car_logic.c / .h       # Mode switching, FreeRTOS task
            ├── joycon/
            │   ├── joycon_classic.c / .h        # BT HID connection to Joy-Con
            │   ├── joycon_drive_logic.c / .h    # Hat input → motor commands
            │   └── joycon_status_led.c / .h     # LED status indicator
            └── selfdriving/
                ├── self_driving.c / .h          # Autonomous navigation logic
                ├── ultrasonic.c / .h            # HC-SR04 distance sensing
                ├── imu.c / .h                   # MPU6050 accelerometer/gyro
                └── speed.c / .h                 # Wheel encoder speed tracking
```

---

## Operating Modes

### Manual Mode (Bluetooth / Joy-Con)

The firmware scans for a Joy-Con by name (`Joy-Con (L)` or `Joy-Con (R)`) over classic Bluetooth (BR/EDR HID). Once connected, the D-pad controls the car:

| Hat Input | Action |
|-----------|--------|
| UP | Forward |
| LEFT / UP-LEFT | Turn Left |
| RIGHT / UP-RIGHT | Turn Right |
| CENTER / other | Stop |

**Button mappings:**

| Button (btn0 bitmask) | Action |
|-----------------------|--------|
| `0x08` | Switch to Self-Driving mode |
| `0x04` | Switch back to Manual mode |
| `0x02` | Restart self-driving if halted |

**Status LED (P13_7):**

| State | LED |
|--------|-----|
| Searching / Connecting | Blinking |
| Connected + input received | Solid ON |

---

### Self-Driving Mode

A rule-based autonomous controller that runs every 50 ms and reacts to sensor input in real time.

| Sensor Reading | Action |
|----------------|--------|
| Both sensors < 18 cm | Stop (full halt) |
| Right sensor < 25 cm | Turn left (away from right obstacle) |
| Left sensor < 25 cm | Turn right (away from left obstacle) |
| Both clear | Drive forward at 80% PWM |

**Safety features:**

- **IMU Lift Detection** — the MPU6050 detects anomalous acceleration when the car is picked up or crashes. After 6 consecutive anomaly cycles the car halts and waits for a manual restart.
- **Startup Ignore Window** — the first 20 step-cycles after entering self-driving mode are ignored to allow the motors to spin up without triggering false positives.
- **Baseline Calibration** — 16 IMU samples are averaged at startup to establish a resting baseline for anomaly detection.

---

## Pin Assignment

| Peripheral | Signal | Pin |
|------------|---------|-----|
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
| IMU SDA | I²C Data | P6_1 |
| IMU SCL | I²C Clock | P6_0 |
| IMU Interrupt | GPIO IRQ | P10_5 |
| Encoder Right | GPIO IRQ | P9_6 |
| Encoder Left | GPIO IRQ | P9_7 |
| Start/Stop (SW2) | GPIO Input | P0_4 |
| Status LED | GPIO Output | P13_7 |

> **Note:** The HC-SR04 ECHO pins output 5V. A voltage divider (1 kΩ / 2 kΩ) is required on each ECHO line to bring the signal down to 3.3V before connecting to the PSoC GPIO.

---

## Firmware Architecture

The firmware starts in `main.c`, detects the board at compile time, and calls `rc_car_logic_init()` followed by `rc_car_logic_run()`.

`rc_car_logic_run()` creates a single FreeRTOS task (`app_task`, priority 5, stack 4096) that runs the main loop:

```text
app_task
│
├── motor_init()
├── joycon_status_led_init()
├── joycon_classic_init()
│
└── loop:
    ├── joycon_classic_process()
    ├── joycon_status_led_process()
    └── if SELF_DRIVING:
        └── self_driving_step()
```

Mode switching is handled by `rc_car_logic_set_mode()`. Switching to self-driving initializes the ultrasonic, IMU, and encoder subsystems once and calls `self_driving_enter()`.

---

## Bluetooth Stack

Built on the WICED BT stack (`wiced_bt_*` APIs). Connection flow:

```text
BT enabled
  → Inquiry (scan for Joy-Con by name or HID UUID)
  → SDP (service discovery)
  → Security (bonding + BR/EDR encryption)
  → L2CAP HID channels (PSM 0x0011 control, PSM 0x0013 interrupt)
  → Subcommand 0x03 sent (enable standard full input report mode)
  → Input reports received → drive logic applied
```

The board advertises as `PSoC6-JoyConHost`. A keepalive subcommand is sent every 100 ticks to maintain the HID connection.

---

## Sensor Details

### HC-SR04 Ultrasonic (×2)

- 3 readings per cycle with 20 ms between each
- Median of the 3 values is used to filter out spikes
- Returns `1000.0 cm` if all readings time out (no obstacle in range)
- Thresholds: 25 cm triggers a turn, 18 cm triggers a full stop

### MPU6050 IMU

- I²C at 400 kHz on P6_0 / P6_1
- `WHO_AM_I` register verified on init (expected: `0x68`)
- Motion interrupt: threshold = 80, duration = 10
- Calibration offsets applied at read time:
  - `ax -= -950`, `ay -= -300`, `az -= 0`
  - `gx -= 480`, `gy -= 170`, `gz -= 210`
- Low-pass filter (α = 0.20) applied to accelerometer data during self-driving

### Speed Encoders

- Hall-effect encoders on P9_6 (right) and P9_7 (left)
- Rising-edge interrupt increments a counter per wheel
- Speed formula: `(count / COUNTS_PER_METER) × 3.6` → km/h
- Counters reset on each `self_driving_enter()` call

---

## Related Documentation

- [Main Project README](../../README.md)
- [Joy-Con Subsystem](../../docs/en/joycon_en.md)
- [Self-Driving Subsystem](../../docs/en/self_driving_en.md)
- [Bluetooth Explained](../../docs/en/Bluetooth_explained_en.md)