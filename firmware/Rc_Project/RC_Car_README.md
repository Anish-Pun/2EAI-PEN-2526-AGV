# RC Car – Firmware Documentation (`Rc_Project`)

> **Platform:** CY8CKIT-062-WIFI-BT (PSoC 6)
> **Author:** Anish
> **Part of:** AGV Project — 2EAI-PEN-2526 — PXL Hogeschool

---

## 📖 Overview

This is the firmware for the **4-wheel RC Car** — my part of the AGV project. The car runs on a **PSoC 6 microcontroller** and supports two operating modes:

- **Manual (Bluetooth) Mode** — controlled wirelessly via a Nintendo Joy-Con controller
- **Self-Driving Mode** — autonomous obstacle avoidance using ultrasonic sensors, with IMU-based safety detection

The firmware is built with **ModusToolbox** and uses **FreeRTOS** to manage tasks concurrently.

---

## 📁 Project Structure

```
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
            │   ├── joycon_classic.c / .h         # BT HID connection to Joy-Con
            │   ├── joycon_drive_logic.c / .h     # Hat input → motor commands
            │   └── joycon_status_led.c / .h      # LED status indicator
            └── selfdriving/
                ├── self_driving.c / .h           # Autonomous navigation logic
                ├── ultrasonic.c / .h             # HC-SR04 distance sensing
                ├── imu.c / .h                    # MPU6050 accelerometer/gyro
                └── speed.c / .h                  # Wheel encoder speed tracking
```

---

## ⚙️ Operating Modes

### 🎮 Manual Mode (Bluetooth / Joy-Con)

The car connects to a **Nintendo Joy-Con** over classic Bluetooth (BR/EDR HID).

- The firmware scans for a Joy-Con by name (`Joy-Con (L)` or `Joy-Con (R)`)
- Once connected, the **D-pad (hat switch)** drives the car:

| Hat Input | Action |
|---|---|
| UP | Forward |
| LEFT / UP-LEFT | Turn Left |
| RIGHT / UP-RIGHT | Turn Right |
| CENTER / other | Stop |

**Button mappings:**

| Button (btn0 bitmask) | Action |
|---|---|
| `0x08` | Switch to **Self-Driving** mode |
| `0x04` | Switch back to **Manual** mode |
| `0x02` (X) | Restart self-driving if halted |

**Status LED (P13_7):**

| State | LED |
|---|---|
| Searching / Connecting | Blinking |
| Connected + input received | Solid ON |

---

### 🤖 Self-Driving Mode

An autonomous rule-based controller that avoids obstacles in real time.

**Logic (runs every 50 ms):**

| Sensor Reading | Action |
|---|---|
| Both sensors < 18 cm | **STOP** (full halt) |
| Right sensor < 25 cm | Turn right (avoid left obstacle) |
| Left sensor < 25 cm | Turn left (avoid right obstacle) |
| Both clear | Drive forward at 80% speed |

**Safety features:**

- **IMU Lift Detection** — if the car is picked up or crashes, the MPU6050 detects anomalous acceleration. After 6 consecutive anomaly cycles, the car stops and waits for the X button to restart.
- **Startup Ignore Window** — the first 20 self-driving cycles ignore IMU readings to avoid false triggers on launch.
- **Baseline Calibration** — when self-driving starts, 16 IMU samples are averaged to establish a rest baseline.

---

## 🔌 Pin Assignment (CY8CKIT-062-WIFI-BT)

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
| IMU SDA | I²C Data | P6_1 |
| IMU SCL | I²C Clock | P6_0 |
| IMU Interrupt | GPIO IRQ | P10_5 |
| Encoder Right | GPIO IRQ | P9_6 |
| Encoder Left | GPIO IRQ | P9_7 |
| Start/Stop (SW2) | GPIO Input | P0_4 |
| Status LED | GPIO Output | P13_7 |

---

## 🧠 Firmware Architecture

The firmware starts in `main.c`, detects the board type at compile time, and calls `rc_car_logic_init()` + `rc_car_logic_run()`.

`rc_car_logic_run()` creates a **FreeRTOS task** (`app_task`) that runs the main loop:

```
app_task (FreeRTOS, priority 5, stack 4096)
│
├── motor_init()                 // Initialize 4x PWM channels
├── joycon_status_led_init()     // LED setup
├── joycon_classic_init()        // Start Bluetooth stack (WICED)
│
└── loop:
    ├── joycon_classic_process() // BT keepalive + input polling
    ├── joycon_status_led_process() // Blink LED while connecting
    └── if SELF_DRIVING:
        └── self_driving_step()  // Sensor read + motor decision
```

**Mode switching** is handled by `rc_car_logic_set_mode()`. Switching to self-driving initializes the ultrasonic, IMU, and encoder subsystems (only once) and calls `self_driving_enter()`.

---

## 📡 Bluetooth Stack Details

The Bluetooth layer is built on the **WICED BT stack** (`wiced_bt_*` APIs). The full connection flow is:

```
BT enabled
  → Inquiry (scan for Joy-Con by name or HID UUID)
  → SDP (service discovery)
  → Security (bonding + BR/EDR encryption)
  → L2CAP HID channels (PSM 0x0011 control, PSM 0x0013 interrupt)
  → Subcommand 0x03 sent (enable standard full input report mode)
  → Input reports received → drive logic applied
```

The device advertises as `PSoC6-JoyConHost`. A keepalive subcommand is sent every 100 ticks to maintain the HID connection.

---

## 📏 Sensor Details

### HC-SR04 Ultrasonic (×2)

- 3 readings taken per measurement cycle with 20 ms between each
- **Median** of the 3 readings is used (removes spikes)
- Returns `1000.0 cm` if all readings time out (no obstacle detected)
- Detection threshold: **25 cm** (turn), **18 cm** (stop)

### MPU6050 IMU

- Connected via I²C at 400 kHz on pins P6_0/P6_1
- `WHO_AM_I` register checked on init (expected: `0x68`)
- Motion interrupt enabled with threshold = 80 and duration = 10
- Calibration offsets applied at read time:
  - `ax -= -950`, `ay -= -300`, `az -= 0`
  - `gx -= 480`, `gy -= 170`, `gz -= 210`
- A low-pass filter (α = 0.20) smooths accelerometer data in self-driving

### Speed Encoders

- Two hall-effect encoders on P9_6 (right) and P9_7 (left)
- Rising-edge interrupt increments a counter per wheel
- Speed calculated as: `(count / COUNTS_PER_METER) × 3.6` → km/h
- Counters reset on each `self_driving_enter()` call
---

## 🔗 Related Documentation

- [Main Project README](../../README.md)
- [Joy-Con Subsystem (EN)](../../docs/en/joycon_en.md)
- [Self-Driving Subsystem (EN)](../../docs/en/self_driving_en.md)
- [Bluetooth Explained (EN)](../../docs/en/Bluetooth_explained_en.md)
