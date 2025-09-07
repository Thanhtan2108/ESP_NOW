# Tìm hiểu về ESP-NOW

## 1. ESP-NOW là gì (tóm tắt)

ESP-NOW là một giao thức kết nối không cần Wi-Fi(connectionless) do Espressif định nghĩa, cho phép gửi frame (action frame) trực tiếp từ thiết bị này sang thiết bị khác mà không cần router / TCP-IP stack. Nó thích hợp cho các tin nhắn nhỏ, phản hồi nhanh, tiêu thụ năng lượng thấp (remote control, sensor,…).

## 2. Nguyên lý hoạt động chính

Hoạt động ở lớp MAC / Wi-Fi: dữ liệu được đóng gói trong một vendor-specific action frame và phát trực tiếp trên sóng. Vì vậy kênh Wi-Fi (channel) mà cả hai dùng phải trùng nhau.

Peer table: ESP-NOW quản lý “peers” (định danh bằng MAC address). Để gửi unicast tới một peer thường cần thêm peer vào bảng peer (peer info = MAC + channel + encrypt flag). Broadcast thì gửi tới MAC FF:FF:FF:FF:FF:FF.

Bảo mật: hỗ trợ chế độ mã hóa (CCMP/keys). Khi bật encrypted mode, cần trao/đặt khóa (ví dụ local master key) giữa các peer. (Có giới hạn số peer mã hóa).

Kích thước/payload: phiên bản cổ điển (ESP-NOW v1) giới hạn ~250 bytes payload mỗi packet; các bản mới trong ESP-IDF có mở rộng (v2 ~1470 bytes trong một số bản) — nhưng thực tế khi tương tác với ESP8266/thiết bị cũ vẫn thường coi 250 bytes là giới hạn. Nếu cần gửi lớn hơn, phải chẻ (fragment) ở tầng ứng dụng.

Callback/RT context: hàm callback nhận gửi/nhận chạy trong task Wi-Fi (nghĩa là không nên làm tác vụ dài/khóa trong callback — chỉ ghi dữ liệu vào queue/flag và xử lý ở task khác).

## 3. Kiến trúc truyền/nhận giữa 2 board (tóm tắt)

Cả 2 board phải bật Wi-Fi và cùng kênh (ví dụ đặt WiFi.mode(WIFI_STA)).

Khởi tạo ESP-NOW (esp_now_init()), và đăng ký callback (send/recv).

Sender: thêm peer (hoặc dùng broadcast); gọi esp_now_send() với MAC đích + payload; đăng ký callback send để biết kết quả (ACK ở MAC layer).

Receiver: đăng ký callback nhận (esp_now_register_recv_cb) — callback này nhận mac + data + len để xử lý. Nếu muốn gửi phản hồi, receiver có thể cũng gọi esp_now_send() về MAC của sender (hai chiều).

## 4. Mẫu code (Arduino/ESP32) — nhanh, đủ để thử

Lưu ý: thay receiverMac bằng MAC thật (xem Serial.print(WiFi.macAddress())) nếu gửi unicast; hoặc dùng broadcast {0xFF,...}.

Ví dụ: Sender (gửi 1 chuỗi, 1-to-1)

```c
#include <WiFi.h>
#include <esp_now.h>

uint8_t receiverMac[6] = {0x24,0x6F,0x28,0xAA,0xBB,0xCC}; // ví dụ -> thay bằng MAC thật

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status to ");
  for(int i=0;i<6;i++){ Serial.printf("%02X", mac_addr[i]); if(i<5) Serial.print(":"); }
  Serial.print(" -> ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
  // KHÔNG làm việc nặng ở đây; chỉ ghi log hoặc post message vào queue
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);           // ESP-NOW thường chạy tốt khi ở STA mode
  WiFi.disconnect();            // tránh auto connect tới AP
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(onDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;   // 0 = use current channel; hoặc đặt số kênh cố định
  peerInfo.encrypt = false;
  if (!esp_now_is_peer_exist(receiverMac)) esp_now_add_peer(&peerInfo);
}

void loop() {
  const char *msg = "Hello from sender";
  esp_err_t err = esp_now_send(receiverMac, (uint8_t *)msg, strlen(msg) + 1);
  if (err != ESP_OK) Serial.printf("esp_now_send error: %d\n", err);
  delay(1000);
}
```

Receiver (nhận và in ra Serial)

```c
#include <WiFi.h>
#include <esp_now.h>

void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  Serial.print("Received from ");
  for(int i=0;i<6;i++){ Serial.printf("%02X", mac[i]); if(i<5) Serial.print(":"); }
  Serial.print(" : ");
  // incomingData không chữ hoàn toàn string luôn, nên dùng len
  Serial.write(incomingData, len);
  Serial.println();
  // Nếu cần phản hồi, có thể lưu mac vào queue và gửi từ task khác
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(onDataRecv);
}

void loop() {
  // Xử lý chính ở đây. Đừng làm nặng trong receive callback.
  delay(1000);
}
```

(Các ví dụ trên dựa trên API Arduino-ESP32 esp_now và patterns thường thấy trong docs/tutorials).

## 5. Giải thích từng bước code — tại sao cần làm vậy

WiFi.mode(WIFI_STA) + WiFi.disconnect()
-> Để ESP-NOW hoạt ổn định và không auto connect tới AP; ESP-NOW dùng radio Wi-Fi nên phải khởi động Wi-Fi trước. Kênh hoạt động sẽ phụ thuộc vào chế độ/softAP nếu bạn dùng cả AP và STA. Nếu hai thiết bị không cùng kênh, chúng sẽ không thể nhận nhau.

esp_now_init()
-> Khởi tạo stack ESP-NOW; bắt buộc. Nếu không init thì không thể send/recv.

esp_now_register_send_cb() / esp_now_register_recv_cb()
-> Đăng ký callback để biết kết quả gửi (ACK ở tầng MAC) và để nhận dữ liệu khi có packet đến. Callback chạy trong Wi-Fi task (không nên làm xử lý nặng trong đó).

esp_now_add_peer() (với peerInfo)
-> Để gửi unicast tới device cụ thể cần có peer record (MAC + channel + encrypt). Broadcast có thể không cần add peer (gửi tới FF:...). Việc add peer còn cần nếu bạn dùng mã hóa (encrypt) vì cần chỉ ra peer đó sử dụng key.

esp_now_send(mac, data, len)
-> API thực hiện gửi; trả về lỗi nếu không thể gửi (ví dụ peer không tồn tại, kênh khác, wifi chưa init…). Send callback sẽ báo success/fail.

Vì sao không dùng TCP/UDP ở nhiều trường hợp?
-> ESP-NOW bỏ qua stack TCP/IP, nên latency thấp, tiêu thụ năng lượng ít hơn cho các tin nhắn ngắn, và không cần router. Nhưng bù lại phải tự lo reliability, fragmentation, và quản lý peers.

## 6. Lưu ý / best practices & troubleshooting

Cùng channel: đảm bảo các thiết bị cùng channel Wi-Fi; nếu một thiết bị đang kết nối AP khác channel thì ESP-NOW có thể không nhận.

Payload > 250 bytes: nếu cần gửi nhiều hơn thì phải chia gói ở tầng ứng dụng (sequence number + reassemble) hoặc dùng API thấp hơn theo ESP-IDF (tùy bản firmware) — kiểm tra version ESP-IDF/Arduino.

Số lượng peers & mã hóa: giới hạn số peer đã mã hóa (ví dụ 6 trong SoftAP), tổng peer không mã hóa cũng có giới hạn tổng ~20; nếu cần nhiều node, dùng broadcast hoặc thiết kế “many-to-one” với hub.

Đồng thời với Wi-Fi regular: ESP-NOW có thể coexist với Wi-Fi station/AP trong nhiều trường hợp nhưng kênh/behaviour có điều kiện (đọc docs để tránh xung đột).

Không làm nặng trong callbacks: post dữ liệu sang queue/task khác để xử lý.
