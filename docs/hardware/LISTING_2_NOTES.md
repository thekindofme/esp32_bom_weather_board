# Listing 2 Notes (ESP32-2432S028 / HW-458 style)

Source context:
- Extracted from a second product listing provided in-chat on 2026-03-01.
- Includes marketing text plus 4 images (spec tables, pin map, board dimensions).

Confidence legend:
- High: seen in image pin map and/or consistent with working board behavior.
- Medium: appears in listing text but not fully cross-verified on device.
- Low: generic marketing claim.

## High-confidence hardware details

- Board marking shown: `ESP32-2432S028` and `HW-458`.
- MCU module family: `ESP-WROOM-32`.
- Display: `2.8"`, `240x320`, SPI TFT.
- Display config verified working in this repo:
  - Driver: `ST7789`
  - Color order: `TFT_BGR`
  - Pins:
    - `TFT_MOSI=13`
    - `TFT_MISO=12`
    - `TFT_SCLK=14`
    - `TFT_CS=15`
    - `TFT_DC=2`
    - `TFT_RST=-1`
    - `TFT_BL=21`
  - Also needed in code: `tft.invertDisplay(false)`
  - Also needed in build flags for text: `LOAD_GLCD`, `LOAD_FONT2`, `LOAD_FONT4`

## Pin map details inferred from image (candidate map)

Touch section (strongly resembles XPT2046 SPI touch wiring):
- `TP CLK: IO25`
- `TP CS: IO33`
- `TP DIN: IO32`
- `TP OUT: IO39`
- `TP IRQ: IO36`

Other labeled functions:
- `AUDIO: IO26`
- `Screen backlight: LED` (runtime BL pin in working TFT config is `IO21`)
- TF/microSD:
  - `CD/DAT3: IO5`
  - `CMD: IO23`
  - `CLK: IO18`
  - `DAT0: IO19`
- RGB indicator:
  - `IO17`, `IO4`, `IO16`

Expansion headers shown:
- `Expand IO1` (1p 1.25mm): `GND`, `IO35 (input only)`, `IO22 (I2C SCL)`, `IO21 (I2C SDA)`
- `Expand IO2` (4p 1.25mm): `GND`, `IO22 (I2C SCL)`, `IO27 (TOUCH7/ADC2_CH7)`, `3.3V`
- UART header: `VIN`, `TX`, `RX`, `GND`

## Mechanical data from listing image

- Overall dimensions shown: approximately `85.85mm x 49.40mm`
- Inner length marker shown: `78.00mm`
- Vertical hole spacing marker shown: `42.00mm`
- Mounting hole diameter shown: `3.4mm`

## Listing spec snippets worth retaining (medium confidence)

- SPI flash: default `32Mbit` (4MB) (matches direct probe).
- Bluetooth: `4.2 BR/EDR + BLE`.
- Wi-Fi: `802.11 b/g/n`.
- Power:
  - USB supply `5V`
  - VIN supply `6.0V ~ 12V` (use cautiously; verify regulator thermals)
- Serial rate default: `115200`.
- Operating temperature claimed: `-20C ~ +80C`.

## Notes on reliability of listing text

- Many claims are generic ESP32 marketing statements.
- Treat touch-controller and extra-I/O claims as "candidate until tested".
- For firmware defaults, prioritize:
  1) direct hardware probe,
  2) known-good runtime behavior,
  3) then listing text.

