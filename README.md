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

## Panel wiring

Connect to the panel's **input** connector, typically marked `5V` or `V+`, `GND`, and `DIN` (not `DOUT`). The wire colors are not a reliable substitute for these labels.

| Panel connector | Connect to | Notes |
| --- | --- | --- |
| `5V` / `V+` | External regulated 5V power supply positive | Do not power a panel array from the XIAO's 5V/VBUS pin. |
| `GND` | External power supply negative and XIAO `GND` | All devices must share this common ground. |
| `DIN` | XIAO GPIO 2 (`D2`) through a 330-470 ohm series resistor | This firmware's default LED data pin is `kLedPin = 2`. |

Use a 5V supply sized for the number of LEDs. At full-white maximum brightness, a WS2812B can draw up to 60 mA per LED; estimate the supply requirement as $0.06 \times \text{LED count}$ amperes and include headroom. For the default 144 LEDs, this is up to 8.64 A, although this firmware's brightness limit reduces normal draw.

Place a 1000 uF or larger electrolytic capacitor across the panel's `5V` and `GND` terminals, close to its input. A 3.3V-to-5V level shifter, such as a 74AHCT125 or 74HCT125, is recommended between GPIO 2 and `DIN` for dependable signaling to a 5V WS2812B panel. Keep the data wire short and connect power to additional panel injection points where the panel manufacturer recommends it.
