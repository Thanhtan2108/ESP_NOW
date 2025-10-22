#include <Arduino.h>
// Khai báo thư viện dùng cho ESP-NOW
#include <esp_now.h>
#include <WiFi.h>
// Khai báo các thư viện Sensor
#include <LiquidCrystal_I2C.h>


/*=================Initial for Sensor============*/
// Định nghĩa LCD có địa chỉ để giao tiếp I2C, có 16 cột, 2 hàng
// Địa chỉ I2C của LCD1602 (thường là 0x27 hoặc 0x3F, kiểm tra bằng I2C scanner nếu cần)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Xác định ngưỡng cảnh báo
#define TEMP_THRESHOLD 30.0 // Nhiệt độ vượt 30°C
#define HUM_THRESHOLD 80.0  // Độ ẩm vượt 80%

// Khai báo chân buzzer
#define BUZZER_PIN 5

// PWM cho buzzer thụ động
#define PWM_CHANNEL 0
#define PWM_FREQ 1000 // Tần số cố định 1000 Hz
#define PWM_RESOLUTION 8 // Độ phân giải PWM 8 bit
#define PWM_MAX_DUTY 255 // Duty cycle 100%
#define PWM_STEP 5 // Bước tăng duty cycle
#define PWM_STEP_DELAY 50 // Thời gian mỗi bước (ms)
#define PWM_HOLD_DURATION 3000 // Giữ 100% duty cycle trong 3 giây

/*=================Initial for ESP-NOW============*/
// xây dựng 1 struct chứa các thông tin nhận được từ bên gửi
typedef struct struct_message {
  float Temperature;
  float Humidity;
} struct_message;

// khởi tạo 1 đối tượng chứa các thông tin được nhận
struct_message myData;
struct_message lastData; // Lưu dữ liệu trước để so sánh

// Biến để điều khiển buzzer không chặn
unsigned long buzzerStartTime = 0;
bool buzzerActive = false;
int currentDutyCycle = 0;
bool holdMaxDuty = false;

// Callback khi nhận dữ liệu và giải thuật xử lý dữ liệu nhận từ Sensor
// Nhận dữ liệu
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));

// Xử lý giải thuật với dữ liệu nhận được
  // Kiểm tra dữ liệu hợp lệ
  if (myData.Temperature < -40 || myData.Temperature > 80 || myData.Humidity < 0 || myData.Humidity > 100) {
    Serial.println("Dữ liệu không hợp lệ!");
    return;
  }

  // In dữ liệu ra Serial để debug
  Serial.print("Nhận được - Nhiệt độ: ");
  Serial.print(myData.Temperature);
  Serial.print("°C, Độ ẩm: ");
  Serial.print(myData.Humidity);
  Serial.println("%");

  // Chỉ cập nhật LCD nếu dữ liệu thay đổi
  if (myData.Temperature != lastData.Temperature || myData.Humidity != lastData.Humidity) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(myData.Temperature);
    lcd.print((char)223); // Ký tự độ (°)
    lcd.print("C");

    lcd.setCursor(0, 1);
    lcd.print("Hum: ");
    lcd.print(myData.Humidity);
    lcd.print("%");

    lastData = myData; // Lưu dữ liệu hiện tại
  }

  // Kích hoạt buzzer nếu vượt ngưỡng
  if (myData.Temperature > TEMP_THRESHOLD || myData.Humidity > HUM_THRESHOLD) {
    buzzerActive = true;
    buzzerStartTime = millis();
    currentDutyCycle = 0; // Bắt đầu từ 0% duty cycle
    holdMaxDuty = false;
    ledcWrite(PWM_CHANNEL, currentDutyCycle); // Bật buzzer với duty cycle ban đầu
  }
}


void setup() {
  // Khởi tạo Serial để debug
  Serial.begin(115200);
  delay(100); // Đợi Serial ổn định

  /*===================CONFIG COMMUNICATE SENSOR=====================*/
  // Khởi tạo LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(2000);

  // Cấu hình PWM cho buzzer thụ động
  pinMode(BUZZER_PIN, OUTPUT);
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(BUZZER_PIN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 0); // Tắt buzzer ban đầu

  /*====================CONFIG ESP-NOW======================*/
  // Khởi tạo ESP-NOW
  WiFi.mode(WIFI_STA);
  // Serial.println("Địa chỉ MAC: " + WiFi.macAddress());

  // Bật cho phép ESP-NOW hoạt động và kiểm tra lỗi kết nối ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Lỗi khởi tạo ESP-NOW!");
    lcd.clear();
    lcd.print("ESP-NOW Error");
    while (1); // Dừng nếu lỗi
  }
  Serial.println("ESP-NOW khởi tạo thành công!");
  lcd.clear();
  lcd.print("ESP-NOW OK");

  // Đăng ký callback nhận dữ liệu
  esp_now_register_recv_cb(OnDataRecv);

  delay(1000);
  lcd.clear();
}

void loop() {
  // Điều khiển buzzer thụ động (tăng duty cycle và giữ 100%)
  if (buzzerActive) {
    if (!holdMaxDuty) {
      // Tăng duty cycle từ 0 đến 100%
      if (currentDutyCycle < PWM_MAX_DUTY) {
        if (millis() - buzzerStartTime >= PWM_STEP_DELAY) {
          currentDutyCycle += PWM_STEP;
          if (currentDutyCycle > PWM_MAX_DUTY) currentDutyCycle = PWM_MAX_DUTY;
          ledcWrite(PWM_CHANNEL, currentDutyCycle);
          buzzerStartTime = millis(); // Cập nhật thời gian
          Serial.print("Duty Cycle: ");
          Serial.print((currentDutyCycle * 100.0) / PWM_MAX_DUTY);
          Serial.println("%");
        }
      } else {
        // Đạt 100% duty cycle, chuyển sang giữ 3 giây
        holdMaxDuty = true;
        buzzerStartTime = millis();
      }
    } else {
      // Giữ 100% duty cycle trong 3 giây
      if (millis() - buzzerStartTime >= PWM_HOLD_DURATION) {
        ledcWrite(PWM_CHANNEL, 0); // Tắt buzzer
        buzzerActive = false;
        holdMaxDuty = false;
        currentDutyCycle = 0;
      }
    }
  }
}
