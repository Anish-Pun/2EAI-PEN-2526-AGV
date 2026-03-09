# Hardware

> Deze map bevat alle hardware-gerelateerde bestanden voor het project

---

## 📂 Structuur

```
hardware/
├── README.md              
├── RC_CarT/ 
│   ├── MD22_board.png        
│   ├── MD22_config.png
│   └── README.md
└── RC_MiniCar/                  
    └── README.md
         
```

---

## 🧩 Componentenlijst

> **Update deze lijst met alle hardware componenten die jullie gebruiken**

| Component | Type/Model | Functie | Datasheet | Aantal | Autotje |
|-----------|-----------|---------|-----------|--------|---------|
| Microcontroller | PSoC 6 | Hoofdcontroller | [Link](/docs/datasheets/PSoC6.pdf)| 1 | alle twee |
| PS4 controller | Dualshock | Controller | Geen datasheet [Link]()| 1 | alle twee |
| Sensor | [Type] | [Functie] | [Link](/docs/datasheets/) | 1 | |
| UltraSensor | [HC-SR04] | Sensor | [Link](/docs/datasheets/HCSR04.pdf) | 2 | Mini RC Car |
| Line Tracker | [KY-033] | Sensor | [Link](/docs/datasheets/p587610-KY-033-Joy-IT.pdf) | 2 | Mini RC Car |
| DC Motor | [MY1018] | Aansturing | Geen datasheet [Link](/docs/datasheets/) | 2 | RC CarT |
| DC Motor | [Type] | Aansturing | Geen datasheet [Link](/docs/datasheets/) | 4 | Mini RC Car |
| DC Converter | [Type] | Spanning omvormen | geen datasheet [Link](/docs/datasheets/) | 1 | RC CarT |
| Motor Driver | [MD22] | Aansturing | [Link](/docs/datasheets/MD22_TechnicalDocumentation.pdf) | 1 | RC CarT |
| Motor Driver | [DRV8843] | Aansturing | [Link](/docs/datasheets/drv8843.pdf) | 2 | Mini RC Car |
| ... | ... | ... | ... | ... | ... |

**Vul deze tabel in met:**
- Exacte type nummers
- Wat elke component doet
- Link naar datasheet (bewaar lokaal in `/docs/datasheets/`)
- Aantal dat je nodig hebt

---

## 🔌 Blokdiagram / Schema

**Schematisch overzicht:**

> **Voeg hier een afbeelding of diagram toe waarin de bedrading en verbindingen van de componenten worden weergegeven.**
>
> - Kijken bij [AGV_Schema.png](/docs/images/AGV_Schema.png)
> - Later: Fritzing of KiCAD schematic export (PNG of PDF) 

---

## ⚡ Specificaties

### Mini RC Car

#### Voeding
- **Spanning:** [bijv. 5V DC via USB]
- **Stroomverbruik:** [bijv. max 500mA]
- **Voedingsbron:** [bijv. USB powerbank]

#### Communicatie
- **Protocollen:** [bijv. UART, I²C, SPI]
- **Pinnen:** [lijst belangrijke pinnen]
- **Logica level:** [bijv. 3.3V]

###  RC CarT

#### Voeding
- **Spanning:** [bijv. 5V DC via USB]
- **Stroomverbruik:** [bijv. max 500mA]
- **Voedingsbron:** [bijv. USB powerbank]

#### Communicatie
- **Protocollen:** [bijv. UART, I²C, SPI]
- **Pinnen:** [lijst belangrijke pinnen]
- **Logica level:** [bijv. 3.3V]
---

## 🔧 PCB Ontwerp (optioneel)

**Alleen invullen als je een PCB ontwerpt**

- **KiCAD versie:** [bijv. 7.0]
- **Aantal lagen:** [bijv. 2-layer]
- **Afmetingen:** [bijv. 100x80mm]
- **Producent:** [bijv. JLCPCB]

---

## 📝 Aantekeningen

**Belangrijke opmerkingen:**
- ...
- ...

**Bekende problemen:**
- ...
- ...

---

## ✅ Checklist

- [ ] Componentenlijst volledig ingevuld (Bijna volledig)
- [X] Schema/blokdiagram toegevoegd
- [ ] Datasheets lokaal opgeslagen in `/docs/datasheets/`
- [X] Specificaties gedocumenteerd
