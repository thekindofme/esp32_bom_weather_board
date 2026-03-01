# Hardware Specification

Last updated: 2026-03-01

## 1) Verified by direct probe on this machine

- Host OS detected USB serial bridge:
  - Device: `QinHeng Electronics CH340 serial converter`
  - USB VID:PID: `1a86:7523`
  - Serial device: `/dev/ttyUSB0`
  - Stable link: `/dev/serial/by-id/usb-1a86_USB_Serial-if00-port0`
- ESP tool probe results (`esptool v5.2.0`):
  - Chip: `ESP32-D0WD-V3` (revision `v3.1`)
  - Features: `Wi-Fi`, `Bluetooth`, dual-core, up to `240MHz`
  - Crystal: `40MHz`
  - Flash size: `4MB` (`32Mbit`)
  - Flash IDs: manufacturer `0x68`, device `0x4016`
  - Base MAC: `d4:e9:f4:ae:93:5c`

## 2) Board identity (from your board info/images)

- Family/module: `ESP32-WROOM-32` / `ESP32-2432S028` style board
- USB connector: `Micro-USB`
- Extra hardware seen in listing/images:
  - `TF` (microSD) slot
  - Reset + Boot buttons
  - Extended IO header
  - 4-pin 1.25 power/signal connector

## 3) Display and UI hardware

- Display controller IC (provided): `ST7789`
- Display class/name hint from board naming:
  - `2432S028` commonly indicates `2.8"` and `240x320`
- Touch panel:
  - Present physically (per your original note), but controller IC is **not yet confirmed**
  - Common candidates on these boards: `XPT2046`, `CST816`, `FT6236`

## 4) MCU and radio capabilities (from provided board specs)

- CPU: low-power dual-core 32-bit ESP32, up to `240MHz`
- RAM: internal `520KB SRAM`
- Flash: `32Mbit` default (matches `4MB` probe)
- Wi-Fi: `802.11 b/g/n/e/i`
- Bluetooth: `BT 4.2 BR/EDR + BLE`
- Typical interfaces: `UART`, `SPI`, `I2C`, `PWM`, `ADC`, `DAC`
- RTOS/network stack support: `FreeRTOS`, `lwIP`

## 5) Electrical and operating info (from provided board specs)

- Input voltage: `4.75V` to `5.25V`
- Default serial baud: `115200`
- Operating temperature: `-20C` to `70C`
- Storage temperature: `-40C` to `125C`

## 6) BOM weather data integration notes

- BOM web JSON endpoints are protected by anti-scraping controls.
- BOM anonymous FTP feed is reachable and suitable for automated embedded fetch:
  - Host: `ftp.bom.gov.au`
  - Example observations feed path: `/anon/gen/fwo/IDV60920.xml` (VIC)
- Station selection is done via the station `bom-id` in the XML.

## 7) Unknowns to verify before final production firmware

- Exact board variant/revision silk text (front/back)
- Confirmed display resolution (`240x320` expected, not yet measured on-device)
- Confirmed ST7789 wiring pins (`CS/DC/RST/SCLK/MOSI/BL`)
- Confirmed touch controller IC and wiring pins
- Confirmed display orientation and color order (`RGB`/`BGR`)
- Confirmed whether PSRAM exists on this exact board

## 8) Known-good display config (verified working on this unit)

The following TFT_eSPI configuration produced a working weather UI:

- Driver: `ST7789`
- Color order: `TFT_BGR`
- SPI pins:
  - `TFT_MOSI = 13`
  - `TFT_MISO = 12`
  - `TFT_SCLK = 14`
  - `TFT_CS = 15`
  - `TFT_DC = 2`
  - `TFT_RST = -1`
- Backlight:
  - `TFT_BL = 21`
  - `TFT_BACKLIGHT_ON = HIGH`
- Geometry:
  - `TFT_WIDTH = 240`
  - `TFT_HEIGHT = 320`
  - `setRotation(1)` in code (landscape)
- Display color fix in code:
  - `tft.invertDisplay(false)`
- Font flags required for visible text in this project:
  - `LOAD_GLCD`
  - `LOAD_FONT2`
  - `LOAD_FONT4`

Observed failure mode before enabling font flags:
- Screen showed mostly black with a blue line/header outline but missing text.

## 9) Recommended verification commands (already used)

```bash
lsusb
ls -l /dev/serial/by-id
ls -l /dev/ttyUSB*
.venv/bin/esptool --port /dev/ttyUSB0 chip-id
.venv/bin/esptool --port /dev/ttyUSB0 flash-id
.venv/bin/esptool --port /dev/ttyUSB0 read-mac
```

## 10) Additional listing references saved

- Secondary listing analysis:
  - `docs/hardware/LISTING_2_NOTES.md`
- Image storage location for pin map/spec tables:
  - `docs/hardware/images/`
