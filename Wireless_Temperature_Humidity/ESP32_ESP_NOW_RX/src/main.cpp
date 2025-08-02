/*
Kết nối phần cứng:
- LCD 1602 (với module I2C):
  SDA → GPIO 21
  SCL → GPIO 22
  VCC → 5V
  GND → GND
- Buzzer thụ động:
  Chân dương(do) → GPIO 5
  Chân âm(den) → GND
- LED:
  Chân dương (anode) → GPIO 2
  Chân âm (cathode) → qua điện trở 220Ω -> GND
*/

/*
Cài đặt thư viện LCD:
- Trong PlatformIO, vào tab Libraries.
- Tìm "LiquidCrystal I2C" và thêm vào dự án.
*/

#include <Arduino.h>

// Thu vien LCD1602
#include <LiquidCrystal_I2C.h>

// Trien khai thu vien ESP-NOW
#include <esp_now.h>
#include <WiFi.h>

// Dinh nghia cac chan ket noi
#define LEDPIN 2

// Xac dinh nguong canh bao
#define TEMP_THRESHOLD 35.0
#define HUM_THRESHOLD 80.0

// Khoi tao LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Địa chỉ I2C có thể cần điều chỉnh

// Dinh nghia cau truc du lieu de nhan (giong khoi truyen):
typedef struct {
  float temperature;
  float humidity;
} data_struct;

// khoi tao doi tuong, dat ten doi tuong tu cau truc du lieu
data_struct receivedData;

// Ham hien thi tren LCD1602
void displayData() {
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(receivedData.temperature);
  lcd.print(" C");
  lcd.setCursor(0, 1);
  lcd.print("Hum: ");
  lcd.print(receivedData.humidity);
  lcd.print(" %");
}

// Ham canh bao qua buzzer khi vuot nguong canh bao
void checkThresholds() {
  if (receivedData.temperature > TEMP_THRESHOLD || receivedData.humidity > HUM_THRESHOLD) {
    tone(5, 1000, 500);
  }
}

// Ham callback khi nhan duoc du lieu:
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  Serial.println("Nhận dữ liệu thành công");
  digitalWrite(LEDPIN, HIGH);
  displayData();
  checkThresholds();
}

void setup() {
  Serial.begin(115200);
  pinMode(LEDPIN, OUTPUT);

  // Khoi tao LCD
  lcd.init();
  lcd.backlight();

  // Thiet lap ESP-NOW trong setup()
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Khởi tạo ESP-NOW thất bại");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // khong can xu ly gi trong loop
}
