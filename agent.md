# CarPanel Project Agent

## Goal
Build a XIAO ESP32-S3 firmware project that drives a WS2812B panel array with a button-based scene cycle and a local Wi-Fi web interface for remote control.

## Current status
- Project skeleton created
- Firmware and scene engine scaffolding added
- Wi-Fi web server shell added
- Build verification pending

## Architecture
- `src/config.h`: hardware pin config, LED counts, Wi-Fi SSID, and runtime constants
- `src/scene_manager.*`: shared scene model and animation engine used by both input methods
- `src/main.cpp`: setup, button input, Wi-Fi startup, and main loop orchestration
- `src/web_server.*`: local HTTP controller for the web UI and API

## Important implementation notes
- Keep the scene/state logic centralized so button presses and web actions drive the same scene engine.
- Confirm the final LED pin, power budget, and panel layout before hardware validation.
- Use the web UI as a thin control layer over the same state machine rather than separate logic.

## Next actions
1. Validate the firmware builds successfully.
2. Add richer effect presets and config persistence.
3. Test button cycling on the real hardware.
4. Confirm the web interface works over the board's AP.
