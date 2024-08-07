#include <WiFi.h>
#include <FirebaseESP32.h>
#include <ArduinoJson.h>

// Kredensial WiFi
const char* ssid = "MATURNUWUN.ID";
const char* password = "Gantiterus";

// Kredensial Firebase
#define FIREBASE_HOST "track-c309e-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "iE0E8gLv5bdyqfBTtiZ7e8YgTUVApVsLUGPZNsGV"
const char* vehicleId = "vehicle1";

FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

const int MAX_RETRIES = 3;
const int RETRY_DELAY = 5000; // 5 detik

HardwareSerial arduinoSerial(2); // RX2=16, TX2=17 untuk komunikasi Arduino

// Deklarasi fungsi (function prototypes)
void checkWiFiConnection();
bool sendToFirebase(String gpsData);
bool sendToFirebaseWithRetry(String gpsData, int maxRetries = MAX_RETRIES);
String getCommandFromFirebase();
void updateCommandExecutionStatus(bool executed);

void setup() {
  Serial.begin(115200); // Serial debug
  arduinoSerial.begin(9600, SERIAL_8N1, 16, 17);

  // Hubungkan ke Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menghubungkan ke Wi-Fi...");
  }
  Serial.println("Terhubung ke Wi-Fi");

  // Inisialisasi Firebase
  config.database_url = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  checkWiFiConnection();

  if (arduinoSerial.available()) {
    String gpsData = arduinoSerial.readStringUntil('\n');
    if (gpsData.length() > 0) {
      Serial.println("Menerima data GPS: " + gpsData);
      sendToFirebaseWithRetry(gpsData);
    } else {
      Serial.println("Menerima data GPS kosong.");
    }
  }

  String command = getCommandFromFirebase();
  if (command != "") {
    arduinoSerial.println(command);
    updateCommandExecutionStatus(true);
  }

  delay(1000);
}

void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Koneksi WiFi terputus. Mencoba menghubungkan kembali...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println("\nTerhubung kembali ke WiFi");
  }
}

bool sendToFirebase(String gpsData) {
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, gpsData);
  
  if (error) {
    Serial.print(F("Gagal deserialize JSON: "));
    Serial.println(error.f_str());
    return false;
  }

  String path = String("/vehicles/") + vehicleId + "/location";
  FirebaseJson json;
  json.set("latitude", doc["latitude"].as<float>());
  json.set("longitude", doc["longitude"].as<float>());
  json.set("speed", doc["speed"].as<float>());
  json.set("altitude", doc["altitude"].as<float>());
  json.set("timestamp", millis());

  Serial.println("JSON yang akan dikirim:");
  String jsonStr;
  json.toString(jsonStr);
  Serial.println(jsonStr);

  if (!Firebase.setJSON(firebaseData, path.c_str(), json)) {
    Serial.println("Gagal mengirim data JSON ke Firebase");
    Serial.print("Kode error: ");
    Serial.println(firebaseData.errorCode());
    Serial.print("Alasan error: ");
    Serial.println(firebaseData.errorReason());
    
    if (firebaseData.errorCode() == 1) {
      Serial.println("Kemungkinan masalah autentikasi. Periksa kembali FIREBASE_AUTH.");
    } else if (firebaseData.errorCode() == 2) {
      Serial.println("Kemungkinan masalah jaringan. Periksa koneksi WiFi.");
    }
    
    return false;
  }

  Serial.println("Data berhasil dikirim ke Firebase");
  return true;
}

bool sendToFirebaseWithRetry(String gpsData, int maxRetries) {
  for (int i = 0; i < maxRetries; i++) {
    if (sendToFirebase(gpsData)) {
      return true;
    }
    Serial.printf("Percobaan %d gagal. Mencoba lagi dalam %d detik...\n", i + 1, RETRY_DELAY / 1000);
    delay(RETRY_DELAY);
  }
  Serial.println("Gagal mengirim data setelah beberapa percobaan.");
  return false;
}

String getCommandFromFirebase() {
  String path = String("/vehicles/") + vehicleId + "/command";

  if (Firebase.getJSON(firebaseData, path.c_str())) {
    FirebaseJson &json = firebaseData.jsonObject();
    FirebaseJsonData jsonData;

    String command = "";
    bool executed = false;

    Serial.println("Berhasil mengambil JSON perintah");

    if (json.get(jsonData, "type")) {
      Serial.println("Tipe ditemukan dalam JSON perintah");
      if (jsonData.typeNum == FirebaseJson::JSON_STRING) {
        command = jsonData.stringValue;
      } else {
        Serial.println("Tipe bukan string");
      }
    } else {
      Serial.println("Tipe tidak ditemukan dalam JSON perintah");
    }

    if (json.get(jsonData, "executed")) {
      Serial.println("Status eksekusi ditemukan dalam JSON perintah");
      if (jsonData.typeNum == FirebaseJson::JSON_BOOL) {
        executed = jsonData.boolValue;
      } else {
        Serial.println("Status eksekusi bukan boolean");
      }
    } else {
      Serial.println("Status eksekusi tidak ditemukan dalam JSON perintah");
    }

    if (command != "" && !executed) {
      return command;
    } else {
      Serial.println("Perintah kosong atau sudah dieksekusi");
    }
  } else {
    Serial.println("Gagal mendapatkan perintah dari Firebase");
    Serial.println(firebaseData.errorReason());
  }
  
  return "";
}

void updateCommandExecutionStatus(bool executed) {
  String path = String("/vehicles/") + vehicleId + "/command/executed";
  if (!Firebase.setBool(firebaseData, path.c_str(), executed)) {
    Serial.println("Gagal memperbarui status eksekusi perintah");
    Serial.println(firebaseData.errorReason());
  } else {
    Serial.println("Status eksekusi perintah berhasil diperbarui");
  }
}