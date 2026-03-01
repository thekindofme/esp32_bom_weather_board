# ESP32 BOM Weather Board

ESP32 weather dashboard firmware for `ESP32-2432S028`-style boards (ST7789, 2.8" 240x320), using Bureau of Meteorology (BOM) FTP feeds.

Current UI is portrait, night-friendly, and shows:
- live conditions (temp, feels-like, humidity, wind, rain)
- daily ranges
- rain outlook
- next 3-day summary
- live time/date

## Getting Started

## 1) Requirements
- ESP32 board connected by USB (`/dev/ttyUSB0` on Linux)
- Python 3
- PlatformIO (installed in local `.venv` in this repo)

## 2) Configure Wi-Fi (local, not committed)
Create `include/ConfigLocal.h` from the example:

```bash
cp include/ConfigLocal.example.h include/ConfigLocal.h
```

Edit with your real values:
- `WIFI_SSID`
- `WIFI_PASSWORD`

`include/ConfigLocal.h` is git-ignored.

## 3) Install tooling
```bash
python3 -m venv .venv
.venv/bin/pip install platformio
```

## 4) Run tests
```bash
.venv/bin/pio test -e native
```

## 5) Build and flash
```bash
.venv/bin/pio run -e esp32dev -t upload --upload-port /dev/ttyUSB0
```

## 6) Monitor serial logs (optional)
```bash
.venv/bin/pio device monitor -b 115200 -p /dev/ttyUSB0
```

## Notes
- BOM web JSON endpoints are protected; this project uses BOM FTP XML feeds.
- Hardware + pin mapping details: `HARDWARE_SPEC.md`
- Agent/developer workflow notes: `AGENTS.md`
