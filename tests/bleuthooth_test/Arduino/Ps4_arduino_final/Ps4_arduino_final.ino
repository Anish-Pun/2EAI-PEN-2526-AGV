#include <PS4Controller.h>

void setup() {
  Serial.begin(115200);
  PS4.begin();

  Serial.println("Waiting for controller...");
}

void loop() {

  if (PS4.isConnected()) {

    Serial.print("LX: ");
    Serial.print(PS4.LStickX());

    Serial.print("  LY: ");
    Serial.print(PS4.LStickY());

    Serial.print("  RX: ");
    Serial.print(PS4.RStickX());

    Serial.print("  RY: ");
    Serial.println(PS4.RStickY());
  }

  delay(100);
}