#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>


// --- CONFIGURAÇÃO REDE (MODO AP) ---
const char* ssid = "ESP32";   // Nome da rede 
const char* password = "12345%"; // Senha (min 8 caracteres)

WebServer server(80);

#define LED_PIN 22
uint8_t flag = 0;

bool ledState = false;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>LED Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      font-family: sans-serif;
      text-align: center;
      background: #1a1a1a;
      color: #fff;
    }

    h1 { color: #FFC107; }

    button {
      width: 120px;
      height: 60px;
      font-size: 18px;
      margin: 10px;
      border: none;
      border-radius: 10px;
      color: white;
      cursor: pointer;
    }

    .on  { background: #4CAF50; }
    .off { background: #F44336; }
  </style>
</head>
<body>

  <h1>LED CONTROL</h1>

  <button class="on" onclick="sendCmd('1')">ON</button>
  <button class="off" onclick="sendCmd('0')">OFF</button>

  <div style="margin-top:20px;">
  <input type="number" id="tempo" placeholder="Tempo (s)" 
         style="padding:10px; width:120px; text-align:center;">
  </div>


<button style="background:orange;" onclick="invert()">INVERTER</button>

  <h2 id="state">Estado: --</h2>

  <script>
    function sendCmd(cmd) {
      fetch("/cmd?v=" + cmd);
    }

  function invert() {
    let t = document.getElementById("tempo").value;

    if (t == "" || t <= 0) {
      alert("Tempo inválido");
    return;
    }
    if (t > 0) {
    sendCmd("2 " + t);
    }
  }

    setInterval(() => {
      fetch("/status")
      .then(r => r.text())
      .then(s => {
        document.getElementById("state").innerText = "Estado: " + s;
      });
    }, 500);
  </script>

</body>
</html>
)rawliteral";

// Serve a página principal
void handleRoot() {
  server.send(200, "text/html", index_html);
}
void handleStatus() {
  server.send(200, "text/plain", ledState ? "ON" : "OFF");
}
void handleCmd() {
  String v = server.arg("v");

  if (v == "1") {
    digitalWrite(LED_PIN, HIGH);
    ledState = true;
  }
  else if (v == "0") {
    digitalWrite(LED_PIN, LOW);
    ledState = false;
  }
  else if (v.startsWith("2 ")) {
    int tempo = v.substring(2).toInt() * 1000;

    digitalWrite(LED_PIN, !ledState);
    delay(tempo);
    digitalWrite(LED_PIN, ledState);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);

    WiFi.softAP(ssid, password);
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/cmd", handleCmd);
  server.on("/status", handleStatus);
  server.begin();
  
}

void loop() {
  server.handleClient();

  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "0") {
      digitalWrite(LED_PIN, LOW);
      Serial.println("LED OFF");
      flag = 0; 
    }

    else if (cmd == "1") {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LED ON");
      flag = 1; 
    }

    else if (cmd.startsWith("2")) {
    int tempo = cmd.substring(2).toInt() * 1000;

        if(flag == 1) {digitalWrite(LED_PIN, LOW); delay(tempo); digitalWrite(LED_PIN, HIGH);}
        if(flag == 0) {digitalWrite(LED_PIN, HIGH); delay(tempo); digitalWrite(LED_PIN, LOW);}
}
  }
}