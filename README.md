# Báo Cáo Khoa Học: Giao Thức ESP-NOW Trên ESP32/ESP8266 Và Ứng Dụng Trong Hệ Thống Cảm Biến

## Tóm Tắt (Abstract)

Báo cáo này tóm tắt các khái niệm lý thuyết chính về giao thức ESP-NOW trên các vi điều khiển ESP32 và ESP8266, dựa trên cuộc thảo luận kỹ thuật. Nội dung bao gồm cấu trúc code tiêu chuẩn, cơ chế hoạt động không đồng bộ, định nghĩa cấu trúc dữ liệu (struct), và ứng dụng thực tế với cảm biến. Báo cáo nhấn mạnh tính hiệu quả của ESP-NOW trong giao tiếp không dây low-power, low-latency, và cách thiết kế code để đảm bảo tính modular và dễ mở rộng. Các phân tích dựa trên nguyên tắc lập trình Arduino/ESP và tài liệu kỹ thuật từ Espressif. Phần cấu trúc code được trình bày chi tiết với demo code xây dựng trên PlatformIO (framework Arduino, board DOIT ESP32 Devkit V1, baudrate 115200) để minh họa cụ thể.

## 1. Mở Đầu (Introduction)

Giao thức ESP-NOW là một công nghệ giao tiếp không dây được phát triển bởi Espressif Systems, dành cho các thiết bị dựa trên chip ESP32 và ESP8266. Nó cho phép truyền dữ liệu nhanh chóng mà không cần kết nối Wi-Fi đầy đủ, phù hợp cho các ứng dụng IoT (Internet of Things) như giám sát cảm biến. Trong báo cáo này, chúng ta tập trung vào lý thuyết cốt lõi từ cuộc thảo luận, bao gồm cấu trúc code chung, cơ chế xử lý dữ liệu, và vai trò của cấu trúc dữ liệu. Mục tiêu là cung cấp cái nhìn tổng quan khoa học, giúp hiểu rõ cách triển khai ESP-NOW trong môi trường thực tế.

## 2. Giao Thức ESP-NOW: Nguyên Lý Hoạt Động (ESP-NOW Protocol: Operating Principles)

ESP-NOW hoạt động dựa trên các frame Wi-Fi đặc biệt (vendor-specific action frames), cho phép giao tiếp connectionless (không cần thiết lập kết nối). Nó hỗ trợ truyền dữ liệu lên đến 250 bytes với độ trễ thấp và tiêu thụ năng lượng tối ưu, phù hợp cho các hệ thống low-power.

### 2.1. Tính Không Đồng Bộ (Asynchronous Nature)

Một đặc trưng quan trọng của ESP-NOW là cơ chế không đồng bộ. Các hoạt động gửi và nhận dữ liệu được xử lý qua callback functions, chạy trong task Wi-Fi có độ ưu tiên cao. Điều này tránh blocking (treo) vòng lặp chính, đảm bảo hiệu suất thời gian thực.

- **Bên Gửi (Sender)**: Dữ liệu được xử lý trong vòng lặp `loop()` vì sender là bên chủ động. Ví dụ, đọc giá trị cảm biến, tính toán ngưỡng, và gọi `esp_now_send()` – hàm này return ngay lập tức mà không chờ kết quả. Kết quả gửi được báo qua callback `OnDataSent()`.

- **Bên Nhận (Receiver)**: Dữ liệu được xử lý trong callback `OnDataRecv()` vì receiver là bên thụ động, không biết khi nào dữ liệu đến. Callback được gọi tự động bởi hệ thống Wi-Fi khi có frame đến, tránh polling liên tục trong `loop()` (tiết kiệm CPU và đảm bảo timely processing).

Lý do thiết kế này dựa trên nguyên tắc event-driven của ESP-NOW, theo tài liệu Espressif, giúp tối ưu hóa cho các ứng dụng IoT nơi dữ liệu đến bất ngờ.

## 3. Cấu Trúc Code Tiêu Chuẩn (Standard Code Structure)

Cấu trúc code cho ESP-NOW được chia thành các giai đoạn logic, dựa trên framework Arduino/ESP, để đảm bảo tính modular và dễ debug. Các giai đoạn bao gồm include thư viện, định nghĩa struct và biến toàn cục, xây dựng hàm callback, setup(), và loop(). Code được xây dựng trong PlatformIO (một IDE dựa trên VS Code hỗ trợ lập trình embedded), sử dụng framework Arduino cho tính tương thích cao, board DOIT ESP32 Devkit V1 (một biến thể ESP32 phổ biến với 30 GPIO, hỗ trợ Wi-Fi/Bluetooth), và baudrate 115200 cho Serial debug (tốc độ tiêu chuẩn để tránh lỗi truyền dữ liệu). Trong PlatformIO, cấu hình board được đặt trong file `platformio.ini` như sau:

```c++
[env:doit_esp32_devkit_v1]
platform = espressif32
board = doit_esp32_devkit_v1
framework = arduino
monitor_speed = 115200
```

Dưới đây là trình bày cụ thể từng mục nhỏ, kèm demo code minh họa (đoạn code ngắn, có thể copy-paste vào file `main.cpp` trong PlatformIO để test). Demo tập trung vào sender để đơn giản, nhưng áp dụng tương tự cho receiver.

### 3.1. Include Thư Viện (Library Inclusion)

Nhiệm vụ: Khai báo thư viện cần thiết như `esp_now.h` (cho giao thức ESP-NOW) và `WiFi.h` (cho khởi tạo chế độ WiFi).

- Xây dựng cụ thể: Sử dụng `#include` ở đầu file `main.cpp`. Trong PlatformIO, thư viện tự động include qua framework Arduino.
- Vị trí: Đầu file, trước mọi định nghĩa.

- Thực hiện nhiệm vụ: Load API để sử dụng hàm như `esp_now_init()`.

- Thành phần và mục đích: `#include <esp_now.h>` để hỗ trợ gửi/nhận; `#include <WiFi.h>` để đặt chế độ WIFI_STA (yêu cầu bắt buộc).

- Demo code (chỉ phần include):

  ```cpp
  #include <esp_now.h>  // Hỗ trợ giao thức ESP-NOW
  #include <WiFi.h>     // Hỗ trợ WiFi mode

  void setup() {
    // Các phần sau sử dụng thư viện này
  }

  void loop() {}
  ```

  Giải thích: Include ở đầu, baudrate 115200 được đặt trong setup() sau.

### 3.2. Định Nghĩa Struct Và Biến Toàn Cục (Struct Definition and Global Variables)

Nhiệm vụ: Tạo cấu trúc dữ liệu để chứa thông tin gửi/nhận, và biến toàn cục như địa chỉ MAC.

- Xây dựng cụ thể: Sử dụng `typedef struct` để định nghĩa, sau đó khai báo biến. Kích thước struct phải < 250 bytes.

- Vị trí: Sau include, trước callback.

- Thực hiện nhiệm vụ: Đóng gói dữ liệu thành byte array cho `esp_now_send()`.

- Thành phần và mục đích: Struct với trường như `bool isRaining;` (lưu trạng thái); biến toàn cục như `struct_message myData;` (truy cập khắp nơi); mảng MAC cho peer.

- Demo code (chỉ phần struct):

  ```cpp
  #include <esp_now.h>
  #include <WiFi.h>

  typedef struct struct_message {  // Struct định nghĩa
    bool isRaining;  // Trường trạng thái mưa
  } struct_message;

  struct_message myData;  // Biến toàn cục
  uint8_t receiverAddress[] = {0x24, 0x0A, 0xC4, 0xXX, 0xXX, 0xXX};  // MAC receiver

  void setup() {}
  void loop() {}
  ```
  
  Giải thích: Struct dễ mở rộng, dùng trong PlatformIO với board ESP32.

### 3.3. Xây Dựng Hàm Callback (Callback Function Construction)

Nhiệm vụ: Định nghĩa hàm xử lý sự kiện gửi/nhận không đồng bộ.

- Xây dựng cụ thể: Định nghĩa hàm với tham số API (ví dụ: MAC và status), đăng ký trong setup().

- Vị trí: Sau struct, trước setup().

- Thực hiện nhiệm vụ: Xử lý kết quả gửi (sender) hoặc giải gói dữ liệu (receiver).

- Thành phần và mục đích: `OnDataSent()` cho sender (kiểm tra status); `OnDataRecv()` cho receiver (memcpy và xử lý).

- Demo code (chỉ callback cho sender):

  ```cpp
  #include <esp_now.h>
  #include <WiFi.h>

  void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {  // Callback gửi
    Serial.print("Gửi: "); Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "Fail");
  }

  void setup() {}
  void loop() {}
  ```
  
  Giải thích: Callback chạy asynchronous, baudrate 115200 dùng cho Serial.print().

### 3.4. Giai Đoạn Setup() (Setup Phase)

Nhiệm vụ: Khởi tạo ban đầu như Serial, WiFi, ESP-NOW, callback, và peer.

- Xây dựng cụ thể: Viết trong `void setup() { ... }`, gọi `esp_now_init()` và `esp_now_add_peer()`.

- Vị trí: Sau callback, trước loop().

- Thực hiện nhiệm vụ: Thiết lập môi trường, đăng ký callback.

- Thành phần và mục đích: `Serial.begin(115200);` cho debug; `WiFi.mode(WIFI_STA);` cho chế độ; `esp_now_register_send_cb()` cho callback.

- Demo code (chỉ setup):

  ```cpp
  #include <esp_now.h>
  #include <WiFi.h>

  uint8_t receiverAddress[] = {0x24, 0x0A, 0xC4, 0xXX, 0xXX, 0xXX};

  void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {}

  void setup() {
    Serial.begin(115200);  // Baudrate debug
    WiFi.mode(WIFI_STA);
    esp_now_init();
    esp_now_register_send_cb(OnDataSent);
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, receiverAddress, 6);
    esp_now_add_peer(&peerInfo);
  }

  void loop() {}
  ```
  
  Giải thích: Setup chạy một lần, tương thích board DOIT ESP32 Devkit V1.

### 3.5. Giai Đoạn Loop() (Loop Phase)

Nhiệm vụ: Thực hiện hoạt động lặp lại như đọc cảm biến và gửi dữ liệu.

- Xây dựng cụ thể: Viết trong `void loop() { ... }`, dùng `esp_err_t variable = esp_now_send(const uint8_t *peer_addr, const uint8_t *data, size_t len);` cho sender.

  ```c++
  Tham số:

  peer_addr: Địa chỉ MAC của thiết bị nhận (mảng uint8_t 6 bytes).

  data: Con trỏ đến dữ liệu cần gửi, kiểu const uint8_t * (con trỏ đến mảng byte).

     Với (con trỏ đến mảng byte):

    Nếu biến đã là con trỏ (như char *, int *), truyền trực tiếp con trỏ (cast nếu cần).

    Nếu biến là đối tượng (như struct, int, float), dùng & để lấy địa chỉ trước khi cast.

  len: Kích thước dữ liệu (byte), thường tính bằng sizeof() hoặc strlen() + 1 (cho string).

  Yêu cầu: Tham số data phải là một con trỏ đến vùng bộ nhớ chứa dữ liệu thực tế (byte array). Điều này có nghĩa là bạn phải truyền địa chỉ của dữ liệu vào hàm.
  ```

- Vị trí: Cuối file, sau setup().

- Thực hiện nhiệm vụ: Đọc dữ liệu, tính toán, gửi định kỳ.

- Thành phần và mục đích: Đọc analog, tính ngưỡng; `delay()` kiểm soát tần suất.

- Demo code (chỉ loop cho sender):

  ```cpp
  #include <esp_now.h>
  #include <WiFi.h>

  typedef struct struct_message { bool isRaining; } struct_message;
  struct_message myData;
  uint8_t receiverAddress[] = {0x24, 0x0A, 0xC4, 0xXX, 0xXX, 0xXX};

  void setup() {}  // Giả định đã setup

  void loop() {
    int rainValue = analogRead(34);  // Đọc cảm biến
    myData.isRaining = (rainValue < 3500);  // Tính ngưỡng
    esp_err_t result = esp_now_send(receiverAddress, (uint8_t *) &myData, sizeof(myData));  // Gửi
    delay(1000);  // Lặp mỗi giây
  }
  ```
  
  Giải thích: Loop chạy lặp, xử lý dữ liệu chính cho sender.

## 4. Định Nghĩa Cấu Trúc Dữ Liệu (Struct) Trong ESP-NOW (Data Structure Definition in ESP-NOW)

Struct là kiểu dữ liệu tổng hợp trong C/C++, dùng để nhóm các biến.

### 4.1. Các Trường Thường Gặp (Common Fields)

- ID (int): Phân biệt thiết bị.

- Giá trị cảm biến (float/int): Lưu dữ liệu thô như analog từ cảm biến mưa (0-4095).

- Trạng thái (bool): Ví dụ: true/false cho có mưa dựa trên ngưỡng 3500.

- Thông điệp (char[]): Gửi text ngắn.

- Timestamp (unsigned long): Ghi thời gian.

### 4.2. Cơ Sở Tạo Trường (Basis for Field Creation)

- Dựa vào yêu cầu ứng dụng: Dữ liệu cần truyền (ví dụ: trạng thái mưa cho buzzer).

- Giới hạn kỹ thuật: Kích thước < 250 bytes, kiểu dữ liệu cơ bản để tránh overflow.

- Tương thích: Struct giống nhau giữa sender/receiver.

- Hiệu suất: Tiết kiệm để giảm latency và năng lượng.

### 4.3. Mục Đích (Purposes)

- Đóng gói dữ liệu: Truyền cấu trúc qua ESP-NOW.

- Dễ xử lý: Giải gói và truy cập trường đơn giản.

- Mở rộng: Thêm trường mới cho ứng dụng phức tạp.

## 5. Ứng Dụng Thực Tế Với Cảm Biến (Practical Applications with Sensors)

Trong ví dụ, sender sử dụng cảm biến mưa (analog, ngưỡng 3500) để gửi trạng thái bool đến receiver, nơi hiển thị trên LCD1602 I2C và kích hoạt buzzer nếu có mưa. Điều này minh họa cách ESP-NOW tích hợp cảm biến bất kỳ, với xử lý thuật toán (ngưỡng) ở sender's loop và phản hồi ở receiver's callback.

## 6. Kết Luận (Conclusion)

ESP-NOW là giao thức mạnh mẽ cho giao tiếp không dây trong IoT, với thiết kế không đồng bộ đảm bảo hiệu suất cao. Cấu trúc code tiêu chuẩn và struct dữ liệu cung cấp nền tảng modular, dễ mở rộng cho các cảm biến. Báo cáo này tổng hợp lý thuyết từ thảo luận, nhấn mạnh tính thực tiễn trong lập trình ESP32/ESP8266. Các nghiên cứu tương lai có thể khám phá bảo mật (mã hóa PMK/LMK) và tích hợp nhiều thiết bị.

## Tài Liệu Tham Khảo (References)

- Tài liệu Espressif Systems về ESP-NOW (API Reference).

- Các tutorial Arduino/ESP về giao tiếp cảm biến và callback functions.
