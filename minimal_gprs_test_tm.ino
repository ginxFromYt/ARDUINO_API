// Minimal GPRS Test for Touch Mobile (TM)
// LED blinks every 3 seconds during timeouts
// Minimal memory usage for Arduino Uno

#include <SoftwareSerial.h>

SoftwareSerial sim800(7, 8);  // RX=7, TX=8
#define LED_PIN 13

// Touch Mobile APN
const char APN[] = "internet";

unsigned long lastBlink = 0;
bool ledState = false;

void setup() {
  Serial.begin(9600);
  sim800.begin(9600);
  pinMode(LED_PIN, OUTPUT);

  Serial.println(F("TM GPRS Test Starting..."));

  // Basic tests
  testModule();
  testSMSFunction();  // New: Test SMS functionality first
  testNetwork();
  testGPRS();

  Serial.println(F("Test Complete"));
}

void loop() {
  // Blink LED every 3 seconds
  if (millis() - lastBlink >= 3000) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    lastBlink = millis();
    Serial.println(F("Heartbeat..."));
  }
}

void testModule() {
  Serial.println(F("1. Testing module..."));

  // Power on sequence (if needed)
  sim800.println(F("AT"));
  delay(1000);

  for (int i = 0; i < 5; i++) {
    sim800.println(F("AT"));
    if (waitResponse("OK", 3000)) {
      Serial.println(F("✓ Module responding"));

      // Check SIM
      sim800.println(F("AT+CPIN?"));
      if (waitResponse("READY", 3000)) {
        Serial.println(F("✓ SIM ready"));
      }

      // Signal strength
      sim800.println(F("AT+CSQ"));
      waitResponse("OK", 3000);
      return;
    }

    Serial.print(F("Retry "));
    Serial.println(i + 1);
    delay(2000);
  }

  Serial.println(F("✗ Module not responding"));
}

void testSMSFunction() {
  Serial.println(F("2. Testing SMS/AT commands..."));

  // Set SMS text mode
  sim800.println(F("AT+CMGF=1"));
  if (!waitResponse("OK", 3000)) {
    Serial.println(F("✗ SMS mode failed"));
    return;
  }

  Serial.println(F("✓ SMS mode set"));

  // Send DATA BAL to 8080 to check balance
  Serial.println(F("Sending DATA BAL to 8080..."));

  sim800.println(F("AT+CMGS=\"8080\""));
  if (waitResponse(">", 5000)) {
    Serial.println(F("✓ SMS prompt received"));

    // Send the message
    sim800.print(F("DATA BAL"));
    delay(100);
    sim800.write(26); // Ctrl+Z to send SMS

    if (waitResponse("OK", 10000)) {
      Serial.println(F("✓ DATA BAL sent to 8080"));
      Serial.println(F("Wait for balance reply..."));

      // Wait for incoming SMS response
      delay(5000);

      // Check for incoming messages
      sim800.println(F("AT+CMGL=\"ALL\""));
      waitResponse("OK", 5000);

    } else {
      Serial.println(F("✗ Failed to send DATA BAL"));
    }
  } else {
    Serial.println(F("✗ SMS prompt failed"));
  }

  Serial.println(F("SMS test complete"));

  // Verify AT commands still work after SMS
  Serial.println(F("Verifying AT commands after SMS..."));
  delay(2000);

  for (int i = 0; i < 3; i++) {
    sim800.println(F("AT"));
    if (waitResponse("OK", 3000)) {
      Serial.println(F("✓ AT commands still working"));
      return;
    }
    Serial.print(F("AT retry "));
    Serial.println(i + 1);
    delay(1000);
  }

  Serial.println(F("✗ AT commands not responding after SMS"));
}

void testNetwork() {
  Serial.println(F("3. Testing network..."));

  for (int i = 0; i < 10; i++) {
    sim800.println(F("AT+CREG?"));

    if (waitResponse("+CREG: 0,1", 3000) || waitResponse("+CREG: 0,5", 3000)) {
      Serial.println(F("✓ Network registered"));

      // Show operator
      sim800.println(F("AT+COPS?"));
      waitResponse("OK", 3000);
      return;
    }

    Serial.print(F("Network attempt "));
    Serial.println(i + 1);
    blinkWait(3000);
  }

  Serial.println(F("✗ Network registration failed"));
}

void testGPRS() {
  Serial.println(F("4. Testing GPRS..."));

  // Set APN
  sim800.print(F("AT+CSTT=\""));
  sim800.print(APN);
  sim800.println(F("\",\"\",\"\""));

  if (!waitResponse("OK", 3000)) {
    Serial.println(F("✗ APN failed"));
    return;
  }

  Serial.println(F("✓ APN set"));

  // Start GPRS
  sim800.println(F("AT+CIICR"));
  if (waitResponse("OK", 15000)) {
    Serial.println(F("✓ GPRS connected"));

    // Get IP
    sim800.println(F("AT+CIFSR"));
    if (waitResponse(".", 5000)) {
      Serial.println(F("✓ IP obtained"));

      // Test ping
      testPing();
    }
  } else {
    Serial.println(F("✗ GPRS failed"));
  }
}

void testPing() {
  Serial.println(F("5. Testing connectivity..."));

  sim800.println(F("AT+CIPSTART=\"TCP\",\"8.8.8.8\",53"));
  if (waitResponse("CONNECT OK", 10000)) {
    Serial.println(F("✓ Internet connected!"));
    sim800.println(F("AT+CIPCLOSE"));
    waitResponse("CLOSE OK", 3000);
  } else {
    Serial.println(F("✗ No internet"));
  }
}

bool waitResponse(const char* expected, unsigned long timeout) {
  unsigned long start = millis();

  while (millis() - start < timeout) {
    // Blink LED during wait
    if (millis() - lastBlink >= 500) {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
      lastBlink = millis();
    }

    if (sim800.available()) {
      String response = sim800.readString();
      Serial.print(response);

      if (response.indexOf(expected) >= 0) {
        return true;
      }

      if (response.indexOf("ERROR") >= 0) {
        return false;
      }
    }

    delay(50);
  }

  Serial.println(F(" TIMEOUT"));
  return false;
}

void blinkWait(unsigned long duration) {
  unsigned long start = millis();

  while (millis() - start < duration) {
    if (millis() - lastBlink >= 500) {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
      lastBlink = millis();
    }
    delay(50);
  }
}
