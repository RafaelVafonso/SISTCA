#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#define PWM_CHANNEL 0
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8

// --- CONFIGURAÇÃO REDE (MODO AP) ---
const char* ssid = "ESP32";   // Nome da rede 
const char* password = "1234567%"; // Senha (min 8 caracteres)

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

<div style="margin-top:20px;">
  <input type="range" id="pwm" min="0" max="255" value="0" 
       oninput="updatePWM(this.value); sendCmd('3 ' + this.value)">
  <div>PWM: <span id="pwm_val">0</span></div>
</div>


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
    function updatePWM(val) {
    document.getElementById("pwm_val").innerText = val;
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
    ledcWrite(PWM_CHANNEL, 255);
    ledState = true;
  }

  else if (v == "0") {
    ledcWrite(PWM_CHANNEL, 0);
    ledState = false;
  }

else if (v.startsWith("2 ")) {
  int tempo = v.substring(2).toInt() * 1000;

  int prevPWM = ledState ? 255 : 0;
  int newPWM  = ledState ? 0   : 255;

  ledcWrite(PWM_CHANNEL, newPWM);  // inverter
  delay(tempo);
  ledcWrite(PWM_CHANNEL, prevPWM); // voltar ao estado anterior
}

  else if (v.startsWith("3 ")) {
    int pwm = v.substring(2).toInt();

    
    ledcWrite(PWM_CHANNEL, pwm);

    ledState = (pwm > 0);
  }

  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED_PIN, PWM_CHANNEL);
  Serial.begin(115200);
  Serial.println("BOOT OK");
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
      ledcWrite(PWM_CHANNEL, 0);
      Serial.println("LED OFF");
      flag = 0; 
    }

    else if (cmd == "1") {
      ledcWrite(PWM_CHANNEL, 255);
      Serial.println("LED ON");
      flag = 1; 
    }

    else if (cmd.startsWith("2")) {
    int tempo = cmd.substring(2).toInt() * 1000;

        if(flag == 1) {ledcWrite(PWM_CHANNEL, 0); delay(tempo); ledcWrite(PWM_CHANNEL, 255);}
        if(flag == 0) {ledcWrite(PWM_CHANNEL, 255); delay(tempo); ledcWrite(PWM_CHANNEL, 0);}
}
  }
}