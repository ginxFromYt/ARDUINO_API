// TDS + Turbidity + pH + SMS + LCD Display + API Data Transmission
// Enhanced with GPRS HTTP POST to Laravel API
// Compatible with SIM800A/SIM800L GPRS modules

#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

SoftwareSerial sim800(7, 8);  // RX, TX (SIM800 -> Arduino 9, Arduino 10 -> SIM800)
LiquidCrystal_I2C lcd(0x27, 20, 4);  // I2C address 0x27, 20 columns, 4 rows

/////////////////////
// User settings
/////////////////////
const char* phoneNumber = "+639679863871"; // Your phone number for SMS alerts
const char* apnName = "internet";          // Your mobile carrier's APN
const char* apnUser = "";                  // APN username (if required)
const char* apnPass = "";                  // APN password (if required)

// API Configuration
const char* apiHost = "arduino-api.ginxproduction.com";   // Replace with your domain
const char* apiPath = "/api/water-quality/data";
const char* apiKey = "supersecretkey";       // Replace with your API key
const char* deviceId = "WQM001";           // Unique device identifier

// Thresholds (same as original)
const float TDS_THRESHOLD = 500.0;     // ppm
const float TURB_THRESHOLD = 2.32;     // V (below = turbid)
const float PH_LOW = 6.5;              // WHO safe range
const float PH_HIGH = 8.5;

// Timing
const unsigned long reminderInterval = 30UL * 60UL * 1000UL; // 30 min
const unsigned long debounceDuration = 10UL * 1000UL;        // 10 s stable abnormal before alert
const unsigned long recoveryCooldown = 2UL * 60UL * 1000UL;  // 2 min after recovery before next alert
const unsigned long lcdUpdateInterval = 1000UL;              // LCD update every 1 second
const unsigned long apiTransmitInterval = 5UL * 60UL * 1000UL; // Send to API every 5 minutes
const unsigned long apiAlertTransmitInterval = 30UL * 1000UL;  // Send alerts immediately after 30s

// Moving average
const int MA_SAMPLES = 7;
const int SAMPLE_DELAY_MS = 200;
const int REQUIRED_CONSECUTIVE = 3;

// ADC
const float VREF = 5.0;
const int ADC_RES = 1023;

/////////////////////
// Pins
/////////////////////
#define TURB_PIN A0
#define TDS_PIN  A1
#define PH_PIN   A2

/////////////////////
// State variables
/////////////////////
float maBuffer[MA_SAMPLES];
int maIndex = 0;
bool maFilled = false;

bool tdsAlertActive = false;
bool turbAlertActive = false;
bool phAlertActive = false;

bool turbState = false;       // false = CLEAR, true = TURBID
int turbConsecutive = 0;

unsigned long lastTDSSMS = 0;
unsigned long lastTurbSMS = 0;
unsigned long lastPhSMS   = 0;
unsigned long lastLCDUpdate = 0;
unsigned long lastAPITransmit = 0;

unsigned long tdsAbnormalSince = 0;
unsigned long turbAbnormalSince = 0;
unsigned long phAbnormalSince = 0;

unsigned long lastRecoveryTDS = 0;
unsigned long lastRecoveryTurb = 0;
unsigned long lastRecoveryPH = 0;

bool gprsConnected = false;
bool apiEnabled = true;

/////////////////////
// Helpers
/////////////////////
float readRawVoltage(int pin, int samples = 8, int delayMs = 6) {
  long sum = 0;
  for (int i = 0; i < samples; ++i) {
    sum += analogRead(pin);
    delay(delayMs);
  }
  return (sum / (float)samples) * (VREF / ADC_RES);
}

float readMovingAverageTurbidity() {
  float v = readRawVoltage(TURB_PIN, 4, 6);
  maBuffer[maIndex] = v;
  maIndex = (maIndex + 1) % MA_SAMPLES;
  maFilled = maFilled || (maIndex == 0);
  int count = maFilled ? MA_SAMPLES : maIndex;
  float sum = 0;
  for (int i = 0; i < count; ++i) sum += maBuffer[i];
  return sum / count;
}

// --- TDS conversion ---
float calcTDSppm(float voltage) {
  float raw = (133.42 * voltage * voltage * voltage)
            - (255.86 * voltage * voltage)
            + (857.39 * voltage);
  const float CAL_FACTOR = 0.5;
  return raw * CAL_FACTOR;
}

// --- pH conversion ---
float calcPH(float voltage) {
  const float slope = 5.5;
  const float intercept = -0.8827;
  return slope * voltage + intercept;
}

void sendSMS(const char* message) {
  Serial.print(F("Sending SMS: "));
  Serial.println(message);
  sim800.println(F("AT+CMGF=1"));
  delay(200);
  sim800.print(F("AT+CMGS=\""));
  sim800.print(phoneNumber);
  sim800.println(F("\""));
  delay(200);
  sim800.print(message);
  delay(200);
  sim800.write(26);
  delay(4000);
}

/////////////////////
// GPRS Functions
/////////////////////
bool initializeGPRS() {
  Serial.println(F("Initializing GPRS..."));

  // Check if module is ready
  sim800.println(F("AT"));
  delay(1000);
  if (!waitForResponse((char*)"OK", 5000)) {
    Serial.println(F("GSM module not responding"));
    return false;
  }

  // Check network registration
  sim800.println(F("AT+CREG?"));
  delay(1000);
  if (!waitForResponse((char*)"+CREG: 0,1", 10000) && !waitForResponse((char*)"+CREG: 0,5", 10000)) {
    Serial.println(F("Network registration failed"));
    return false;
  }

  // Set APN
  sim800.print(F("AT+CSTT=\""));
  sim800.print(apnName);
  sim800.print(F("\",\""));
  sim800.print(apnUser);
  sim800.print(F("\",\""));
  sim800.print(apnPass);
  sim800.println(F("\""));
  delay(1000);

  if (!waitForResponse((char*)"OK", 5000)) {
    Serial.println(F("APN setup failed"));
    return false;
  }

  // Start task and bring up wireless connection
  sim800.println(F("AT+CIICR"));
  delay(5000);
  if (!waitForResponse((char*)"OK", 10000)) {
    Serial.println(F("GPRS connection failed"));
    return false;
  }

  // Get local IP address
  sim800.println(F("AT+CIFSR"));
  delay(2000);

  Serial.println(F("GPRS initialized successfully"));
  return true;
}

bool waitForResponse(char* expected, unsigned long timeout) {
  unsigned long start = millis();

  while (millis() - start < timeout) {
    if (sim800.find(expected)) {
      return true;
    }
    delay(100);
  }
  return false;
}

// Build JSON directly to SIM800 to save memory
void sendJSONPayload(float turbidity, float tds, float ph, float rawTurbV, float rawTdsV, float rawPhV) {
  // Send JSON directly without storing in String
  sim800.print("{\"device_id\":\"");
  sim800.print(deviceId);
  sim800.print("\",\"turbidity\":");
  sim800.print(turbidity, 3);
  sim800.print(",\"tds\":");
  sim800.print(tds, 2);
  sim800.print(",\"ph\":");
  sim800.print(ph, 2);
  sim800.print(",\"raw_turbidity_voltage\":");
  sim800.print(rawTurbV, 3);
  sim800.print(",\"raw_tds_voltage\":");
  sim800.print(rawTdsV, 3);
  sim800.print(",\"raw_ph_voltage\":");
  sim800.print(rawPhV, 3);
  sim800.print(",\"sensor_states\":{\"tds_alert\":");
  sim800.print(tdsAlertActive ? "true" : "false");
  sim800.print(",\"turb_alert\":");
  sim800.print(turbAlertActive ? "true" : "false");
  sim800.print(",\"ph_alert\":");
  sim800.print(phAlertActive ? "true" : "false");
  sim800.print("},\"alert_message\":\"");

  // Build alert message efficiently
  if (tds > TDS_THRESHOLD) sim800.print("High TDS ");
  if (turbidity < TURB_THRESHOLD) sim800.print("Turbid ");
  if (ph < PH_LOW || ph > PH_HIGH) sim800.print("pH range ");

  sim800.print("\"}");
}

bool sendDataToAPI(float turbidity, float tds, float ph, float rawTurbV, float rawTdsV, float rawPhV) {
  if (!gprsConnected || !apiEnabled) {
    return false;
  }

  Serial.println(F("Sending data to API..."));

  // Start TCP connection
  sim800.print(F("AT+CIPSTART=\"TCP\",\""));
  sim800.print(apiHost);
  sim800.println(F("\",80"));

  if (!waitForResponse((char*)"CONNECT OK", 10000)) {
    Serial.println(F("TCP connection failed"));
    return false;
  }

  // Calculate approximate payload size (estimate to avoid String operations)
  int payloadSize = 200; // Rough estimate for JSON payload

  // Calculate total request size
  int totalSize = 100; // Headers
  totalSize += strlen(apiHost);
  totalSize += strlen(apiPath);
  totalSize += strlen(apiKey);
  totalSize += payloadSize;

  // Send data length
  sim800.print(F("AT+CIPSEND="));
  sim800.println(totalSize);

  if (!waitForResponse((char*)">", 5000)) {
    Serial.println(F("Failed to enter send mode"));
    sim800.println(F("AT+CIPCLOSE"));
    return false;
  }

  // Send HTTP headers directly
  sim800.print(F("POST "));
  sim800.print(apiPath);
  sim800.println(F(" HTTP/1.1"));
  sim800.print(F("Host: "));
  sim800.println(apiHost);
  sim800.println(F("Content-Type: application/json"));
  sim800.print(F("X-API-KEY: "));
  sim800.println(apiKey);
  sim800.print(F("Content-Length: "));
  sim800.println(payloadSize);
  sim800.println(F("Connection: close"));
  sim800.println(); // Empty line

  // Send JSON payload directly
  sendJSONPayload(turbidity, tds, ph, rawTurbV, rawTdsV, rawPhV);

  if (!waitForResponse((char*)"SEND OK", 10000)) {
    Serial.println(F("Failed to send HTTP request"));
    sim800.println(F("AT+CIPCLOSE"));
    return false;
  }

  // Wait for response
  delay(3000);
  bool success = false;
  while (sim800.available()) {
    if (sim800.find((char*)"200")) {
      success = true;
      break;
    }
  }

  // Close connection
  sim800.println(F("AT+CIPCLOSE"));
  delay(1000);

  if (success) {
    Serial.println(F("Data sent successfully to API"));
    return true;
  } else {
    Serial.println(F("API request failed"));
    return false;
  }
}

void updateLCD(float tdsValue, float turbVoltage, float phValue) {
  lcd.clear();

  // Line 1: Header with GPRS status
  lcd.setCursor(0, 0);
  lcd.print(F("WQ Monitor "));
  lcd.print(gprsConnected ? F("[GPRS]") : F("[SMS]"));

  // Line 2: TDS
  lcd.setCursor(0, 1);
  lcd.print(F("TDS:"));
  lcd.print(tdsValue, 1);
  lcd.print(F("ppm"));
  if (tdsValue > TDS_THRESHOLD) {
    lcd.print(F(" HIGH!"));
  } else {
    lcd.print(F(" OK  "));
  }

  // Line 3: Turbidity
  lcd.setCursor(0, 2);
  lcd.print(F("Turb:"));
  lcd.print(turbVoltage, 2);
  lcd.print(F("V "));
  if (turbState) {
    lcd.print(F("TURBID!"));
  } else {
    lcd.print(F("CLEAR  "));
  }

  // Line 4: pH
  lcd.setCursor(0, 3);
  lcd.print(F("pH:"));
  lcd.print(phValue, 2);
  if (phValue < PH_LOW || phValue > PH_HIGH) {
    lcd.print(F(" OUT RANGE!"));
  } else {
    lcd.print(F(" SAFE      "));
  }
}

void showInitScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("WATER QUALITY"));
  lcd.setCursor(0, 1);
  lcd.print(F("MONITORING SYSTEM"));
  lcd.setCursor(0, 2);
  lcd.print(F("with API transmission"));
  lcd.setCursor(0, 3);
  lcd.print(F("Initializing..."));
  delay(3000);
}

void showSystemStatus() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("SYSTEM STATUS CHECK"));

  lcd.setCursor(0, 1);
  lcd.print(F("LCD Display: OK"));
  delay(1000);

  lcd.setCursor(0, 2);
  lcd.print(F("Testing sensors..."));
  delay(1000);

  float testTDS = readRawVoltage(TDS_PIN, 4, 50);
  float testTurb = readRawVoltage(TURB_PIN, 4, 50);
  float testPH = readRawVoltage(PH_PIN, 4, 50);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("SENSOR STATUS:"));
  lcd.setCursor(0, 1);
  lcd.print(F("TDS: "));
  lcd.print((testTDS > 0.1) ? F("OK") : F("CHECK"));
  lcd.setCursor(0, 2);
  lcd.print(F("Turb: "));
  lcd.print((testTurb > 0.1) ? F("OK") : F("CHECK"));
  lcd.setCursor(0, 3);
  lcd.print(F("pH: "));
  lcd.print((testPH > 0.1) ? F("OK") : F("CHECK"));
  delay(3000);
}

void showGPRSStatus() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("GPRS SETUP"));
  lcd.setCursor(0, 1);
  lcd.print(F("Connecting..."));

  gprsConnected = initializeGPRS();

  lcd.setCursor(0, 2);
  if (gprsConnected) {
    lcd.print(F("GPRS: Connected"));
    lcd.setCursor(0, 3);
    lcd.print(F("API: Enabled"));
  } else {
    lcd.print(F("GPRS: Failed"));
    lcd.setCursor(0, 3);
    lcd.print(F("SMS Mode Only"));
  }
  delay(3000);
}

/////////////////////
// Setup
/////////////////////
void setup() {
  Serial.begin(9600);
  sim800.begin(9600);

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Show initialization screens
  showInitScreen();
  showSystemStatus();
  showGPRSStatus();

  Serial.println(F("Starting Water Quality Monitor with API transmission..."));

  // Initialize turbidity buffer
  float first = readRawVoltage(TURB_PIN, 8, 6);
  for (int i = 0; i < MA_SAMPLES; ++i) maBuffer[i] = first;
  maFilled = true;

  // Initialize GSM for SMS
  sim800.println("AT");
  delay(500);
  sim800.println("AT+CMGF=1");
  delay(500);

  // Send system startup notification
  if (gprsConnected) {
    sendSMS("WQ Monitor ONLINE [GPRS+API]");
  } else {
    sendSMS("WQ Monitor ONLINE [SMS Only]");
  }

  // Final ready message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("MONITORING STARTED"));
  lcd.setCursor(0, 1);
  lcd.print(F("Device: "));
  lcd.print(deviceId);
  lcd.setCursor(0, 2);
  lcd.print(F("API: "));
  lcd.print(gprsConnected ? F("ON") : F("OFF"));
  lcd.setCursor(0, 3);
  lcd.print(F("SMS: ENABLED"));
  delay(3000);
}

/////////////////////
// Main loop
/////////////////////
void loop() {
  unsigned long now = millis();

  // --- Turbidity ---
  float turbVoltage = readMovingAverageTurbidity();
  bool instantTurb = (turbVoltage < TURB_THRESHOLD);
  if (instantTurb == turbState) {
    turbConsecutive = 0;
  } else {
    turbConsecutive++;
    if (turbConsecutive >= REQUIRED_CONSECUTIVE) {
      turbState = instantTurb;
      turbConsecutive = 0;
      Serial.print(F("== Turbidity state -> "));
      Serial.println(turbState ? F("TURBID") : F("CLEAR"));
    }
  }

  // --- TDS & pH ---
  float tdsVoltage = readRawVoltage(TDS_PIN, 8, 6);
  float tdsValue = calcTDSppm(tdsVoltage);
  float phVoltage = readRawVoltage(PH_PIN, 8, 6);
  float phValue = calcPH(phVoltage);

  Serial.print(F("Turb: ")); Serial.print(turbVoltage, 2);
  Serial.print(F("V ")); Serial.print(turbState ? F("TURBID") : F("CLEAR"));
  Serial.print(F(" | TDS: ")); Serial.print(tdsValue, 1);
  Serial.print(F(" ppm | pH: ")); Serial.println(phValue, 2);

  // Update LCD display
  if (now - lastLCDUpdate >= lcdUpdateInterval) {
    updateLCD(tdsValue, turbVoltage, phValue);
    lastLCDUpdate = now;
  }

  // --- API Data Transmission ---
  bool shouldSendToAPI = false;

  // Regular interval transmission
  if (now - lastAPITransmit >= apiTransmitInterval) {
    shouldSendToAPI = true;
  }

  // Immediate transmission for alerts
  bool hasActiveAlert = tdsAlertActive || turbAlertActive || phAlertActive;
  if (hasActiveAlert && now - lastAPITransmit >= apiAlertTransmitInterval) {
    shouldSendToAPI = true;
  }

  if (shouldSendToAPI && gprsConnected) {
    if (sendDataToAPI(turbVoltage, tdsValue, phValue, turbVoltage, tdsVoltage, phVoltage)) {
      lastAPITransmit = now;
    } else {
      // If API fails, mark as disconnected (will retry on next interval)
      Serial.println(F("API transmission failed"));
      gprsConnected = false;
    }
  }

  // --- SMS Alert logic with 10-second debouncing ---

  // TDS Alert logic
  if (tdsValue > TDS_THRESHOLD) {
    if (tdsAbnormalSince == 0) tdsAbnormalSince = now;
    if (!tdsAlertActive && now - tdsAbnormalSince >= debounceDuration && now - lastRecoveryTDS >= recoveryCooldown) {
      sendSMS("ALERT: TDS high! Check water quality");
      tdsAlertActive = true;
      lastTDSSMS = now;
      Serial.println(F("TDS ALERT triggered after 10s debounce"));
    } else if (tdsAlertActive && now - lastTDSSMS >= reminderInterval) {
      sendSMS("REMINDER: TDS still high");
      lastTDSSMS = now;
    }
  } else {
    tdsAbnormalSince = 0;
    if (tdsAlertActive) {
      sendSMS("RECOVERY: TDS back to normal");
      tdsAlertActive = false;
      lastRecoveryTDS = now;
      Serial.println(F("TDS RECOVERY sent"));
    }
  }

  // Turbidity Alert logic
  if (turbState) {
    if (turbAbnormalSince == 0) {
      turbAbnormalSince = now;
      Serial.println(F("Turbidity abnormal timer started"));
    }
    unsigned long timeSinceAbnormal = now - turbAbnormalSince;
    Serial.print(F("Turb abnormal for: ")); Serial.print(timeSinceAbnormal/1000); Serial.println(F("s"));

    if (!turbAlertActive && timeSinceAbnormal >= debounceDuration && now - lastRecoveryTurb >= recoveryCooldown) {
      sendSMS("ALERT: Water turbid! Check filters");
      turbAlertActive = true;
      lastTurbSMS = now;
      Serial.println(F("TURBIDITY ALERT triggered after 10s debounce"));
    } else if (turbAlertActive && now - lastTurbSMS >= reminderInterval) {
      sendSMS("REMINDER: Water still turbid");
      lastTurbSMS = now;
    }
  } else {
    turbAbnormalSince = 0;
    if (turbAlertActive) {
      sendSMS("RECOVERY: Water clear");
      turbAlertActive = false;
      lastRecoveryTurb = now;
      Serial.println(F("TURBIDITY RECOVERY sent"));
    }
  }

  // pH Alert logic
  bool phBad = (phValue < PH_LOW || phValue > PH_HIGH);
  if (phBad) {
    if (phAbnormalSince == 0) {
      phAbnormalSince = now;
      Serial.println(F("pH abnormal timer started"));
    }
    unsigned long timeSinceAbnormal = now - phAbnormalSince;
    Serial.print(F("pH abnormal for: ")); Serial.print(timeSinceAbnormal/1000); Serial.println(F("s"));

    if (!phAlertActive && timeSinceAbnojajarmal >= debounceDuration && now - lastRecoveryPH >= recoveryCooldown) {
      sendSMS("ALERT: pH out of range! Check system");
      phAlertActive = true;
      lastPhSMS = now;
      Serial.println(F("pH ALERT triggered after 10s debounce"));
    } else if (phAlertActive && now - lastPhSMS >= reminderInterval) {
      sendSMS("REMINDER: pH still out of range");
      lastPhSMS = now;
    }
  } else {
    phAbnormalSince = 0;
    if (phAlertActive) {
      sendSMS("RECOVERY: pH back to safe range");
      phAlertActive = false;
      lastRecoveryPH = now;
      Serial.println(F("pH RECOVERY sent"));
    }
  }

  delay(SAMPLE_DELAY_MS);
}
