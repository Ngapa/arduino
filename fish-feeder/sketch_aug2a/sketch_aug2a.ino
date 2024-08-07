#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Alamat I2C mungkin perlu disesuaikan (0x27 atau 0x3F)
Servo myservo;

const int pinTrig = 2;    // Pin trigger sensor ultrasonik
const int pinEcho = 3;    // Pin echo sensor ultrasonik
const int pinBuzzer = 11; // Pin buzzer
const int pinServo = 5;   // Pin servo

int StatusPakan = 0;
float jarak = 0;

void setup() {
  Serial.begin(9600);  // Komunikasi UART dengan ESP32
  
  myservo.attach(pinServo);
  myservo.write(0);

  pinMode(pinTrig, OUTPUT);
  pinMode(pinEcho, INPUT);
  pinMode(pinBuzzer, OUTPUT);
  digitalWrite(pinBuzzer, HIGH);  // Matikan buzzer pada awalnya

  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(3, 0);
  lcd.print("PAKAB IKAN OTOMATIS");
  delay(2000);
  lcd.clear();

  Serial.println("System initialized");
}

void loop() {
  cekLevelPakan();

  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    StatusPakan = input.toInt();
    Serial.println("Diterima: " + String(StatusPakan));
  }

  if (StatusPakan == 1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WAKTUNYA MAKAN");
    Serial.println("MAKAN CUY..!!!");
    BeriPakan();
    lcd.clear();
    
    StatusPakan = 0;
    Serial.println("0");  // Kirim kembali ke ESP32
    delay(2000);
  }

  // Tampilkan jarak pada LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Jarak: ");
  lcd.print(jarak);
  lcd.print(" cm");
}

void BeriPakan() {
  for (int posisi = 0; posisi <= 180; posisi += 5) {
    myservo.write(posisi);
    delay(50);
  }
  
  for (int posisi = 180; posisi >= 0; posisi -= 5) {
    myservo.write(posisi);
    delay(50);
  }

  Serial.print("0");
}

void cekLevelPakan() {
  // Picu sensor ultrasonik
  digitalWrite(pinTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig, LOW);

  // Ukur waktu echo
  long durasi = pulseIn(pinEcho, HIGH);

  // Hitung jarak dalam cm
  jarak = durasi * 0.034 / 2;

  // Cek jika jarak lebih dari 15 cm
  if (jarak > 15) {
    // Aktifkan buzzer
    digitalWrite(pinBuzzer, LOW);
    
    // Tampilkan peringatan di LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pakan Habis!");
    lcd.setCursor(0, 1);
    lcd.print("Jarak: ");
    lcd.print(jarak);
    lcd.print(" cm");
    
    // Kirim peringatan ke ESP32
    Serial.println("PERINGATAN:PakanRendah");
  } else {
    // Matikan buzzer
    digitalWrite(pinBuzzer, HIGH);
  }

  // Kirim jarak ke ESP32 untuk pemantauan
  Serial.print("Jarak:");
  Serial.println(jarak);

  delay(1000); // Cek setiap detik
}