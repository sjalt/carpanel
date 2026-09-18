#pragma once

#include <FastLED.h>
#include <array>
#include <cstdint>
#include <string>

namespace carpanel {

enum class SceneType : uint8_t {
  Test,
  Rainbow,
  Chase,
  Pulse,
  Solid
};

enum class WiringMode : uint8_t {
  Rows,
  RowsSerpentine,
  Columns,
  ColumnsSerpentine
};

enum class ChainDirection : uint8_t {
  Horizontal,
  Vertical
};

struct SceneState {
  SceneType type = SceneType::Test;
  uint8_t brightness = 128;
  uint8_t speed = 32;
  CRGB color = CRGB::Blue;
};

struct PanelLayout {
  uint16_t panel_rows = 8;
  uint16_t panel_columns = 32;
  uint8_t panel_count = 1;
  ChainDirection chain_direction = ChainDirection::Horizontal;
  WiringMode wiring = WiringMode::RowsSerpentine;

  uint16_t rows() const {
    return chain_direction == ChainDirection::Vertical
               ? panel_rows * panel_count
               : panel_rows;
  }

  uint16_t columns() const {
    return chain_direction == ChainDirection::Horizontal
               ? panel_columns * panel_count
               : panel_columns;
  }
};

class SceneManager {
 public:
  SceneManager();

  void begin(CRGB* leds, uint16_t led_count, CLEDController* controller);
  void setBrightness(uint8_t brightness);
  bool setLayout(uint16_t panel_rows, uint16_t panel_columns,
                 uint8_t panel_count, ChainDirection chain_direction,
                 WiringMode wiring);
  void nextScene();
  void setScene(SceneType scene);
  void update(uint32_t now_ms);

  const SceneState& currentScene() const { return current_scene_; }
  const PanelLayout& layout() const { return layout_; }

 private:
  void renderTest(uint32_t now_ms);
  void renderRainbow(uint32_t now_ms);
  void renderChase(uint32_t now_ms);
  void renderPulse(uint32_t now_ms);
  void renderSolid();
  uint16_t indexForPosition(uint16_t row, uint16_t column) const;
  void clearLeds();

  CRGB* leds_ = nullptr;
  CLEDController* controller_ = nullptr;
  uint16_t led_count_ = 0;
  SceneState current_scene_;
  PanelLayout layout_;
  uint32_t last_update_ms_ = 0;
  uint16_t test_led_index_ = 0;
  uint8_t chase_offset_ = 0;
  uint8_t pulse_phase_ = 0;
};

}  // namespace carpanel
