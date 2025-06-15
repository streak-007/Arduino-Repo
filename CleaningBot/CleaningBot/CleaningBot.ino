#define DEBUG true
// ---------------- Pin Definitions ----------------
#define MOTOR_LEFT_POSITIVE 4
#define MOTOR_LEFT_NEGATIVE 5
#define MOTOR_RIGHT_POSITIVE 6
#define MOTOR_RIGHT_NEGATIVE 7
#define CUTOFF_LEFT 2
#define CUTOFF_RIGHT 3

#define SWEEPER_PIN A3

#define TRIG_FRONT 8
#define ECHO_FRONT 9
#define TRIG_LEFT 10
#define ECHO_LEFT 11
#define TRIG_RIGHT 12
#define ECHO_RIGHT 13
#define TRIG_BACK A4
#define ECHO_BACK A5

#define CH_FWD_BACK A0     //CH3
#define CH_LEFT_RIGHT A1   //CH1
#define CH_SWEEPER A2      //CH5
#define CH_MODE_SWITCH A0  //CH3

// ---------------- Constants ----------------
#define OBSTACLE_DISTANCE 20  // cm
#define MAX_OBSTACLE_DIST 200 //failsafe
#define RC_HIGH_THRESHOLD_FRONTBACK 1600
#define RC_LOW_THRESHOLD_FRONTBACK 1300
#define RC_HIGH_THRESHOLD_RIGHTLEFT 1650
#define RC_LOW_THRESHOLD_RIGHTLEFT 1350
#define RC_THRESHOLD_SWEEPER 1400
#define RC_THRESHOLD_MANUALAUTO 1000

// ---------------- Variables ----------------
bool isManual = false;
bool sweeperState = false;
enum Direction { right,
                 left };
Direction currentPattern = right;

void setup() {
  // Motor relay pins
  pinMode(MOTOR_LEFT_POSITIVE, OUTPUT);
  pinMode(MOTOR_LEFT_NEGATIVE, OUTPUT);
  pinMode(MOTOR_RIGHT_POSITIVE, OUTPUT);
  pinMode(MOTOR_RIGHT_NEGATIVE, OUTPUT);
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

  pinMode(CH_FWD_BACK, INPUT);
  pinMode(CH_LEFT_RIGHT, INPUT);
  pinMode(CH_SWEEPER, INPUT);

  if (DEBUG)
    Serial.begin(9600);
}

void loop() {
  int mode = pulseIn(CH_MODE_SWITCH, HIGH, 25000);
  if (DEBUG) {
    Serial.print("CH_MODE_SWITCH");
    Serial.print(": ");
    Serial.print(mode);
    Serial.print("\t");
  }

  isManual = mode > RC_THRESHOLD_MANUALAUTO;

  if (isManual) {
    if (DEBUG)
      Serial.print("Manual Mode\t");
    handleManualMode();
  } else if (mode > 500) {
    if (DEBUG)
      Serial.print("Auto Mode\t");
    handleAutoMode();
  } else {
    stopMotors();
    if (DEBUG)
      Serial.print("Sweeper Off\t");
    digitalWrite(SWEEPER_PIN, HIGH);
  }

  if (DEBUG)
    Serial.println();
  delay(200);
}

// ---------------- Manual Mode Handler ----------------
void handleManualMode() {
  int chFwdBack = pulseIn(CH_FWD_BACK, HIGH, 25000);
  int chLeftRight = pulseIn(CH_LEFT_RIGHT, HIGH, 25000);
  int chSweeper = pulseIn(CH_SWEEPER, HIGH, 25000);

  if (DEBUG) {
    Serial.print("chFwdBack");
    Serial.print(": ");
    Serial.print(chFwdBack);
    Serial.print("\t");

    Serial.print("chLeftRight");
    Serial.print(": ");
    Serial.print(chLeftRight);
    Serial.print("\t");

    Serial.print("chSweeper");
    Serial.print(": ");
    Serial.print(chSweeper);
    Serial.print("\t");
  }

  // Sweeper control
  sweeperState = chSweeper > RC_THRESHOLD_SWEEPER;
  if (DEBUG) {
    if (sweeperState) {
      Serial.print("Sweeper On\t");
    } else {
      Serial.print("Sweeper Off\t");
    }
  }
  digitalWrite(SWEEPER_PIN, sweeperState ? LOW : HIGH);

  // Movement control
  if (chLeftRight > RC_HIGH_THRESHOLD_RIGHTLEFT) {
    turnRight();
  } else if (chLeftRight < RC_LOW_THRESHOLD_RIGHTLEFT) {
    turnLeft();
  } else if (chFwdBack > RC_HIGH_THRESHOLD_FRONTBACK) {
    moveForward();
  } else if (chFwdBack < RC_LOW_THRESHOLD_FRONTBACK) {
    moveBackward();
  } else {
    stopMotors();
  }
}

// ---------------- Auto Mode Handler ----------------
void handleAutoMode() {
  digitalWrite(SWEEPER_PIN, LOW);  // Sweeper ON in auto mode

  int frontDist = getDistance(TRIG_FRONT, ECHO_FRONT);
  int leftDist = getDistance(TRIG_LEFT, ECHO_LEFT);
  int rightDist = getDistance(TRIG_RIGHT, ECHO_RIGHT);
  int backDist = getDistance(TRIG_BACK, ECHO_BACK);

  if (DEBUG) {
    Serial.print("frontDist");
    Serial.print(": ");
    Serial.print(frontDist);
    Serial.print("\t");

    Serial.print("leftDist");
    Serial.print(": ");
    Serial.print(leftDist);
    Serial.print("\t");

    Serial.print("rightDist");
    Serial.print(": ");
    Serial.print(rightDist);
    Serial.print("\t");

    Serial.print("backDist");
    Serial.print(": ");
    Serial.print(backDist);
    Serial.print("\t");
  }

  if (frontDist < MAX_OBSTACLE_DIST && leftDist < MAX_OBSTACLE_DIST && rightDist < MAX_OBSTACLE_DIST && backDist < MAX_OBSTACLE_DIST) {
    if (frontDist > OBSTACLE_DISTANCE) {
      moveForward();
    } else {
      if (DEBUG) Serial.print("Obstacle detected - Stopping\t");
      stopMotors();
      delay(300);

      if (currentPattern == right) {
        if (rightDist > OBSTACLE_DISTANCE) {
          if (DEBUG) Serial.print("Turning Right\t");
          turnRight();
          delay(4000);
          stopMotors();
          currentPattern = left;  // Toggle pattern
        } else if (leftDist > OBSTACLE_DISTANCE) {
          if (DEBUG) Serial.print("Turning Left (fallback)\t");
          turnLeft();
          delay(4000);
          stopMotors();
          currentPattern = left;  // Toggle anyway
        } else {
          if (backDist > OBSTACLE_DISTANCE) {
            if (DEBUG) Serial.print("Backing up\t");
            moveBackward();
            delay(2000);
            stopMotors();
            delay(300);
          } else {
            if (DEBUG) Serial.print("No space to move\t");
            stopMotors();
            delay(500);
          }
        }
      } else {  // currentPattern == left
        if (leftDist > OBSTACLE_DISTANCE) {
          if (DEBUG) Serial.print("Turning Left\t");
          turnLeft();
          delay(4000);
          stopMotors();
          currentPattern = right;  // Toggle pattern
        } else if (rightDist > OBSTACLE_DISTANCE) {
          if (DEBUG) Serial.print("Turning Right (fallback)\t");
          turnRight();
          delay(4000);
          stopMotors();
          currentPattern = right;  // Toggle anyway
        } else {
          if (backDist > OBSTACLE_DISTANCE) {
            if (DEBUG) Serial.print("Backing up\t");
            moveBackward();
            delay(2000);
            stopMotors();
            delay(300);
          } else {
            if (DEBUG) Serial.print("No space to move\t");
            stopMotors();
            delay(500);
          }
        }
      }
    }
  }
  delay(50);
}


// ---------------- Motor Control ----------------
void stopMotors() {
  if (DEBUG)
    Serial.print("Stop\t");
  digitalWrite(MOTOR_LEFT_POSITIVE, HIGH);
  digitalWrite(MOTOR_LEFT_NEGATIVE, HIGH);
  digitalWrite(MOTOR_RIGHT_POSITIVE, HIGH);
  digitalWrite(MOTOR_RIGHT_NEGATIVE, HIGH);

  digitalWrite(CUTOFF_LEFT, HIGH);
  digitalWrite(CUTOFF_RIGHT, HIGH);
}

void moveForward() {
  if (DEBUG)
    Serial.print("Forward\t");
  digitalWrite(CUTOFF_LEFT, LOW);
  digitalWrite(CUTOFF_RIGHT, LOW);

  digitalWrite(MOTOR_LEFT_POSITIVE, HIGH);
  digitalWrite(MOTOR_LEFT_NEGATIVE, HIGH);
  digitalWrite(MOTOR_RIGHT_POSITIVE, HIGH);
  digitalWrite(MOTOR_RIGHT_NEGATIVE, HIGH);
}

void moveBackward() {
  if (DEBUG)
    Serial.print("Backward\t");
  digitalWrite(CUTOFF_LEFT, LOW);
  digitalWrite(CUTOFF_RIGHT, LOW);

  digitalWrite(MOTOR_LEFT_POSITIVE, LOW);
  digitalWrite(MOTOR_LEFT_NEGATIVE, LOW);
  digitalWrite(MOTOR_RIGHT_POSITIVE, LOW);
  digitalWrite(MOTOR_RIGHT_NEGATIVE, LOW);
}

void turnLeft() {
  if (DEBUG)
    Serial.print("Left\t");
  digitalWrite(CUTOFF_LEFT, LOW);
  digitalWrite(CUTOFF_RIGHT, HIGH);

  digitalWrite(MOTOR_LEFT_POSITIVE, LOW);
  digitalWrite(MOTOR_LEFT_NEGATIVE, LOW);
  digitalWrite(MOTOR_RIGHT_POSITIVE, HIGH);
  digitalWrite(MOTOR_RIGHT_NEGATIVE, HIGH);
}

void turnRight() {
  if (DEBUG)
    Serial.print("Right\t");
  digitalWrite(CUTOFF_LEFT, HIGH);
  digitalWrite(CUTOFF_RIGHT, LOW);

  digitalWrite(MOTOR_LEFT_POSITIVE, HIGH);
  digitalWrite(MOTOR_LEFT_NEGATIVE, HIGH);
  digitalWrite(MOTOR_RIGHT_POSITIVE, LOW);
  digitalWrite(MOTOR_RIGHT_NEGATIVE, LOW);
}

// ---------------- Ultrasonic Distance ----------------
int getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 20000);  // 20ms timeout
  if (duration == 0) return 1000;                 // No echo
  return duration * 0.034 / 2;                    // cm
}
