#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

// ESP32 Smart Door Lock System with Buzzer Alert
// Features: RFID access control, WiFi API integration, LCD display, buzzer alerts

// ESP32 S3 Smart Door Lock System
// Compatible with ESP32 S3 board
// SPI pins: SCK=18, MISO=19, MOSI=23
// I2C pins: SDA=8, SCL=9 (defaults)

const char* ssid = "JAJA's_WIFI"; // Replace with your SSID
const char* password = "Elai's_WIFI_2025"; // Replace with your WiFi password
const char* commandUrl = "https://arduino-api.ginxproduction.com/api/door/command?door_lock_id=1";

// IMPORTANT: Check your API key in the Laravel .env file and update if necessary
// The API key should match the value in your Laravel application's .env file
const char* apiKey = "supersecretkey";

const int lockPin = 4; // Pin connected to relay for door lock
const int buzzerPin = 26; // Pin connected to buzzer for access denied alerts

// RFID pins (using VSPI for ESP32 S3 compatibility)
#define SS_PIN 15  // Can use 15 or 5
#define RST_PIN 25  // Changed from 22 to avoid I2C conflict
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

// LCD setup
hd44780_I2Cexp lcd(0x27); // Changed to 0x27 based on I2C scan

unsigned long unlockTime = 0; // Timer for auto-lock after RFID unlock
bool isLocked = true; // Track current lock state

void scanI2C() {
  Serial.println("Scanning I2C devices...");
  int nDevices = 0;
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found");
  } else {
    Serial.println("I2C scan complete");
  }
}

void soundAccessDeniedBuzzer() {
  // Sound pattern: 3 short beeps
  for (int i = 0; i < 3; i++) {
    tone(buzzerPin, 1000, 200); // 1000Hz tone for 200ms
    delay(300); // Wait 300ms between beeps
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(lockPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(lockPin, LOW); // Ensure door is locked on startup
  digitalWrite(buzzerPin, LOW); // Ensure buzzer is off on startup
  isLocked = true; // Start locked

  // Initialize SPI and MFRC522 (using VSPI for ESP32 S3)
  SPI.begin(18, 19, 23, SS_PIN); // SCK, MISO, MOSI, SS for ESP32 S3
  mfrc522.PCD_Init();
  Serial.println("RFID reader initialized.");

  // Initialize LCD
  Wire.begin(); // Ensure I2C is initialized
  scanI2C(); // Scan for I2C devices
  int status = lcd.begin(20, 4);
  if (status) {
    Serial.println("LCD init failed, status: " + String(status));
    Serial.println("Check wiring: VCC 5V, GND, SDA GPIO8, SCL GPIO9 (ESP32 S3 defaults)");
    // Handle error, perhaps halt or retry
  }
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Smart Door Lock");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");

  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 30000) {  // 30 seconds timeout
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected!");
    Serial.println("My IP: " + WiFi.localIP().toString());
    lcd.setCursor(0, 1);
    lcd.print("WiFi Connected");
    lcd.setCursor(0, 2);
    lcd.print("IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nFailed to connect to WiFi in setup. Check SSID and password.");
    Serial.println("WiFi status: " + String(WiFi.status()));
  }
}

void loop() {
  // Check WiFi connection and reconnect if needed
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected! Attempting to reconnect...");
    WiFi.begin(ssid, password);
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {  // Try for 10 seconds
      delay(500);
      Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nReconnected to WiFi!");
      Serial.println("My IP: " + WiFi.localIP().toString());
    } else {
      Serial.println("\nFailed to reconnect to WiFi. Skipping API call.");
      delay(5000);
      return;  // Skip the rest of the loop
    }
  }

  // Auto-lock after RFID unlock if timer expired
  if (unlockTime > 0 && millis() - unlockTime > 10000) { // 10 seconds
    digitalWrite(lockPin, LOW);
    isLocked = true;
    unlockTime = 0;
    Serial.println("Auto-locked after RFID unlock");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Door Auto-Locked");
    delay(2000); // Show message for 2 seconds
  }

  // Check for RFID card
  if (mfrc522.PICC_IsNewCardPresent()) {
    Serial.println("RFID card detected.");
    if (mfrc522.PICC_ReadCardSerial()) {
      Serial.print("Card UID: ");
      String uid = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        uid += String(mfrc522.uid.uidByte[i], HEX);
        if (i < mfrc522.uid.size - 1) uid += ":";
      }
      Serial.println(uid);

      // Validate RFID UID with API
      HTTPClient httpValidate;
      String validateUrl = "https://arduino-api.ginxproduction.com/api/door/validate-rfid";
      String validateBody = "door_lock_id=1&uid=" + uid;
      httpValidate.begin(validateUrl);
      httpValidate.addHeader("Content-Type", "application/x-www-form-urlencoded");
      httpValidate.addHeader("Accept", "application/json");
      httpValidate.addHeader("x-api-key", apiKey);
      int validateCode = httpValidate.POST(validateBody);
      bool isAuthorized = false;
      if (validateCode == 200) {
        String validateResponse = httpValidate.getString();
        JsonDocument validateDoc;
        DeserializationError validateError = deserializeJson(validateDoc, validateResponse);
        if (!validateError && validateDoc["authorized"]) {
          isAuthorized = true;
        }
      }
      httpValidate.end();

      if (isAuthorized) {
        if (isLocked) {
          // Unlock
          digitalWrite(lockPin, HIGH);
          isLocked = false;
          unlockTime = millis(); // Start auto-lock timer
          Serial.println("Door Unlocked via RFID");

          // Display on LCD
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("RFID Unlocked");
          lcd.setCursor(0, 1);
          lcd.print("UID: " + uid);

          // Update status to API
          HTTPClient httpPost;
          String updateUrl = "https://arduino-api.ginxproduction.com/api/door/status";
          String body = "door_lock_id=1&status=unlocked&rfid_uid=" + uid;
          httpPost.begin(updateUrl);
          httpPost.addHeader("Content-Type", "application/x-www-form-urlencoded");
          httpPost.addHeader("Accept", "application/json");
          httpPost.addHeader("x-api-key", apiKey);
          int postCode = httpPost.POST(body);
          if (postCode == 200) {
            Serial.println("RFID unlock status updated");
          } else {
            Serial.printf("Failed to update RFID status: %s\n", httpPost.errorToString(postCode).c_str());
          }
          httpPost.end();
        } else {
          // Lock
          digitalWrite(lockPin, LOW);
          isLocked = true;
          unlockTime = 0; // Reset timer
          Serial.println("Door Locked via RFID");

          // Display on LCD
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("RFID Locked");
          lcd.setCursor(0, 1);
          lcd.print("UID: " + uid);

          // Update status to API
          HTTPClient httpPost;
          String updateUrl = "https://arduino-api.ginxproduction.com/api/door/status";
          String body = "door_lock_id=1&status=locked&rfid_uid=" + uid;
          httpPost.begin(updateUrl);
          httpPost.addHeader("Content-Type", "application/x-www-form-urlencoded");
          httpPost.addHeader("Accept", "application/json");
          httpPost.addHeader("x-api-key", apiKey);
          int postCode = httpPost.POST(body);
          if (postCode == 200) {
            Serial.println("RFID lock status updated");
          } else {
            Serial.printf("Failed to update RFID status: %s\n", httpPost.errorToString(postCode).c_str());
          }
          httpPost.end();
        }
      } else {
        Serial.println("RFID card not authorized");
        soundAccessDeniedBuzzer(); // Sound buzzer for access denied
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Access Denied");
        lcd.setCursor(0, 1);
        lcd.print("UID: " + uid);
        delay(2000); // Show for 2 seconds
      }

      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
    } else {
      Serial.println("Failed to read card serial.");
    }
  }

  HTTPClient http;
  http.begin(commandUrl);
  http.addHeader("Accept", "application/json");
  http.addHeader("x-api-key", apiKey);
  int httpCode = http.GET();

  Serial.println("HTTP Code: " + String(httpCode));

  if (httpCode == 200) {
    String payload = http.getString();
    Serial.println("Command response: " + payload);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      String command = doc["command"];
      int commandId = doc["id"];
      Serial.println("Received command: " + command);

      String newStatus = "";
      if (command == "lock") {
        digitalWrite(lockPin, LOW);
        isLocked = true;
        unlockTime = 0; // Reset auto-lock timer
        Serial.println("Door Locked");
        newStatus = "locked";
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Door Locked");
      } else if (command == "unlock") {
        digitalWrite(lockPin, HIGH);
        isLocked = false;
        unlockTime = millis(); // Start auto-lock timer
        Serial.println("Door Unlocked");
        newStatus = "unlocked";
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Door Unlocked");
      } else if (command == "status") {
        Serial.println("Blinking lock for status...");
        unsigned long startTime = millis();
        while (millis() - startTime < 5000) {
          digitalWrite(lockPin, HIGH);
          delay(250);
          digitalWrite(lockPin, LOW);
          delay(250);
        }
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Status Check");
      }

      // Always update to mark as executed
      HTTPClient httpPost;
      String updateUrl = "https://arduino-api.ginxproduction.com/api/door/status";
      String body = "door_lock_id=1&command_id=" + String(commandId);
      if (newStatus != "") {
        body += "&status=" + newStatus;
      }

      httpPost.begin(updateUrl);
      httpPost.addHeader("Content-Type", "application/x-www-form-urlencoded");
      httpPost.addHeader("Accept", "application/json");
      httpPost.addHeader("x-api-key", apiKey);
      int postCode = httpPost.POST(body);

      if (postCode == 200) {
        Serial.println("Status updated successfully");
      } else {
        Serial.printf("Failed to update status: %s\n", httpPost.errorToString(postCode).c_str());
      }
      httpPost.end();
    } else {
      Serial.println("JSON parse error");
    }
  } else if (httpCode == 404) {
    Serial.println("No pending command");
  } else {
    String errorResponse = http.getString();
    Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
    Serial.println("Error response: " + errorResponse);
  }
  http.end();

  delay(2000); // Increased delay to prevent overwhelming the API and watchdog resets
  yield(); // Allow other tasks to run and prevent watchdog resets
}

