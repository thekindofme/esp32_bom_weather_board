# AGENTS.md

This file is the working guide for any coding agent operating in this repository.

## Project summary

- Project: ESP32 weather display firmware for an ST7789 screen.
- Goal: continuously show live weather data for the Greenvale area using BOM data.
- Current data source strategy: BOM anonymous FTP XML feed (`ftp.bom.gov.au`) because BOM web endpoints are protected against scraping.

## Current hardware baseline

Verified via direct probe:
- MCU: `ESP32-D0WD-V3` (ESP32-WROOM-32 class), rev `v3.1`
- Flash: `4MB` (`32Mbit`)
- Crystal: `40MHz`
- USB bridge: `CH340`
- Serial device on Linux: `/dev/ttyUSB0`

From board details:
- Board family: `ESP32-2432S028` style
- Display controller: `ST7789` (SPI)
- Touch exists physically, controller still unverified

Reference: `HARDWARE_SPEC.md`

## Repository layout

- `src/main.cpp`
  - Firmware entrypoint.
  - Wi-Fi connect/reconnect, BOM FTP fetch, display rendering loop.
- `include/Config.h`
  - Runtime configuration: Wi-Fi credentials, BOM file path, station ID, refresh intervals.
- `include/BomParser.h` + `src/BomParser.cpp`
  - Station XML parser used to extract weather values.
- `include/FtpUtils.h` + `src/FtpUtils.cpp`
  - FTP utility parsing (`PASV` response parsing).
- `test/test_bom_parser/test_main.cpp`
  - Native host unit tests for parser/util logic.
- `platformio.ini`
  - PlatformIO environments:
    - `esp32dev` for firmware build/upload.
    - `native` for host-side tests.

## BOM integration details

- Default feed: `/anon/gen/fwo/IDV60920.xml` (Victoria observations).
- Default station for Greenvale: `086282` (`Melbourne Airport`), because Greenvale is not a direct station entry in this feed.
- Parser fields currently extracted:
  - `air_temperature`
  - `apparent_temp`
  - `rel-humidity`
  - `wind_spd_kmh`
  - `wind_dir`
  - `rainfall`
  - station description and observation local time

## Build, test, flash

## Prerequisites
- Python virtualenv at `.venv` with `platformio` installed.
- USB serial access (`dialout` group on Linux) for flashing/monitoring.

## Commands

Run tests:
```bash
.venv/bin/pio test -e native
```

Build firmware:
```bash
.venv/bin/pio run -e esp32dev
```

Upload firmware:
```bash
.venv/bin/pio run -e esp32dev -t upload --upload-port /dev/ttyUSB0
```

Serial monitor:
```bash
.venv/bin/pio device monitor -b 115200 -p /dev/ttyUSB0
```

## Display/pin assumptions (current)

Defined in `platformio.ini` build flags:
- `TFT_MOSI=13`
- `TFT_SCLK=14`
- `TFT_CS=15`
- `TFT_DC=2`
- `TFT_RST=-1`
- `TFT_BL=21`
- `TFT_WIDTH=240`
- `TFT_HEIGHT=320`
- Driver: `ST7789`

If display is blank/garbled, pins and/or rotation likely differ for this board revision.

## Working rules for agents

- Keep changes focused and testable.
- Prefer updating parser/util logic under unit tests before touching runtime display/network flow.
- Preserve BOM FTP approach unless BOM policy/availability changes.
- Do not hardcode secrets in git:
  - `Config.h` should keep placeholder Wi-Fi credentials by default.
- Before finishing substantive changes:
  - Run `pio test -e native`.
  - If runtime code changed, run at least `pio run -e esp32dev` (when toolchain is available).

## Known gaps / TODO

- Verify actual touch controller IC and integration.
- Confirm final working display pin map for this exact board unit.
- Optional: switch to a more robust XML parser if feed structure changes.
- Optional: add integration test with captured BOM XML fixture.

## Safety notes

- BOM website blocks scraping; use approved/public data channels (currently FTP feed in use).
- Respect BOM terms/disclaimer references in distributed products.
