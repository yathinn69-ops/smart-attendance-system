# IoT-Based Smart Attendance System

An IoT-based Smart Attendance System developed using ESP32 to automate
attendance recording and maintain digital attendance records.

## 📌 Project Overview

Traditional attendance systems are time-consuming and can be prone to
human errors and proxy attendance.

This project demonstrates an IoT-based approach to attendance management
using an ESP32 microcontroller, Wi-Fi connectivity, and an LCD display.

The system processes user identification, records attendance with the
current date and time, and stores attendance information digitally.

## 🚀 Features

- Automated attendance recording
- ESP32-based IoT system
- Wi-Fi connectivity
- LCD display for system feedback
- User identification and verification
- Date and time-based attendance records
- CSV-based attendance storage
- Web-based interaction with the ESP32
- Attendance record download

## 🛠️ Technologies Used

- ESP32
- C/C++
- Arduino IDE
- Wi-Fi
- LCD Display
- SPIFFS
- HTTP/Web Server
- CSV

## 🔧 Hardware Components

- ESP32
- LCD Display
- Jumper Wires
- Power Supply
- Computer/Laptop

## ⚙️ System Working

1. The ESP32 starts and initializes the LCD and storage system.
2. The ESP32 connects to a Wi-Fi network.
3. The system receives the user's identification.
4. The ESP32 verifies the user.
5. If the user is valid, attendance is recorded.
6. The current date and time are stored with the attendance record.
7. The attendance data is saved in CSV format.
8. The attendance record can be accessed through the ESP32 web server.

## 📂 Project Structure

```text
smart-attendance-system/
│
├── smart_attendance.ino
└── README.md
