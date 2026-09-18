#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "scene_manager.h"
#include "web_server.h"

namespace {

CRGB leds[carpanel::kMaxLedCount];
carpanel::SceneManager scene_manager;
carpanel::WebServerController web_server(scene_manager);

void setupWifi() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(carpanel::kApSsid, carpanel::kApPassword);

  Serial.println();
  Serial.print("AP SSID: ");
  Serial.println(carpanel::kApSsid);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
}

void handleButtonInput() {
  static bool last_button_state = false;
  static uint32_t last_transition_ms = 0;

  const bool pressed = digitalRead(carpanel::kButtonPin) == LOW;
  const uint32_t now = millis();

  if (pressed != last_button_state) {
    last_transition_ms = now;
    last_button_state = pressed;
    return;
  }

  if (pressed && (now - last_transition_ms) > carpanel::kSceneCycleDebounceMs) {
    scene_manager.nextScene();
    last_transition_ms = now;
  }
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(carpanel::kButtonPin, INPUT_PULLUP);

  CLEDController& led_controller =
      FastLED.addLeds<WS2812B, carpanel::kLedPin, GRB>(
          leds, carpanel::kDefaultLedCount);
  FastLED.setBrightness(carpanel::kDefaultBrightness);
  FastLED.clear(true);

  scene_manager.begin(leds, carpanel::kDefaultLedCount, &led_controller);
  scene_manager.setBrightness(carpanel::kDefaultBrightness);

  setupWifi();
  web_server.begin();
}

void loop() {
  handleButtonInput();
  scene_manager.update(millis());
  web_server.handleClient();
}
