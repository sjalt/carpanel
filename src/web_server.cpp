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

  const uint16_t panel_rows = doc["panel_rows"] | 0;
  const uint16_t panel_columns = doc["panel_columns"] | 0;
  const uint8_t panel_count = doc["panel_count"] | 0;
  const String chain_name = doc["chain"] | "horizontal";
  ChainDirection chain_direction = ChainDirection::Horizontal;
  if (chain_name == "vertical") {
    chain_direction = ChainDirection::Vertical;
  } else if (chain_name != "horizontal") {
    server_.send(400, "text/plain", "Invalid chain direction");
    return;
  }
  const String wiring_name = doc["wiring"] | "rows-serpentine";
  WiringMode wiring = WiringMode::RowsSerpentine;
  if (wiring_name == "rows") {
    wiring = WiringMode::Rows;
  } else if (wiring_name == "columns") {
    wiring = WiringMode::Columns;
  } else if (wiring_name == "columns-serpentine") {
    wiring = WiringMode::ColumnsSerpentine;
  } else if (wiring_name != "rows-serpentine") {
    server_.send(400, "text/plain", "Invalid wiring mode");
    return;
  }

  if (!scene_manager_.setLayout(panel_rows, panel_columns, panel_count,
                                chain_direction, wiring)) {
    server_.send(400, "text/plain", "Panel dimensions or count exceed the supported LED buffer");
    return;
  }

  server_.send(200, "application/json", "{\"ok\":true}");
}

void WebServerController::handleJsonState() {
  const auto& scene = scene_manager_.currentScene();

  JsonDocument doc;
  doc["scene"] = static_cast<int>(scene.type);
  doc["brightness"] = scene.brightness;
  doc["panel_rows"] = scene_manager_.layout().panel_rows;
  doc["panel_columns"] = scene_manager_.layout().panel_columns;
  doc["panel_count"] = scene_manager_.layout().panel_count;
  doc["rows"] = scene_manager_.layout().rows();
  doc["columns"] = scene_manager_.layout().columns();
  doc["chain"] = scene_manager_.layout().chain_direction ==
                         ChainDirection::Vertical
                     ? "vertical"
                     : "horizontal";
  switch (scene_manager_.layout().wiring) {
    case WiringMode::Rows:
      doc["wiring"] = "rows";
      break;
    case WiringMode::Columns:
      doc["wiring"] = "columns";
      break;
    case WiringMode::ColumnsSerpentine:
      doc["wiring"] = "columns-serpentine";
      break;
    case WiringMode::RowsSerpentine:
    default:
      doc["wiring"] = "rows-serpentine";
      break;
  }

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

      <label for="panelRows">Panel height (LEDs)</label>
      <input id="panelRows" type="number" min="1" value="8" />

      <label for="panelColumns">Panel width (LEDs)</label>
      <input id="panelColumns" type="number" min="1" value="32" />

      <label for="panelCount">Panels connected together</label>
      <input id="panelCount" type="number" min="1" max="4" value="1" />

      <label for="chain">Panel chain direction</label>
      <select id="chain">
        <option value="horizontal">Side by side</option>
        <option value="vertical">Stacked vertically</option>
      </select>

      <label for="rows">Total layout rows</label>
      <input id="rows" type="number" value="8" disabled />

      <label for="columns">Total layout columns</label>
      <input id="columns" type="number" value="32" disabled />
      <input id="rows" type="number" min="1" value="8" />

      <label for="columns">Panel columns</label>
      <input id="columns" type="number" min="1" value="32" />

      <label for="wiring">Connection order</label>
      <select id="wiring">
        <option value="rows-serpentine">Rows, alternating direction</option>
        <option value="rows">Rows, same direction</option>
        <option value="columns-serpentine">Columns, alternating direction</option>
        <option value="columns">Columns, same direction</option>
      </select>

      <button id="applyBtn" type="button">Apply</button>
      <div id="status" class="status">Loading...</div>
    </div>

    <script>
      const sceneEl = document.getElementById('scene');
      const brightnessEl = document.getElementById('brightness');
      const rowsEl = document.getElementById('rows');
      const columnsEl = document.getElementById('columns');
      const panelRowsEl = document.getElementById('panelRows');
      const panelColumnsEl = document.getElementById('panelColumns');
      const panelCountEl = document.getElementById('panelCount');
      const chainEl = document.getElementById('chain');
      const wiringEl = document.getElementById('wiring');
      const statusEl = document.getElementById('status');

      function updateTotals() {
        const panelRows = Number(panelRowsEl.value) || 0;
        const panelColumns = Number(panelColumnsEl.value) || 0;
        const panelCount = Number(panelCountEl.value) || 0;
        rowsEl.value = chainEl.value === 'vertical' ? panelRows * panelCount : panelRows;
        columnsEl.value = chainEl.value === 'horizontal' ? panelColumns * panelCount : panelColumns;
      }

      async function fetchState() {
        const response = await fetch('/api/state');
        const state = await response.json();
        sceneEl.value = ['test', 'rainbow', 'chase', 'pulse', 'solid'][state.scene] || 'test';
        brightnessEl.value = state.brightness || 128;
        panelRowsEl.value = state.panel_rows;
        panelColumnsEl.value = state.panel_columns;
        panelCountEl.value = state.panel_count;
        chainEl.value = state.chain;
        rowsEl.value = state.rows;
        columnsEl.value = state.columns;
        wiringEl.value = state.wiring;
        statusEl.textContent = 'Connected';
      }

      [panelRowsEl, panelColumnsEl, panelCountEl, chainEl].forEach((element) => {
        element.addEventListener('input', updateTotals);
        element.addEventListener('change', updateTotals);
      });

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
          body: JSON.stringify({
            panel_rows: Number(panelRowsEl.value),
            panel_columns: Number(panelColumnsEl.value),
            panel_count: Number(panelCountEl.value),
            chain: chainEl.value,
            wiring: wiringEl.value
          })
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
