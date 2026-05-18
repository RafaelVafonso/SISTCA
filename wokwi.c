#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// --- PWM ---
#define LED_PIN 22

// --- WiFi (Wokwi) ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// --- Web Server ---
WebServer server(80);

// --- Estado ---
int currentPWM = 0;
unsigned long invertStart = 0;
unsigned long invertDuration = 0;
bool invertActive = false;
int previousPWM = 0;

// --- HTML ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<body style="text-align:center;">
<h1>ESP32 LED</h1>

<button onclick="fetch('/on')">ON</button>
<button onclick="fetch('/off')">OFF</button>

<br><br>

<input type="number" id="tempo" placeholder="Tempo (s)">
<button onclick="invert()">INVERTER</button>

<br><br>

<input type="range" min="0" max="255" value="0"
oninput="fetch('/pwm?v='+this.value)">

<script>
function invert() {
  let t = document.getElementById("tempo").value;
  if(t > 0) fetch("/invert?t=" + t);
}
</script>

</body>
</html>
)rawliteral";


// --- HANDLERS ---

void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleOn() {
  currentPWM = 255;
  ledcWrite(LED_PIN, currentPWM);
  server.send(200, "text/plain", "ON");
}

void handleOff() {
  currentPWM = 0;
  ledcWrite(LED_PIN, currentPWM);
  server.send(200, "text/plain", "OFF");
}

void handlePWM() {
  currentPWM = server.arg("v").toInt();
  ledcWrite(LED_PIN, currentPWM);
  server.send(200, "text/plain", "PWM OK");
}

void handleInvert() {
  int tempo = server.arg("t").toInt();

  previousPWM = currentPWM;
  currentPWM = (currentPWM > 0) ? 0 : 255;

  ledcWrite(LED_PIN, currentPWM);

  invertStart = millis();
  invertDuration = tempo * 1000;
  invertActive = true;

  server.send(200, "text/plain", "INVERT");
}


// --- SETUP ---

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("BOOT OK");

  // API NOVA (ESP32 core 3.x)
  ledcAttach(LED_PIN, 5000, 8);  // pin, freq, resolution

  WiFi.begin(ssid, password);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  
  server.on("/", []() {
  server.send(200, "text/html", "HELLO WOKWI");
});
  server.on("/on", []() {
    ledcWrite(LED_PIN, 255);
    server.send(200, "text/plain", "ON");
  });

  server.on("/off", []() {
    ledcWrite(LED_PIN, 0);
    server.send(200, "text/plain", "OFF");
  });

  server.on("/pwm", []() {
    int pwm = server.arg("v").toInt();
    ledcWrite(LED_PIN, pwm);
    server.send(200, "text/plain", "PWM OK");
  });

  server.begin();
}


// --- LOOP ---

void loop() {
  server.handleClient();
  int flag;
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "0") {
      ledcWrite(LED_PIN, 0);
      Serial.println("LED OFF");
      flag = 0; 
    }

    else if (cmd == "1") {
      ledcWrite(LED_PIN, 255);
      Serial.println("LED ON");
      flag = 1; 
    }

    else if (cmd.startsWith("2")) {
    int tempo = cmd.substring(2).toInt() * 1000;
    Serial.printf("Inverter LED %d milisegundos\n", tempo);

        if(flag == 1) {ledcWrite(LED_PIN, 0); delay(tempo); ledcWrite(LED_PIN, 255);}
        if(flag == 0) {ledcWrite(LED_PIN, 255); delay(tempo); ledcWrite(LED_PIN, 0);}
}
  }
  // Inversão não bloqueante
  if (invertActive && millis() - invertStart >= invertDuration) {
    currentPWM = previousPWM;
    ledcWrite(LED_PIN, currentPWM);
    invertActive = false;
  }
}