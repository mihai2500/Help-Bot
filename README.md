# HelpBot 
**Smart IoT Baby Monitor & Environmental Safety System**

> **Award:** 2nd Place — *InfoEducatie* Contest (Robots Section), County Stage.

## About The Project
**HelpBot** is an advanced baby monitoring system designed to go beyond simple video streaming. Unlike passive monitors, HelpBot actively analyzes the environment to ensure safety and comfort.

Developed using Arduino and ESP32 technology, this robot monitors critical data points—temperature, humidity, air quality, and noise levels—and autonomously alerts the parent via SMS if any anomalies are detected. The system features a dual-processor architecture: an Arduino handles sensor logic and GSM communication, while an ESP32-CAM handles real-time video streaming.

## Key Features

 **Real-Time Video Streaming:** Uses an ESP32-CAM to host a web server, allowing parents to view a live feed from any browser on the local network.
 **GSM SMS Alerts:** Integrated SIM800L module sends instant text messages for:
    * Gas/Smoke detection.
    * High noise levels (e.g., baby crying).
    * High temperature or humidity alerts.
 **Air Quality Analysis:**
   **Dust:** Monitors fine particles (PM1.0 and PM2.5) using a DSM501A sensor.
   **Gas:** Detects hazardous gases via an MQ-2 sensor.
 **Interactive Hardware UI:**
    **4x4 Keypad:** For menu navigation and secure phone number association.
    **Dual LCD Screens:** Displays live sensor data, system status, and the camera's IP address.

## Hardware Components

**Main Controller:** Arduino UNO.
**Video Module:** ESP32-CAM.
**Communication:** SIM800L GSM Module.
**Sensors:**
    **DHT11:** Temperature & Humidity.
    **MQ-2:** Analog Gas & Smoke Sensor.
    **Microphone Module:** Analog Sound detection.
    **DSM501A:** Dust Sensor.
**Display & Input:**
    * 2x LiquidCrystal I2C Displays (16x2) (Addresses `0x27` & `0x26`).
    * 4x4 Matrix Keypad.

## Pin Configuration

| Component | Arduino Pin | Description |
| :--- | :--- | :--- |
| **DHT11** | 13 | Temp/Hum Sensor |
| **MQ-2** | A2 | Gas Sensor |
| **Mic** | A3 | Microphone |
| **PM1** | 10 | Dust Sensor PM1.0 |
| **PM2.5** | 11 | Dust Sensor PM2.5 |
| **SIM800L** | 14, 15 | RX, TX (SoftwareSerial) |
| **Keypad Rows** | 9, 8, 7, 6 | Matrix Rows |
| **Keypad Cols** | 5, 4, 3, 2 | Matrix Columns |

## Software Setup

The project consists of two distinct firmware uploads:

### 1. ESP32-CAM Setup (`CameraStream.ino`)
1.  Open `CameraStream.ino`.
2.  **Important:** Edit the `ssid` and `password` variables to match your local WiFi credentials.
3.  Select "ESP32-CAM" in the Board Manager.
4.  Upload the code and open the Serial Monitor to find the IP Address.

### 2. Arduino Setup (`HelpBot.ino`)
1.  Open `HelpBot.ino`.
2.  Update the `String ip` variable with the IP address obtained from the ESP32-CAM.
3.  Ensure the following libraries are installed:
    * `Keypad.h`
    * `DHT_Async.h`
    * `LiquidCrystal_I2C.h`
4.  Upload the code to your Arduino board.

## How to Use

### Initialization
Upon booting, the system displays "Initializing..." for approximately **20 seconds**. This delay allows the MQ-2 gas sensor to warm up and the SIM800L to connect to the mobile network.

### User Association
1.  The LCD will prompt: `Phone Number:`.
2.  Type the parent's mobile number using the keypad.
3.  Press `*` to **Confirm** and save the number. A confirmation SMS will be sent.
4.  Press `#` to **Reset** the number if needed.

### Menu System
Use the Keypad to navigate the interface:
**Press 'A':** Display the saved Phone Number and Camera IP.
**Press 'B':** Display Temperature (°C) and Humidity (%).
**Press 'C':** Display Gas and Smoke detection status.
**Press 'D':** Display Dust level analysis.

---
