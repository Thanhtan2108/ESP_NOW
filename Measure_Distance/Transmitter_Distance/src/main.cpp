#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_task_wdt.h>

// Define chân kết nối HC-SR04
#define TRIG_PIN 27
#define ECHO_PIN 26

// Địa chỉ MAC của receiver - THAY ĐỔI THEO ĐỊA CHỈ THỰC TẾ
uint8_t receiverAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Thay bằng MAC thật

// Cấu trúc dữ liệu gửi
struct DistanceData {
  float distance;
  uint32_t timestamp;
  uint16_t sequence;
};

DistanceData dataToSend;
uint16_t packetCounter = 0;
bool peerAdded = false;

// Hàm đọc khoảng cách
float readDistance() {
  // Đọc 3 lần và lấy giá trị trung bình
  float sum = 0;
  int validReadings = 0;
  
  for(int i = 0; i < 3; i++) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    if (duration > 0) {
      float distance = (duration * 0.0343) / 2.0;
      
      if (distance >= 2.0 && distance <= 400.0) {
        sum += distance;
        validReadings++;
      }
    }
    delay(50);
  }
  
  return (validReadings > 0) ? sum / validReadings : -1.0;
}

// Callback khi gửi dữ liệu
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
  Serial.print(" | Sequence: ");
  Serial.println(dataToSend.sequence);
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP-NOW Sender Starting...");
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Khởi tạo WiFi
  WiFi.mode(WIFI_STA);
  
  // Lấy MAC address
  Serial.print("Sender MAC: ");
  Serial.println(WiFi.macAddress());
  
  // Khởi tạo ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
    return;
  }
  
  // Đăng ký callback
  esp_now_register_send_cb(onDataSent);
  
  // Thêm peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) == ESP_OK) {
    peerAdded = true;
    Serial.println("Peer added successfully");
  } else {
    Serial.println("Failed to add peer");
  }

  // Khởi tạo watchdog
  esp_task_wdt_init(30, true); // Timeout 30 giây
}

void loop() {
  static uint32_t lastSendTime = 0;
  static uint32_t lastMemoryCheck = 0;
  uint32_t currentTime = millis();
  
  // Reset watchdog
  esp_task_wdt_reset();

  // Gửi dữ liệu mỗi 500ms
  if (currentTime - lastSendTime >= 500) {
    float distance = readDistance();
    
    if (distance > 0) {
      dataToSend.distance = distance;
      dataToSend.timestamp = currentTime;
      dataToSend.sequence = packetCounter++;
      
      if (peerAdded) {
        esp_err_t result = esp_now_send(receiverAddress, 
                                      (uint8_t *)&dataToSend, 
                                      sizeof(dataToSend));
        
        Serial.print("Distance: ");
        Serial.print(distance);
        Serial.println(" cm");
        
        if (result != ESP_OK) {
          Serial.println("Send failed!");
        }
      }
    } else {
      Serial.println("Invalid distance measurement");
    }
    
    lastSendTime = currentTime;
  }

  // Kiểm tra memory leak mỗi 30 giây
  if (currentTime - lastMemoryCheck > 30000) {
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    lastMemoryCheck = currentTime;
  }
  
  delay(10); // Giảm CPU usage
}
