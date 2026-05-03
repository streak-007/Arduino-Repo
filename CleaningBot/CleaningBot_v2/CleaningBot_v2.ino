// ---------------- Pin Definitions ----------------
#define RPWM_L   2    // PWM Forward - Left Motor
#define LPWM_L   3    // PWM Reverse - Left Motor
#define R_EN_L   4    // Forward Enable - Left Motor
#define L_EN_L   5    // Reverse Enable - Left Motor

#define RPWM_R   6    // PWM Forward - Right Motor
#define LPWM_R   7    // PWM Reverse - Right Motor
#define R_EN_R   8    // Forward Enable - Right Motor
#define L_EN_R   9    // Reverse Enable - Right Motor

#define SWEEPER_PIN   30  // Sweeper control pin
#define BUZZER_PIN    31  // Buzzer pin

#define TRIG_FRONT    32
#define ECHO_FRONT    33
#define TRIG_RIGHT    34
#define ECHO_RIGHT    35
#define TRIG_LEFT     36
#define ECHO_LEFT     37
#define TRIG_BACK     38
#define ECHO_BACK     39

#define CH_FWD_BACK     A0     // Forward/Backward RC input (CH3)
#define CH_LEFT_RIGHT   A1   // Left/Right RC input (CH1)
#define CH_SWEEPER      A2      // Sweeper toggle input (CH5)
#define CH_MODE_SWITCH  A0  // Mode selection input (CH3 shared)

#define DEBUG true

// ---------------- Constants ----------------
#define OBSTACLE_DISTANCE 40
#define RC_THRESHOLD_SWEEPER 1400      
#define RC_THRESHOLD_MANUALAUTO 1000

// Input PWM Mapping Ranges

#define RC_FORWARD_MAX 1800
#define RC_FORWARD_MIN 1550
#define RC_BACKWARD_MAX 1300
#define RC_BACKWARD_MIN 1140


#define RC_RIGHT_MAX 1840
#define RC_RIGHT_MIN 1650
#define RC_LEFT_MAX 1350
#define RC_LEFT_MIN 1090

#define MAX_SPEED_FWD 255
#define MAX_SPEED_REV 100
#define MIN_SPEED_FWD 20
#define MIN_SPEED_REV 10



// ---------------- Variables ----------------
bool isManual = false;
bool sweeperState = false;
enum Direction { right, left };
Direction currentPattern = right;
int frontDist;
int leftDist;
int rightDist;
int backDist;
  

void setup() {
  pinMode(RPWM_L, OUTPUT);
  pinMode(LPWM_L, OUTPUT);
  pinMode(R_EN_L, OUTPUT);
  pinMode(L_EN_L, OUTPUT);
  pinMode(RPWM_R, OUTPUT);
  pinMode(LPWM_R, OUTPUT);
  pinMode(R_EN_R, OUTPUT);
  pinMode(L_EN_R, OUTPUT);

  pinMode(SWEEPER_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

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

  digitalWrite(R_EN_L, HIGH);
  digitalWrite(L_EN_L, HIGH);
  digitalWrite(R_EN_R, HIGH);
  digitalWrite(L_EN_R, HIGH);

  Serial.begin(9600);
}

void loop() {

  frontDist = getDistance(TRIG_FRONT, ECHO_FRONT);
  leftDist = getDistance(TRIG_LEFT, ECHO_LEFT);
  rightDist = getDistance(TRIG_RIGHT, ECHO_RIGHT);
  backDist = getDistance(TRIG_BACK, ECHO_BACK);

  if (frontDist < OBSTACLE_DISTANCE || leftDist < 20 || rightDist < 20 || backDist < 20) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
  } 
  int mode = pulseIn(CH_MODE_SWITCH, HIGH, 25000);
  isManual = mode > RC_THRESHOLD_MANUALAUTO;
  Serial.print(mode);Serial.print(" | ");
  if (isManual) {
    if (DEBUG) Serial.print("Manual Mode | "); 
    handleManualMode(mode);
  } else if (mode > 500) {
    if (DEBUG) Serial.print("Auto Mode | ") ;
    handleAutoMode();
  } else {
    stopMotors();
    digitalWrite(SWEEPER_PIN, HIGH);
  }


  Serial.println();
  delay(200);
}

void handleManualMode(int chFwdBack) {
  int chLeftRight = pulseIn(CH_LEFT_RIGHT, HIGH, 25000);
  int chSweeper = pulseIn(CH_SWEEPER, HIGH, 25000);

  sweeperState = chSweeper > RC_THRESHOLD_SWEEPER;
  digitalWrite(SWEEPER_PIN, sweeperState ? LOW : HIGH);
  if (sweeperState) Serial.print(" | Sweeper On");

  int speedFwd = constrain(map(chFwdBack, RC_FORWARD_MIN, RC_FORWARD_MAX, MIN_SPEED_FWD, MAX_SPEED_FWD), MIN_SPEED_FWD, MAX_SPEED_FWD);
  int speedLeftFwd = constrain(map(chLeftRight, RC_LEFT_MAX, RC_LEFT_MIN, MIN_SPEED_FWD, MAX_SPEED_FWD), MIN_SPEED_FWD, MAX_SPEED_FWD);
  int speedRightFwd = constrain(map(chLeftRight, RC_RIGHT_MIN, RC_RIGHT_MAX, MIN_SPEED_FWD, MAX_SPEED_FWD), MIN_SPEED_FWD, MAX_SPEED_FWD);

  int speedRev = constrain(map(chFwdBack, RC_BACKWARD_MAX, RC_BACKWARD_MIN, MIN_SPEED_REV, MAX_SPEED_REV), MIN_SPEED_REV, MAX_SPEED_REV);
  int speedLeftRev = constrain(map(chLeftRight, RC_LEFT_MAX, RC_LEFT_MIN, MIN_SPEED_REV, MAX_SPEED_REV), MIN_SPEED_REV, MAX_SPEED_REV);
  int speedRightRev = constrain(map(chLeftRight, RC_RIGHT_MIN, RC_RIGHT_MAX, MIN_SPEED_REV, MAX_SPEED_REV), MIN_SPEED_REV, MAX_SPEED_REV);

  int leftMotorSpeed = 0;
  int rightMotorSpeed = 0;

  Serial.print(" | CH_FWD_BACK: "); Serial.print(chFwdBack);
  Serial.print(" | CH_LEFT_RIGHT: "); Serial.print(chLeftRight);
  Serial.print(" | speedFwd: "); Serial.print(speedFwd);
  Serial.print(" | speedLeftFwd: "); Serial.print(speedLeftFwd);
  Serial.print(" | speedRightFwd: "); Serial.print(speedRightFwd);
  Serial.print(" | speedRev: "); Serial.print(speedRev);
  Serial.print(" | speedLeftRev: "); Serial.print(speedLeftRev);
  Serial.print(" | speedRightRev: "); Serial.print(speedRightRev);

  if (chFwdBack > RC_FORWARD_MIN) 
  {
      leftMotorSpeed = constrain(speedFwd - speedLeftFwd, 0,MAX_SPEED_FWD);
      rightMotorSpeed = constrain(speedFwd - speedRightFwd, 0,MAX_SPEED_FWD);
      moveForward(leftMotorSpeed, rightMotorSpeed);
  } 
  else if (chFwdBack < RC_BACKWARD_MAX) 
  {
      leftMotorSpeed = constrain(speedRev - speedLeftRev, 0,MAX_SPEED_REV);
      rightMotorSpeed = constrain(speedRev - speedRightRev, 0,MAX_SPEED_REV);
      moveBackward(leftMotorSpeed, rightMotorSpeed);
  } 
  else 
  {
      leftMotorSpeed = 0;
      rightMotorSpeed = 0;
      stopMotors();
  }
}

int getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 20000);
  if (duration == 0) return 1000;
  return duration * 0.034 / 2;
}

void moveForward(int leftspeed, int rightspeed) {
  if(frontDist < OBSTACLE_DISTANCE){
        stopMotors();
        return;
    }

  //Reverse pins set to LOW
  // digitalWrite(L_EN_L, LOW); digitalWrite(L_EN_R, LOW);
  analogWrite(LPWM_L, 0); analogWrite(LPWM_R, 0);

  //Forward pins set to HIGH
  // digitalWrite(R_EN_L, HIGH); digitalWrite(R_EN_R, HIGH);
  analogWrite(RPWM_L, leftspeed); analogWrite(RPWM_R, rightspeed);
  
      
  if (DEBUG) Serial.print(" | Motors: FORWARD | Speed: "); if (DEBUG) Serial.print(leftspeed); if (DEBUG) Serial.print(" | "); if (DEBUG) Serial.print(rightspeed);
}

void moveBackward(int leftspeed, int rightspeed) {
  if(backDist < 20){
        stopMotors();
        return;
    }
    //Forward pins set to LOW
  // digitalWrite(R_EN_L, LOW); digitalWrite(R_EN_R, LOW);
  analogWrite(RPWM_L, 0); analogWrite(RPWM_R, 0);

  //Reverse pins set to HIGH
  // digitalWrite(L_EN_L, HIGH); digitalWrite(L_EN_R, HIGH);
  analogWrite(LPWM_L, leftspeed); analogWrite(LPWM_R, rightspeed);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
  if (DEBUG) Serial.print(" | Motors: REVERSE | Speed: "); if (DEBUG) Serial.print(leftspeed); if (DEBUG) Serial.print(" | "); if (DEBUG) Serial.print(rightspeed);
}

void stopMotors() {
  analogWrite(RPWM_L, 0); analogWrite(LPWM_L, 0);
  analogWrite(RPWM_R, 0); analogWrite(LPWM_R, 0);
  // digitalWrite(R_EN_L, LOW); digitalWrite(L_EN_L, LOW);
  // digitalWrite(R_EN_R, LOW); digitalWrite(L_EN_R, LOW);
  if (DEBUG) Serial.print(" | Motors: STOP");
}

//-------------------------------------------------------------------------------------------
void handleAutoMode() {
  digitalWrite(SWEEPER_PIN, LOW);

  int autoSpeed = 50;
  int delayPeriod = 2000;

  if (frontDist > OBSTACLE_DISTANCE) {
    moveForward(autoSpeed,autoSpeed);
  } else {
    stopMotors(); delay(delayPeriod);
    if (currentPattern == right && rightDist > OBSTACLE_DISTANCE) {
      moveForward(autoSpeed, 0); delay(5000); stopMotors(); currentPattern = left;
    } else if (currentPattern == left && leftDist > OBSTACLE_DISTANCE) {
      moveForward(0, autoSpeed); delay(5000); stopMotors(); currentPattern = right;
    } else if (backDist > OBSTACLE_DISTANCE) {
      moveBackward(autoSpeed, autoSpeed); delay(5000); stopMotors();
    } else {
      stopMotors(); delay(500);
    }
  }
  delay(50);
}
