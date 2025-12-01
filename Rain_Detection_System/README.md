# Hệ Thống Phát Hiện Mưa Không Dây ESP-NOW

## Mô Tả Dự Án

Dự án này là một hệ thống phát hiện mưa không dây sử dụng giao thức ESP-NOW để giao tiếp giữa hai module ESP32. Hệ thống bao gồm:

- Khối cảm biến (Sender): ESP32 kết nối với cảm biến mưa, đọc giá trị analog và gửi dữ liệu qua ESP-NOW

- Khối hiển thị (Receiver): ESP32 kết nối với màn hình LCD 1602 (giao tiếp I2C) và buzzer, nhận dữ liệu và hiển thị thông báo

Khi phát hiện mưa, hệ thống sẽ hiển thị cảnh báo trên màn hình LCD và kích hoạt buzzer phát âm thanh cảnh báo.

## Linh Kiện Cần Thiết

- 2 x ESP32 DevKit V1 (30 chân) dựa trên ESP-WROOM-32

- Cảm biến mưa (Rain Sensor Module)

- Màn hình LCD 1602 với module I2C

- Buzzer thụ động (Passive Buzzer)

- Dây nối (jumper wires)

- Breadboard (2 cái)

- Điện trở 10kΩ (tuỳ chọn, cho cảm biến mưa)

## Sơ Đồ Kết Nối

### Cảm Biến (Sender)

```text
Cảm biến mưa   ->   ESP32
   VCC         ->    3.3V
   GND         ->    GND
   AO (Analog) ->    GPIO 34
```

### Khối Hiển Thị (Receiver)

```text
LCD I2C        ->   ESP32
   VCC         ->    5V (hoặc 3.3V)
   GND         ->    GND
   SDA         ->    GPIO 21
   SCL         ->    GPIO 22

Buzzer         ->   ESP32
   Chân (+)    ->    GPIO 5 (thông qua resistor 100Ω)
   Chân (-)    ->    GND
```

## Cài Đặt Phần Mềm

### Yêu Cầu Hệ Thống

- PlatformIO IDE (Extension trong VSCode)

- Thư viện LiquidCrystal I2C cho PlatformIO

### Các Bước Cài Đặt

1. Clone hoặc tải dự án

    ```bash
   git clone https://github.com/Thanhtan2108/ESP_NOW.git
   cd ESP_NOW/Rain_Detection_System
    ```

2. Mở project trong PlatformIO

    - Mở thư mục sender_node trong PlatformIO

    - Mở thư mục receiver_node trong PlatformIO

3. Cài đặt thư viện

    - Thư viện sẽ tự động được cài đặt thông qua platformio.ini

## Cách Chạy Dự Án

Bước 1: Nạp Code Cho Receiver

- Mở project receiver_node trong PlatformIO

- Kết nối ESP32 receiver với máy tính

- Nạp code bằng cách nhấn nút Upload trong PlatformIO

- Mở Serial Monitor để xem địa chỉ MAC của receiver

Bước 2: Cấu Hình Sender

- Mở project sender_node trong PlatformIO

- Trong file src/main.cpp, thay đổi địa chỉ MAC trong dòng:

```cpp
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
```

thành địa chỉ MAC của receiver (lấy từ Serial Monitor)

Bước 3: Nạp Code Cho Sender

- Kết nối ESP32 sender với máy tính

- Nạp code bằng cách nhấn nút Upload trong PlatformIO

Bước 4: Kết Nối Phần Cứng

- Đấu nối cảm biến mưa với sender ESP32 theo sơ đồ

- Đấu nối LCD và buzzer với receiver ESP32 theo sơ đồ

- Cấp nguồn cho cả hai board ESP32

Bước 5: Kiểm Tra Hoạt Động

- Kiểm tra kết nối không dây giữa hai ESP32

- Thử làm ướt cảm biến mưa để kiểm tra hoạt động

- Xác minh thông báo hiển thị trên LCD và âm thanh từ buzzer

## Cấu Hình Và Tùy Chỉnh

### Điều Chỉnh Ngưỡng Mưa

Trong code sender, bạn có thể điều chỉnh ngưỡng mưa bằng cách thay đổi giá trị:

```cpp
const int RAIN_THRESHOLD = 3500;  // Giá trị từ 0-4095
```

### Điều Chỉnh Âm Thanh Buzzer

Trong code receiver, bạn có thể điều chỉnh tần số buzzer:

```cpp
const int buzzerFrequency = 1000;  // Thay đổi tần số (Hz)
```

## Khắc Phục Sự Cố

### LCD Không Hiển Thị

- Kiểm tra địa chỉ I2C (0x27 hoặc 0x3F)

- Kiểm tra kết nối nguồn và dây tín hiệu

### ESP-NOW Không Kết Nối

- Kiểm tra địa chỉ MAC đã chính xác chưa

- Đảm bảo cả hai ESP32 cùng chế độ WiFi (STA)

### Buzzer Không Kêu

- Kiểm tra buzzer thụ động (passive) hay chủ động (active)

- Kiểm tra kết nối chân và điện trở

## Cấu Trúc Thư Mục

```text
rain_detection_system/
├── sender_node/
│   ├── include/
│   ├── lib/
│   ├── src/
│   │   └── main.cpp
│   └── platformio.ini
└── receiver_node/
    ├── include/
    ├── lib/
    ├── src/
    │   └── main.cpp
    └── platformio.ini
```
