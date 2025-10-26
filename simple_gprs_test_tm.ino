// Simple GPRS Test for Touch Mobile (TM) Philippines
// Tests SIM800A/SIM800L module initialization
// LED blinks every 3 seconds during timeout periods

#include <SoftwareSerial.h>

// Pin Configuration
SoftwareSerial sim800(7, 8);  // RX=7, TX=8 (connect to SIM800 TX, RX respectively)
#define LED_PIN 13             // Built-in LED on most Arduino boards
#define SIM_POWER_PIN 9        // Optional: SIM800 power control pin

// Touch Mobile (TM) APN Settings
const char* apnName = "internet";     // TM's APN
const char* apnUser = "";             // Usually empty for TM
const char* apnPass = "";             // Usually empty for TM

// Test Configuration
const unsigned long TIMEOUT_3SEC = 3000;
const unsigned long BLINK_INTERVAL = 500;  // LED blink rate
bool ledState = false;
unsigned long lastBlink = 0;

void setup() {
  Serial.begin(9600);
  sim800.begin(9600);

  pinMode(LED_PIN, OUTPUT);
  pinMode(SIM_POWER_PIN, OUTPUT);

  Serial.println(F("=== Touch Mobile GPRS Test ==="));
  Serial.println(F("Initializing SIM800 module..."));

  // Power cycle the SIM800 module
  powerCycleSIM800();

  // Wait for module to start
  delay(5000);

  // Test basic communication
  testBasicCommunication();

  // Test network registration
  testNetworkRegistration();

  // Test GPRS initialization
  testGPRSInitialization();

  // Test internet connectivity
  testInternetConnectivity();

  Serial.println(F("=== Test Complete ==="));
}

void loop() {
  // Blink LED to show Arduino is running
  blinkLED();

  // Optional: Repeat tests every 30 seconds
  static unsigned long lastTest = 0;
  if (millis() - lastTest > 30000) {
    Serial.println(F("\n--- Repeating connectivity test ---"));
    testInternetConnectivity();
    lastTest = millis();
  }

  delay(100);
}

void powerCycleSIM800() {
  Serial.println(F("Power cycling SIM800..."));
  digitalWrite(SIM_POWER_PIN, LOW);
  delay(1000);
  digitalWrite(SIM_POWER_PIN, HIGH);
  delay(2000);
  digitalWrite(SIM_POWER_PIN, LOW);
  Serial.println(F("SIM800 power cycle complete"));
}

void blinkLED() {
  if (millis() - lastBlink >= BLINK_INTERVAL) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    lastBlink = millis();
  }
}

bool sendATCommand(const char* command, const char* expectedResponse, unsigned long timeout = TIMEOUT_3SEC) {
  Serial.print(F("Sending: "));
  Serial.println(command);

  sim800.println(command);

  unsigned long startTime = millis();
  String response = "";

  while (millis() - startTime < timeout) {
    blinkLED(); // Blink LED during timeout

    while (sim800.available()) {
      char c = sim800.read();
      response += c;
      Serial.print(c);
    }

    if (response.indexOf(expectedResponse) >= 0) {
      Serial.println(F(" -> SUCCESS"));
      return true;
    }

    if (response.indexOf("ERROR") >= 0) {
      Serial.println(F(" -> ERROR"));
      return false;
    }

    delay(100);
  }

  Serial.println(F(" -> TIMEOUT"));
  return false;
}

void testBasicCommunication() {
  Serial.println(F("\n1. Testing Basic Communication..."));

  // Test AT command
  if (sendATCommand("AT", "OK")) {
    Serial.println(F("✓ SIM800 is responding"));
  } else {
    Serial.println(F("✗ SIM800 not responding - check wiring"));
    return;
  }

  // Check SIM card
  if (sendATCommand("AT+CPIN?", "READY")) {
    Serial.println(F("✓ SIM card detected"));
  } else {
    Serial.println(F("✗ SIM card not detected or PIN required"));
  }

  // Check signal quality
  Serial.println(F("Checking signal quality..."));
  sendATCommand("AT+CSQ", "OK");

  // Check operator
  Serial.println(F("Checking network operator..."));
  sendATCommand("AT+COPS?", "OK");
}

void testNetworkRegistration() {
  Serial.println(F("\n2. Testing Network Registration..."));

  // Set network registration report
  sendATCommand("AT+CREG=1", "OK");

  // Check registration status
  for (int attempt = 0; attempt < 10; attempt++) {
    Serial.print(F("Registration attempt "));
    Serial.print(attempt + 1);
    Serial.println(F("/10"));

    if (sendATCommand("AT+CREG?", "+CREG: 1,1") ||
        sendATCommand("AT+CREG?", "+CREG: 1,5")) {
      Serial.println(F("✓ Registered to Touch Mobile network"));
      return;
    }

    Serial.println(F("Waiting for network registration..."));
    delay(5000);
  }

  Serial.println(F("✗ Failed to register to network"));
}

void testGPRSInitialization() {
  Serial.println(F("\n3. Testing GPRS Initialization..."));

  // Set APN
  Serial.print(F("Setting APN to: "));
  Serial.println(apnName);

  String apnCommand = "AT+CSTT=\"";
  apnCommand += apnName;
  apnCommand += "\",\"";
  apnCommand += apnUser;
  apnCommand += "\",\"";
  apnCommand += apnPass;
  apnCommand += "\"";

  if (sendATCommand(apnCommand.c_str(), "OK")) {
    Serial.println(F("✓ APN configured"));
  } else {
    Serial.println(F("✗ APN configuration failed"));
    return;
  }

  // Start task
  if (sendATCommand("AT+CIICR", "OK", 10000)) {
    Serial.println(F("✓ GPRS task started"));
  } else {
    Serial.println(F("✗ GPRS task failed"));
    return;
  }

  // Get IP address
  Serial.println(F("Getting IP address..."));
  if (sendATCommand("AT+CIFSR", ".", 5000)) {
    Serial.println(F("✓ IP address obtained"));
  } else {
    Serial.println(F("✗ Failed to get IP address"));
  }
}

void testInternetConnectivity() {
  Serial.println(F("\n4. Testing Internet Connectivity..."));

  // Test connection to Google DNS
  if (sendATCommand("AT+CIPSTART=\"TCP\",\"8.8.8.8\",53", "CONNECT OK", 10000)) {
    Serial.println(F("✓ TCP connection to Google DNS successful"));

    // Close connection
    sendATCommand("AT+CIPCLOSE", "CLOSE OK", TIMEOUT_3SEC);
  } else {
    Serial.println(F("✗ TCP connection failed"));
  }

  // Test HTTP request to a simple API
  Serial.println(F("Testing HTTP request..."));
  if (sendATCommand("AT+CIPSTART=\"TCP\",\"httpbin.org\",80", "CONNECT OK", 10000)) {
    Serial.println(F("✓ Connected to httpbin.org"));

    // Send HTTP request
    if (sendATCommand("AT+CIPSEND=50", ">", TIMEOUT_3SEC)) {
      sim800.print("GET /ip HTTP/1.1\r\nHost: httpbin.org\r\n\r\n");

      delay(3000);

      // Read response
      Serial.println(F("HTTP Response:"));
      while (sim800.available()) {
        Serial.write(sim800.read());
      }

      Serial.println(F("\n✓ HTTP request completed"));
    }

    // Close connection
    sendATCommand("AT+CIPCLOSE", "CLOSE OK", TIMEOUT_3SEC);
  } else {
    Serial.println(F("✗ HTTP connection failed"));
  }
}
