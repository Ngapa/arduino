#include <TinyGPS++.h>
#include <SoftwareSerial.h>

TinyGPSPlus gps;
SoftwareSerial gpsSerial(4, 3); // RX, TX

const int relayPinHorn = 7;
const int relayPinStarter = 8;

void setup() {
  Serial.begin(9600); // Communication with ESP32
  gpsSerial.begin(9600);
  
  pinMode(relayPinHorn, OUTPUT);
  pinMode(relayPinStarter, OUTPUT);
}

void loop() {
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (gps.location.isUpdated()) {
        sendGPSData();
      }
    }
  }

  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    executeCommand(command);
  }
}

void sendGPSData() {
  String gpsData = "{\"latitude\":" + String(gps.location.lat(), 6) + 
                   ",\"longitude\":" + String(gps.location.lng(), 6) + 
                   ",\"speed\":" + String(gps.speed.kmph()) + 
                   ",\"altitude\":" + String(gps.altitude.meters()) + "}";
  Serial.println(gpsData);
}

void executeCommand(String command) {
  if (command == "START_ENGINE") {
    digitalWrite(relayPinStarter, HIGH);
    delay(1000);
    digitalWrite(relayPinStarter, LOW);
  } else if (command == "HORN") {
    digitalWrite(relayPinHorn, HIGH);
    delay(500);
    digitalWrite(relayPinHorn, LOW);
  }
}