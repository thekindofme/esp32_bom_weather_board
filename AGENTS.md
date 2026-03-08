# AGENTS.md

This file is the working guide for any coding agent operating in this repository.

## Project summary

- Project: ESP32 weather display firmware for an ST7789 screen.
- Goal: continuously show live weather data for the Greenvale area using BOM data.
- Current UX:
  - Portrait dashboard (`240x320`) with current conditions + 3-day summary cards.
  - Night-friendly dark theme with reduced brightness at night.
  - Touch tap toggles theme (`dark <-> light`) without leaving the current screen.
  - During refresh, existing weather data remains visible and only a small animated header indicator is shown.
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

### Architecture: shared core + pluggable layouts

The firmware is split into a shared core (data, WiFi, NTP, touch, loop) and
pluggable layout files. Only ONE layout `.cpp` is compiled per build, selected
via PlatformIO environment.

- `src/main.cpp`
  - Firmware entrypoint (core only, no drawing).
  - Wi-Fi connect/reconnect, BOM FTP fetch, NTP, touch theme toggle, main loop.
  - Calls `layoutDraw*()` functions declared in `DisplayLayout.h`.
- `src/DisplayCommon.cpp` + `include/DisplayCommon.h`
  - Shared drawing helpers available to all layouts:
    - Theme color globals and `initThemeColors()`
    - `drawWeatherIcon()`, `setBacklightPercent()`, `applyBacklightForTime()`
    - `getCurrentTimeString12h()`, `getCurrentDateStringShort()`
    - State globals: `isLightTheme`, `refreshAnimating`, `spinnerFrame`
- `include/WeatherTypes.h`
  - `WeatherData` struct shared between core and layouts.
- `include/DisplayLayout.h`
  - Declares the 5 functions each layout must implement:
    - `layoutDrawWeather()`, `layoutDrawHeader()`, `layoutDrawNowAndDate()`
    - `layoutDrawStatus()`, `layoutDrawRefreshIndicator()`
- `src/layouts/LayoutHeroTemp.cpp`
  - Layout A: temperature-dominant, big temp readable from across the room.
- `src/layouts/LayoutRainFirst.cpp`
  - Layout B: rain-focused with 96px hero rain band. Perfect for Melbourne.
- `src/layouts/LayoutHudGrid.cpp`
  - Layout C: information-dense grid, instrument panel style.
- `include/Config.h`
  - Runtime configuration: Wi-Fi credentials, BOM file path, station ID, refresh intervals.
  - Includes touch-toggle settings:
    - `TOUCH_IRQ_PIN` (default `36`)
    - `TOUCH_TOGGLE_DEBOUNCE_MS` (default `400`)
- `include/BomParser.h` + `src/BomParser.cpp`
  - Station XML parser used to extract weather values.
- `include/FtpUtils.h` + `src/FtpUtils.cpp`
  - FTP utility parsing (`PASV` response parsing).
- `test/test_bom_parser/test_main.cpp`
  - Native host unit tests for parser/util logic.
- `test/fixtures/IDV60920_live_2026-03-01.xml`
  - Real BOM XML snapshot used by integration-style parser test.
- `docs/hardware/LISTING_2_NOTES.md`
  - Secondary listing-derived board/pin notes with confidence levels.
- `docs/hardware/images/`
  - Storage location for listing screenshots (pin map/spec tables).
- `platformio.ini`
  - PlatformIO environments:
    - `hero_temp` / `rain_first` / `hud_grid` for firmware build/upload (select layout).
    - `native` for host-side tests.

## BOM integration details

- Observation feed: `/anon/gen/fwo/IDV60920.xml` (Victoria observations).
- Default station for Greenvale: `086282` (`Melbourne Airport`), because Greenvale is not a direct station entry in this feed.
- Forecast feed: `/anon/gen/fwo/IDV10753.xml` (VIC location forecasts)
- Forecast location currently used for Greenvale context: `Tullamarine`
- Parser fields currently extracted:
  - `air_temperature`
  - `apparent_temp`
  - `rel-humidity`
  - `wind_spd_kmh`
  - `wind_dir`
  - `rainfall`
  - running daily min/max from observations (`minimum_air_temperature`, `maximum_air_temperature`)
  - station description and observation local time
  - from forecast feed:
    - icon code
    - precipitation range and probability
    - daily min/max
    - next 3-day summary fields

## Build, test, flash

## Prerequisites
- Python virtualenv at `.venv` with `platformio` installed.
- USB serial access (`dialout` group on Linux) for flashing/monitoring.

## Commands

Run tests:
```bash
.venv/bin/pio test -e native
```

Refresh BOM fixture snapshot:
```bash
curl -s ftp://ftp.bom.gov.au/anon/gen/fwo/IDV60920.xml > test/fixtures/IDV60920_live_YYYY-MM-DD.xml
```

Build firmware (pick a layout):
```bash
.venv/bin/pio run -e hero_temp
.venv/bin/pio run -e rain_first
.venv/bin/pio run -e hud_grid
```

Build all layouts:
```bash
.venv/bin/pio run -e hero_temp && .venv/bin/pio run -e rain_first && .venv/bin/pio run -e hud_grid
```

Upload firmware (pick a layout):
```bash
.venv/bin/pio run -e hero_temp -t upload --upload-port /dev/ttyUSB0
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
- Color order: `TFT_BGR`
- Also required for this board in current setup:
  - `TFT_MISO=12`
  - `LOAD_GLCD=1`
  - `LOAD_FONT2=1`
  - `LOAD_FONT4=1`
  - `tft.invertDisplay(false)` in app code

If display is blank/garbled, pins and/or rotation likely differ for this board revision.

Current orientation in app:
- `setRotation(0)` portrait mode

## Working rules for agents

- Keep changes focused and testable.
- Prefer updating parser/util logic under unit tests before touching runtime display/network flow.
- Preserve BOM FTP approach unless BOM policy/availability changes.
- Do not hardcode secrets in git:
  - Keep placeholders/defaults in `include/Config.h`.
  - Put real local values in `include/ConfigLocal.h` (gitignored).
  - Use `include/ConfigLocal.example.h` as the template.
- Before finishing substantive changes:
  - Run `pio test -e native`.
  - If runtime code changed, run at least `pio run -e hero_temp` (when toolchain is available).
  - If layout code changed, build the affected layout env(s) to verify.

## Known gaps / TODO

- Verify actual touch controller IC and full coordinate reading integration.
- Add optional calibration mode if moving from IRQ-only touch to full touch-point UI interactions.
- Optional: switch to a more robust XML parser if feed structure changes.
- Optional: add test coverage for forecast parsing logic (current tests focus on observation parsing/PASV).
- Improve AM/PM rain split with a true higher-resolution source (currently marked as estimate `~`).

## Safety notes

- BOM website blocks scraping; use approved/public data channels (currently FTP feed in use).
- Respect BOM terms/disclaimer references in distributed products.
