#include <Servo.h>

// Motor pins
const int motor1Pin1 = 4;
const int motor1Pin2 = 5;
const int motor2Pin1 = 6;
const int motor2Pin2 = 7;

// Joystick pins
const int joystickX = A0; // X-axis connected to A0
const int joystickY = A1; // Y-axis connected to A1

// Servo pins
const int servoLeftPin = 2;
const int servoRightPin = 3;

// Servo objects
Servo servoLeft;
Servo servoRight;

// Joystick calibration values
const int joystickCenter = 330; // Assuming a value of 512 for centered joystick
const int joystickDeadzone = 100; // Deadzone for joystick

// Servo angle limits
const int servoLeftMin = 60;
const int servoLeftMax = 120;
const int servoRightMin = 60;
const int servoRightMax = 120;

void setup() {
  // Set motor pins as outputs
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);

  // Attach servos
  servoLeft.attach(servoLeftPin);
  servoRight.attach(servoRightPin);

  // Initialize servos to center
  servoLeft.write((servoLeftMin + servoLeftMax) / 2);
  servoRight.write((servoRightMin + servoRightMax) / 2);

  // Start serial communication for debugging
  Serial.begin(9600);
}

void loop() {
  // Read joystick values
  int xValue = analogRead(joystickX);
  int yValue = analogRead(joystickY);

  // Debug joystick values
  Serial.print("Joystick X: ");
  Serial.print(xValue);
  Serial.print(" | Joystick Y: ");
  Serial.println(yValue);

  // Motor control based on Y-axis
  if (yValue > joystickCenter + joystickDeadzone) {
    moveForward();
  } else if (yValue < joystickCenter - joystickDeadzone) {
    moveBackward();
  } else {
    stopMotors();
  }

  // Steering control based on X-axis
  int servoAngleLeft = map(xValue, 0, 664, servoLeftMin, servoLeftMax);
  servoAngleLeft = constrain(servoAngleLeft, servoLeftMin, servoLeftMax);

  int servoAngleRight = map(xValue, 0, 664, servoRightMin, servoRightMax);
  servoAngleRight = constrain(servoAngleRight, servoRightMin, servoRightMax);

  servoLeft.write(servoAngleLeft);
  servoRight.write(servoAngleRight);

  Serial.print(servoAngleRight);
  // Small delay for stability
  delay(20);
}

void moveForward() {
  Serial.print("moveForward");
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);
}

void moveBackward() {
  Serial.print("moveBackward");
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);
}

void stopMotors() {
  Serial.print("stopMotors");
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
}
