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
  server_.on("/layout", HTTP_POST, [this]() { handleSetLayout(); });
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
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    server_.send(400, "text/plain", "Invalid JSON");
    return;
  }

  const String scene_name = doc["scene"] | "rainbow";
  if (scene_name == "test") {
    scene_manager_.setScene(SceneType::Test);
  } else if (scene_name == "rainbow") {
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

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, server_.arg("plain"));
  if (err) {
    server_.send(400, "text/plain", "Invalid JSON");
    return;
  }

  scene_manager_.setBrightness(doc["brightness"] | 128);
  server_.send(200, "application/json", "{\"ok\":true}");
}

void WebServerController::handleSetLayout() {
  if (!server_.hasArg("plain")) {
    server_.send(400, "text/plain", "Missing JSON body");
    return;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, server_.arg("plain"));
  if (err) {
    server_.send(400, "text/plain", "Invalid JSON");
    return;
  }

  const uint16_t rows = doc["rows"] | 0;
  const uint16_t columns = doc["columns"] | 0;
  if (!scene_manager_.setLayout(rows, columns)) {
    server_.send(400, "text/plain", "Rows multiplied by columns must equal the LED count");
    return;
  }

  server_.send(200, "application/json", "{\"ok\":true}");
}

void WebServerController::handleJsonState() {
  const auto& scene = scene_manager_.currentScene();

  JsonDocument doc;
  doc["scene"] = static_cast<int>(scene.type);
  doc["brightness"] = scene.brightness;
  doc["rows"] = scene_manager_.layout().rows;
  doc["columns"] = scene_manager_.layout().columns;

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
        <option value="test">LED Test</option>
        <option value="rainbow">Rainbow</option>
        <option value="chase">Chase</option>
        <option value="pulse">Pulse</option>
        <option value="solid">Solid</option>
      </select>

      <label for="brightness">Brightness</label>
      <input id="brightness" type="range" min="0" max="255" step="1" value="128" />

      <label for="rows">Panel rows (rows x columns = 144 LEDs)</label>
      <input id="rows" type="number" min="1" value="12" />

      <label for="columns">Panel columns</label>
      <input id="columns" type="number" min="1" value="12" />

      <button id="applyBtn" type="button">Apply</button>
      <div id="status" class="status">Loading...</div>
    </div>

    <script>
      const sceneEl = document.getElementById('scene');
      const brightnessEl = document.getElementById('brightness');
      const rowsEl = document.getElementById('rows');
      const columnsEl = document.getElementById('columns');
      const statusEl = document.getElementById('status');

      async function fetchState() {
        const response = await fetch('/api/state');
        const state = await response.json();
        sceneEl.value = ['test', 'rainbow', 'chase', 'pulse', 'solid'][state.scene] || 'test';
        brightnessEl.value = state.brightness || 128;
        rowsEl.value = state.rows;
        columnsEl.value = state.columns;
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
        const layoutResponse = await fetch('/layout', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ rows: Number(rowsEl.value), columns: Number(columnsEl.value) })
        });
        if (!layoutResponse.ok) {
          statusEl.textContent = await layoutResponse.text();
          return;
        }
        statusEl.textContent = 'Updated';
      });

      fetchState();
    </script>
  </body>
</html>
)HTML";
}

}  // namespace carpanel
