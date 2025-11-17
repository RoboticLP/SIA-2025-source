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

      .controls-container {
        background: white;
        padding: 20px;
        border-radius: 12px;
        box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
        margin-bottom: 16px;
      }

      .error-container {
        background: white;
        padding: 20px;
        border-radius: 12px;
        box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
      }

      .error-title {
        font-size: 14px;
        font-weight: 600;
        color: #333;
        margin-bottom: 12px;
      }

      .log-entry {
        font-size: 13px;
        padding: 10px 12px;
        border-radius: 6px;
        margin-bottom: 8px;
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

      #btn0 {
        width: 100%;
        height: 50px;
        border: none;
        border-radius: 10px;
        font-size: 16px;
        font-weight: 500;
        color: white;
        background: #5c6bc0;
        cursor: pointer;
        margin-bottom: 24px;
        transition: all 0.2s ease;
      }

      #btn0:active {
        background: #4a5bb5;
        transform: translateY(1px);
      }

      .slider-label {
        font-size: 14px;
        font-weight: 500;
        color: #333;
        margin-bottom: 12px;
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
    </style>
    <script>
      function updateSliderProgress() {
        const slider = document.getElementById('brightness-slider');
        const value = (slider.value / slider.max) * 100;
        slider.style.background = `linear-gradient(to right, #5c6bc0 ${value}%, #e0e0e0 ${value}%)`;
      }

      function addLog(type, message) {
        const logContainer = document.getElementById('log-container');
        const logEntry = document.createElement('div');
        logEntry.className = 'log-entry log-' + type;
        logEntry.textContent = message;
        logContainer.appendChild(logEntry);
      }
    </script>
  </head>
  
  <body onload="process(); updateSliderProgress();">
    <div class="controls-container">
      <button id="btn0" onclick="handleButtonPress0()">Turn on</button>
      <div class="slider-label">LED Brightness</div>
      <input type="range" id="brightness-slider" min="0" max="255" value="0" oninput="handleSliderInput(this.value)"/>
    </div>

    <div class="error-container">
      <div class="error-title">Log</div>
      <div id="log-container">
        <div class="log-entry log-info">Device connected successfully</div>
        <div class="log-entry log-error">Connection timeout - Failed to reach device at 192.168.1.100</div>
      </div>
    </div>

    <script type="text/javascript">
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

      function handleButtonPress0() {
        var xhttp = new XMLHttpRequest();
        xhttp.open("PUT", "BUTTON_0", false);
        xhttp.send();
      }

      function handleUpdateSlider(value) {
        var xhttp = new XMLHttpRequest();
        xhttp.open("PUT", "BRIGHTNESS_SLIDER?value=" + value, false);
        xhttp.send();
      }

      // Debounce slider updates for better performance
      let sliderTimeout;
      function handleSliderInput(value) {
        updateSliderProgress();
        clearTimeout(sliderTimeout);
        sliderTimeout = setTimeout(() => {
          handleUpdateSlider(value);
        }, 50);
      }

      function response() {
        var xmlResponse;
        var xml_tag_data;
        var message_data;
        
        if (this.responseXML != null) {
          xmlResponse = xmlHttp.responseXML;

          xml_tag_data = xmlResponse.getElementsByTagName("B0");
          message_data = xml_tag_data[0].firstChild.nodeValue;
          if (message_data == 0) {
            document.getElementById("btn0").innerHTML = "Turn on";
          } else {
            document.getElementById("btn0").innerHTML = "Turn off";
          }

          xml_tag_data = xmlResponse.getElementsByTagName("SL_V");
          message_data = xml_tag_data[0].firstChild.nodeValue;
          document.getElementById("brightness-slider").value = message_data;
          updateSliderProgress();
        }
      }

      function process() {
        if(xmlHttp.readyState==0 || xmlHttp.readyState==4) {
          xmlHttp.open("PUT","xml",true);
          xmlHttp.onreadystatechange=response;
          xmlHttp.send(null);
        }       
        setTimeout("process()", 1000);
      }
    </script>
  </body>
</html>
)=====";

#endif