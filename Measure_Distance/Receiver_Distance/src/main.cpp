#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>

// Khởi tạo LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); // Địa chỉ I2C có thể cần thay đổi

// Cấu trúc dữ liệu nhận
struct DistanceData {
  float distance;
  uint32_t timestamp;
  uint16_t sequence;
};

// Biến toàn cục
uint32_t lastReceivedTime = 0;
bool lcdInitialized = false;

// Hàm xử lý khi nhận dữ liệu
void onDataReceive(const uint8_t *mac, const uint8_t *incomingData, int len) {
  if (len != sizeof(DistanceData)) {
    Serial.println("Invalid data length received");
    return;
  }
  
  DistanceData receivedData;
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  
  // Chỉ update LCD nếu giá trị thay đổi đáng kể (> 0.5cm)
  static float lastDisplayedDistance = -1;
  if (fabs(receivedData.distance - lastDisplayedDistance) > 0.5 || lastDisplayedDistance < 0) {
    lastDisplayedDistance = receivedData.distance;
    lastReceivedTime = millis();
    
    // Hiển thị lên Serial
    Serial.print("Distance: ");
    Serial.print(receivedData.distance);
    Serial.print(" cm | Seq: ");
    Serial.println(receivedData.sequence);
    
    // Hiển thị lên LCD
    if (lcdInitialized) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Distance:");
      lcd.setCursor(0, 1);
      lcd.print(receivedData.distance, 1);
      lcd.print(" cm");
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP-NOW Receiver Starting...");
  
  // Khởi tạo LCD
  lcd.init();
  lcd.backlight();
  lcdInitialized = true;
  
  // Hiển thị thông báo khởi động
  lcd.setCursor(0, 0);
  lcd.print("ESP-NOW Receiver");
  lcd.setCursor(0, 1);
  lcd.print("Waiting...");
  
  // Khởi tạo WiFi
  WiFi.mode(WIFI_STA);
  
  // Hiển thị MAC address
  Serial.print("Receiver MAC: ");
  Serial.println(WiFi.macAddress());
  
  // Khởi tạo ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    lcd.clear();
    lcd.print("ESP-NOW Error");
    return;
  }
  
  // Đăng ký callback nhận dữ liệu
  esp_now_register_recv_cb(onDataReceive);
  
  Serial.println("Receiver ready - Waiting for data...");
}

void loop() {
  // Kiểm tra mất kết nối (quá 5 giây không nhận được data)
  if (lcdInitialized && millis() - lastReceivedTime > 5000 && lastReceivedTime != 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No Signal");
    lcd.setCursor(0, 1);
    lcd.print("Check Connection");
    lastReceivedTime = millis(); // Reset để không liên tục clear LCD
  }
  
  delay(100);
}
