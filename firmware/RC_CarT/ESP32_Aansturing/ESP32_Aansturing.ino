// ================= CONFIG =================
#define motor1Pin 26
#define motor2Pin 27

// ESP32 PWM (LEDC)
#define CH1 0
#define CH2 1

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  ledcSetup(CH1, 1000, 8);
  ledcAttachPin(motor1Pin, CH1);

  ledcSetup(CH2, 1000, 8);
  ledcAttachPin(motor2Pin, CH2);

  // STOP
  ledcWrite(CH1, 128);
  ledcWrite(CH2, 128);
}

// ================= FUNCTIES =================
void vooruit() {
  ledcWrite(CH1, 200);
  ledcWrite(CH2, 200);
}

void achteruit() {
  ledcWrite(CH1, 50);
  ledcWrite(CH2, 50);
}

void rechts() {
  ledcWrite(CH1, 200);
  ledcWrite(CH2, 50);
}

void links() {
  ledcWrite(CH1, 50);
  ledcWrite(CH2, 200);
}

void stopMotors() {
  ledcWrite(CH1, 128);
  ledcWrite(CH2, 128);
}

// ================= LOOP =================
void loop() {

  vooruit();
  delay(2000);

  achteruit();
  delay(2000);

  rechts();
  delay(2000);

  links();
  delay(2000);

  stopMotors();
  delay(2000);
}