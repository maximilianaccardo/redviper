#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Stepper.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Stepper motor setup
// 28BYJ-48 has 2048 steps per revolution (with gear reduction)
const int stepsPerRevolution = 2048;

// ULN2003 connection pins (adjust based on your wiring)
// Connect IN1, IN2, IN3, IN4 to these GPIO pins
#define IN1 D1  // GPIO5
#define IN2 D2  // GPIO4
#define IN3 D3  // GPIO0
#define IN4 D4  // GPIO2

// Initialize stepper library
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

// Web server on port 80
ESP8266WebServer server(80);

// Current position tracking
long currentPosition = 0;

void setup() {
  Serial.begin(115200);
  delay(10);
  
  // Set motor speed (RPM)
  myStepper.setSpeed(10);
  
  // Connect to WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/move", handleMove);
  server.on("/status", handleStatus);
  server.on("/zero", handleZero);
  server.on("/setpos", handleSetPosition);
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

// Root page with control interface
void handleRoot() {
  // Using raw string literal (R"===( ... )===")
  // Much cleaner and easier to edit!
  String html = R"===(
<!DOCTYPE html>
<html>
<head>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <title>Stepper Motor Control</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 50px; }
    input { padding: 10px; margin: 10px; font-size: 16px; }
    button { padding: 10px 20px; font-size: 16px; margin: 5px; }
    .status { margin-top: 20px; font-size: 18px; }
  </style>
</head>
<body>
  <h1>Red Viper Control Panel</h1>
  <p>Current Position: <span id='pos'>)===" + String(currentPosition * 360.0 / stepsPerRevolution, 1) + R"===(</span>&deg;</p>
  <div>
    <input type='number' id='degrees' placeholder='Enter degrees' value='90'>
    <button onclick='moveMotor()'>Move</button>
  </div>
  <div>
    <button onclick='moveDegrees(90)'>+90</button>
    <button onclick='moveDegrees(-90)'>-90</button>
    <button onclick='moveDegrees(180)'>+180</button>
    <button onclick='moveDegrees(-180)'>-180</button>
  </div>
  <div style='margin-top: 20px; border-top: 1px solid #ccc; padding-top: 20px;'>
    <h3>Calibration</h3>
    <button onclick='setZero()' style='background: #4CAF50; color: white;'>Set Zero Position</button>
    <div style='margin-top: 10px;'>
      <input type='number' id='setpos' placeholder='Set position (degrees)' style='width: 150px;'>
      <button onclick='setPosition()'>Set Position</button>
    </div>
  </div>
  <div class='status' id='status'></div>
  
  <script>
    function moveMotor() {
      var deg = document.getElementById('degrees').value;
      moveDegrees(deg);
    }
    
    function moveDegrees(deg) {
      document.getElementById('status').innerHTML = 'Moving...';
      fetch('/move?degrees=' + deg)
        .then(response => response.text())
        .then(data => {
          document.getElementById('status').innerHTML = data;
          updateStatus();
        });
    }
    
    function updateStatus() {
      fetch('/status')
        .then(response => response.json())
        .then(data => {
          document.getElementById('pos').innerHTML = data.degrees.toFixed(1);
        });
    }
    
    function setZero() {
      document.getElementById('status').innerHTML = 'Setting zero position...';
      fetch('/zero')
        .then(response => response.text())
        .then(data => {
          document.getElementById('status').innerHTML = data;
          updateStatus();
        });
    }
    
    function setPosition() {
      var degrees = document.getElementById('setpos').value;
      if (degrees === '') {
        document.getElementById('status').innerHTML = 'Please enter a position';
        return;
      }
      var steps = Math.round(degrees * 2048 / 360.0);
      document.getElementById('status').innerHTML = 'Setting position...';
      fetch('/setpos?position=' + steps)
        .then(response => response.text())
        .then(data => {
          document.getElementById('status').innerHTML = data;
          updateStatus();
        });
    }
    
    setInterval(updateStatus, 2000);
  </script>
</body>
</html>
)===";
  
  server.send(200, "text/html", html);
}

// Handle move command
void handleMove() {
  if (server.hasArg("degrees")) {
    float degrees = server.arg("degrees").toFloat();
    
    // Convert degrees to steps
    // 2048 steps = 360 degrees
    int steps = (int)(degrees * stepsPerRevolution / 360.0);
    
    Serial.print("Moving ");
    Serial.print(degrees);
    Serial.print(" degrees (");
    Serial.print(steps);
    Serial.println(" steps)");
    
    // Move the motor
    myStepper.step(steps);
    currentPosition += steps;
    
    String response = "Moved " + String(degrees) + " degrees";
    server.send(200, "text/plain", response);
  } else {
    server.send(400, "text/plain", "Missing 'degrees' parameter");
  }
}

// Handle status request
void handleStatus() {
  float degrees = currentPosition * 360.0 / stepsPerRevolution;
  String json = "{\"position\":" + String(currentPosition) + ",\"degrees\":" + String(degrees, 1) + "}";
  server.send(200, "application/json", json);
}

// Handle zero calibration
void handleZero() {
  currentPosition = 0;
  Serial.println("Position reset to zero");
  server.send(200, "text/plain", "Zero position set");
}

// Handle set position
void handleSetPosition() {
  if (server.hasArg("position")) {
    long newPosition = server.arg("position").toInt();
    currentPosition = newPosition;
    float degrees = currentPosition * 360.0 / stepsPerRevolution;
    
    Serial.print("Position set to: ");
    Serial.print(currentPosition);
    Serial.print(" steps (");
    Serial.print(degrees, 1);
    Serial.println(" degrees)");
    
    String response = "Position set to " + String(degrees, 1) + " degrees";
    server.send(200, "text/plain", response);
  } else {
    server.send(400, "text/plain", "Missing 'position' parameter");
  }
}

// Handle 404
void handleNotFound() {
  server.send(404, "text/plain", "Not Found");
}