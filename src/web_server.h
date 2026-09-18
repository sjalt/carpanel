#pragma once

#include <WiFi.h>
#include <WebServer.h>

#include "scene_manager.h"

namespace carpanel {

class WebServerController {
 public:
  explicit WebServerController(SceneManager& scene_manager);

  void begin();
  void handleClient();

 private:
  void serveRoot();
  void handleSetScene();
  void handleSetBrightness();
  void handleJsonState();
  String buildHtml() const;

  WebServer server_{80};
  SceneManager& scene_manager_;
};

}  // namespace carpanel
