# Epaper_Clock_display

A silent, light-free bedside clock built with:

- **STM32F103C** (“Blue Pill”)
- **ESP-01** (Wi‑Fi)
- **3.52" e-paper HAT (B)**
- **BME280** (temperature, humidity, pressure sensor)

No display backlight, no ticking noise – ideal to keep next to your bed without disturbing sleep.

---

## Features

- **Always-readable e-paper display**
  - Time and date
  - Temperature, humidity (BME280)
- **Low light pollution**
  - E-paper only refreshes when needed
  - No LEDs or backlight by default
- **Wi‑Fi connectivity (ESP‑01)**
  - Intended for time sync
---

## Project Structure

```text
Epaper_Clock_stm32/
├─ 3D_print/                 # Enclosure and mechanical parts
├─ Electronics and schematics/  # Schematics, PCB files, wiring diagrams
└─ Stm32 Code/               # STM32 firmware (CubeIDE project)
```

- **`3D_print/`**  
  Contains all files related to the case/enclosure:
  - STL files for 3D printing

- **`Electronics and schematics/`**  
  Contains all electronics documentation:
  - Schematics
  - gerber

- **`Stm32 Code/`**  
  STM32CubeIDE project implementing:
  - Display update logic
  - Sensor reading (BME280 via I2C)
  - Interface with ESP‑01 (UART)
  - Timer-based refresh (TIM3, 1‑minute interrupt)  
---

## Hardware Overview

- **MCU**: STM32F103C
- **Display**: 3.52" e-paper HAT (B)
- **Wi‑Fi module**: ESP‑01
- **Sensor**: BME280
- **Power**: 5 V input (e.g. USB), with on-board regulation to 3.3 V for logic

---

## Getting Started

1. **Print the enclosure**  
   - Open `3D_print/` and print the parts

2. **Build the electronics**  
   - Use the schematics and wiring diagrams from `Electronics and schematics/`.

3. **Flash the STM32 firmware**
   - Open the project in **STM32CubeIDE** from the `Stm32 Code/` folder.
   - Check that the pin configuration matches your STM32 variant and board.
   - Build and flash using an ST‑Link.

4. **Assemble the clock**
   - Mount the boards and e‑paper into the printed enclosure.
   - Route cables and ensure no strain on connectors.
   - Close the enclosure.

---

## Future Improvements / Ideas

- Meteo information
- Battery backup
