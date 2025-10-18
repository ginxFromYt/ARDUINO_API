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
String phoneNumber = "+639679863871"; // Your phone number for SMS alerts
String apnName = "internet";          // Your mobile carrier's APN
String apnUser = "";                  // APN username (if required)
String apnPass = "";                  // APN password (if required)

// API Configuration
String apiHost = "arduino-api.ginxproduction.com";   // Replace with your domain
String apiPath = "/api/water-quality/data";
String apiKey = "supersecretkey";       // Replace with your API key
String deviceId = "WQM001";           // Unique device identifier

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

void sendSMS(String message) {
  Serial.println("Sending SMS: " + message);
  sim800.println("AT+CMGF=1");
  delay(200);
  sim800.print("AT+CMGS=\"");
  sim800.print(phoneNumber);
  sim800.println("\"");
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
  Serial.println("Initializing GPRS...");
  
  // Check if module is ready
  sim800.println("AT");
  delay(1000);
  if (!waitForResponse("OK", 5000)) {
    Serial.println("GSM module not responding");
    return false;
  }
  
  // Check network registration
  sim800.println("AT+CREG?");
  delay(1000);
  if (!waitForResponse("+CREG: 0,1", 10000) && !waitForResponse("+CREG: 0,5", 10000)) {
    Serial.println("Network registration failed");
    return false;
  }
  
  // Set APN
  sim800.print("AT+CSTT=\"");
  sim800.print(apnName);
  sim800.print("\",\"");
  sim800.print(apnUser);
  sim800.print("\",\"");
  sim800.print(apnPass);
  sim800.println("\"");
  delay(1000);
  
  if (!waitForResponse("OK", 5000)) {
    Serial.println("APN setup failed");
    return false;
  }
  
  // Start task and bring up wireless connection
  sim800.println("AT+CIICR");
  delay(5000);
  if (!waitForResponse("OK", 10000)) {
    Serial.println("GPRS connection failed");
    return false;
  }
  
  // Get local IP address
  sim800.println("AT+CIFSR");
  delay(2000);
  
  Serial.println("GPRS initialized successfully");
  return true;
}

bool waitForResponse(String expected, unsigned long timeout) {
  unsigned long start = millis();
  String response = "";
  
  while (millis() - start < timeout) {
    while (sim800.available()) {
      char c = sim800.read();
      response += c;
    }
    
    if (response.indexOf(expected) >= 0) {
      return true;
    }
    
    if (response.indexOf("ERROR") >= 0) {
      return false;
    }
    
    delay(100);
  }
  
  return false;
}

String buildJSONPayload(float turbidity, float tds, float ph, float rawTurbV, float rawTdsV, float rawPhV) {
  String alertLevel = "normal";
  String alertMessage = "";
  
  // Determine alert level and message
  if (tds > TDS_THRESHOLD || turbidity < TURB_THRESHOLD || ph < PH_LOW || ph > PH_HIGH) {
    alertLevel = "warning";
    if ((tds > TDS_THRESHOLD ? 1 : 0) + (turbidity < TURB_THRESHOLD ? 1 : 0) + ((ph < PH_LOW || ph > PH_HIGH) ? 1 : 0) >= 2) {
      alertLevel = "critical";
    }
    
    if (tds > TDS_THRESHOLD) alertMessage += "High TDS (" + String(tds, 1) + " ppm). ";
    if (turbidity < TURB_THRESHOLD) alertMessage += "Turbid water (" + String(turbidity, 2) + "V). ";
    if (ph < PH_LOW || ph > PH_HIGH) alertMessage += "pH out of range (" + String(ph, 2) + "). ";
  }
  
  String sensorStates = "{";
  sensorStates += "\"tds_alert\":" + String(tdsAlertActive ? "true" : "false") + ",";
  sensorStates += "\"turb_alert\":" + String(turbAlertActive ? "true" : "false") + ",";
  sensorStates += "\"ph_alert\":" + String(phAlertActive ? "true" : "false");
  sensorStates += "}";
  
  String payload = "{";
  payload += "\"device_id\":\"" + deviceId + "\",";
  payload += "\"turbidity\":" + String(turbidity, 3) + ",";
  payload += "\"tds\":" + String(tds, 2) + ",";
  payload += "\"ph\":" + String(ph, 2) + ",";
  payload += "\"raw_turbidity_voltage\":" + String(rawTurbV, 3) + ",";
  payload += "\"raw_tds_voltage\":" + String(rawTdsV, 3) + ",";
  payload += "\"raw_ph_voltage\":" + String(rawPhV, 3) + ",";
  payload += "\"sensor_states\":" + sensorStates + ",";
  payload += "\"alert_message\":\"" + alertMessage + "\"";
  payload += "}";
  
  return payload;
}

bool sendDataToAPI(float turbidity, float tds, float ph, float rawTurbV, float rawTdsV, float rawPhV) {
  if (!gprsConnected || !apiEnabled) {
    return false;
  }
  
  Serial.println("Sending data to API...");
  
  String payload = buildJSONPayload(turbidity, tds, ph, rawTurbV, rawTdsV, rawPhV);
  
  // Start TCP connection
  sim800.print("AT+CIPSTART=\"TCP\",\"");
  sim800.print(apiHost);
  sim800.println("\",80");
  
  if (!waitForResponse("CONNECT OK", 10000)) {
    Serial.println("TCP connection failed");
    return false;
  }
  
  // Prepare HTTP request
  String httpRequest = "POST " + apiPath + " HTTP/1.1\r\n";
  httpRequest += "Host: " + apiHost + "\r\n";
  httpRequest += "Content-Type: application/json\r\n";
  httpRequest += "X-API-KEY: " + apiKey + "\r\n";
  httpRequest += "Content-Length: " + String(payload.length()) + "\r\n";
  httpRequest += "Connection: close\r\n\r\n";
  httpRequest += payload;
  
  // Send data length
  sim800.print("AT+CIPSEND=");
  sim800.println(httpRequest.length());
  
  if (!waitForResponse(">", 5000)) {
    Serial.println("Failed to enter send mode");
    sim800.println("AT+CIPCLOSE");
    return false;
  }
  
  // Send HTTP request
  sim800.print(httpRequest);
  
  if (!waitForResponse("SEND OK", 10000)) {
    Serial.println("Failed to send HTTP request");
    sim800.println("AT+CIPCLOSE");
    return false;
  }
  
  // Wait for response
  delay(3000);
  String response = "";
  while (sim800.available()) {
    response += char(sim800.read());
  }
  
  // Close connection
  sim800.println("AT+CIPCLOSE");
  delay(1000);
  
  // Check if successful
  if (response.indexOf("200 OK") >= 0 || response.indexOf("201") >= 0) {
    Serial.println("Data sent successfully to API");
    return true;
  } else {
    Serial.println("API request failed");
    Serial.println("Response: " + response);
    return false;
  }
}

void updateLCD(float tdsValue, float turbVoltage, float phValue) {
  lcd.clear();
  
  // Line 1: Header with GPRS status
  lcd.setCursor(0, 0);
  lcd.print("WQ Monitor ");
  lcd.print(gprsConnected ? "[GPRS]" : "[SMS]");
  
  // Line 2: TDS
  lcd.setCursor(0, 1);
  lcd.print("TDS:");
  lcd.print(tdsValue, 1);
  lcd.print("ppm");
  if (tdsValue > TDS_THRESHOLD) {
    lcd.print(" HIGH!");
  } else {
    lcd.print(" OK  ");
  }
  
  // Line 3: Turbidity
  lcd.setCursor(0, 2);
  lcd.print("Turb:");
  lcd.print(turbVoltage, 2);
  lcd.print("V ");
  if (turbState) {
    lcd.print("TURBID!");
  } else {
    lcd.print("CLEAR  ");
  }
  
  // Line 4: pH
  lcd.setCursor(0, 3);
  lcd.print("pH:");
  lcd.print(phValue, 2);
  if (phValue < PH_LOW || phValue > PH_HIGH) {
    lcd.print(" OUT RANGE!");
  } else {
    lcd.print(" SAFE      ");
  }
}

void showInitScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WATER QUALITY");
  lcd.setCursor(0, 1);
  lcd.print("MONITORING SYSTEM");
  lcd.setCursor(0, 2);
  lcd.print("with API transmission");
  lcd.setCursor(0, 3);
  lcd.print("Initializing...");
  delay(3000);
}

void showSystemStatus() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SYSTEM STATUS CHECK");
  
  lcd.setCursor(0, 1);
  lcd.print("LCD Display: OK");
  delay(1000);
  
  lcd.setCursor(0, 2);
  lcd.print("Testing sensors...");
  delay(1000);
  
  float testTDS = readRawVoltage(TDS_PIN, 4, 50);
  float testTurb = readRawVoltage(TURB_PIN, 4, 50);
  float testPH = readRawVoltage(PH_PIN, 4, 50);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SENSOR STATUS:");
  lcd.setCursor(0, 1);
  lcd.print("TDS: ");
  lcd.print((testTDS > 0.1) ? "OK" : "CHECK");
  lcd.setCursor(0, 2);
  lcd.print("Turb: ");
  lcd.print((testTurb > 0.1) ? "OK" : "CHECK");
  lcd.setCursor(0, 3);
  lcd.print("pH: ");
  lcd.print((testPH > 0.1) ? "OK" : "CHECK");
  delay(3000);
}

void showGPRSStatus() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("GPRS SETUP");
  lcd.setCursor(0, 1);
  lcd.print("Connecting...");
  
  gprsConnected = initializeGPRS();
  
  lcd.setCursor(0, 2);
  if (gprsConnected) {
    lcd.print("GPRS: Connected");
    lcd.setCursor(0, 3);
    lcd.print("API: Enabled");
  } else {
    lcd.print("GPRS: Failed");
    lcd.setCursor(0, 3);
    lcd.print("SMS Mode Only");
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
  
  Serial.println("Starting Water Quality Monitor with API transmission...");
  
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
  String startupMsg = "WQ Monitor ONLINE. Device: " + deviceId;
  if (gprsConnected) {
    startupMsg += " [GPRS+API]";
  } else {
    startupMsg += " [SMS Only]";
  }
  sendSMS(startupMsg);
  
  // Final ready message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MONITORING STARTED");
  lcd.setCursor(0, 1);
  lcd.print("Device: " + deviceId);
  lcd.setCursor(0, 2);
  lcd.print("API: " + String(gprsConnected ? "ON" : "OFF"));
  lcd.setCursor(0, 3);
  lcd.print("SMS: ENABLED");
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
      Serial.print("== Turbidity state -> ");
      Serial.println(turbState ? "TURBID" : "CLEAR");
    }
  }

  // --- TDS & pH ---
  float tdsVoltage = readRawVoltage(TDS_PIN, 8, 6);
  float tdsValue = calcTDSppm(tdsVoltage);
  float phVoltage = readRawVoltage(PH_PIN, 8, 6);
  float phValue = calcPH(phVoltage);

  Serial.print("Turb: "); Serial.print(turbVoltage, 2);
  Serial.print("V "); Serial.print(turbState ? "TURBID" : "CLEAR");
  Serial.print(" | TDS: "); Serial.print(tdsValue, 1);
  Serial.print(" ppm | pH: "); Serial.println(phValue, 2);

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
      // If API fails, reinitialize GPRS
      Serial.println("API transmission failed, reinitializing GPRS...");
      gprsConnected = initializeGPRS();
    }
  }

  // --- SMS Alert logic (same as original) ---
  // TDS Alert logic
  if (tdsValue > TDS_THRESHOLD) {
    if (tdsAbnormalSince == 0) tdsAbnormalSince = now;
    if (!tdsAlertActive && now - tdsAbnormalSince >= debounceDuration && now - lastRecoveryTDS >= recoveryCooldown) {
      sendSMS("ALERT: TDS high! " + String(tdsValue, 1) + " ppm | Turb=" + String(turbVoltage,2)+"V | pH="+String(phValue,2));
      tdsAlertActive = true;
      lastTDSSMS = now;
    } else if (tdsAlertActive && now - lastTDSSMS >= reminderInterval) {
      sendSMS("REMINDER: TDS still high (" + String(tdsValue, 1) + " ppm)");
      lastTDSSMS = now;
    }
  } else {
    tdsAbnormalSince = 0;
    if (tdsAlertActive) {
      sendSMS("RECOVERY: TDS back to normal (" + String(tdsValue, 1) + " ppm)");
      tdsAlertActive = false;
      lastRecoveryTDS = now;
    }
  }

  // Turbidity Alert logic
  if (turbState) {
    if (turbAbnormalSince == 0) turbAbnormalSince = now;
    if (!turbAlertActive && now - turbAbnormalSince >= debounceDuration && now - lastRecoveryTurb >= recoveryCooldown) {
      sendSMS("ALERT: Water turbid! " + String(turbVoltage,2)+"V | TDS="+String(tdsValue,1)+" ppm | pH="+String(phValue,2));
      turbAlertActive = true;
      lastTurbSMS = now;
    } else if (turbAlertActive && now - lastTurbSMS >= reminderInterval) {
      sendSMS("REMINDER: Water still turbid (" + String(turbVoltage,2)+"V)");
      lastTurbSMS = now;
    }
  } else {
    turbAbnormalSince = 0;
    if (turbAlertActive) {
      sendSMS("RECOVERY: Water clear (" + String(turbVoltage,2)+"V)");
      turbAlertActive = false;
      lastRecoveryTurb = now;
    }
  }

  // pH Alert logic
  bool phBad = (phValue < PH_LOW || phValue > PH_HIGH);
  if (phBad) {
    if (phAbnormalSince == 0) phAbnormalSince = now;
    if (!phAlertActive && now - phAbnormalSince >= debounceDuration && now - lastRecoveryPH >= recoveryCooldown) {
      sendSMS("ALERT: pH out of range! pH=" + String(phValue,2) + " | TDS=" + String(tdsValue,1)+" ppm | Turb="+String(turbVoltage,2)+"V");
      phAlertActive = true;
      lastPhSMS = now;
    } else if (phAlertActive && now - lastPhSMS >= reminderInterval) {
      sendSMS("REMINDER: pH still out of range (" + String(phValue,2) + ")");
      lastPhSMS = now;
    }
  } else {
    phAbnormalSince = 0;
    if (phAlertActive) {
      sendSMS("RECOVERY: pH back to safe range (" + String(phValue,2) + ")");
      phAlertActive = false;
      lastRecoveryPH = now;
    }
  }

  delay(SAMPLE_DELAY_MS);
}