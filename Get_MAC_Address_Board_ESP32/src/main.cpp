#include <Arduino.h>
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(100);

  // Lấy MAC của Station interface
  WiFi.mode(WIFI_STA);
  delay(100);
  String macSTA = WiFi.macAddress();               // lấy MAC STA => địa chỉ MAC dùng trong ESP-NOW
  Serial.print("MAC address (STA): ");
  Serial.println(macSTA);

  // Lấy MAC của Soft-AP interface
  WiFi.mode(WIFI_AP);
  delay(100);
  String macAP = WiFi.softAPmacAddress();          // lấy MAC AP
  Serial.print("MAC address (AP): ");
  Serial.println(macAP);
}

void loop() {
  // không cần làm gì thêm
}