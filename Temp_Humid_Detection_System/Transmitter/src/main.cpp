#include <Arduino.h>
// Khai báo các thư viện dùng cho ESP-NOW
#include <esp_now.h>
#include <WiFi.h>
// Khai báo thư viện Sensor
#include <DHT.h>


/*=================Initial for Sensor DHT============*/
// Chỉ định loại Sensor DHT do có nhiều loại
#define DHTTYPE DHT11

// Khai báo các chân kết nối
#define DHT11_PIN 4

// Khởi tạo kết nối loại cảm biến DHT với chân ESP
DHT dht(DHT11_PIN, DHTTYPE);


/*=================Initial for ESP-NOW============*/
// xác định địa chỉ MAC của module nhận data
// Thay bằng MAC thực tế
uint8_t receiverAddress[] = {0x24, 0x0A, 0xC4, 0xFF, 0xFF, 0xFF};

// xây dựng 1 struct chứa các thông tin cần gửi đi
typedef struct struct_message {
  float Temperature;
  float Humidity;
} struct_message;

// khởi tạo 1 đối tượng mang các thông tin cần gửi đi
struct_message myData;

// xây dựng hàm callback, được gọi khi data được gửi xong
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {  // Callback gửi
  Serial.print("Gửi data: "); 
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Thành công!" : "Thất bại!");
}


void setup() {
  // khởi tạo UART để debug và hiển thị trên monitor
  Serial.begin(115200);
  // Đợi Serial ổn định
  delay(100);

  /*===================CONFIG COMMUNICATE SENSOR=====================*/
  // Khởi động cảm biến DHT bắt đầu hoạt động
  dht.begin();

  // In ra thông báo khi khởi động thành công
  Serial.println("Cảm biến DHT11 khởi động!");

  /*====================CONFIG ESP-NOW======================*/
  // Bật chế độ WiFi trên ESP
  WiFi.mode(WIFI_STA);
  // Xem địa chỉ MAC của thiết bị gửi(có thể dùng cho mục đích khác)
  // Serial.println("Địa chỉ MAC của thiết bị: " + WiFi.macAddress());

  // Bật cho phép ESP-NOW hoạt động và kiểm tra lỗi kết nối ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Lỗi khởi tạo ESP-NOW!");
    while (1); // Dừng chương trình nếu lỗi
  }
  Serial.println("ESP-NOW khởi tạo thành công!");

  // Đăng ký hàm callback để khi gửi data xong hàm sẽ được gọi
  esp_now_register_send_cb(OnDataSent);

  // cấu hình peer (thiết bị nhận data) và kiểm tra lỗi 
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0; // Kênh WiFi (0 = tự động)
  peerInfo.encrypt = false; // Không mã hóa
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Lỗi thêm peer!");
    while (1); // Dừng chương trình nếu lỗi
  }
  Serial.println("Thêm peer thành công!");
}

void loop() {
  /*=============Giải thuật xử lý giao tiếp với Sensor========================*/
  // Trước khi đọc cảm biến delay 2s đọc 1 lần theo datasheet
  delay(2000);

  // Đọc giá trị nhiệt độ từ cảm biến
  float temperature = dht.readTemperature();

  // Đọc giá trị độ ẩm từ cảm biến
  float humidity = dht.readHumidity();

  // Kiểm tra nếu đọc dữ liệu thất bại
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Lỗi: Không thể đọc dữ liệu từ DHT11!");
    return;
  }

  // In dữ liệu để debug
  Serial.print("Nhiệt độ: ");
  Serial.print(temperature);
  Serial.print("°C, Độ ẩm: ");
  Serial.print(humidity);
  Serial.println("%");

  /*========================Gửi data qua ESP-NOW========================*/
  // Gán giá trị đọc được vào đối tượng để mang data gửi đi
  myData.Temperature = temperature;
  myData.Humidity = humidity;

  // Gửi dữ liệu qua ESP-NOW và lưu kết quả trả về để kiểm tra lỗi
  esp_err_t result = esp_now_send(receiverAddress, (uint8_t *) &myData, sizeof(myData));
  
  // Kiểm tra lỗi khi gửi nếu có
  if (result != ESP_OK) {
    Serial.println("Lỗi gửi dữ liệu qua ESP-NOW!");
  }
}
