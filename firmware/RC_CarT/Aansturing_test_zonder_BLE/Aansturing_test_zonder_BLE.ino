const int motor1Pin = 25; // links
const int motor2Pin = 26; // rechts

const int stopValue = 128;
const int forwardSpeed = 191;
const int reverseSpeed = 65;

void setup() {
  Serial.begin(115200);

  pinMode(motor1Pin, OUTPUT);
  pinMode(motor2Pin, OUTPUT);

  Serial.println("START TEST...");
}

void loop() {

  // VOORUIT
  Serial.println("VOORUIT");
  analogWrite(motor1Pin, forwardSpeed);
  analogWrite(motor2Pin, forwardSpeed);
  delay(3000);

  // STOP
  Serial.println("STOP");
  analogWrite(motor1Pin, stopValue);
  analogWrite(motor2Pin, stopValue);
  delay(2000);

  // ACHTERUIT
  Serial.println("ACHTERUIT");
  analogWrite(motor1Pin, reverseSpeed);
  analogWrite(motor2Pin, reverseSpeed);
  delay(3000);

  // STOP
  Serial.println("STOP");
  analogWrite(motor1Pin, stopValue);
  analogWrite(motor2Pin, stopValue);
  delay(2000);

  // LINKS
  Serial.println("LINKS");
  analogWrite(motor1Pin, reverseSpeed);
  analogWrite(motor2Pin, forwardSpeed);
  delay(3000);

  // STOP
  analogWrite(motor1Pin, stopValue);
  analogWrite(motor2Pin, stopValue);
  delay(2000);

  // RECHTS
  Serial.println("RECHTS");
  analogWrite(motor1Pin, forwardSpeed);
  analogWrite(motor2Pin, reverseSpeed);
  delay(3000);

  // STOP
  analogWrite(motor1Pin, stopValue);
  analogWrite(motor2Pin, stopValue);
  delay(4000);
}