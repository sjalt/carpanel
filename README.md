# CarPanel

A compact ESP32-S3 project for controlling a WS2812B panel array with a physical button and a lightweight local web UI.

## Features
- ESP32-S3 firmware using PlatformIO and Arduino
- WS2812B LED output with a shared scene/state engine
- Button-driven scene cycling
- Wi-Fi access point for local control
- Simple web interface for scene and brightness adjustment

## Quick start

1. Install PlatformIO.
2. Open this folder in VS Code.
3. Run:

```bash
pio run
```

4. Upload to the board:

```bash
pio run --target upload
```

## Hardware notes
- LED data pin is configured in `src/config.h` and should be checked against the actual XIAO ESP32-S3 layout and wiring.
- The default AP SSID is `CarPanel` with password `carpanel123`.
- The button input defaults to GPIO 0 and can be adjusted if the board wiring differs.
