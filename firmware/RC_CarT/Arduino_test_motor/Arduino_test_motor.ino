// ===== MD22 PWM CONTROL (PROPER VERSION) =====

const int motorLeftPin  = 9;
const int motorRightPin = 10;

const int STOP = 128;

// Kalibratie (MOET je zelf tunen)
float rightCorrection = 0.97;  // pas aan na testen!

// Huidige snelheid (voor ramping)
int currentLeft  = STOP;
int currentRight = STOP;

// Timing
unsigned long lastUpdate = 0;
const int rampInterval = 10; // ms

// Target speeds
int targetLeft  = STOP;
int targetRight = STOP;

// ===== MOTOR CONTROL =====
void setMotors(int left, int right) {
  analogWrite(motorLeftPin, left);
  analogWrite(motorRightPin, right);
}

// ===== SET TARGET =====
void setTarget(int left, int right) {
  targetLeft  = left;
  targetRight = right;
}

// ===== RAMP FUNCTION =====
void updateMotors() {
  if (millis() - lastUpdate < rampInterval) return;
  lastUpdate = millis();

  // Left motor ramp
  if (currentLeft < targetLeft) currentLeft++;
  else if (currentLeft > targetLeft) currentLeft--;

  // Right motor ramp
  if (currentRight < targetRight) currentRight++;
  else if (currentRight > targetRight) currentRight--;

  setMotors(currentLeft, currentRight);
}

// ===== HIGH LEVEL MOVES =====
void forward(int speed) {
  int left  = speed;
  int right = speed * rightCorrection;
  setTarget(left, right);
}

void backward(int speed) {
  int left  = speed;
  int right = speed * rightCorrection;
  setTarget(left, right);
}

void turnLeft(int speed) {
  setTarget(speed - 60, speed + 60);
}

void turnRight(int speed) {
  setTarget(speed + 60, speed - 60);
}

void stopMotors() {
  setTarget(STOP, STOP);
}

// ===== STATE MACHINE =====
enum State {
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT,
  STOPPED
};

State currentState = FORWARD;
unsigned long stateStart = 0;
const int stateDuration = 3000; // ms

void nextState() {
  switch (currentState) {
    case FORWARD: currentState = BACKWARD; break;
    case BACKWARD: currentState = LEFT; break;
    case LEFT: currentState = RIGHT; break;
    case RIGHT: currentState = STOPPED; break;
    case STOPPED: currentState = FORWARD; break;
  }
  stateStart = millis();
}

// ===== SETUP =====
void setup() {
  pinMode(motorLeftPin, OUTPUT);
  pinMode(motorRightPin, OUTPUT);

  Serial.begin(9600);
  Serial.println("MD22 CONTROL START");

  stateStart = millis();
}

// ===== LOOP =====
void loop() {

  // Update motor ramp continu
  updateMotors();

  // State timing
  if (millis() - stateStart > stateDuration) {
    nextState();
  }

  // State acties
  switch (currentState) {
    case FORWARD:
      Serial.println("FORWARD");
      forward(190);
      break;

    case BACKWARD:
      Serial.println("BACKWARD");
      backward(60);
      break;

    case LEFT:
      Serial.println("LEFT");
      turnLeft(160);
      break;

    case RIGHT:
      Serial.println("RIGHT");
      turnRight(160);
      break;

    case STOPPED:
      Serial.println("STOP");
      stopMotors();
      break;
  }
}