#pragma once

#include <Arduino.h>
#include <FastLED.h>

namespace carpanel {

constexpr uint8_t kLedPin = 2;
constexpr uint16_t kMaxLedCount = 1024;
constexpr uint16_t kDefaultLedCount = 256;
constexpr uint16_t kDefaultPanelRows = 8;
constexpr uint16_t kDefaultPanelColumns = 32;
constexpr uint8_t kButtonPin = 0;
constexpr uint8_t kDefaultBrightness = 128;
constexpr uint16_t kWebPort = 80;
constexpr uint32_t kSceneCycleDebounceMs = 200;
constexpr char kApSsid[] = "CarPanel";
constexpr char kApPassword[] = "carpanel123";

}  // namespace carpanel
