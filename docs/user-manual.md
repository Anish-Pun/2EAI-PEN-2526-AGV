# Gebruikershandleiding — AGV Project (2EAI-PEN-2526)

**Project:** AGV — 2EAI-PEN-2526 — PXL Hogeschool  
**Team:** Anish (RC Car) · Kaelig (RC Cart) · Joel (RC Car 2)

---

## 1. Overzicht van de voertuigen

| Voertuig | Bestuurder | Besturing | Modi |
|----------|-----------|-----------|------|
| **RC Car** | Anish | Nintendo Joy-Con (Bluetooth) | Manueel + Self-Driving (ultrasoon) |
| **RC Cart** | Kaelig | Nintendo Joy-Con (Bluetooth) | Manueel |
| **RC Car 2** | Joel | Geen controller | Automatisch (lijntracking) |

---

## 2. Aanzetten

### RC Cars & RC Cart

1. Sluit een USB-powerbank aan op het board.
2. De status-LED begint te **knipperen** — de auto zoekt naar een Joy-Con.
3. Zet de auto op een vlakke ondergrond met voldoende ruimte rondom.

---

## 3. Controller verbinden (RC Car & RC Cart)

Beide voertuigen gebruiken een Nintendo Joy-Con via Bluetooth. De verbindingsprocedure is identiek.

**Eerste keer koppelen:**
1. Houd de **SYNC-knop** op de zijkant van de Joy-Con ingedrukt tot de LED knippert.
2. Het voertuig scant automatisch en maakt verbinding.
3. De status-LED stopt met knipperen en **blijft branden**.

**Opnieuw verbinden (al eerder gekoppeld):**
1. Druk op een willekeurige knop op de Joy-Con om hem te wekken.
2. Het voertuig herkent hem via opgeslagen koppelingsgegevens en verbindt automatisch.

Als de verbinding verbroken wordt, zoekt het voertuig automatisch opnieuw.

---

## 4. Besturing

### 4.1 RC Car & RC Cart — Manuele modus

Standaard modus bij opstarten. Bestuur via de **D-pad** van de Joy-Con:

| D-pad | Actie |
|-------|-------|
| Omhoog ↑ | Vooruit |
| Links ← | Draai links |
| Rechts → | Draai rechts |
| Center / loslaten | Stoppen |

**Modi wisselen (alleen RC Car):**

| Knop | Actie |
|------|-------|
| Y | Schakel naar Self-Driving |
| B | Schakel terug naar Manueel |
| X | Herstart Self-Driving na stilstand |

> **RC Cart:** heeft alleen manuele modus. De Y/B/X knoppen hebben geen functie op de RC Cart.

---

## 5. Automatische modi

### 5.1 RC Car — Self-Driving (Anish)

Druk op **Y** om Self-Driving te activeren. De auto:
1. Kalibreeert de IMU (~0.3 seconden).
2. Rijdt automatisch vooruit en ontwijkt obstakels.

| Situatie | Reactie |
|----------|---------|
| Pad vrij | Vooruit op 80% snelheid |
| Rechter sensor < 25 cm | Draai links |
| Linker sensor < 25 cm | Draai rechts |
| Beide sensoren < 18 cm | Volledige stop — halted state |

**Noodstop (halted state):**  
De auto stopt ook automatisch als de IMU een botsing of optillen detecteert. Druk op **X** om te herstarten, of **B** om terug te schakelen naar manueel.

### 5.2 RC Car 2 — Lijntracking (Joel)

> **Te documenteren door Joel.**

---

## 6. LED-indicatie

### RC Car & RC Cart

| LED | Betekenis |
|-----|-----------|
| Knippert | Zoekt naar Joy-Con |
| Brandt | Joy-Con verbonden en actief |
| Uit | Auto uit of niet verbonden |

---

## 7. Problemen oplossen

### RC Car & RC Cart

| Probleem | Oplossing |
|----------|-----------|
| LED knippert, geen verbinding | Druk op SYNC-knop van de Joy-Con |
| Auto reageert niet op D-pad | Controleer of LED brandt, herstart Joy-Con |
| Geen reactie meer | USB-kabel loskoppelen, 5 seconden wachten, opnieuw aansluiten |

### Alleen RC Car

| Probleem | Oplossing |
|----------|-----------|
| Self-driving staat stil | Druk **X** om te herstarten of **B** voor manueel |
| Auto stopt bij optillen | Normaal gedrag — zet neer en druk **X** |

### RC Car 2 (Joel)

> **Te documenteren door Joel.**

---

## 8. Gerelateerde documentatie

- [Hoofd-README](../README.md)
- (momenteen staat er niet bij )[RC Car Firmware (Anish)](../firmware/Rc_Project/RC_Car_README.md)
- [RC Cart Firmware (Kaelig)](../firmware/RC_Cart/README.md)
-  (momenteen staat er niet bij )[RC Car 2 Firmware (Joel)](../firmware/Rc_Project2/README.md)
- [Hardware Overzicht](../hardware/README.md)
- [Joy-Con Subsysteem](en/joycon_en.md)
- [Self-Driving Subsysteem](en/self_driving_en.md)
- [Bluetooth Uitleg](en/Bluetooth_explained_en.md)