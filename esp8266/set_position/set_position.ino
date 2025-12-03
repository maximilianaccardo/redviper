#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Stepper.h>
#include "index_html.h"

// WiFi credentials
const char* ssid = "Flowers by Irene";
const char* password = "smileyhouse";

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
long targetPosition = 0;

void setup() {
  Serial.begin(9600);
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

  // Non-blocking movement
  if (currentPosition != targetPosition) {
    if (currentPosition < targetPosition) {
      myStepper.step(1);
      currentPosition++;
    } else {
      myStepper.step(-1);
      currentPosition--;
    }

    // Normalize current position
    if (currentPosition >= stepsPerRevolution) {
      currentPosition = 0;
    }
    if (currentPosition < 0) {
      currentPosition = stepsPerRevolution - 1;
    }
    yield(); // Feed the watchdog
  }
}

// Root page with control interface
void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

// Handle lights


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

    // Set target position (incremental)
    targetPosition += steps;

    // Normalize target position to 0-360 degrees (0-stepsPerRevolution)
    targetPosition = targetPosition % stepsPerRevolution;
    if (targetPosition < 0) {
      targetPosition += stepsPerRevolution;
    }

    String response = "Moving " + String(degrees) + " degrees...";
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
  targetPosition = 0;
  Serial.println("Position reset to zero");
  server.send(200, "text/plain", "Zero position set");
}

// Handle set position
void handleSetPosition() {
  if (server.hasArg("position")) {
    long newPosition = server.arg("position").toInt();

    // Normalize position
    newPosition = newPosition % stepsPerRevolution;
    if (newPosition < 0) {
      newPosition += stepsPerRevolution;
    }

    currentPosition = newPosition;
    targetPosition = newPosition;
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