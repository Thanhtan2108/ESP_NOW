# Wireless Temperature and Humidity Monitoring System

This project implements a wireless temperature and humidity monitoring system using two ESP32 development boards communicating via ESP-NOW protocol. One ESP32 acts as a transmitter (TX) to collect sensor data, while the other acts as a receiver (RX) to display and monitor the data.

## Project Overview

The system consists of two main components:

1. **Transmitter (ESP32_ESP_NOW_TX)**: Reads temperature and humidity data from a DHT11 sensor and transmits it wirelessly.
2. **Receiver (ESP32_ESP_NOW_RX)**: Receives the sensor data and displays it on an LCD screen, with alert functionality when thresholds are exceeded.

## Project Structure

```plaintext
Wireless_Temperature_Humidity/
├── ESP32_ESP_NOW_RX/          # Receiver node
│   ├── platformio.ini         # PlatformIO configuration
│   └── src/
│       └── main.cpp          # Receiver code
└── ESP32_ESP_NOW_TX/          # Transmitter node
    ├── platformio.ini         # PlatformIO configuration
    └── src/
        └── main.cpp          # Transmitter code
```

## Hardware Components

### Transmitter Node (ESP32_ESP_NOW_TX)

- 1x ESP32 DevKit V1 board
- 1x DHT11 Temperature and Humidity sensor
- 1x LED (for status indication)
- 1x 220Ω resistor
- Jumper wires

### Receiver Node (ESP32_ESP_NOW_RX)

- 1x ESP32 DevKit V1 board
- 1x LCD1602 display with I2C module
- 1x Passive buzzer
- 1x LED (for status indication)
- 1x 220Ω resistor
- Jumper wires

## Wiring Instructions

### Transmitter (ESP32_ESP_NOW_TX)

- **DHT11 Sensor**:
  - VCC → 3.3V
  - GND → GND
  - DATA → GPIO4
- **Status LED**:
  - Anode (+) → GPIO2
  - Cathode (-) → 220Ω resistor → GND

### Receiver (ESP32_ESP_NOW_RX)

- **LCD1602 (with I2C module)**:
  - SDA → GPIO21
  - SCL → GPIO22
  - VCC → 5V
  - GND → GND
- **Buzzer**:
  - Positive (+) → GPIO5
  - Negative (-) → GND
- **Status LED**:
  - Anode (+) → GPIO2
  - Cathode (-) → 220Ω resistor → GND

## Theoretical Background

### ESP-NOW Protocol

ESP-NOW is a fast, connectionless communication protocol developed by Espressif for ESP32 and ESP8266 devices. It allows devices to communicate directly without requiring a traditional Wi-Fi router or access point. Key features:

- Low power consumption
- No need for Wi-Fi connection
- Fast data transmission
- Supports one-to-many communication

### Sensor Operation

The DHT11 sensor uses a capacitive humidity sensor and a thermistor to measure the surrounding air, outputting a digital signal on the data pin.

## Alert Thresholds

The system includes built-in alerts for:

- Temperature > 35°C
- Humidity > 80%

## Installation and Setup Instructions

1. **Prerequisites**:

   - Install [Visual Studio Code](https://code.visualstudio.com/)
   - Install [PlatformIO extension](https://platformio.org/install/ide?install=vscode)

2. **Clone the Repository**:

   ```bash
   git clone https://github.com/Thanhtan2108/ESP_NOW.git
   cd ESP_NOW/Wireless_Temperature_Humidity
   ```

3. **Install Dependencies**:

   - For Transmitter:
     - Open ESP32_ESP_NOW_TX in VS Code
     - PlatformIO will automatically install:
       - DHT sensor library by Adafruit
   - For Receiver:
     - Open ESP32_ESP_NOW_RX in VS Code
     - PlatformIO will automatically install:
       - LiquidCrystal_I2C library

4. **Configure MAC Address**:

   - Get the MAC address of your receiver ESP32:
     - Upload and run the basic WiFi scanner example
     - Note the MAC address displayed in serial monitor
   - Update the `receivedAddressMAC` array in the transmitter's code

5. **Upload Code**:

   - Connect the receiver ESP32 to your computer
   - Open ESP32_ESP_NOW_RX in VS Code
   - Click the PlatformIO Upload button
   - Repeat the process for the transmitter (ESP32_ESP_NOW_TX)

6. **Verify Operation**:
   - The transmitter's LED will light up when data is successfully sent
   - The receiver's LCD will display temperature and humidity readings
   - The buzzer will sound if thresholds are exceeded

## Troubleshooting

1. **LCD Not Displaying**:

   - Verify I2C address (default is 0x27, might need adjustment)
   - Check I2C connections

2. **No Data Transmission**:

   - Verify MAC address configuration
   - Ensure both ESP32s are powered and within range
   - Check DHT11 sensor connections

3. **Compilation Errors**:
   - Ensure all libraries are properly installed
   - Check PlatformIO environment settings

## Contributing

Feel free to submit issues and enhancement requests!
