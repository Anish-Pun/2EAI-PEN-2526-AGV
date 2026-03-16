#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("ESP32 MAC addresses:");

  // WiFi MAC
  Serial.print("WiFi MAC: ");
  Serial.println(WiFi.macAddress());

  // Bluetooth Classic MAC
  uint64_t chipid = ESP.getEfuseMac();
  Serial.print("Base MAC (eFuse): ");
  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n",
                (uint8_t)(chipid >> 40),
                (uint8_t)(chipid >> 32),
                (uint8_t)(chipid >> 24),
                (uint8_t)(chipid >> 16),
                (uint8_t)(chipid >> 8),
                (uint8_t)(chipid));
}

void loop() {
}