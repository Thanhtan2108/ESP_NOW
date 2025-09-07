#include <esp_now.h>
#include <WiFi.h>

// Thay đổi địa chỉ MAC này thành địa chỉ MAC của receiver
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Cấu trúc dữ liệu để gửi
typedef struct struct_message {
    bool isRaining;
    int sensorValue;  // Thêm giá trị cảm biến để debug
} struct_message;

struct_message myData;

// Callback khi dữ liệu được gửi
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

const int rainSensorPin = 34;  // GPIO kết nối với cảm biến mưa (chân analog)
const int RAIN_THRESHOLD = 3500;  // Ngưỡng xác định trời mưa

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    // Khởi tạo ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Đăng ký callback
    esp_now_register_send_cb(OnDataSent);
    
    // Thêm peer
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }

    // Không cần thiết lập pinMode cho chân analog
    // analogRead() sẽ tự động thiết lập chân là input
}

void loop() {
    // Đọc giá trị analog từ cảm biến mưa (0-4095)
    int sensorValue = analogRead(rainSensorPin);
    
    // Giá trị thấp hơn ngưỡng nghĩa là có mưa
    myData.isRaining = (sensorValue < RAIN_THRESHOLD); // True | False
    myData.sensorValue = sensorValue;  // Lưu giá trị để debug
    
    // In giá trị ra Serial để debug
    Serial.print("Sensor Value: ");
    Serial.print(sensorValue);
    Serial.print(" - Status: ");
    Serial.println(myData.isRaining ? "Mưa" : "Không mưa");
    
    // Gửi dữ liệu via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
    if (result == ESP_OK) {
        Serial.println("Sent with success");
    } else {
        Serial.println("Error sending the data");
    }
    
    delay(2000);  // Gửi dữ liệu mỗi 2 giây
}
