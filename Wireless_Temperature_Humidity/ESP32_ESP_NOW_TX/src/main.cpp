/*
Ket noi phan cung:
- Cam bien DHT11
  VCC -> 3.3V
  GND -> GND
  DATA -> GPIO4
- LED
  Chan duong (anode) -> GPIO2
  Chân âm (cathode) → qua điện trở 220Ω -> GND
*/

/*
Cài đặt thư viện DHT11:
- Trong PlatformIO, vào tab Libraries.
- Tìm "DHT sensor library" của Adafruit và thêm vào dự án bằng cách nhấp Add to Project
*/

#include <Arduino.h>

// Thu vien DHT
#include <DHT.h>

// Trien khai thu vien ESP-NOW
#include <esp_now.h>
#include <WiFi.h>

// Dinh nghia loai cam bien, khai bao chan ket noi cua cam bien va LED
#define DHTPIN 4
#define DHTTYPE DHT11
#define LEDPIN 2

// Khoi tao doi tuong ten dht tu tu cau truc du lieu DHT trong thu vien DHT.h
DHT dht(DHTPIN, DHTTYPE);

/*
- Dinh nghia dia chi MAC cua khoi nhan(ESP32_ESP_NOW_RX)
- Thay bằng MAC thực tế
*/
uint8_t receivedAddressMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  

// Tao Cau truc du lieu de gui
typedef struct {
  float temperature;
  float humidity;
} data_struct;

// Khai bao doi tuong, doi tuong duoc dat ten tu cau truc du lieu
data_struct sendData;

void setup() {
  // Khoi tao Serial
  Serial.begin(115200);

  // Khoi tao cam bien DHT
  dht.begin();

  // Cau hinh chan cho LED
  pinMode(LEDPIN, OUTPUT);

  //Khoi tao ESP-NOW trong setup()
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Khởi tạo ESP-NOW thất bại");
    return;
  }
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, receivedAddressMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Thêm peer thất bại");
    return;
  }
}

void loop() {
  // Doc du lieu tu cam bien DHT11
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // Kiem tra du lieu doc co loi hay khong
  if (isnan(temp) || isnan(hum)) { // Co loi doc du lieu
    Serial.println("Không đọc được dữ liệu từ DHT11");
  } else{ // Khong co loi doc du lieu -> tien hanh gui du lieu qua ESP-NOW
    // Trong loop() thuc hien gui du lieu:
    sendData.temperature = temp;
    sendData.humidity = hum;
    esp_err_t result = esp_now_send(receivedAddressMAC, (uint8_t *)&sendData, sizeof(sendData));
    if (result == ESP_OK) {
      Serial.println("Gửi dữ liệu thành công");
      digitalWrite(LEDPIN, HIGH);  // Bật LED
    } else {
      Serial.println("Lỗi khi gửi dữ liệu");
      digitalWrite(LEDPIN, LOW);   // Tắt LED
    }
    delay(2000);
  }
}
