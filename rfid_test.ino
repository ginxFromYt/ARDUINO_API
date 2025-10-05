#include <SPI.h>
#include <MFRC522.h>

// RFID pins (using HSPI to match main sketch)
#define SS_PIN 15
#define RST_PIN 22
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial to connect (for some boards)

  // Initialize SPI and MFRC522 (using HSPI)
  SPI.begin(14, 12, 13, SS_PIN); // SCK, MISO, MOSI, SS
  mfrc522.PCD_Init();
  Serial.println("RFID Test: Reader initialized. Tap a card/tag.");
}

void loop() {
  // Check for RFID card
  if (mfrc522.PICC_IsNewCardPresent()) {
    Serial.println("Card detected.");
    if (mfrc522.PICC_ReadCardSerial()) {
      Serial.print("Card UID: ");
      String uid = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        uid += String(mfrc522.uid.uidByte[i], HEX);
        if (i < mfrc522.uid.size - 1) uid += ":";
      }
      Serial.println(uid);
      Serial.println("Test successful! Card read.");
    } else {
      Serial.println("Failed to read card serial.");
    }
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
  delay(1000); // Check every second
}