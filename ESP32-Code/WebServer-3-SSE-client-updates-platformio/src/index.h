#ifndef WEBPAGE_H
#define WEBPAGE_H

const char* webpage_main = R"=====(
<!DOCTYPE HTML>
<html>
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Control Panel</title>

    <style>
      * {
        margin: 0;
        padding: 0;
        box-sizing: border-box;
      }

      body {
        font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
        padding: 20px;
        background: #f5f5f5;
      }

      .section-container {
        background: white;
        padding: 20px;
        border-radius: 12px;
        box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
        margin-bottom: 16px;
      }

       /* Grid Layout - 2 Spalten für Controls */
      .controls-grid {
        display: grid;
        grid-template-columns: 1fr 1fr;
        gap: 20px;
      }

      .section-title {
        font-size: 14px;
        font-weight: 600;
        color: #333;
        margin-bottom: 16px;
      }

      /* Button Styles */
      .btn {
        width: 100%;
        height: 50px;
        border: none;
        border-radius: 10px;
        font-size: 16px;
        font-weight: 500;
        cursor: pointer;
        transition: all 0.2s ease;
        font-family: inherit;
      }

      .btn-primary {
        background: #5c6bc0;
        color: white;
      }

      .btn-primary:hover {
        background: #4a5bb5;
      }

      .btn-primary:active {
        background: #3949ab;
        transform: translateY(1px);
      }

      .btn-secondary {
        background: white;
        color: #5c6bc0;
        border: 2px solid #5c6bc0;
      }

      .btn-secondary:hover {
        background: #f5f7ff;
      }

      .btn-secondary:active {
        background: #e8ebf9;
        transform: translateY(1px);
      }

      .btn-danger {
        background: white;
        color: #c62828;
        border: 2px solid #c62828;
      }

      .btn-danger:hover {
        background: #fff5f5;
      }

      .btn-danger:active {
        background: #ffebee;
        transform: translateY(1px);
      }

      .btn-small {
        height: auto;
        padding: 8px 16px;
        font-size: 13px;
        width: auto;
      }

      /* Log Styles */
      .log-header {
        display: flex;
        justify-content: space-between;
        align-items: center;
        margin-bottom: 16px;
      }

      .log-entry {
        font-size: 13px;
        padding: 10px 12px;
        border-radius: 6px;
        margin-bottom: 8px;
        display: flex;
        justify-content: space-between;
        align-items: center;
        gap: 12px;
      }

      .log-entry:last-child {
        margin-bottom: 0;
      }

      .log-error {
        color: #c62828;
        background: #ffebee;
        border-left: 3px solid #c62828;
      }

      .log-info {
        color: #1565c0;
        background: #e3f2fd;
        border-left: 3px solid #1565c0;
      }

      .log-timestamp {
        color: #9e9e9e;
        font-size: 11px;
        white-space: nowrap;
        flex-shrink: 0;
        opacity: 0.8;
      }

      /* Settings Styles */
      .setting-item {
        margin-bottom: 16px;
      }

      .setting-item:last-child {
        margin-bottom: 0;
      }

      .section-subtitle {
        font-size: 14px;
        font-weight: 600;
        color: #333;
        margin-bottom: 8px;
        display: block;
      }

      .toggle-container {
        font-size: 14px;
        color: #333;
        display: flex;
        align-items: center;
        gap: 12px;
        margin-bottom: 16px;
      }

      .toggle {
        position: relative;
        width: 50px;
        height: 28px;
        background: #e0e0e0;
        border-radius: 14px;
        cursor: pointer;
        transition: background 0.3s;
      }

      .toggle.active {
        background: #5c6bc0;
      }

      .toggle-thumb {
        position: absolute;
        top: 3px;
        left: 3px;
        width: 22px;
        height: 22px;
        background: white;
        border-radius: 50%;
        transition: left 0.3s;
        box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2);
      }

      .toggle.active .toggle-thumb {
        left: 25px;
      }

      .text-input {
        width: 100%;
        padding: 10px 12px;
        border: 1px solid #e0e0e0;
        border-radius: 8px;
        font-size: 14px;
        font-family: inherit;
        outline: none;
        transition: border-color 0.2s;
      }

      .text-input:focus {
        border-color: #5c6bc0;
      }

      /* Slider Styles */
      .slider-label {
        font-size: 14px;
        font-weight: 500;
        color: #333;
        margin-bottom: 12px;
        margin-top: 24px;
      }

      #brightness-slider {
        -webkit-appearance: none;
        appearance: none;
        width: 100%;
        height: 8px;
        border-radius: 4px;
        background: #e0e0e0;
        outline: none;
        cursor: pointer;
        touch-action: none;
      }

      #brightness-slider::-webkit-slider-thumb {
        -webkit-appearance: none;
        appearance: none;
        width: 22px;
        height: 22px;
        border-radius: 50%;
        background: white;
        cursor: pointer;
        box-shadow: 0 2px 6px rgba(0, 0, 0, 0.2);
        border: 2px solid #5c6bc0;
        transition: all 0.2s ease;
      }

      #brightness-slider::-webkit-slider-thumb:hover {
        transform: scale(1.1);
      }

      #brightness-slider::-webkit-slider-thumb:active {
        transform: scale(1.05);
      }

      #brightness-slider::-moz-range-thumb {
        width: 22px;
        height: 22px;
        border-radius: 50%;
        background: white;
        cursor: pointer;
        box-shadow: 0 2px 6px rgba(0, 0, 0, 0.2);
        border: 2px solid #5c6bc0;
        transition: all 0.2s ease;
      }

      #brightness-slider::-moz-range-thumb:hover {
        transform: scale(1.1);
      }

      #brightness-slider::-moz-range-thumb:active {
        transform: scale(1.05);
      }
    </style>
  </head>
  
  <body onload="updateSliderProgress();">

    <!-- Test LED Controls -->
    <div class="section-container">
      <div class="section-title">Test Controls</div>
       <div class="toggle-container">
          <div id="led-toggle" class="toggle" onclick="toggleSetting(this); handleButtonPress0()">
            <div class="toggle-thumb"></div>
          </div>
          LED 1
        </div>
      <div class="slider-label">Brightness LED 2</div>
      <input type="range" id="brightness-slider" min="0" max="255" value="0" oninput="handleSliderInput(this.value)"/>
    </div>

    <!-- Lighting Settings (later) -->
    <div class="section-container">
      <div class="section-title">Lighting Controls</div>

      <div class="section-subtitle">Effects</div>
      <div class="controls-grid">
        <div class="toggle-container">
          <div id="led-toggle" class="toggle" onclick="toggleSetting(this);">
            <div class="toggle-thumb"></div>
          </div>
          Rainbow
        </div>
        <div class="toggle-container">
          <div id="led-toggle" class="toggle" onclick="toggleSetting(this);">
            <div class="toggle-thumb"></div>
          </div>
          Strobe
        </div>
      </div>

      <div class="section-subtitle">Some other section</div>
    </div>

    <!-- Settings -->
    <div class="section-container">
      <div class="section-title">Settings (not yet working)</div>
      
      <div class="setting-item">
        <label class="section-subtitle">Auto Mode</label>
        <div class="toggle-container">
          <div id="auto-mode-toggle" class="toggle" onclick="toggleSetting(this)">
            <div class="toggle-thumb"></div>
          </div>
        </div>
      </div>

      <div class="setting-item">
        <label class="section-subtitle">Device Name</label>
        <input type="text" id="device-name" class="text-input" placeholder="Enter device name" value="My Device">
      </div>

      <div class="setting-item">
        <button class="btn btn-danger" onclick="resetGame()">Reset Game</button>
      </div>

      <button class="btn btn-primary" onclick="applySettings()">Apply Settings</button>
    </div>

    <div class="section-container">
      <div class="log-header">
        <div class="section-title">Log</div>
        <button class="btn btn-secondary btn-small" onclick="exportLogs()">Export Logs</button>
      </div>
      <div id="log-container"></div>
    </div>

    <script type="text/javascript">
      
      //**     Log related stuff     **//

      const logType = {
        error: 'error',
        info: 'info'
      };

      //** Fügt neuen Log ganz oben hinzu
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
  
        // if (type === logType.info) {
        //   setTimeout(() => {
        //     logEntry.remove();
        //   }, 20000);
        // }
      }
  
      //** Für Toggle-Buttons, um deren Aktiv-Status umzuschalten
      function toggleSetting(element) {
        element.classList.toggle('active');
      }
  
      //** Apply settings
      // Alle eingegebenen Einstellungen auf ihren Weg schicken
      function applySettings() { // bisher nur von KI gekochter shit
        const autoMode = document.getElementById('auto-mode-toggle').classList.contains('active');
        const deviceName = document.getElementById('device-name').value;
          
        var xhttp = new XMLHttpRequest();
        xhttp.open("PUT", "SETTINGS?autoMode=" + autoMode + "&deviceName=" + deviceName, true);
        xhttp.send();
          
        addLog(logType.info, 'Settings have been sent.');
      }
  
      // Spiel zurücksetzen
      function resetGame() {
        if (confirm('Are you sure you want to reset the game?')) {
          var xhttp = new XMLHttpRequest();
          xhttp.open("PUT", "RESET_GAME", true);
          xhttp.send();
          addLog(logType.info, 'Game reset request has been sent.');
        }
      }
  
      //** Logs exportieren
      function exportLogs() {
        const logEntries = document.querySelectorAll('.log-entry');
        let exportText = 'Device Logs Export\n';
        exportText += '='.repeat(50) + '\n\n';
        
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
      //** Beispiel Logs
      // addLog(logType.error, 'Connection timeout - Failed to reach device at 192.168.1.100');
      // addLog(logType.info, 'Device connected successfully');
      
      //**     Kommunikation     **//
      var xmlHttp = createXmlHttpObject();
      
      function createXmlHttpObject(){
        if(window.XMLHttpRequest){
          xmlHttp=new XMLHttpRequest();
        }
        else{
          xmlHttp=new ActiveXObject("Microsoft.XMLHTTP");
        }
        return xmlHttp;
      }
      
      //**     User input handlers     **//
      function handleButtonPress0() {
        var xhttp = new XMLHttpRequest();
        xhttp.open("PUT", "BUTTON_0", true);
        xhttp.send();
      }
      
      let sliderTimeout;
      function handleSliderInput(value) {
        requestAnimationFrame(() => updateSliderProgress());
        clearTimeout(sliderTimeout);
        sliderTimeout = setTimeout(() => {
          handleUpdateSlider(value);
        }, 100);
      }
      function handleUpdateSlider(value) {
        var xhttp = new XMLHttpRequest();
        xhttp.open("PUT", "BRIGHTNESS_SLIDER?value=" + value, true);
        xhttp.send();
      }
      function updateSliderProgress() {
        const slider = document.getElementById('brightness-slider');
        const value = (slider.value / slider.max) * 100;
        slider.style.background = `linear-gradient(to right, #5c6bc0 ${value}%, #e0e0e0 ${value}%)`;
      }
      
      //**     SSE stuff (Server send events)     **//
      if(typeof(EventSource) !== "undefined") {
        addLog(logType.info, 'Your Browser supports SSE. Opening connection...');
        const eventSource = new EventSource('/sse');
        eventSource.onmessage = function(event) {
          onSSEmessage(event);
        }
        
        eventSource.onopen = function() {
          addLog(logType.info, 'SSE connection established');
        }
        eventSource.onerror = function() {
          addLog(logType.error, 'SSE connection lost, retrying...');
        }
      } else {
        addLog(logType.error, 'Your Browser doensn\'t support SSE! Cannot open connection for updating data.');
      }

      function onSSEmessage(event) {
        const parser = new DOMParser();
        const xmlData = parser.parseFromString(event.data, 'text/xml');
        var xml_tag_data;
        var message_data;
        
        // toggle update
        xml_tag_data = xmlData.getElementsByTagName("B0");
        if (xml_tag_data && xml_tag_data.length > 0) {
          message_data = xml_tag_data[0].firstChild.nodeValue;
          const ledToggle = document.getElementById("led-toggle");
          if (message_data == 0) {
            ledToggle.classList.remove("active"); // disable toggle (class kann nur 1x pro element eingetragen sein -> mehrfaches ausführen fügt max 1 hinzu)
          } else {
            ledToggle.classList.add("active"); // enable toggle
          }
        }
          
        // slider update
        xml_tag_data = xmlData.getElementsByTagName("SL_V");
        if (xml_tag_data && xml_tag_data.length > 0) {
          message_data = xml_tag_data[0].firstChild.nodeValue;
          document.getElementById("brightness-slider").value = message_data;
          updateSliderProgress();
        }

        // add log
        var logType;
        var logMessage;
        var logTimestamp;
        xml_tag_data = xmlData.getElementsByTagName("log");
        if (xml_tag_data && xml_tag_data.length > 0) {
          logType = xml_tag_data[0].getElementsByTagName("logType");
          logMessage = xml_tag_data[0].getElementsByTagName("logMessage");
          logTimestamp = xml_tag_data[0].getElementsByTagName("logTimestamp");
          if (logType.length > 0 && logMessage.length > 0 && logTimestamp.length > 0) {
            logType = logType[0].firstChild.nodeValue;
            logMessage = logMessage[0].firstChild.nodeValue;
            logTimestamp = logTimestamp[0].firstChild.nodeValue;
            addLog(logType, logMessage, logTimestamp);
          }
        }
      }
    </script>
  </body>
</html>
)=====";

#endif
