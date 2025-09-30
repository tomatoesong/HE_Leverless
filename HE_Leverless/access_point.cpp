#include <cstddef>
#include <WiFi.h>
#include <WebServer.h>
#include "config.h"
#include "trigger_mode.h"
#include "usb_helper.h"
#include "calibration.h"

const char* ssid = "HE_Leverless-AP";
const char* password = "streetfighter6";

uint8_t currentButtonIndex = 0;

// Static IP configuration for Access Point
IPAddress local_ip(192, 168, 10, 1);  // ESP32 AP IP
IPAddress gateway(192, 168, 10, 1);   // Usually same as local_ip in AP mode
IPAddress subnet(255, 255, 255, 0);   // Standard subnet mask

WebServer server(80);

const char html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Rapid Trigger Settings</title>
  <style>
    body {
      margin: 0;
      height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      background: #f0f0f0;
      font-family: sans-serif;
      gap: 3vh;
    }

    .grey-box {
    	position: relative;
  		width: 80vw;             /* Responsive width */
  		height: 50vh;            /* Responsive height */

  		min-width: 700px;        /* Minimum width */
  		max-width: 1000px;        /* Maximum width */

  		min-height: 550px;       /* Minimum height */
  		max-height: 1000px;       /* Maximum height */

  		background: #ccc;
  		border-radius: 2vw;
  		box-shadow: 0 0.5vw 1vw rgba(0, 0, 0, 0.2);
    }

    .circle-button {
      position: absolute;
      width: 7.5vw;
      min-width: 70px;
      max-width: 90px;
      height: 7.5vw;
      min-height: 70px;
      max-height: 90px;
      border-radius: 50%;
      border: none;
      background: #fff;
      border: 0.3vw solid #999;
      font-size: clamp(14px, 2vw, 20px);
      font-weight: bold;
      cursor: pointer;
      transition: background 0.2s, transform 0.2s;
    }

    .circle-button:hover {
      background: #ddd;
      transform: scale(1.05);
    }
    
    .circle-button.active {
  	background-color: red;
  	color: white;
	}

    .buttons {
      display: grid;
      grid-template-columns: repeat(2, 10vw);
      gap: 2vw;
      justify-content: center;
    }

    .buttons button {
      padding: 2vh 0;
      font-size: clamp(14px, 1.5vw, 20px);
      border: none;
      border-radius: 1vw;
      background: #555;
      color: white;
      cursor: pointer;
    }

    .buttons button:hover {
      background: #333;
    }
    
    .input-area {
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 1.5vh;
      margin-top: 3vh;
    }

    .input-with-button {
      display: flex;
      gap: 1vw;
      align-items: center;
    }

    .input-with-button input {
      padding: 1vh 1vw;
      font-size: clamp(14px, 2vw, 20px);
      border: 1px solid #ccc;
      border-radius: 0.6vw;
      width: 30vw;
    }

    .input-with-button button {
      padding: 1vh 2vw;
      font-size: clamp(14px, 1.2vw, 20px);
      border: none;
      border-radius: 0.6vw;
      background-color: #555;
      color: white;
      cursor: pointer;
      transition: background 0.2s;
    }

    .input-with-button button:hover {
      background-color: #333;
    }


    
  </style>
</head>
<body>
  <div class="grey-box">
    <!-- Responsive positioning using % -->
    <button onclick="buttonClicked(1)" class="circle-button" style="top: 22%; left: 12%;">1</button>
    <button onclick="buttonClicked(2)" class="circle-button" style="top: 20%; left: 23%;">2</button>
    <button onclick="buttonClicked(3)" class="circle-button" style="top: 26%; left: 33%;">3</button>
    
    <button onclick="buttonClicked(4)" class="circle-button" style="top: 60%; left: 37%;">4</button>
    <button onclick="buttonClicked(5)" class="circle-button" style="top: 60%; left: 53%;">5</button>

    <button onclick="buttonClicked(6)" class="circle-button" style="top: 30%; left: 49%;">6</button>
    
    <button onclick="buttonClicked(7)" class="circle-button" style="top: 38%; left: 59%;">7</button>
    <button onclick="buttonClicked(8)" class="circle-button" style="top: 33%; left: 69%;">8</button>
    <button onclick="buttonClicked(9)" class="circle-button" style="top: 35%; left: 79%;">9</button>
    
    <button onclick="buttonClicked(10)" class="circle-button" style="top: 21%; left: 59%;">10</button>
    <button onclick="buttonClicked(11)" class="circle-button" style="top: 16%; left: 69%;">11</button>
    <button onclick="buttonClicked(12)" class="circle-button" style="top: 18%; left: 79%;">12</button>


    <button onclick="buttonClicked(13)" class="circle-button" style="top: 27%; left: 88%;">13</button>
  </div>
  
  <div class="input-area">
    <div class="input-with-button">
    <label>Trigger Distance:</label>
    <input id="input1" type="text" value="0" placeholder="First input..." />
    <button disabled onclick="submit(1)">-</button>
    <button disabled onclick="submit(2)">+</button>
  	</div>

  <div class="input-with-button">
    <label>Reset Distance:</label>
    <input id="input2" type="text" value="0" placeholder="Second input..." />
    <button disabled onclick="submit(3)">-</button>
    <button disabled onclick="submit(4)">+</button>
  	</div>
  </div>

  <div class="buttons">
    <button>Calibrate</button>
    <button onclick="saveUserSettings()">Save</button>
  </div>
  
  <script>
 	 document.querySelectorAll('.circle-button').forEach(button => {
     	button.addEventListener('click', () => {
        	// Remove "active" class from all buttons
        	document.querySelectorAll('.circle-button').forEach(btn => btn.classList.remove('active'));
            document.querySelectorAll('.input-area input, .input-area button').forEach(el => el.disabled = false);
    	  	// Add "active" to the clicked one
     	 	button.classList.add('active');
    	});
 	 });
    function submit(buttonId) {
    	fetch('/submit?id=' + buttonId)
     	 .then(res => res.text())
    	 .then(data => {
      	  if (buttonId === 1 || buttonId === 2) {
       	  	document.getElementById('input1').value = data;
     	    } else if (buttonId === 3 || buttonId === 4) {
            document.getElementById('input2').value = data;     	  	
          }
     	});
  	}
    function buttonClicked(buttonId) {
    	fetch('/button?id=' + buttonId)
     	 .then(res => res.text())
    	 .then(data => {
          const values = data.split(",");
       	  document.getElementById('input1').value = values[0];
          document.getElementById('input2').value = values[1];
     	});
  	}

    function saveUserSettings() {
      fetch('/save')
      .then(res => res.text())
      .then(data => {
        alert("Saving " + data);
      });
    }
    
    window.addEventListener('DOMContentLoaded', () => {
    	document.querySelectorAll('.input-area input, .input-area button').forEach(el => el.disabled = true);
    });
  </script>

</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", html);
}

void accessPointInit() {
  WiFi.softAPConfig(local_ip, gateway, subnet);

  // Start the access point
  WiFi.softAP(ssid, password);

  // Serial.println("Access Point Started");
  // Serial.print("Static IP Address: ");
  // Serial.println(WiFi.softAPIP());  // Should print: 192.168.10.1

  server.on("/", handleRoot);
  server.on("/save", []() {
    if (save_RT_Data()) {
      server.send(200, "text/plain", "Successful");
    }else{
      server.send(200, "text/plain", "Failed");
    }
  });
  server.on("/submit", []() {
    if (server.hasArg("id")) {
      String id = server.arg("id");
      if (id == "1") {  // Trigger Distance DOWN
        rT_Triggers[currentButtonIndex]--;
        if (rT_Triggers[currentButtonIndex] < 1) rT_Triggers[currentButtonIndex] = 1;
        server.send(200, "text/plain", String(rT_Triggers[currentButtonIndex]));
      } else if (id == "2") {  // Trigger Distance UP
        rT_Triggers[currentButtonIndex]++;
        if (rT_Triggers[currentButtonIndex] > (highestPoints[currentButtonIndex] - lowestPoints[currentButtonIndex])) rT_Triggers[currentButtonIndex] = highestPoints[currentButtonIndex] - lowestPoints[currentButtonIndex];
        server.send(200, "text/plain", String(rT_Triggers[currentButtonIndex]));
      } else if (id == "3") {  // Reset Distance Down
        rT_ResetDistances[currentButtonIndex]--;
        if (rT_ResetDistances[currentButtonIndex] < 1) rT_ResetDistances[currentButtonIndex] = 1;
        server.send(200, "text/plain", String(rT_ResetDistances[currentButtonIndex]));
      } else if (id == "4") {  // Reset Distance UP
        rT_ResetDistances[currentButtonIndex]++;
        if (rT_ResetDistances[currentButtonIndex] > (highestPoints[currentButtonIndex] - lowestPoints[currentButtonIndex])) rT_ResetDistances[currentButtonIndex] = highestPoints[currentButtonIndex] - lowestPoints[currentButtonIndex];
        server.send(200, "text/plain", String(rT_ResetDistances[currentButtonIndex]));
      } else {
        server.send(400, "text/plain", "Invalid ID");
      }
    } else {
      server.send(400, "text/plain", "Missing id");
    }
  });
  server.on("/button", []() {
    if (server.hasArg("id")) {
      String id = server.arg("id");
      int index = id.toInt();
      if (index > 0 && index <= pincount) {
        currentButtonIndex = index - 1;
        server.send(200, "text/plain", String(rT_Triggers[currentButtonIndex]) + "," + String(rT_ResetDistances[currentButtonIndex]));
      } else {
        server.send(200, "text/plain", "ID out of bounds!");
      }
    } else {
      server.send(400, "text/plain", "Missing id");
    }
  });
  server.begin();
  rgbLedWrite(RGB_BUILTIN, 0, 0, RGB_BRIGHTNESS);
}

void wifiProtocol() {
  server.handleClient();
}