// ---------------- Pin Definitions ----------------
#define MOTOR_LEFT_FORWARD     10
#define MOTOR_LEFT_BACKWARD    11
#define MOTOR_RIGHT_FORWARD    12
#define MOTOR_RIGHT_BACKWARD   13
#define CUTOFF_LEFT            8
#define CUTOFF_RIGHT           9

#define SWEEPER_PIN            A3

#define TRIG_FRONT             2
#define ECHO_FRONT             3
#define TRIG_LEFT              4
#define ECHO_LEFT              5
#define TRIG_RIGHT             6
#define ECHO_RIGHT             7
#define TRIG_BACK              A4
#define ECHO_BACK              A5

#define CH_FWD_BACK            A0
#define CH_LEFT_RIGHT          A1
#define CH_SWEEPER             A2
#define CH_MODE_SWITCH         A0

// ---------------- Constants ----------------
#define OBSTACLE_DISTANCE               150  // cm
#define RC_HIGH_THRESHOLD_FRONTBACK     1600
#define RC_LOW_THRESHOLD_FRONTBACK      1300
#define RC_HIGH_THRESHOLD_RIGHTLEFT     1650
#define RC_LOW_THRESHOLD_RIGHTLEFT      1350
#define RC_THRESHOLD_SWEEPER            1400
#define RC_THRESHOLD_MANUALAUTO         1000

// ---------------- Variables ----------------
bool isManual = false;
bool sweeperState = false;

void setup() {
  // Motor relay pins
  pinMode(MOTOR_LEFT_FORWARD, OUTPUT);
  pinMode(MOTOR_LEFT_BACKWARD, OUTPUT);
  pinMode(MOTOR_RIGHT_FORWARD, OUTPUT);
  pinMode(MOTOR_RIGHT_BACKWARD, OUTPUT);
  pinMode(CUTOFF_LEFT, OUTPUT);
  pinMode(CUTOFF_RIGHT, OUTPUT);

  // Sweeper
  pinMode(SWEEPER_PIN, OUTPUT);

  // Ultrasonic sensors
  pinMode(TRIG_FRONT, OUTPUT);
  pinMode(ECHO_FRONT, INPUT);
  pinMode(TRIG_LEFT, OUTPUT);
  pinMode(ECHO_LEFT, INPUT);
  pinMode(TRIG_RIGHT, OUTPUT);
  pinMode(ECHO_RIGHT, INPUT);
  pinMode(TRIG_BACK, OUTPUT);
  pinMode(ECHO_BACK, INPUT);
}

void loop() {
  isManual = analogRead(CH_MODE_SWITCH) < RC_THRESHOLD_MANUALAUTO;

  if (isManual) {
    handleManualMode();
  } else {
    handleAutoMode();
  }
}

// ---------------- Manual Mode Handler ----------------
void handleManualMode() {
  int chFwdBack = analogRead(CH_FWD_BACK);
  int chLeftRight = analogRead(CH_LEFT_RIGHT);
  int chSweeper = analogRead(CH_SWEEPER);

  // Sweeper control
  sweeperState = chSweeper > RC_THRESHOLD_SWEEPER;
  digitalWrite(SWEEPER_PIN, sweeperState ? HIGH : LOW);

  // Movement control
  if (chFwdBack > RC_HIGH_THRESHOLD_FRONTBACK) {
    moveForward();
  } else if (chFwdBack < RC_LOW_THRESHOLD_FRONTBACK) {
    moveBackward();
  } else if (chLeftRight > RC_HIGH_THRESHOLD_RIGHTLEFT) {
    turnRight();
  } else if (chLeftRight < RC_LOW_THRESHOLD_RIGHTLEFT) {
    turnLeft();
  } else {
    stopMotors();
  }
}

// ---------------- Auto Mode Handler ----------------
void handleAutoMode() {
  digitalWrite(SWEEPER_PIN, HIGH);  // Sweeper ON in auto mode

  int frontDist = getDistance(TRIG_FRONT, ECHO_FRONT);
  int leftDist = getDistance(TRIG_LEFT, ECHO_LEFT);
  int rightDist = getDistance(TRIG_RIGHT, ECHO_RIGHT);
  int backDist = getDistance(TRIG_BACK, ECHO_BACK);

  if (frontDist > OBSTACLE_DISTANCE) {
    moveForward();
  } else {
    stopMotors();
    delay(300);

    if (rightDist > OBSTACLE_DISTANCE && rightDist >= leftDist) {
      turnRight(); delay(400); stopMotors(); delay(300);
      moveForward(); delay(600); stopMotors(); delay(300);
      turnRight(); delay(400); stopMotors(); delay(300);
    } else if (leftDist > OBSTACLE_DISTANCE) {
      turnLeft(); delay(400); stopMotors(); delay(300);
      moveForward(); delay(600); stopMotors(); delay(300);
      turnLeft(); delay(400); stopMotors(); delay(300);
    } else {
      // If both sides blocked, try moving backward if back is clear
      if (backDist > OBSTACLE_DISTANCE) {
        moveBackward();
        delay(500);
        stopMotors();
        delay(300);
      } else {
        // No space to move backward or turn, stop
        stopMotors();
        delay(500);
      }
    }
  }

  delay(50);
}


// ---------------- Motor Control ----------------
void stopMotors() {
  digitalWrite(MOTOR_LEFT_FORWARD, LOW);
  digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
  digitalWrite(MOTOR_RIGHT_FORWARD, LOW);
  digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);

  digitalWrite(CUTOFF_LEFT, LOW);
  digitalWrite(CUTOFF_RIGHT, LOW);
}

void moveForward() {
  digitalWrite(CUTOFF_LEFT, HIGH);
  digitalWrite(CUTOFF_RIGHT, HIGH);

  digitalWrite(MOTOR_LEFT_FORWARD, HIGH);
  digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
  digitalWrite(MOTOR_RIGHT_FORWARD, HIGH);
  digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);
}

void moveBackward() {
  digitalWrite(CUTOFF_LEFT, HIGH);
  digitalWrite(CUTOFF_RIGHT, HIGH);

  digitalWrite(MOTOR_LEFT_FORWARD, LOW);
  digitalWrite(MOTOR_LEFT_BACKWARD, HIGH);
  digitalWrite(MOTOR_RIGHT_FORWARD, LOW);
  digitalWrite(MOTOR_RIGHT_BACKWARD, HIGH);
}

void turnLeft() {
  digitalWrite(CUTOFF_LEFT, LOW);
  digitalWrite(CUTOFF_RIGHT, HIGH);

  digitalWrite(MOTOR_LEFT_FORWARD, LOW);
  digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
  digitalWrite(MOTOR_RIGHT_FORWARD, HIGH);
  digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);
}

void turnRight() {
  digitalWrite(CUTOFF_LEFT, HIGH);
  digitalWrite(CUTOFF_RIGHT, LOW);

  digitalWrite(MOTOR_LEFT_FORWARD, HIGH);
  digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
  digitalWrite(MOTOR_RIGHT_FORWARD, LOW);
  digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);
}

// ---------------- Ultrasonic Distance ----------------
int getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 20000);  // 20ms timeout
  if (duration == 0) return 1000;  // No echo
  return duration * 0.034 / 2;     // cm
}
