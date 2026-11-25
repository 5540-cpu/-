#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// --- กำหนดขาอุปกรณ์ (ตรงตามผังที่ต่อไว้) ---
#define MQ135_PIN   36  // ขา SP บนบอร์ด ESP32 คือ GPIO 36
#define DHT_PIN     20   // ขา G4
#define BUZZER_PIN  31  // ขา G26 (Low-level trigger: LOW=ดัง, HIGH=เงียบ)
#define LED_GREEN   25  // ขา G25
#define LED_YELLOW  33  // ขา G33
#define LED_RED     32  // ขา G32

// --- ตั้งค่าเซนเซอร์และจอ LCD ---
#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

// Address มาตรฐานของจอ LCD I2C คือ 0x27 (ถ้าไม่ขึ้นให้ลองเปลี่ยนเป็น 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- เกณฑ์ระดับคุณภาพอากาศ (ปรับจูนได้ตามสภาพแวดล้อมจริง) ---
const int AIR_WARNING = 1200; // เกินค่านี้ = เฝ้าระวัง (ไฟเหลือง)
const int AIR_DANGER  = 2000; // เกินค่านี้ = อันตราย (ไฟแดง + ลำโพงดัง)

void setup() {
  Serial.begin(115200);

  // ตั้งค่าโหมดขา Output
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // ปิดไฟและลำโพงทั้งหมดตอนเริ่มต้น (Buzzer เป็น Low-level trigger จึงต้องสั่ง HIGH เพื่อปิด)
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  digitalWrite(BUZZER_PIN, HIGH);

  // เริ่มการทำงานของเซนเซอร์และจอ
  dht.begin();
  lcd.init();
  lcd.backlight();

  // ข้อความต้อนรับบนจอ LCD
  lcd.setCursor(0, 0);
  lcd.print("Air Quality Sys");
  lcd.setCursor(0, 1);
  lcd.print("Warming up...");
  delay(2000);
  lcd.clear();
}

void loop() {
  // 1. อ่านค่าจากเซนเซอร์
  int airValue = analogRead(MQ135_PIN); // ค่าดิบ 0 - 4095
  float humidity = dht.readHumidity();
  float temp = dht.readTemperature();

  // เช็กว่า DHT11 อ่านค่าได้ปกติไหม
  if (isnan(humidity) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    temp = 0;
    humidity = 0;
  }

  // แสดงผลใน Serial Monitor (ไว้ดูค่าตัวเลขละเอียดในคอม)
  Serial.print("Air: "); Serial.print(airValue);
  Serial.print(" | Temp: "); Serial.print(temp, 1);
  Serial.print("C | Hum: "); Serial.print(humidity, 0);
  Serial.println("%");

  // 2. แสดงผลบนจอ LCD 16x2
  // บรรทัดบน: แสดงอุณหภูมิและความชื้น
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temp, 1);
  lcd.print("C  H:");
  lcd.print(humidity, 0);
  lcd.print("%   "); // เว้นวรรคท้ายเพื่อลบตัวอักษรเก่าที่ค้าง

  // บรรทัดล่าง: แสดงค่าฝุ่น/ก๊าซ และสถานะ
  lcd.setCursor(0, 1);
  lcd.print("Air:");
  lcd.print(airValue);
  lcd.print(" ");

  // 3. เงื่อนไขควบคุม LED และ Buzzer
  if (airValue < AIR_WARNING) {
    // สถานะปกติ: ไฟเขียวติด, ลำโพงเงียบ
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);
    digitalWrite(BUZZER_PIN, HIGH); // HIGH = เงียบ
    lcd.print("[GOOD]  ");
    
  } else if (airValue < AIR_DANGER) {
    // สถานะเฝ้าระวัง: ไฟเหลืองติด, ลำโพงเงียบ
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, HIGH);
    digitalWrite(LED_RED, LOW);
    digitalWrite(BUZZER_PIN, HIGH); // HIGH = เงียบ
    lcd.print("[WARN]  ");
    
  } else {
    // สถานะอันตราย: ไฟแดงติด, ลำโพงดัง
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, HIGH);
    digitalWrite(BUZZER_PIN, LOW);  // LOW = ดัง!
    lcd.print("[DANGER]");
  }

  // โครงสร้างเผื่อรอบจริง: จุดส่งค่าขึ้น Blynk.virtualWrite(...) จะนำมาใส่ตรงนี้
  
  delay(2000); // หน่วงเวลา 2 วินาที (เหมาะสมกับรอบการอ่านค่าของ DHT11)
}