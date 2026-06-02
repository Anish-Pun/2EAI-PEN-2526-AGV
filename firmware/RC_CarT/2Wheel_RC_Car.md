# 2 Wheel RC Car – Firmware (`Rc_CarT`)

**Platform:** CY8CPROTO-062-4343W (PSoC 6)  
**Author:** Kaelig  
**Part of:** AGV Project — 2EAI-PEN-2526 — PXL Hogeschool

---

## Overview

Firmware voor de RC Cart / 2-wiel RC Car, ontwikkeld op een Infineon PSoC 6 microcontroller. Het voertuig ondersteunt een specifieke bedrijfsmodus:

- **Handmatige Modus** — Draadloze real-time besturing via een Nintendo Joy-Con controller over klassieke Bluetooth (BR/EDR).

De firmware is gebouwd met ModusToolbox™ en maakt gebruik van FreeRTOS voor gelijktijdig taakbeheer, deterministische timing en optimale resourceplanning.

---

## Project Structure

```text
Rc_CarT/
├── mtb_shared                         # Gedeelde Infineon/ModusToolbox bibliotheken
└── Bluetooth_RC_Cart/
    ├── Makefile
    ├── .clangd
    ├── .cyignore
    ├── .gitignore
    ├── LICENSE
    ├── deps/
    │   ├── assetlocks.json                        
    │   ├── btstack-integration.mtb        
    │   ├── btstack.mtb               
    │   └── retarget-io.mtb    
    │
    └── source/
        ├── main.c                  # Entry point en hardware peripheral initialisatie
        ├── motor.c / motor.h       # PWM motorbesturing configuratie (4-wiel / H-brug)
        ├── pins.h                  # Gecentraliseerde GPIO pin-definities
        ├── app_bt_utils.c / .h     # Bluetooth stack hulpmiddelen
        ├── joycon_classic.c / .h   # Bluetooth HID connectielaag voor Joy-Con
        ├── joycon_drive_logic.c / .h # Input vertaalstatus (Hat-input → motorcommando's)
        └── joycon_status_led.c / .h  # LED status- en animatiebesturing
```

---

## Operating Modes

### Handmatige Modus (Bluetooth / Joy-Con)

Bij het opstarten scant de firmware continu naar een beschikbare Joy-Con controller die overeenkomt met de apparaatnaam `Joy-Con (L)` of `Joy-Con (R)` via klassieke Bluetooth (BR/EDR HID). Zodra de verbindingslaag is hersteld, wordt de D-pad (Hat-input) direct gekoppeld aan de bewegingsvectoren van het voertuig:

| Hat Input | Action |
|-----------|--------|
| UP | Forward |
| BACK  |  Backward  | 
| LEFT  | Turn Left |
| RIGHT  | Turn Right |
| CENTER | Nothing |


**Status LED (P13_7):**

| State | LED |
|--------|-----|
| Zoeken / Verbinden | Knipperend |
| Verbonden + actieve input ontvange | Continu AAN |

---

## Pinbezetting

### PSoC6 CY8CPROTO-062-4343W Pin Mapping
| Perifeer Subsysteem | Signaal | PSoC6 Pin | Richting (I/O) |
| :--- | :--- | :--- | :--- |
| **Draadloos (BT)** | Ingebouwde Antenne | **Intern (4343W)** | I/O |
| **Power (2-Wheel)** | L298N 5V Out | **J8 (Micro USB)** | Voeding Input |
| **Power (Cart)** | SBC-BUCK01 Out | **VBACK / GND** | Voeding Input |
| **UART (Debug/Log)** | TX / RX | **P5_1 / P5_0** | I/O | 
| **Aandrijving (2-Wheel)**| L298N EN A / EN B | **P9_6  / P9_7** | Output |
| **Aandrijving (2-Wheel)**| L298N IN1 t/m IN4 | **P9_0, P9_1, P9_2, P9_4**| Output |
| **Aandrijving (Cart)** | Sabertooth M1 PWM | **N.v.t.** | Output |
| **Aandrijving (Cart)** | Sabertooth M2 PWM | **N.v.t.** | Output |
| **User Interface** | Status LED | **P13_7** | Output |

> **Note:** The SBC-BUCK01 and the Sabertooth are for the RC CarT not for the 2 Wheel RC car.

---

### Firmware Architectuur (`main.c`)

De firmware start in `main.c`, waar de initiële hardware- en board-peripherals via de Infineon BSP (Board Support Package) en HAL-functies worden geïnitialiseerd. 

Na een succesvolle hardware-initialisatie en het inschakelen van de globale interrupts, start de applicatie de FreeRTOS-scheduler. De scheduler activeert direct de centrale applicatietaak:

* **Tasknaam:** `JoyCon Task` (gekoppeld aan de functie `app_task`)
* **Prioriteit:** `5` (`APP_TASK_PRIORITY`)
* **Stackgrootte:** `4096` woorden (`APP_TASK_STACK_SIZE`)

#### Operationele Loop (`app_task`)
Binnen deze FreeRTOS-taak worden de hardware-specifieke subsystemen sequentieel geïnitialiseerd en cyclisch afgehandeld:

1. **Initialisatiefase:** De subsystemen voor de motoraansturing (`motor_init`), Joy-Con communicatie (`joycon_classic_init`) en de status-leds (`joycon_status_led_init`) worden geconfigureerd. Uit veiligheidsoverwegingen start de motor direct in een gedwongen stopstatus (`motor_stop`) en wordt de led-indicator op de status 'connecting' gezet.
2. **Main Loop (Super-Loop):** Na de initialisatie treedt de taak in een oneindige loop waarin continu de Joy-Con input (`joycon_classic_process`) en de led-animaties (`joycon_status_led_process`) cyclisch worden verwerkt.
3. **Timing & Resourcebeheer:** Om processorbronnen efficiënt vrij te geven voor eventuele lagere prioriteitstaken en de systeemstabiliteit te waarborgen, is er een deterministische vertraging van 10 milliseconden (`vTaskDelay(pdMS_TO_TICKS(10))`) in de loop geïntegreerd.

#### Implementatie `app_task`
De bovenstaande logica is als volgt geïmplementeerd in de broncode:

```c
static void app_task(void *arg)
{
    (void)arg;

    // Hardware- en subsysteeminitialisatie
    motor_init();
    motor_stop();
    
    joycon_classic_init();
    joycon_status_led_init();
    joycon_status_led_set_connecting();
    
    // Oneindige applicatieloop
    for (;;)
    {
        joycon_classic_process();
        joycon_status_led_process();
        
        // Vrijgeven van CPU-tijd (10 ms)
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```



---

## Bluetooth Stack

De draadloze sub-architectuur is gebouwd bovenop de WICED® BT stack (`wiced_bt_*` API's). De interne verbindingsstroom verloopt sequentieel als volgt:

```text
[BT Enabled]
       │
       ▼
[Inquiry Scan] ────────► Scans for active Joy-Con peripherals by name or HID UUID
       │
       ▼
[SDP Discovery] ───────► Service Discovery Protocol fetches HID descriptors
       │
       ▼
[Security Layer] ──────► Executes pairing, secure bonding, and BR/EDR encryption
       │
       ▼
[L2CAP Connection] ────► Establishes HID Control (PSM 0x0011) & Interrupt (PSM 0x0013) channels
       │
       ▼
[HID Configuration] ───► Transmits Subcommand 0x03 to enable standard full input report mode
       │
       ▼
[Data Processing] ─────► Inbound input reports are parsed and parsed to the drive logic
```

Het embedded systeem adverteert zijn hostprofiel lokaal als `PSoC6-JoyConHost`. Om te voorkomen dat de verbinding wordt verbroken door inactiviteit, wordt er elke 100 systeemticks een keepalive-subcommando naar de controller verzonden.

---

## Related Documentation

- [Main Project README](../../README.md)
- [Joy-Con Subsystem](../../docs/en/joycon.md)
- [Bluetooth Explained](../../docs/en/Bluetooth_explained.md)