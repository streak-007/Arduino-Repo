// Define pins
const int potPin = A0;    // Potentiometer connected to A0
const int relayPin = 8;   // Relay connected to digital pin 8

// Time delay range (adjust as needed)
const int minDelay = 500;    // Minimum delay in milliseconds
const int maxDelay = 10000;   // Maximum delay in milliseconds

void setup() {
  pinMode(potPin, INPUT);    // Set relay pin as output
  pinMode(relayPin, OUTPUT);    // Set relay pin as output
  digitalWrite(relayPin, LOW);  // Start with relay off
  Serial.begin(9600);           // Initialize serial communication
}

void loop() {
  // Read the potentiometer value (0 to 1023)
  int potValue = analogRead(potPin);

  // Print the potentiometer value to the Serial Monitor
  Serial.print("Potentiometer Value: ");
  Serial.println(potValue);

  // Map the potentiometer value to the desired delay range
  int delayTime = map(potValue, 0, 1023, minDelay, maxDelay);

  // Debugging: Print the delay time to the Serial Monitor
  Serial.print("Mapped Delay Time: ");
  Serial.println(delayTime);

  // Turn the relay ON
  digitalWrite(relayPin, HIGH);
  delay(delayTime); // Wait for the mapped delay time

  // Turn the relay OFF
  digitalWrite(relayPin, LOW);
  delay(delayTime); // Wait for the mapped delay time
}
