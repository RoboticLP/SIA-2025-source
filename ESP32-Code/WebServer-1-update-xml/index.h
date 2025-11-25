#ifndef WEBPAGE_H
#define WEBPAGE_H

const char* webpage_main = R"=====(
<!DOCTYPE HTML>
<html>
  <head>
    <link rel='icon' href='data:,'>
    <title>Controlpanel</title>
    <style>
	form {
	  text-align: center;
	}
    button {
      width: 75%;
      height: 200px;
      margin-left: 30px;
	    font-size: 60px;
    }
    #brightness-slider {
      width: 75%;
      height: 55px;
      margin-left: 30px;
      outline: none;
    }
    </style>
  </head>
  <body onload="process()">
	  <button id="btn0" onclick="handleButtonPress0()">Turn on</button>
    <br>
    <input type="range" id="brightness-slider" min="0" max="255" value = "0" width = "0%" oninput="handleUpdateSlider(this.value)"/>
  </body>

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


  // Action handlers \\  
  function handleButtonPress0() {
	  var xhttp = new XMLHttpRequest();

    xhttp.open("PUT", "BUTTON_0", false);
    xhttp.send();
  }

  function handleUpdateSlider(value) { // this only fires if the user interacts, not a .value change by js
    var xhttp = new XMLHttpRequest();

    xhttp.open("PUT", "BRIGHTNESS_SLIDER?value=" + value, false);
    xhttp.send();
  }

  // runs every time the server sends updates
  function response() {
    var xmlResponse;
    var xml_tag_data;
    var message_data;
    
    if (this.responseXML != null) {
      xmlResponse = xmlHttp.responseXML;

      // update button state
      xml_tag_data = xmlResponse.getElementsByTagName("B0");
      message_data = xml_tag_data[0].firstChild.nodeValue;
      if (message_data == 0) {
        document.getElementById("btn0").innerHTML = "Turn on";
      } else {
        document.getElementById("btn0").innerHTML = "Turn off";
      }

      // update slider value
      xml_tag_data = xmlResponse.getElementsByTagName("SL_V");
      message_data = xml_tag_data[0].firstChild.nodeValue;
      document.getElementById("brightness-slider").value = message_data;
    }
  }

  function process() {
    if(xmlHttp.readyState==0 || xmlHttp.readyState==4) {
      xmlHttp.open("PUT","xml",true); // to activate xml communication on server
      xmlHttp.onreadystatechange=response;
      xmlHttp.send(null);
    }       
      // you may have to play with this value, big pages need more porcessing time, and hence
      // a longer timeout
      setTimeout("process()", 100);
  }
  </script>
</html>
)=====";

#endif