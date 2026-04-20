// ================= CONFIG =================
#define motor1Pin 5
#define motor2Pin 6

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(motor1Pin, OUTPUT);
  pinMode(motor2Pin, OUTPUT);

  // STOP bij start
  analogWrite(motor1Pin, 128);
  analogWrite(motor2Pin, 128);

  Serial.println("MD22 Uno PWM ready");
}

// ================= FUNCTIES =================
void vooruit() {
  analogWrite(motor1Pin, 200); // vooruit snelheid ~50%
  analogWrite(motor2Pin, 200);
}

void achteruit() {
  analogWrite(motor1Pin, 50);  // achteruit snelheid ~50%
  analogWrite(motor2Pin, 50);
}

void rechts() {
  analogWrite(motor1Pin, 200); // links vooruit
  analogWrite(motor2Pin, 50);  // rechts achteruit
}

void links() {
  analogWrite(motor1Pin, 50);  // links achteruit
  analogWrite(motor2Pin, 200); // rechts vooruit
}

void stopMotors() {
  analogWrite(motor1Pin, 128); // stop
  analogWrite(motor2Pin, 128);
}

// ================= LOOP =================
void loop() {

  // VOORUIT
  vooruit();
  delay(2000);

  // ACHTERUIT
  achteruit();
  delay(2000);

  // RECHTS draaien
  rechts();
  delay(2000);

  // LINKS draaien
  links();
  delay(2000);

  // STOP
  stopMotors();
  delay(2000);
}