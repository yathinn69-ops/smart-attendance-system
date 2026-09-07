/*
 * Smart Attendance System
 * -----------------------
 * IoT-based attendance system using ESP32, Wi-Fi, LCD and SPIFFS.
 *
 * IMPORTANT:
 * Replace the Wi-Fi placeholders below with your own credentials
 * when testing locally. Do NOT commit real passwords to GitHub.
 *
 * Project basis:
 * - ESP32 microcontroller
 * - I2C LCD display
 * - Wi-Fi connectivity
 * - WebServer endpoint for attendance
 * - SPIFFS CSV file for attendance records
 * - Date/time timestamp using NTP
 *
 * Note:
 * This is a clean, GitHub-ready reconstruction based on the
 * functionality and code screenshots in the project report.
 * It is not claimed to be the original source file.
 */

#include <WiFi.h>
#include <WebServer.h>
#include <LiquidCrystal_I2C.h>
#include <FS.h>
#include <SPIFFS.h>
#include <time.h>

// ---------- Wi-Fi configuration ----------
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ---------- LCD ----------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------- Web server ----------
WebServer server(80);

// ---------- Attendance file ----------
const char* attendanceFile = "/Attendance.csv";

// ---------- Time configuration ----------
const long gmtOffset_sec = 19800;       // IST = UTC + 5:30
const int daylightOffset_sec = 0;

// Example student records.
// Add/change these according to your project.
String getStudentName(const String& usn) {

  if (usn == "1AT24CD001") return "Raghu";
  if (usn == "1AT24CD002") return "Chetan Achar";
  if (usn == "1AT24CD003") return "Raghu";
  if (usn == "1AT24CD004") return "Rakesh";

  // Add more students here:
  // if (usn == "YOUR_USN") return "Student Name";

  return "Unknown";
}

// ---------- Get current date/time ----------
String getCurrentDateTime() {
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    return "Time unavailable";
  }

  char timeString[25];
  strftime(timeString, sizeof(timeString),
           "%Y-%m-%d %H:%M:%S", &timeinfo);

  return String(timeString);
}

// ---------- Create CSV file if it does not exist ----------
void createAttendanceFile() {

  if (!SPIFFS.exists(attendanceFile)) {

    File file = SPIFFS.open(attendanceFile, FILE_WRITE);

    if (!file) {
      Serial.println("Failed to create attendance file");
      return;
    }

    file.println("USN,Name,Type,Timestamp");
    file.close();

    Serial.println("Attendance.csv created");
  }
}

// ---------- Check whether student already logged in today ----------
bool alreadyMarkedToday(const String& usn, const String& today) {

  File file = SPIFFS.open(attendanceFile, FILE_READ);

  if (!file) {
    return false;
  }

  while (file.available()) {

    String line = file.readStringUntil('\n');
    line.trim();

    if (line.startsWith(usn + ",")) {

      int lastComma = line.lastIndexOf(',');

      if (lastComma >= 0) {
        String timestamp = line.substring(lastComma + 1);

        if (timestamp.startsWith(today)) {
          file.close();
          return true;
        }
      }
    }
  }

  file.close();
  return false;
}

// ---------- Mark attendance ----------
void handleAttendance() {

  if (!server.hasArg("usn")) {

    server.send(
      400,
      "text/plain",
      "Missing USN"
    );

    return;
  }

  String usn = server.arg("usn");
  usn.trim();

  String name = getStudentName(usn);

  // Reject unknown users
  if (name == "Unknown") {

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Invalid USN");

    server.send(
      404,
      "text/plain",
      "Invalid USN"
    );

    delay(1500);

    lcd.clear();
    lcd.print("Ready to Scan");

    return;
  }

  String timestamp = getCurrentDateTime();

  if (timestamp == "Time unavailable") {

    server.send(
      500,
      "text/plain",
      "Time error"
    );

    return;
  }

  String today = timestamp.substring(0, 10);

  // Prevent duplicate attendance for the same day
  if (alreadyMarkedToday(usn, today)) {

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Already Logged");
    lcd.setCursor(0, 1);
    lcd.print(name.substring(0, 16));

    server.send(
      200,
      "text/plain",
      "Already logged today: " + name
    );

    delay(1500);

    lcd.clear();
    lcd.print("Ready to Scan");

    return;
  }

  // Open attendance file
  File file = SPIFFS.open(attendanceFile, FILE_APPEND);

  if (!file) {

    server.send(
      500,
      "text/plain",
      "File error"
    );

    return;
  }

  // Record attendance
  file.print(usn);
  file.print(",");
  file.print(name);
  file.print(",Login,");
  file.println(timestamp);

  file.close();

  // Show result on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Attendance OK");

  lcd.setCursor(0, 1);
  lcd.print(name.substring(0, 16));

  // Response to client
  server.send(
    200,
    "text/plain",
    "Attendance recorded: " + name + " at " + timestamp
  );

  Serial.println("Attendance recorded");
  Serial.println("USN: " + usn);
  Serial.println("Name: " + name);
  Serial.println("Time: " + timestamp);

  delay(1500);

  lcd.clear();
  lcd.print("Ready to Scan");
}

// ---------- Download attendance CSV ----------
void handleDownload() {

  if (!SPIFFS.exists(attendanceFile)) {

    server.send(
      404,
      "text/plain",
      "File not found"
    );

    return;
  }

  File file = SPIFFS.open(attendanceFile, FILE_READ);

  if (!file) {

    server.send(
      500,
      "text/plain",
      "Unable to open file"
    );

    return;
  }

  server.streamFile(file, "text/csv");
  file.close();
}

// ---------- Clear attendance file ----------
void handleClear() {

  if (SPIFFS.remove(attendanceFile)) {

    createAttendanceFile();

    server.send(
      200,
      "text/plain",
      "Attendance file cleared"
    );

    Serial.println("Attendance file cleared");

  } else {

    server.send(
      500,
      "text/plain",
      "Could not clear file"
    );
  }
}

// ---------- Home page ----------
void handleRoot() {

  String html =
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<title>Smart Attendance System</title>"
    "<style>"
    "body{font-family:Arial;max-width:700px;margin:40px auto;padding:20px;}"
    "input,button{padding:10px;margin:5px;}"
    "button{cursor:pointer;}"
    "</style>"
    "</head>"
    "<body>"
    "<h1>Smart Attendance System</h1>"
    "<p>ESP32 Attendance Server</p>"
    "<form action='/attendance'>"
    "<input name='usn' placeholder='Enter USN' required>"
    "<button type='submit'>Mark Attendance</button>"
    "</form>"
    "<p><a href='/download'>Download Attendance CSV</a></p>"
    "</body>"
    "</html>";

  server.send(200, "text/html", html);
}

// ---------- Setup ----------
void setup() {

  Serial.begin(115200);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Attendance");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {

    Serial.println("SPIFFS Mount Failed");

    lcd.clear();
    lcd.print("SPIFFS Failed");

    while (true) {
      delay(1000);
    }
  }

  createAttendanceFile();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);

  lcd.clear();
  lcd.print("Connecting WiFi");

  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  // Configure NTP time
  configTime(
    gmtOffset_sec,
    daylightOffset_sec,
    "pool.ntp.org",
    "time.nist.gov"
  );

  // Web server routes
  server.on("/", handleRoot);
  server.on("/attendance", handleAttendance);
  server.on("/download", handleDownload);
  server.on("/clear", handleClear);

  server.begin();

  Serial.println("HTTP server started");

  lcd.clear();
  lcd.print("Ready to Scan");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP().toString().substring(0, 16));
}

// ---------- Main loop ----------
void loop() {
  server.handleClient();
}
