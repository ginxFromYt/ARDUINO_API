// ESP32 Simple Blink Test
// Tests basic ESP32 functionality

#define LED_PIN 2  // ESP32 onboard LED is on GPIO 2

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("ESP32 Blink Test Started");
  Serial.println("LED should blink every second");
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  Serial.println("LED ON");
  delay(1000);

  digitalWrite(LED_PIN, LOW);
  Serial.println("LED OFF");
  delay(1000);
}
