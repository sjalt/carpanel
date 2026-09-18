#pragma once

#include <FastLED.h>
#include <array>
#include <cstdint>
#include <string>

namespace carpanel {

enum class SceneType : uint8_t {
  Rainbow,
  Chase,
  Pulse,
  Solid
};

struct SceneState {
  SceneType type = SceneType::Rainbow;
  uint8_t brightness = 128;
  uint8_t speed = 32;
  CRGB color = CRGB::Blue;
};

class SceneManager {
 public:
  SceneManager();

  void begin(CRGB* leds, uint16_t led_count);
  void setBrightness(uint8_t brightness);
  void nextScene();
  void setScene(SceneType scene);
  void update(uint32_t now_ms);

  const SceneState& currentScene() const { return current_scene_; }

 private:
  void renderRainbow(uint32_t now_ms);
  void renderChase(uint32_t now_ms);
  void renderPulse(uint32_t now_ms);
  void renderSolid();
  void clearLeds();

  CRGB* leds_ = nullptr;
  uint16_t led_count_ = 0;
  SceneState current_scene_;
  uint32_t last_update_ms_ = 0;
  uint8_t chase_offset_ = 0;
  uint8_t pulse_phase_ = 0;
};

}  // namespace carpanel
