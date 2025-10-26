#include <SPI.h>
#include <MFRC522.h>

// ESP32 RFID Card Reader - Simple UID Scanner
// This sketch only reads RFID cards and prints their UIDs

// RFID pins (using VSPI for ESP32 S3 compatibility)
#define SS_PIN 15  // Can use 15 or 5
#define RST_PIN 25  // Changed from 22 to avoid I2C conflict
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 RFID Card Scanner");
  Serial.println("=======================");
  Serial.println("Wiring: SCK=18, MISO=19, MOSI=23, SS=15, RST=25");
  Serial.println();

  // Initialize SPI and MFRC522 (using VSPI for ESP32 S3 compatibility)
  SPI.begin(18, 19, 23, SS_PIN); // SCK=18, MISO=19, MOSI=23, SS
  mfrc522.PCD_Init();

  // Check if RFID reader initialized properly
  Serial.print("Initializing RFID reader... ");
  if (mfrc522.PCD_PerformSelfTest()) {
    Serial.println("✅ PASSED");
  } else {
    Serial.println("❌ FAILED - Check wiring and power");
  }

  // Print RFID reader version
  byte version = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print("MFRC522 Version: 0x");
  Serial.print(version, HEX);

  if (version == 0x91 || version == 0x92) {
    Serial.println(" ✅ DETECTED");
    Serial.println();
    Serial.println("🔍 READY TO SCAN RFID CARDS!");
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.println("Instructions:");
    Serial.println("1. Hold an RFID card near the reader");
    Serial.println("2. The UID will be displayed automatically");
    Serial.println("3. Copy the UID for database entry");
    Serial.println("4. Wait 3 seconds before scanning next card");
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  } else {
    Serial.println(" ❌ NOT DETECTED - Check connections");
    Serial.println("Expected versions: 0x91 or 0x92");
  }
}

void loop() {
  // Check for RFID card every 200ms for better responsiveness
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 200) {
    lastCheck = millis();

    // Look for new cards
    if (mfrc522.PICC_IsNewCardPresent()) {
      Serial.println();
      Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
      Serial.print("🎯 RFID CARD DETECTED at ");
      Serial.print(millis());
      Serial.println("ms");

      // Select one of the cards
      Serial.println("📖 Attempting to read card serial...");
      if (mfrc522.PICC_ReadCardSerial()) {
        Serial.println("� Reading card data...");

        // Print UID in multiple formats
        Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
        Serial.print("📋 RAW UID BYTES: ");
        for (byte i = 0; i < mfrc522.uid.size; i++) {
          if (mfrc522.uid.uidByte[i] < 0x10) {
            Serial.print("0"); // Add leading zero
          }
          Serial.print(mfrc522.uid.uidByte[i], HEX);
          if (i < mfrc522.uid.size - 1) Serial.print(" ");
        }
        Serial.println();

        // Print formatted UID (colon-separated)
        Serial.print("🔑 FORMATTED UID: ");
        String uid = "";
        for (byte i = 0; i < mfrc522.uid.size; i++) {
          if (mfrc522.uid.uidByte[i] < 0x10) {
            Serial.print("0");
          }
          Serial.print(mfrc522.uid.uidByte[i], HEX);

          uid += String(mfrc522.uid.uidByte[i], HEX);

          if (i < mfrc522.uid.size - 1) {
            Serial.print(":");
            uid += ":";
          }
        }
        Serial.println();

        // Database format
        Serial.print("💾 DATABASE FORMAT: ");
        Serial.println(uid);

        // Card type information
        Serial.print("🎴 CARD TYPE: ");
        MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
        Serial.println(mfrc522.PICC_GetTypeName(piccType));

        // UID size
        Serial.print("� UID SIZE: ");
        Serial.print(mfrc522.uid.size);
        Serial.println(" bytes");

        Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
        Serial.println("✅ CARD READ SUCCESSFULLY!");
        Serial.println("💡 Copy the 'DATABASE FORMAT' UID above");
        Serial.println("⏳ Cooling down for 3 seconds...");
        Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");

        // Halt PICC
        mfrc522.PICC_HaltA();

        // Stop encryption on PCD
        mfrc522.PCD_StopCrypto1();

        // Wait before next scan to prevent duplicate reads
        delay(3000);
        Serial.println("🔍 Ready for next card scan...");
        Serial.println();

      } else {
        Serial.println("❌ FAILED to read card serial data");
        Serial.println("💡 Try holding the card steadier or check card compatibility");
      }
    }
  }

  // Periodic status check every 15 seconds
  static unsigned long lastStatusCheck = 0;
  if (millis() - lastStatusCheck > 15000) {
    lastStatusCheck = millis();

    Serial.println();
    Serial.print("🔍 Status check at ");
    Serial.print(millis());
    Serial.print("ms: ");

    byte version = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
    if (version == 0x91 || version == 0x92) {
      Serial.println("RFID reader OK ✅");
    } else {
      Serial.print("RFID reader ERROR ❌ (Version: 0x");
      Serial.print(version, HEX);
      Serial.println(")");
    }
  }

  // Small delay to prevent overwhelming the serial output
  delay(50);
}
