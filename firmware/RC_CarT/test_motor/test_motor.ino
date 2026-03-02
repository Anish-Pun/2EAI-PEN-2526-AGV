// MD22 Analog + Turn Mode Test via PWM
const int motor1Pin = 9; // SCL
const int motor2Pin = 10; // SDA

void setup() {
  pinMode(motor1Pin, OUTPUT);
  pinMode(motor2Pin, OUTPUT);
  Serial.begin(9600);
  Serial.println("MD22 PWM test start");
}

void loop() {
  // Motoren vooruit (50% forward)
  Serial.println("Motoren vooruit");
  analogWrite(motor1Pin, 191); // 5V = 255, 2.5V stop = 128, 50% forward ~ 191
  analogWrite(motor2Pin, 191);
  delay(2000);

  // Motoren achteruit (50% reverse)
  Serial.println("Motoren achteruit");
  analogWrite(motor1Pin, 64);  // 0V = 0, 2.5V stop = 128, 50% reverse ~ 64
  analogWrite(motor2Pin, 64);
  delay(2000);

  // Stop motoren
  Serial.println("Motoren stop");
  analogWrite(motor1Pin, 128); // stop
  analogWrite(motor2Pin, 128);
  delay(2000);
}