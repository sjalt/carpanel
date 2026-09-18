#include "scene_manager.h"

#include "config.h"

namespace {

const uint8_t kFont[][5] = {
    {0x7e, 0x11, 0x11, 0x11, 0x7e}, {0x7f, 0x49, 0x49, 0x49, 0x36},
    {0x3e, 0x41, 0x41, 0x41, 0x22}, {0x7f, 0x41, 0x41, 0x22, 0x1c},
    {0x7f, 0x49, 0x49, 0x49, 0x41}, {0x7f, 0x09, 0x09, 0x09, 0x01},
    {0x3e, 0x41, 0x49, 0x49, 0x7a}, {0x7f, 0x08, 0x08, 0x08, 0x7f},
    {0x00, 0x41, 0x7f, 0x41, 0x00}, {0x20, 0x40, 0x41, 0x3f, 0x01},
    {0x7f, 0x08, 0x14, 0x22, 0x41}, {0x7f, 0x40, 0x40, 0x40, 0x40},
    {0x7f, 0x02, 0x0c, 0x02, 0x7f}, {0x7f, 0x04, 0x08, 0x10, 0x7f},
    {0x3e, 0x41, 0x41, 0x41, 0x3e}, {0x7f, 0x09, 0x09, 0x09, 0x06},
    {0x3e, 0x41, 0x51, 0x21, 0x5e}, {0x7f, 0x09, 0x19, 0x29, 0x46},
    {0x46, 0x49, 0x49, 0x49, 0x31}, {0x01, 0x01, 0x7f, 0x01, 0x01},
    {0x3f, 0x40, 0x40, 0x40, 0x3f}, {0x1f, 0x20, 0x40, 0x20, 0x1f},
    {0x7f, 0x20, 0x18, 0x20, 0x7f}, {0x63, 0x14, 0x08, 0x14, 0x63},
    {0x07, 0x08, 0x70, 0x08, 0x07}, {0x61, 0x51, 0x49, 0x45, 0x43},
    {0x3e, 0x51, 0x49, 0x45, 0x3e}, {0x00, 0x42, 0x7f, 0x40, 0x00},
    {0x62, 0x51, 0x49, 0x49, 0x46}, {0x22, 0x49, 0x49, 0x49, 0x36},
    {0x18, 0x14, 0x12, 0x7f, 0x10}, {0x2f, 0x49, 0x49, 0x49, 0x31},
    {0x3e, 0x49, 0x49, 0x49, 0x32}, {0x01, 0x71, 0x09, 0x05, 0x03},
    {0x36, 0x49, 0x49, 0x49, 0x36}, {0x26, 0x49, 0x49, 0x49, 0x3e},
    {0x00, 0x00, 0x00, 0x00, 0x00}};

uint8_t glyphColumn(char character, uint8_t column) {
  if (character >= 'a' && character <= 'z') {
    character = static_cast<char>(character - 'a' + 'A');
  }
  if (character >= 'A' && character <= 'Z') {
    return kFont[character - 'A'][column];
  }
  if (character >= '0' && character <= '9') {
    return kFont[26 + character - '0'][column];
  }
  return kFont[36][column];
}

}  // namespace

namespace carpanel {

SceneManager::SceneManager() {
  current_scene_.type = SceneType::Test;
  current_scene_.brightness = 128;
  current_scene_.speed = 32;
  current_scene_.color = CRGB::Blue;
  layout_.panel_rows = kDefaultPanelRows;
  layout_.panel_columns = kDefaultPanelColumns;
  layout_.panel_count = 1;
  layout_.chain_direction = ChainDirection::Horizontal;
  layout_.wiring = WiringMode::RowsSerpentine;
}

void SceneManager::begin(CRGB* leds, uint16_t led_count,
                         CLEDController* controller) {
  leds_ = leds;
  led_count_ = led_count;
  controller_ = controller;
  last_update_ms_ = millis();
  clearLeds();
}

void SceneManager::setBrightness(uint8_t brightness) {
  current_scene_.brightness = brightness;
  FastLED.setBrightness(brightness);
}

void SceneManager::setText(const String& text) {
  text_ = text;
  text_offset_ = 0;
}

bool SceneManager::setLayout(uint16_t panel_rows, uint16_t panel_columns,
                             uint8_t panel_count,
                             ChainDirection chain_direction,
                             WiringMode wiring) {
  if (panel_rows == 0 || panel_columns == 0 || panel_count == 0) {
    return false;
  }

  PanelLayout next_layout;
  next_layout.panel_rows = panel_rows;
  next_layout.panel_columns = panel_columns;
  next_layout.panel_count = panel_count;
  next_layout.chain_direction = chain_direction;
  next_layout.wiring = wiring;
  const uint32_t led_count = static_cast<uint32_t>(next_layout.rows()) *
                             next_layout.columns();
  if (led_count == 0 || led_count > kMaxLedCount) {
    return false;
  }

  layout_ = next_layout;
  led_count_ = led_count;
  controller_->setLeds(leds_, led_count_);
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
      setScene(SceneType::Text);
      break;
    case SceneType::Text:
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
      current_scene_.color = CRGB::Purple;
      current_scene_.speed = 10;
      break;
    case SceneType::Text:
      current_scene_.color = CRGB::White;
      current_scene_.speed = 90;
      break;
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
      renderSolid();
      break;
    case SceneType::Text:
      renderText(now_ms);
      break;
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

  for (uint16_t row = 0; row < layout_.rows(); ++row) {
    for (uint16_t column = 0; column < layout_.columns(); ++column) {
      const uint16_t i = indexForPosition(row, column);
      const uint16_t position = row * 256 / layout_.rows() +
                                column * 128 / layout_.columns();
      leds_[i] = CHSV(position + (now_ms / 20), 255, 255);
    }
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
  for (uint16_t row = 0; row < layout_.rows(); ++row) {
    for (uint16_t column = 0; column < layout_.columns(); ++column) {
      if ((column + offset) % 8 == 0) {
        leds_[indexForPosition(row, column)] = current_scene_.color;
      }
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

void SceneManager::renderText(uint32_t now_ms) {
  const uint32_t delta = now_ms - last_update_ms_;
  if (delta < 90U) {
    return;
  }

  clearLeds();
  const uint16_t text_width = text_.length() * 6;
  const uint16_t scroll_width = layout_.columns() + text_width;
  if (scroll_width == 0) {
    last_update_ms_ = now_ms;
    return;
  }

  for (uint16_t character_index = 0; character_index < text_.length();
       ++character_index) {
    const uint8_t character = text_[character_index];
    for (uint8_t glyph_column = 0; glyph_column < 5; ++glyph_column) {
      const uint8_t glyph = glyphColumn(character, glyph_column);
      const int16_t column = static_cast<int16_t>(
          character_index * 6 + glyph_column + layout_.columns() - text_offset_);
      if (column < 0 || column >= layout_.columns()) {
        continue;
      }
      for (uint16_t row = 0; row < layout_.rows() && row < 7; ++row) {
        if ((glyph >> row) & 1) {
          leds_[indexForPosition(row, column)] = current_scene_.color;
        }
      }
    }
  }

  text_offset_ = (text_offset_ + 1) % scroll_width;
  last_update_ms_ = now_ms;
}

uint16_t SceneManager::indexForPosition(uint16_t row, uint16_t column) const {
  const bool column_wiring = layout_.wiring == WiringMode::Columns ||
                             layout_.wiring == WiringMode::ColumnsSerpentine;
  const bool serpentine = layout_.wiring == WiringMode::RowsSerpentine ||
                          layout_.wiring == WiringMode::ColumnsSerpentine;

  if (column_wiring) {
    if (serpentine && column % 2 == 1) {
      return column * layout_.rows() + (layout_.rows() - row - 1);
    }
    return column * layout_.rows() + row;
  }

  if (serpentine && row % 2 == 1) {
    return row * layout_.columns() + (layout_.columns() - column - 1);
  }
  return row * layout_.columns() + column;
}

void SceneManager::clearLeds() {
  for (uint16_t i = 0; i < led_count_; ++i) {
    leds_[i] = CRGB::Black;
  }
}

}  // namespace carpanel
