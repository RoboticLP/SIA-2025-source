#ifndef WEBPAGE_H
#define WEBPAGE_H

const char* webpage_main = R"=====(
<!DOCTYPE HTML>
<html>
  <head>
    <meta charset="UTF-8">
    <meta name="apple-mobile-web-app-capable" content="yes">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Flipper Adminpanel</title>

    <style>
      * { margin: 0; padding: 0; box-sizing: border-box; }
      body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif; padding: 20px; background: #f5f5f5; }
      .section-container { background: white; padding: 20px; border-radius: 12px; box-shadow: 0 2px 8px rgba(0,0,0,0.08); margin-bottom: 16px; }
      .controls-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 20px; }
      .section-title { font-size: 14px; font-weight: 600; color: #333; margin-bottom: 16px; }
      .btn { width: 100%; height: 50px; border: none; border-radius: 10px; font-size: 16px; font-weight: 500; cursor: pointer; transition: all 0.2s ease; font-family: inherit; }
      .btn-primary { background: #5c6bc0; color: white; }
      .btn-primary:hover { background: #4a5bb5; }
      .btn-primary:active { background: #3949ab; transform: translateY(1px); }
      .btn-secondary { background: white; color: #5c6bc0; border: 2px solid #5c6bc0; }
      .btn-secondary:hover { background: #f5f7ff; }
      .btn-secondary:active { background: #e8ebf9; transform: translateY(1px); }
      .btn-danger { background: white; color: #c62828; border: 2px solid #c62828; }
      .btn-danger:hover { background: #fff5f5; }
      .btn-danger:active { background: #ffebee; transform: translateY(1px); }
      .btn-small { height: auto; padding: 8px 16px; font-size: 13px; width: auto; }
      .log-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 16px; }
      .log-entry { font-size: 13px; padding: 10px 12px; border-radius: 6px; margin-bottom: 8px; display: flex; justify-content: space-between; align-items: center; gap: 12px; }
      .log-entry:last-child { margin-bottom: 0; }
      .log-error { color: #c62828; background: #ffebee; border-left: 3px solid #c62828; }
      .log-info { color: #1565c0; background: #e3f2fd; border-left: 3px solid #1565c0; }
      .log-timestamp { color: #9e9e9e; font-size: 11px; white-space: nowrap; flex-shrink: 0; opacity: 0.8; }
      .setting-item { margin-bottom: 16px; }
      .setting-item:last-child { margin-bottom: 0; }
      .section-subtitle { font-size: 14px; font-weight: 600; color: #333; margin-bottom: 8px; display: block; }
      .toggle-container { font-size: 14px; color: #333; display: flex; align-items: center; gap: 12px; margin-bottom: 16px; }
      .toggle { position: relative; width: 50px; height: 28px; background: #e0e0e0; border-radius: 14px; cursor: pointer; transition: background 0.3s; }
      .toggle.active { background: #5c6bc0; }
      .toggle-thumb { position: absolute; top: 3px; left: 3px; width: 22px; height: 22px; background: white; border-radius: 50%; transition: left 0.3s; box-shadow: 0 2px 4px rgba(0,0,0,0.2); }
      .toggle.active .toggle-thumb { left: 25px; }
      .number-input { width: 100%; padding: 10px 12px; border: 1px solid #e0e0e0; border-radius: 8px; font-size: 14px; font-family: inherit; outline: none; transition: border-color 0.2s; }
      .number-input:focus { border-color: #5c6bc0; }
      .input-container { display: flex; flex-direction: column; gap: 8px; }
      .input-label { font-size: 14px; color: #666; font-weight: 500; }
      .slider-container { font-size: 14px; color: #333; display: flex; align-items: center; gap: 12px; margin-bottom: 16px; }
      .slider { flex: 1; height: 6px; border-radius: 3px; background: #e0e0e0; outline: none; -webkit-appearance: none; appearance: none; cursor: pointer; }
      .slider::-webkit-slider-thumb { -webkit-appearance: none; appearance: none; width: 20px; height: 20px; border-radius: 50%; background: #5c6bc0; cursor: pointer; box-shadow: 0 2px 4px rgba(0,0,0,0.2); }
      .slider::-moz-range-thumb { width: 20px; height: 20px; border-radius: 50%; background: #5c6bc0; cursor: pointer; border: none; box-shadow: 0 2px 4px rgba(0,0,0,0.2); }
      .slider-value { min-width: 50px; text-align: right; font-weight: 500; color: #5c6bc0; }
    </style>
  </head>
  
  <body>

    <div class="section-container">
      <div class="log-header">
        <div class="section-title">Flipper Adminpanel</div>
        <button class="btn btn-secondary btn-small" onclick="window.location.reload();">Reload window</button>
      </div>
      <label id="last-update-status" class="input-label">xX Update Text here Xx</label>
    </div>

    <div class="section-container">
      <div class="section-title">Module status</div>
      <div class="controls-grid">
        <label id="module2-status" class="input-label">Slingshots (Module 2): </label>
        <label id="module3-status" class="input-label">Bumper Tower (Module 3): </label>
        <label id="module4-status" class="input-label">Targets (Module 4): </label>
        <label id="module5-status" class="input-label">Lichter: </label>
      </div>
    </div>
    
    <div class="section-container">
      <div class="section-title">Settings</div>
      
      <div class="section-subtitle">Game settings</div>
      <div class="controls-grid">
        <div class="input-container">
          <label class="input-label">Multiplier amount</label>
          <input type="number" id="multiplier-amount" class="number-input" min="1" max="100" step="0.5" value="1.0" placeholder="1.0">
        </div>
        <div class="input-container">
          <label class="input-label">Point amount bumper tower</label>
          <input type="number" id="point-amount-bumper" class="number-input" min="1" max="50000" step="10" value="50" placeholder="1">
        </div>
        <div class="input-container">
          <label class="input-label">Point amount slingshot</label>
          <input type="number" id="point-amount-slingshot" class="number-input" min="1" max="50000" step="10" value="60" placeholder="1">
        </div>
        <div class="input-container">
          <label class="input-label">Point amount targets</label>
          <input type="number" id="point-amount-targets" class="number-input" min="1" max="50000" step="10" value="10" placeholder="1">
        </div>
      </div>

      <div class="section-subtitle">Lighting Controls</div>
      <div class="controls-grid">
        <div class="toggle-container">
          <div id="strobe-toggle" class="toggle active" onclick="toggleSetting(this);">
            <div class="toggle-thumb"></div>
          </div>
          Lighting enabled
        </div>
        <!-- FIX: type="ran+ge" → type="range" -->
        <div class="slider-container">
          <label class="input-label" style="min-width: 120px;">Light Effect Speed</label>
          <input type="range" id="light-speed-slider" class="slider" min="0" max="200" step="1" value="100">
          <span class="slider-value" id="light-speed-display">100</span>
        </div>
      </div>

      <div class="section-subtitle">Music Controls</div>
      <div class="controls-grid">
        <!-- FIX: value="15" damit Startwert mit Display übereinstimmt -->
        <div class="slider-container">
          <label class="input-label" style="min-width: 120px;">Music Volume</label>
          <input type="range" id="music-volume-slider" class="slider" min="0" max="30" step="1" value="15">
          <span class="slider-value" id="music-volume-display">15</span>
        </div>
      </div>

      <div class="setting-item">
        <button class="btn btn-danger" onclick="resetGame()">Reset Game</button>
      </div>

      <button class="btn btn-primary" onclick="applySettings()">Apply Settings</button>
    </div>

    <div class="section-container">
      <div class="log-header">
        <div class="section-title">Log</div>
        <div>
          <button class="btn btn-danger btn-small" onclick="deleteInfoLogs()">Delete info logs</button>
          <button class="btn btn-secondary btn-small" onclick="exportLogs()">Export Logs</button>
        </div>
      </div>
      <div id="log-container"></div>
    </div>

    <script type="text/javascript">
      const logType = { error: 'error', info: 'info' };

      let lastUpdate = -1;
      function handleLastUpdateShow() {
        if (lastUpdate == -1) {
          document.getElementById("last-update-status").innerHTML = "No I\u00B2C update recieved yet";
        } else {
          let updateTimePassed = Date.now() - lastUpdate;
          document.getElementById("last-update-status").innerHTML = "Last I\u00B2C update: " + Math.floor(updateTimePassed / 1000) + " seconds ago";
        }
      }
      setInterval(handleLastUpdateShow, 1000);

      function addLog(type, message, timestamp = new Date().toLocaleTimeString('de-DE')) {
        const logContainer = document.getElementById('log-container');
        const logEntry = document.createElement('div');
        logEntry.className = 'log-entry log-' + type;
        const messageSpan = document.createElement('span');
        messageSpan.textContent = message;
        const timestampSpan = document.createElement('span');
        timestampSpan.className = 'log-timestamp';
        timestampSpan.textContent = timestamp;
        logEntry.appendChild(messageSpan);
        logEntry.appendChild(timestampSpan);
        logContainer.prepend(logEntry);
      }

      function toggleSetting(element) {
        element.classList.toggle('active');
      }

      document.getElementById('light-speed-slider').addEventListener('input', function() {
        document.getElementById('light-speed-display').textContent = this.value;
      });

      // FIX: parseInt ohne .toFixed(2) — Volume ist ganzzahlig
      document.getElementById('music-volume-slider').addEventListener('input', function() {
        document.getElementById('music-volume-display').textContent = parseInt(this.value);
      });

      function applySettings() {
        const multiplierAmount = document.getElementById('multiplier-amount').value;
        const points_bumper = document.getElementById('point-amount-bumper').value;
        const points_slingshot = document.getElementById('point-amount-slingshot').value;
        const points_targets = document.getElementById('point-amount-targets').value;
        const lights_enabled = document.getElementById('strobe-toggle').classList.contains('active') ? 1 : 0;
        const lightSpeed = document.getElementById('light-speed-slider').valueAsNumber;
        const volume = document.getElementById('music-volume-slider').value;

        var xhttp = new XMLHttpRequest();
        xhttp.open("PUT",
          "SETTINGS?multiplierAmount=" + multiplierAmount +
          "&points_bumper=" + points_bumper +
          "&points_slingshot=" + points_slingshot +
          "&points_targets=" + points_targets +
          "&lights_enabled=" + lights_enabled +
          "&light_speed=" + lightSpeed +
          "&volume=" + volume,
          true);
        xhttp.send();
        addLog(logType.info, 'Settings have been sent.');
      }

      function resetGame() {
        if (confirm('Are you sure you want to reset the game?')) {
          var xhttp = new XMLHttpRequest();
          xhttp.open("PUT", "RESET_GAME", true);
          xhttp.send();
          addLog(logType.info, 'Game reset request has been sent.');
        }
      }

      function deleteInfoLogs() {
        if (confirm('Are you sure you want to delete every info log?')) {
          document.querySelectorAll('.log-entry').forEach(entry => {
            if (entry.classList.contains('log-info')) entry.remove();
          });
        }
      }

      function exportLogs() {
        const logEntries = document.querySelectorAll('.log-entry');
        let exportText = 'Device Logs Export\n' + '='.repeat(50) + '\n\n';
        logEntries.forEach(entry => {
          const message = entry.querySelector('span:first-child').textContent;
          const timestamp = entry.querySelector('.log-timestamp').textContent;
          const type = entry.classList.contains('log-error') ? '[ERROR]' : '[INFO]';
          exportText += `${message} ${type} <-- ${timestamp}\n`;
        });
        const blob = new Blob([exportText], { type: 'text/plain' });
        const url = window.URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        const dateStr = new Date().toISOString().slice(0, 19).replace(/:/g, '-');
        a.download = `device-logs-${dateStr}.txt`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        window.URL.revokeObjectURL(url);
        addLog(logType.info, 'Logs exported successfully');
      }

      var xmlHttp;
      if (window.XMLHttpRequest) {
        xmlHttp = new XMLHttpRequest();
      } else {
        xmlHttp = new ActiveXObject("Microsoft.XMLHTTP");
      }

      if (typeof(EventSource) !== "undefined") {
        addLog(logType.info, 'Your Browser supports SSE. Opening connection...');
        const eventSource = new EventSource('/sse');
        eventSource.onmessage = function(event) { onSSEmessage(event); }
        eventSource.onopen = function() { addLog(logType.info, 'SSE connection established'); }
        eventSource.onerror = function() { addLog(logType.error, 'SSE connection lost, retrying...'); }
      } else {
        addLog(logType.error, 'Your Browser doesn\'t support SSE! Cannot open connection for updating data.');
      }

      function onSSEmessage(event) {
        const parser = new DOMParser();
        const xmlData = parser.parseFromString(event.data, 'text/xml');
        var xml_tag_data, message_data;

        xml_tag_data = xmlData.getElementsByTagName("lastUpdateRecieved");
        if (xml_tag_data && xml_tag_data.length > 0) lastUpdate = Date.now();

        var moduleNames = {
          2: "Slingshots (Module 2)",
          3: "Bumper Tower (Module 3)",
          4: "Targets (Module 4)",
          5: "Lichter (Module 5)"
        };

        [2, 3, 4, 5].forEach(function(n) {
          xml_tag_data = xmlData.getElementsByTagName("M" + n + "S");
          if (xml_tag_data && xml_tag_data.length > 0) {
            message_data = xml_tag_data[0].firstChild.nodeValue;
            document.getElementById("module" + n + "-status").innerHTML =
              moduleNames[n] + ": " + (message_data == 0 ? "❌" : "✅");
          }
        });

        var logTypeVal, logMessage;
        xml_tag_data = xmlData.getElementsByTagName("log");
        if (xml_tag_data && xml_tag_data.length > 0) {
          logTypeVal = xml_tag_data[0].getElementsByTagName("logType");
          logMessage = xml_tag_data[0].getElementsByTagName("logMessage");
          if (logTypeVal.length > 0 && logMessage.length > 0) {
            addLog(logTypeVal[0].firstChild.nodeValue, logMessage[0].firstChild.nodeValue);
          }
        }
      }
    </script>
  </body>
</html>
)=====";

#endif