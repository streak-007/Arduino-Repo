// ---------------- Pin Definitions ----------------
#define RPWM_L   5    // PWM Forward - Left Motor
#define LPWM_L   6    // PWM Reverse - Left Motor
#define R_EN_L   7    // Forward Enable - Left Motor
#define L_EN_L   8    // Reverse Enable - Left Motor

#define RPWM_R   9    // PWM Forward - Right Motor
#define LPWM_R   10   // PWM Reverse - Right Motor
#define R_EN_R   11   // Forward Enable - Right Motor
#define L_EN_R   12   // Reverse Enable - Right Motor

#define SWEEPER_PIN   A3  // Sweeper control pin
#define BUZZER_PIN    A8  // Buzzer pin

#define TRIG_FRONT    A4
#define ECHO_FRONT    A5
#define TRIG_LEFT     A6
#define ECHO_LEFT     A7
#define TRIG_RIGHT    2
#define ECHO_RIGHT    3
#define TRIG_BACK     4
#define ECHO_BACK     13

#define CH_FWD_BACK A0     // Forward/Backward RC input (CH3)
#define CH_LEFT_RIGHT A1   // Left/Right RC input (CH1)
#define CH_SWEEPER A2      // Sweeper toggle input (CH5)
#define CH_MODE_SWITCH A0  // Mode selection input (CH3 shared)

#define DEBUG true

// ---------------- Constants ----------------
#define OBSTACLE_DISTANCE 100
#define RC_HIGH_THRESHOLD_FRONTBACK 1600
#define RC_LOW_THRESHOLD_FRONTBACK 1300
#define RC_HIGH_THRESHOLD_RIGHTLEFT 1650
#define RC_LOW_THRESHOLD_RIGHTLEFT 1350
#define RC_THRESHOLD_SWEEPER 1400
#define RC_THRESHOLD_MANUALAUTO 1000

// ---------------- Variables ----------------
bool isManual = false;
bool sweeperState = false;
enum Direction { right, left };
Direction currentPattern = right;

void setup() {
  if (DEBUG) Serial.begin(9600);

  // BTS7960 Motor Pins
  pinMode(RPWM_L, OUTPUT);
  pinMode(LPWM_L, OUTPUT);
  pinMode(R_EN_L, OUTPUT);
  pinMode(L_EN_L, OUTPUT);

  pinMode(RPWM_R, OUTPUT);
  pinMode(LPWM_R, OUTPUT);
  pinMode(R_EN_R, OUTPUT);
  pinMode(L_EN_R, OUTPUT);

  // Sweeper and Buzzer
  pinMode(SWEEPER_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Ultrasonic sensors
  pinMode(TRIG_FRONT, OUTPUT);
  pinMode(ECHO_FRONT, INPUT);
  pinMode(TRIG_LEFT, OUTPUT);
  pinMode(ECHO_LEFT, INPUT);
  pinMode(TRIG_RIGHT, OUTPUT);
  pinMode(ECHO_RIGHT, INPUT);
  pinMode(TRIG_BACK, OUTPUT);
  pinMode(ECHO_BACK, INPUT);

  // RC channels
  pinMode(CH_FWD_BACK, INPUT);
  pinMode(CH_LEFT_RIGHT, INPUT);
  pinMode(CH_SWEEPER, INPUT);
}

void loop() {
  int mode = pulseIn(CH_MODE_SWITCH, HIGH, 25000);
  isManual = mode > RC_THRESHOLD_MANUALAUTO;

  // Buzzer based on any obstacle detection
  int fd = getDistance(TRIG_FRONT, ECHO_FRONT);
  int ld = getDistance(TRIG_LEFT, ECHO_LEFT);
  int rd = getDistance(TRIG_RIGHT, ECHO_RIGHT);
  int bd = getDistance(TRIG_BACK, ECHO_BACK);
  if (fd < OBSTACLE_DISTANCE || ld < OBSTACLE_DISTANCE || rd < OBSTACLE_DISTANCE || bd < OBSTACLE_DISTANCE) {
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  if (isManual) {
    if (DEBUG) Serial.println("Mode: Manual");
    handleManualMode();
  } else if (mode > 500) {
    if (DEBUG) Serial.println("Mode: Auto");
    handleAutoMode();
  } else {
    stopMotors();
    digitalWrite(SWEEPER_PIN, HIGH);
  }

  delay(200);
}

int mapSpeed(int val, int minIn, int maxIn, int minOut = 0, int maxOut = 255) {
  val = constrain(val, minIn, maxIn);
  return map(val, minIn, maxIn, minOut, maxOut);
}

void handleManualMode() {
  int chFwdBack = pulseIn(CH_FWD_BACK, HIGH, 25000);
  int chLeftRight = pulseIn(CH_LEFT_RIGHT, HIGH, 25000);
  int chSweeper = pulseIn(CH_SWEEPER, HIGH, 25000);

  sweeperState = chSweeper > RC_THRESHOLD_SWEEPER;
  digitalWrite(SWEEPER_PIN, sweeperState ? LOW : HIGH);

  bool forward = chFwdBack > RC_HIGH_THRESHOLD_FRONTBACK;
  bool backward = chFwdBack < RC_LOW_THRESHOLD_FRONTBACK;
  bool turningRight = chLeftRight > RC_HIGH_THRESHOLD_RIGHTLEFT;
  bool turningLeft = chLeftRight < RC_LOW_THRESHOLD_RIGHTLEFT;

  if (!forward && !backward && !turningRight && !turningLeft) {
    stopMotors();
    return;
  }

  if (forward) {
    int speed = mapSpeed(chFwdBack, RC_HIGH_THRESHOLD_FRONTBACK, 2000);
    if (turningRight) {
      turnRight(speed);
    } else if (turningLeft) {
      turnLeft(speed);
    } else {
      moveForward(speed);
    }
  } else if (backward) {
    int speed = mapSpeed(chFwdBack, 1000, RC_LOW_THRESHOLD_FRONTBACK);
    if (turningRight) {
      reverseTurnRight(speed);
    } else if (turningLeft) {
      reverseTurnLeft(speed);
    } else {
      moveBackward(speed);
    }
  } else {
    if (turningRight) {
      int speed = mapSpeed(chLeftRight, RC_HIGH_THRESHOLD_RIGHTLEFT, 2000);
      turnRight(speed);
    } else if (turningLeft) {
      int speed = mapSpeed(chLeftRight, 1000, RC_LOW_THRESHOLD_RIGHTLEFT);
      turnLeft(speed);
    } else {
      stopMotors();
    }
  }
}

void moveForward(int speed) {
  digitalWrite(R_EN_L, HIGH); digitalWrite(L_EN_L, LOW);
  analogWrite(RPWM_L, speed); analogWrite(LPWM_L, 0);

  digitalWrite(R_EN_R, HIGH); digitalWrite(L_EN_R, LOW);
  analogWrite(RPWM_R, speed); analogWrite(LPWM_R, 0);

  if (DEBUG) Serial.println("Motors: FORWARD");
}

void moveBackward(int speed) {
  digitalWrite(R_EN_L, LOW); digitalWrite(L_EN_L, HIGH);
  analogWrite(RPWM_L, 0); analogWrite(LPWM_L, speed);

  digitalWrite(R_EN_R, LOW); digitalWrite(L_EN_R, HIGH);
  analogWrite(RPWM_R, 0); analogWrite(LPWM_R, speed);

  if (DEBUG) Serial.println("Motors: BACKWARD");
}

void turnLeft(int speed) {
  digitalWrite(R_EN_L, LOW); digitalWrite(L_EN_L, LOW);
  analogWrite(RPWM_L, 0); analogWrite(LPWM_L, 0);

  digitalWrite(R_EN_R, HIGH); digitalWrite(L_EN_R, LOW);
  analogWrite(RPWM_R, speed); analogWrite(LPWM_R, 0);

  if (DEBUG) Serial.println("Motors: TURN LEFT (forward)");
}

void turnRight(int speed) {
  digitalWrite(R_EN_L, HIGH); digitalWrite(L_EN_L, LOW);
  analogWrite(RPWM_L, speed); analogWrite(LPWM_L, 0);

  digitalWrite(R_EN_R, LOW); digitalWrite(L_EN_R, LOW);
  analogWrite(RPWM_R, 0); analogWrite(LPWM_R, 0);

  if (DEBUG) Serial.println("Motors: TURN RIGHT (forward)");
}

void reverseTurnLeft(int speed) {
  digitalWrite(R_EN_L, HIGH); digitalWrite(L_EN_L, LOW);
  analogWrite(RPWM_L, speed); analogWrite(LPWM_L, 0);

  digitalWrite(R_EN_R, LOW); digitalWrite(L_EN_R, HIGH);
  analogWrite(RPWM_R, 0); analogWrite(LPWM_R, speed);

  if (DEBUG) Serial.println("Motors: TURN LEFT (reverse)");
}

void reverseTurnRight(int speed) {
  digitalWrite(R_EN_L, LOW); digitalWrite(L_EN_L, HIGH);
  analogWrite(RPWM_L, 0); analogWrite(LPWM_L, speed);

  digitalWrite(R_EN_R, HIGH); digitalWrite(L_EN_R, LOW);
  analogWrite(RPWM_R, speed); analogWrite(LPWM_R, 0);

  if (DEBUG) Serial.println("Motors: TURN RIGHT (reverse)");
}

void stopMotors() {
  analogWrite(RPWM_L, 0); analogWrite(LPWM_L, 0);
  analogWrite(RPWM_R, 0); analogWrite(LPWM_R, 0);
  if (DEBUG) Serial.println("Motors: STOPPED");
}

int getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 20000);
  if (duration == 0) return 1000;
  return duration * 0.034 / 2;
}

void handleAutoMode() {
  digitalWrite(SWEEPER_PIN, LOW);

  int frontDist = getDistance(TRIG_FRONT, ECHO_FRONT);
  int leftDist = getDistance(TRIG_LEFT, ECHO_LEFT);
  int rightDist = getDistance(TRIG_RIGHT, ECHO_RIGHT);
  int backDist = getDistance(TRIG_BACK, ECHO_BACK);

  if (DEBUG) {
    Serial.print("F: "); Serial.print(frontDist); Serial.print("\t");
    Serial.print("L: "); Serial.print(leftDist); Serial.print("\t");
    Serial.print("R: "); Serial.print(rightDist); Serial.print("\t");
    Serial.print("B: "); Serial.println(backDist);
  }

  if (frontDist > OBSTACLE_DISTANCE) {
    moveForward(200);
  } else {
    stopMotors(); delay(300);

    if (currentPattern == right) {
      if (rightDist > OBSTACLE_DISTANCE) {
        turnRight(200); delay(4000); stopMotors(); currentPattern = left;
      } else if (leftDist > OBSTACLE_DISTANCE) {
        turnLeft(200); delay(4000); stopMotors(); currentPattern = left;
      } else if (backDist > OBSTACLE_DISTANCE) {
        moveBackward(200); delay(2000); stopMotors(); delay(300);
      } else {
        stopMotors(); delay(500);
      }
    } else {
      if (leftDist > OBSTACLE_DISTANCE) {
        turnLeft(200); delay(4000); stopMotors(); currentPattern = right;
      } else if (rightDist > OBSTACLE_DISTANCE) {
        turnRight(200); delay(4000); stopMotors(); currentPattern = right;
      } else if (backDist > OBSTACLE_DISTANCE) {
        moveBackward(200); delay(2000); stopMotors(); delay(300);
      } else {
        stopMotors(); delay(500);
      }
    }
  }
}
