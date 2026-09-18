#include "web_server.h"

#include <ArduinoJson.h>

namespace carpanel {

WebServerController::WebServerController(SceneManager& scene_manager)
    : scene_manager_(scene_manager), server_(80) {
}

void WebServerController::begin() {
  server_.on("/", HTTP_GET, [this]() { serveRoot(); });
  server_.on("/scene", HTTP_POST, [this]() { handleSetScene(); });
  server_.on("/brightness", HTTP_POST, [this]() { handleSetBrightness(); });
  server_.on("/api/state", HTTP_GET, [this]() { handleJsonState(); });
  server_.begin();
}

void WebServerController::handleClient() {
  server_.handleClient();
}

void WebServerController::serveRoot() {
  server_.send(200, "text/html", buildHtml());
}

void WebServerController::handleSetScene() {
  if (!server_.hasArg("plain")) {
    server_.send(400, "text/plain", "Missing JSON body");
    return;
  }

  const String body = server_.arg("plain");
  DynamicJsonDocument doc(256);
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    server_.send(400, "text/plain", "Invalid JSON");
    return;
  }

  const String scene_name = doc["scene"] | "rainbow";
  if (scene_name == "rainbow") {
    scene_manager_.setScene(SceneType::Rainbow);
  } else if (scene_name == "chase") {
    scene_manager_.setScene(SceneType::Chase);
  } else if (scene_name == "pulse") {
    scene_manager_.setScene(SceneType::Pulse);
  } else if (scene_name == "solid") {
    scene_manager_.setScene(SceneType::Solid);
  }

  server_.send(200, "application/json", "{\"ok\":true}");
}

void WebServerController::handleSetBrightness() {
  if (!server_.hasArg("plain")) {
    server_.send(400, "text/plain", "Missing JSON body");
    return;
  }

  DynamicJsonDocument doc(128);
  DeserializationError err = deserializeJson(doc, server_.arg("plain"));
  if (err) {
    server_.send(400, "text/plain", "Invalid JSON");
    return;
  }

  scene_manager_.setBrightness(doc["brightness"] | 128);
  server_.send(200, "application/json", "{\"ok\":true}");
}

void WebServerController::handleJsonState() {
  const auto& scene = scene_manager_.currentScene();

  DynamicJsonDocument doc(256);
  doc["scene"] = static_cast<int>(scene.type);
  doc["brightness"] = scene.brightness;

  String output;
  serializeJson(doc, output);
  server_.send(200, "application/json", output);
}

String WebServerController::buildHtml() const {
  return R"HTML(
<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <title>CarPanel</title>
    <style>
      body { font-family: Arial, sans-serif; background: #111827; color: #f9fafb; margin: 0; padding: 24px; }
      .panel { max-width: 420px; margin: 0 auto; background: #1f2937; border-radius: 12px; padding: 20px; }
      label { display: block; margin-top: 16px; font-weight: bold; }
      select, input, button { width: 100%; box-sizing: border-box; margin-top: 8px; padding: 10px; border-radius: 8px; border: 1px solid #374151; }
      button { background: #22c55e; color: #04130a; font-weight: bold; cursor: pointer; }
      .status { margin-top: 18px; color: #bfdbfe; }
    </style>
  </head>
  <body>
    <div class="panel">
      <h1>CarPanel</h1>
      <label for="scene">Scene</label>
      <select id="scene">
        <option value="rainbow">Rainbow</option>
        <option value="chase">Chase</option>
        <option value="pulse">Pulse</option>
        <option value="solid">Solid</option>
      </select>

      <label for="brightness">Brightness</label>
      <input id="brightness" type="range" min="0" max="255" step="1" value="128" />

      <button id="applyBtn" type="button">Apply</button>
      <div id="status" class="status">Loading...</div>
    </div>

    <script>
      const sceneEl = document.getElementById('scene');
      const brightnessEl = document.getElementById('brightness');
      const statusEl = document.getElementById('status');

      async function fetchState() {
        const response = await fetch('/api/state');
        const state = await response.json();
        sceneEl.value = ['rainbow', 'chase', 'pulse', 'solid'][state.scene] || 'rainbow';
        brightnessEl.value = state.brightness || 128;
        statusEl.textContent = 'Connected';
      }

      document.getElementById('applyBtn').addEventListener('click', async () => {
        statusEl.textContent = 'Sending...';
        await fetch('/scene', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ scene: sceneEl.value })
        });
        await fetch('/brightness', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ brightness: Number(brightnessEl.value) })
        });
        statusEl.textContent = 'Updated';
      });

      fetchState();
    </script>
  </body>
</html>
)HTML";
}

}  // namespace carpanel
