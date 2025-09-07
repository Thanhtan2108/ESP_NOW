#include <esp_now.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>

// Cấu trúc dữ liệu để nhận (phải khớp với sender)
typedef struct struct_message {
    bool isRaining;
    int sensorValue;
} struct_message;

struct_message receivedData;

// Khởi tạo LCD I2C (địa chỉ 0x27, 16 ký tự, 2 dòng)
// Nếu không hoạt động, thử địa chỉ 0x3F
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Cấu hình buzzer
const int buzzerPin = 5;
const int buzzerChannel = 0;
const int buzzerFrequency = 1000;  // 1kHz

// Biến để theo dõi trạng thái buzzer
bool buzzerActive = false;

// Callback khi dữ liệu được nhận
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    memcpy(&receivedData, incomingData, sizeof(receivedData));
    Serial.print("Data received - Raining: ");
    Serial.print(receivedData.isRaining);
    Serial.print(", Value: ");
    Serial.println(receivedData.sensorValue);
    
    // Hiển thị trên LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    
    if (receivedData.isRaining) {
        lcd.print("TROI DANG MUA!");
        lcd.setCursor(0, 1);
        lcd.print("Do am: ");
        lcd.print(4095 - receivedData.sensorValue); // Hiển thị độ ẩm (giá trị nghịch đảo)
        
        // Kêu buzzer với tần số 1kHz nếu chưa kêu
        if (!buzzerActive) {
            ledcWriteTone(buzzerChannel, buzzerFrequency);
            buzzerActive = true;
        }
    } else {
        lcd.print("TROI NANG");
        lcd.setCursor(0, 1);
        lcd.print("Do am: ");
        lcd.print(4095 - receivedData.sensorValue); // Hiển thị độ ẩm
        
        // Tắt buzzer nếu đang kêu
        if (buzzerActive) {
            ledcWriteTone(buzzerChannel, 0);
            buzzerActive = false;
        }
    }
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    // Khởi tạo LCD
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("HE THONG PHAT");
    lcd.setCursor(0, 1);
    lcd.print("HIEN MUA...");

    // In địa chỉ MAC để cấu hình sender
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());

    // Cấu hình buzzer
    ledcSetup(buzzerChannel, buzzerFrequency, 8);
    ledcAttachPin(buzzerPin, buzzerChannel);

    // Khởi tạo ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        lcd.clear();
        lcd.print("LOI ESP-NOW!");
        return;
    }
    
    // Đăng ký callback
    esp_now_register_recv_cb(OnDataRecv);
    
    Serial.println("Receiver initialized. Waiting for data...");
}

void loop() {
    // Không cần làm gì trong loop, mọi thứ xử lý trong callback
    delay(1000);
}
