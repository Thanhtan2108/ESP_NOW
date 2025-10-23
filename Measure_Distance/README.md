# Dự Án Đo Khoảng Cách Không Dây với HC-SR04 & LCD1602 qua ESP-NOW

## Mục Lục

- [1. Tổng quan](#1-tổng-quan)
- [2. Nguyên lý hoạt động](#2-nguyên-lý-hoạt-động)
- [3. Chuẩn Bị Linh Kiện](#3-chuẩn-bị-linh-kiện)
- [4. Sơ Đồ Đấu Nối](#4-sơ-đồ-đấu-nối)
- [4. Hướng Dẫn Chi Tiết Thực Hiện](#4-hướng-dẫn-chi-tiết-thực-hiện)
- [5. Giải thích kỹ thuật](#5-giải-thích-kỹ-thuật)
- [6. Xử lý lỗi thường gặp](#6-xử-lý-lỗi-thường-gặp)
- [7. Mở rộng dự án](#7-mở-rộng-dự-án)

## 1. Tổng quan

Dự án sử dụng 2 module ESP32-WROOM-32 để xây dựng hệ thống đo khoảng cách không dây:

- Transmitter: Đo khoảng cách bằng cảm biến siêu âm HC-SR04 và gửi dữ liệu qua giao thức ESP-NOW

- Receiver: Nhận dữ liệu không dây và hiển thị lên màn hình LCD1602

Toàn bộ phát triển sử dụng PlatformIO với Arduino Framework trên VS Code.

Đặc điểm nổi bật:

- Giao tiếp không dây qua ESP-NOW (tiêu thụ ít năng lượng, độ trễ thấp)

- Đo khoảng cách chính xác từ 2cm - 400cm

- Hiển thị thời gian thực trên LCD

- Xử lý nhiễu và lỗi truyền thông

## 2. Nguyên lý hoạt động

### 2.1 Đo khoảng cách với HC-SR04

```text
Phát xung 10µs → Truyền sóng siêu âm → Nhận sóng phản hồi → Tính thời gian → Tính khoảng cách
```

Công thức: `Khoảng cách = (Thời gian × Vận tốc âm thanh) / 2`

### 2.2 Truyền thông ESP-NOW

- Giao thức peer-to-peer của Espressif

- Không cần kết nối WiFi

- Độ trễ thấp (< 10ms)

- Khoảng cách lên đến 200m (trong điều kiện lý tưởng)

## 3. Chuẩn Bị Linh Kiện

### 3.1 Linh kiện chính

- ESP32-WROOM-32 × 2

- Cảm biến siêu âm HC-SR04 × 1

- Màn hình LCD1602 với module I2C × 1

- Breadboard × 2

- Dây jumper (đực-đực, đực-cái)

### 3.2 Linh kiện phụ trợ

- Điện trở 1kΩ × 1

- Điện trở 2.2kΩ × 1 (để chia áp cho HC-SR04)

- Nguồn 5V ổn định (cho HC-SR04)

- Cable Micro-USB (cho ESP32)

## 4. Sơ Đồ Đấu Nối

### 4.1 Transmitter (HC-SR04 + ESP32)

| HC-SR04 Pin | ESP32 Pin | Ghi chú |
|-------------|-----------| ------- |
| VCC | 5V | Quan trọng: Dùng 5V |
| GND | GND | |
| TRIG | GPIO 27 | |
| ECHO | GPIO 26* | Cần chia áp: 5V → 3.3V |

\* Cần thêm mạch chia điện trở nếu dùng ECHO

```text
HC-SR04 ECHO (5V) → 1kΩ → ESP32 GPIO 26 → 2.2kΩ → GND
```

### 4.2 Receiver (LCD1602 I2C + ESP32)

| LCD I2C Pin | ESP32 Pin | Ghi chú |
|-------------|-----------| ------- |
| VCC | 3.3V/5V | Tùy module |
| GND | GND | |
| SDA | GPIO 21 | |
| SCL | GPIO 22 | |

\* Cần 2 trở kéo lên cho 2 đường SDA và SCL

## 4. Hướng Dẫn Chi Tiết Thực Hiện

- Kết nối linh kiện theo mục 4.1 và 4.2.

- Lập cấu hình PlatformIO: tạo platformio.ini, chọn đúng board esp32dev.

```ini
; cho transmitter
[env:esp32doit-devkit-v1]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
monitor_speed = 115200

; cho receiver
[env:esp32doit-devkit-v1]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
monitor_speed = 115200
lib_deps = marcoschwartz/LiquidCrystal_I2C@^1.1.4
```

- Lấy địa chỉ MAC RX: Nạp code receiver, mở Serial Monitor, xem dòng MAC.

```cpp
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_MODE_STA);
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {}
```

- Điền MAC vào transmitter: cập nhật receiverAddress.

- Upload:

    - Mở folder Receiver, upload receiver code.

    - Mở folder Transmitter, upload transmitter code.

- Kiểm tra: Serial Monitor sẽ hiện dữ liệu gửi/nhận, LCD1602 hiển thị khoảng cách.

## 5. Giải thích kỹ thuật

### 5.1 Cấu trúc dữ liệu ESP-NOW

```cpp
struct DistanceData {
  float distance;    // 4 bytes - Khoảng cách (cm)
  uint32_t timestamp; // 4 bytes - Thời gian gửi
  uint16_t sequence;  // 2 bytes - Số thứ tự gói tin
}; // Tổng: 10 bytes
```

### 5.2 Luồng hoạt động

**Transmitter:**

- Đọc giá trị từ HC-SR04 (lọc nhiễu, lấy trung bình)

- Đóng gói dữ liệu vào struct

- Gửi qua ESP-NOW

- Nhận callback xác nhận

**Receiver:**

- Chờ dữ liệu ESP-NOW

- Kiểm tra tính hợp lệ của dữ liệu

- Hiển thị lên LCD và Serial Monitor

- Phát hiện mất kết nối

### 5.3 Xử lý nhiễu HC-SR04

```c
// Đọc 3 lần lấy trung bình
for(int i = 0; i < 3; i++) {
  // ... đo khoảng cách
  delay(50); // Chống nhiễu
}
// Lọc giá trị ngoài phạm vi 2-400cm
```

## 6. Xử lý lỗi thường gặp

### 6.1 Lỗi không nhận dữ liệu

**Nguyên nhân:**

- Sai địa chỉ MAC

- ESP-NOW chưa khởi tạo

- Khoảng cách 2 ESP32 quá xa

**Giải pháp:**

```cpp
// Kiểm tra khởi tạo ESP-NOW
if (esp_now_init() != ESP_OK) {
  Serial.println("Lỗi khởi tạo ESP-NOW");
  ESP.restart();
}
```

### 6.2 LCD không hiển thị

**Nguyên nhân:**

- Sai địa chỉ I2C

- Chưa kết nối đúng chân

- Thiếu nguồn 5V

**Giải pháp:**

```cpp
// Quét địa chỉ I2C
#include <Wire.h>
void setup() {
  Wire.begin();
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Scanning I2C...");
  for (byte i = 8; i < 120; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found: 0x"); Serial.println(i, HEX);
    }
  }
}
```

### 6.3 HC-SR04 đo sai

**Nguyên nhân:**

- Nhiễu sóng siêu âm

- Vật cản không phẳng

- Nguồn không ổn định

**Giải pháp:**

- Thêm tụ 100µF gần VCC của HC-SR04

- Đảm bảo vật đo vuông góc với cảm biến

- Sử dụng nguồn 5V ổn định

## 7. Mở rộng dự án

### 7.1 Thêm nhiều Receiver

```cpp
// Thêm multiple peers
uint8_t receiver2[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
esp_now_add_peer(receiver2, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
```

### 7.2 Truyền dữ liệu 2 chiều

- Receiver gửi xác nhận về Transmitter

- Hiển thị RSSI (cường độ tín hiệu)

### 7.3 Lưu trữ dữ liệu

- Kết nối SD Card để lưu log

- Gửi dữ liệu lên cloud qua WiFi

## 7.4 Tối ưu năng lượng

```cpp
// Sử dụng deep sleep
esp_sleep_enable_timer_wakeup(1000000); // 1 giây
esp_deep_sleep_start();
```

🎯 Kết luận
Dự án này cung cấp giải pháp đo khoảng cách không dây ổn định, có thể ứng dụng trong:

- Hệ thống cảnh báo khoảng cách

- Đo mức chất lỏng

- Robot tránh vật cản

- Hệ thống giám sát công nghiệp

Với code đã được tối ưu và xử lý lỗi đầy đủ, hệ thống hoàn toàn sẵn sàng cho các ứng dụng thực tế.

Chúc bạn thực hiện dự án thành công! 🚀
