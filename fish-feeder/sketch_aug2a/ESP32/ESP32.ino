#define BLYNK_TEMPLATE_ID "TMPL6DlIIIHvw"
#define BLYNK_TEMPLATE_NAME "fishFeeder"
#define BLYNK_AUTH_TOKEN "U6ySobBlszikbefdymjWmEMVJV4iMxMu"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

const char* ssid = "MATURNUWUN.ID";
const char* pass = "Gantiterus";

BlynkTimer timer;

const int RXD2 = 16;
const int TXD2 = 17;

void setup() {
  Serial.begin(19200);  // Debug console
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  WiFi.begin(ssid, pass);
  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi terhubung");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Blynk terhubung");

  timer.setInterval(1000L, cekDataSerial);
}

void loop() {
  Blynk.run();
  timer.run();
}

void cekDataSerial() {
  while (Serial2.available()) {
    String data = Serial2.readStringUntil('\n');
    data.trim();
    Serial.println("Data mentah dari Arduino: " + data);

    if (data.startsWith("PERINGATAN:")) {
      Serial.println("Peringatan diterima: " + data);
      Blynk.logEvent("level_pakan_rendah", "Level pakan rendah!");
    } else if (data.startsWith("Jarak:")) {
      float jarak = data.substring(6).toFloat();
      if (jarak >= 0 && jarak <= 1000) {
        Serial.println("Jarak valid diterima: " + String(jarak));
        Blynk.virtualWrite(V1, jarak);
      } else {
        Serial.println("Data jarak tidak valid: " + String(jarak));
      }
    } else {
      Serial.println("Data tidak dikenali: " + data);
    }
        // Mengubah status tombol di Blynk menjadi off
  }
}

BLYNK_WRITE(V0) {
  int nilai = param.asInt();
  if (nilai == 1) {
    Serial2.println("1");
    Serial.println("Perintah beri pakan dikirim ke Arduino");
  }
  Serial.println("Arduino selesai memberi pakan");
      Blynk.virtualWrite(V0, 0);
}

