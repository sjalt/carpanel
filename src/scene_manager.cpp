#include "scene_manager.h"

#include "config.h"

namespace carpanel {

SceneManager::SceneManager() {
  current_scene_.type = SceneType::Test;
  current_scene_.brightness = 128;
  current_scene_.speed = 32;
  current_scene_.color = CRGB::Blue;
  layout_.rows = kDefaultPanelRows;
  layout_.columns = kDefaultPanelColumns;
}

void SceneManager::begin(CRGB* leds, uint16_t led_count) {
  leds_ = leds;
  led_count_ = led_count;
  last_update_ms_ = millis();
  clearLeds();
}

void SceneManager::setBrightness(uint8_t brightness) {
  current_scene_.brightness = brightness;
  FastLED.setBrightness(brightness);
}

bool SceneManager::setLayout(uint16_t rows, uint16_t columns) {
  if (rows == 0 || columns == 0 ||
      static_cast<uint32_t>(rows) * columns != led_count_) {
    return false;
  }

  layout_.rows = rows;
  layout_.columns = columns;
  return true;
}

void SceneManager::nextScene() {
  switch (current_scene_.type) {
    case SceneType::Test:
      setScene(SceneType::Rainbow);
      break;
    case SceneType::Rainbow:
      setScene(SceneType::Chase);
      break;
    case SceneType::Chase:
      setScene(SceneType::Pulse);
      break;
    case SceneType::Pulse:
      setScene(SceneType::Solid);
      break;
    case SceneType::Solid:
    default:
      setScene(SceneType::Test);
      break;
  }
}

void SceneManager::setScene(SceneType scene) {
  current_scene_.type = scene;
  switch (scene) {
    case SceneType::Test:
      current_scene_.color = CRGB::White;
      current_scene_.speed = 80;
      break;
    case SceneType::Rainbow:
      current_scene_.color = CRGB::Blue;
      current_scene_.speed = 24;
      break;
    case SceneType::Chase:
      current_scene_.color = CRGB::Green;
      current_scene_.speed = 40;
      break;
    case SceneType::Pulse:
      current_scene_.color = CRGB::Orange;
      current_scene_.speed = 55;
      break;
    case SceneType::Solid:
    default:
      current_scene_.color = CRGB::Purple;
      current_scene_.speed = 10;
      break;
  }
}

void SceneManager::update(uint32_t now_ms) {
  switch (current_scene_.type) {
    case SceneType::Test:
      renderTest(now_ms);
      break;
    case SceneType::Rainbow:
      renderRainbow(now_ms);
      break;
    case SceneType::Chase:
      renderChase(now_ms);
      break;
    case SceneType::Pulse:
      renderPulse(now_ms);
      break;
    case SceneType::Solid:
    default:
      renderSolid();
      break;
  }

  FastLED.show();
}

void SceneManager::renderTest(uint32_t now_ms) {
  const uint32_t delta = now_ms - last_update_ms_;
  if (delta < current_scene_.speed) {
    return;
  }

  clearLeds();
  leds_[test_led_index_] = current_scene_.color;
  test_led_index_ = (test_led_index_ + 1) % led_count_;
  last_update_ms_ = now_ms;
}

void SceneManager::renderRainbow(uint32_t now_ms) {
  const uint32_t delta = now_ms - last_update_ms_;
  if (delta < 20U) {
    return;
  }

  for (uint16_t i = 0; i < led_count_; ++i) {
    const uint16_t row = i / layout_.columns;
    const uint16_t column = i % layout_.columns;
    const uint16_t position = row * 256 / layout_.rows + column * 128 / layout_.columns;
    leds_[i] = CHSV(position + (now_ms / 20), 255, 255);
  }

  last_update_ms_ = now_ms;
}

void SceneManager::renderChase(uint32_t now_ms) {
  const uint32_t delta = now_ms - last_update_ms_;
  if (delta < 40U) {
    return;
  }

  clearLeds();
  const uint8_t offset = chase_offset_++;
  for (uint16_t i = 0; i < led_count_; ++i) {
    const uint16_t column = i % layout_.columns;
    if ((column + offset) % 8 == 0) {
      leds_[i] = current_scene_.color;
    }
  }

  last_update_ms_ = now_ms;
}

void SceneManager::renderPulse(uint32_t now_ms) {
  const uint32_t delta = now_ms - last_update_ms_;
  if (delta < 18U) {
    return;
  }

  const uint8_t pulse_value = (sin8((now_ms / 10) & 0xFF) * 255) / 255;
  for (uint16_t i = 0; i < led_count_; ++i) {
    leds_[i] = current_scene_.color;
    leds_[i].nscale8_video(pulse_value);
  }

  last_update_ms_ = now_ms;
}

void SceneManager::renderSolid() {
  for (uint16_t i = 0; i < led_count_; ++i) {
    leds_[i] = current_scene_.color;
  }
}

void SceneManager::clearLeds() {
  for (uint16_t i = 0; i < led_count_; ++i) {
    leds_[i] = CRGB::Black;
  }
}

}  // namespace carpanel
