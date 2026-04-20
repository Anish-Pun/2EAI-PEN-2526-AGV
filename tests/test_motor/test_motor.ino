// MD22 Analog + Turn Mode Test via PWM
const int motor1Pin = 9;  // Motor links
const int motor2Pin = 10; // Motor rechts

int stopSpeed = 128;
int forwardSpeed = 191;   // ~50% vooruit
int reverseSpeed = 64;    // ~50% achteruit

void setup() {
  pinMode(motor1Pin, OUTPUT);
  pinMode(motor2Pin, OUTPUT);
  Serial.begin(9600);
  Serial.println("MD22 beweging test start");
}

void loop() {

  // Vooruita
  Serial.println("Vooruit");
  analogWrite(motor1Pin, forwardSpeed);
  analogWrite(motor2Pin, forwardSpeed);
  delay(2000);

  // Achteruit
  Serial.println("Achteruit");
  analogWrite(motor1Pin, reverseSpeed);
  analogWrite(motor2Pin, reverseSpeed);
  delay(2000);

  // Links draaien
  Serial.println("Links draaien");
  analogWrite(motor1Pin, reverseSpeed); // linker wiel achteruit
  analogWrite(motor2Pin, forwardSpeed); // rechter wiel vooruit
  delay(2000);

  // Rechts draaien
  Serial.println("Rechts draaien");
  analogWrite(motor1Pin, forwardSpeed); // linker wiel vooruit
  analogWrite(motor2Pin, reverseSpeed); // rechter wiel achteruit
  delay(2000);

  // Stop
  Serial.println("Stop");
  analogWrite(motor1Pin, stopSpeed);
  analogWrite(motor2Pin, stopSpeed);
  delay(2000);
}