#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <EEPROM.h>

// LCD
LiquidCrystal lcd(10, 11, A0, A1, A2, A3); // RS, EN, D4, D5, D6, D7

// Pin Definitions
const int leftMotorPin = 6;          // Motor Driver Pin for Left Motor
const int rightMotorPin = 7;         // Motor Driver Pin for Right Motor
const int ultrasonicTrigPin = 8;     // Ultrasonic Sensor Trigger Pin
const int ultrasonicEchoPin = 9;     // Ultrasonic Sensor Echo Pin
const int leftIRSensorPin = A4;      // Left IR Sensor Pin
const int rightIRSensorPin = A5;     // Right IR Sensor Pin
const int buzzerPin = 12;            // Buzzer Pin

// EEPROM Addresses
const int EEPROM_LATITUDE_ADDR = 0;
const int EEPROM_LONGITUDE_ADDR = 4;

// GPS and GSM
SoftwareSerial gpsSerial(2, 3);      // GPS module RX, TX
SoftwareSerial gsmSerial(4, 5);      // GSM module RX, TX
TinyGPS gps;

// Constants
const int distanceThreshold = 10;   // Ultrasonic sensor distance threshold (cm)

// Function Prototypes
void initializeSystem();
void checkModules();
void moveForward();
void stopVehicle();
void checkCracks();
void checkObstruction();
void fetchGPS(long &latitude, long &longitude);
void sendSMS(String message);
void beepBuzzer();
void updateLCD(String line1, String line2);
long measureDistance();
void updateEEPROM(long latitude, long longitude);
void readGPSFromEEPROM(long &latitude, long &longitude);
void handleSMS();
void parseAndSaveLOC(String sms);

// Globals
long savedLatitude = 15157681;
long savedLongitude = 76891262;

void setup() {
  // Initialize components
  initializeSystem();
  checkModules();
  //readGPSFromEEPROM(savedLatitude, savedLongitude);
}

void loop() {
  moveForward();

  checkCracks();
  checkObstruction();
  handleSMS();
  delay(100); // Loop delay for stability
}

// Function Definitions

void initializeSystem() {
  lcd.begin(16, 2);
  Serial.begin(9600);
  pinMode(leftMotorPin, OUTPUT);
  pinMode(rightMotorPin, OUTPUT);
  pinMode(leftIRSensorPin, INPUT);
  pinMode(rightIRSensorPin, INPUT);
  pinMode(ultrasonicTrigPin, OUTPUT);
  pinMode(ultrasonicEchoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  lcd.print("System Init...");
  delay(2000);
}

void checkModules() {
  updateLCD("Checking Modules", "");
  delay(1000);
  
  // Check GPS module
  gpsSerial.begin(9600);
  if (gpsSerial) {
    updateLCD("GPS Module OK", "");
    delay(2000);
  } else {
    updateLCD("GPS Failed!", "");
    while (true); // Halt execution
  }

  // Check GSM module
  gsmSerial.begin(9600);
  if (gsmSerial) {
    updateLCD("GSM Module OK", "");
    delay(2000);
  } else {
    updateLCD("GSM Failed!", "");
    while (true); // Halt execution
  }
}

void moveForward() {
  digitalWrite(leftMotorPin, HIGH);
  digitalWrite(rightMotorPin, HIGH);
  updateLCD("Moving Forward", "");
}

void stopVehicle() {
  digitalWrite(leftMotorPin, LOW);
  digitalWrite(rightMotorPin, LOW);
  updateLCD("Vehicle Stopped", "");
}

void checkCracks() {
  int leftSignal = digitalRead(leftIRSensorPin);
  int rightSignal = digitalRead(rightIRSensorPin);
  long latitude , longitude ;

  if (leftSignal == HIGH || rightSignal == HIGH) { // Crack detected
    stopVehicle();
    beepBuzzer();
    fetchGPS(latitude, longitude);

    String trackSide = (leftSignal == LOW) ? "Right" : "Left";
    String message = "Track Issue: " + trackSide;
    message += "\nLocation: ";
    message += String(latitude / 1000000.0, 6) + ", " + String(longitude / 1000000.0, 6);
    message += "\nMap: https://maps.google.com/?q=";
    message += String(latitude / 1000000.0, 6) + "," + String(longitude / 1000000.0, 6);

    updateLCD("Crack Detected:", trackSide);
    delay(2000);
    updateLCD("Lat:", String(latitude / 1000000.0, 6));
    delay(2000);
    updateLCD("Lon:", String(longitude / 1000000.0, 6));
    delay(2000);

    sendSMS(message);
  }
}

void checkObstruction() {
  long distance = measureDistance();
  long latitude , longitude;

  if (distance <= distanceThreshold) { // Obstruction detected
    stopVehicle();
    beepBuzzer();
    fetchGPS(latitude, longitude);

    String message = "Obstruction Detected\nLocation: ";
    message += String(latitude / 1000000.0, 6) + ", " + String(longitude / 1000000.0, 6);
    message += "\nMap: https://maps.google.com/?q=";
    message += String(latitude / 1000000.0, 6) + "," + String(longitude / 1000000.0, 6);

    updateLCD("Obstruction!", "Detected");
    delay(2000);
    updateLCD("Lat:", String(latitude / 1000000.0, 6));
    delay(2000);
    updateLCD("Lon:", String(longitude / 1000000.0, 6));
    delay(2000);

    sendSMS(message);
  }
}

long measureDistance() {
  digitalWrite(ultrasonicTrigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(ultrasonicTrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(ultrasonicTrigPin, LOW);
  long duration = pulseIn(ultrasonicEchoPin, HIGH);
  return duration * 0.034 / 2; // Convert to cm
}

void fetchGPS(long &latitude, long &longitude) {
  unsigned long age;
  
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    gps.encode(c);  // Feed data to TinyGPS
  }
  
  // Get the position if available
  gps.get_position(&latitude, &longitude, &age);

  // If the GPS data is invalid, retry
  if (latitude == TinyGPS::GPS_INVALID_ANGLE || longitude == TinyGPS::GPS_INVALID_ANGLE) {
    //updateLCD("GPS Failed!", "No Fix");
    //Serial.println("GPS failed to get valid data.");
    //delay(2000);
    latitude = savedLatitude;
    longitude = savedLongitude;
    return; // Try again
  } else {
    //updateLCD("GPS Updated!", "");
    updateEEPROM(latitude, longitude);
    Serial.print("Latitude: ");
    Serial.println(latitude / 1000000.0, 6);
    Serial.print("Longitude: ");
    Serial.println(longitude / 1000000.0, 6);
  }
}

void sendSMS(String message) {
  gsmSerial.println("AT+CMGF=1"); // Set SMS mode to text
  delay(100);
  gsmSerial.println("AT+CMGS=\"+917353203813\""); // Replace with actual number
  delay(100);
  gsmSerial.println(message);
  delay(100);
  gsmSerial.write(26); // CTRL+Z to send
  updateLCD("SMS Sent!", "");
  delay(2000);
  while(true)
  {
    handleSMS();
  }
}

void beepBuzzer() {
  digitalWrite(buzzerPin, HIGH);
  delay(1000);
  digitalWrite(buzzerPin, LOW);
  delay(1000);
}

void updateLCD(String line1, String line2) {
  Serial.println(line1);
  Serial.println(line2);
  lcd.clear();
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void updateEEPROM(long latitude, long longitude) {
  EEPROM.put(EEPROM_LATITUDE_ADDR, latitude);
  EEPROM.put(EEPROM_LONGITUDE_ADDR, longitude);
  Serial.println("GPS saved to EEPROM.");
}

void readGPSFromEEPROM(long &latitude, long &longitude) {
  EEPROM.get(EEPROM_LATITUDE_ADDR, latitude);
  EEPROM.get(EEPROM_LONGITUDE_ADDR, longitude);
  Serial.print("Latitude from EEPROM: ");
  Serial.println(latitude / 1000000.0, 6);
  Serial.print("Longitude from EEPROM: ");
  Serial.println(longitude / 1000000.0, 6);
}

// void handleSMS() {
//   if (gsmSerial.available() > 0) {
//     String sms = "";
//     while (gsmSerial.available()) {
//       char c = gsmSerial.read();
//       sms += c;
//       delay(10);
//     }

//     Serial.println("Received SMS: " + sms);

//     if (sms.indexOf("LOC") != -1) {
//       parseAndSaveLOC(sms);
//     }
//   }
// }

void handleSMS() {
  if (gsmSerial.available() > 0) {
    String response = "";
    while (gsmSerial.available()) {
      char c = gsmSerial.read();
      response += c;
      delay(10);
    }

    Serial.println("Received Response: " + response);

    // Check if the response indicates a new SMS
    if (response.indexOf("+CMTI:") != -1) {
      int index = getSMSIndex(response);
      if (index != -1) {
        readAndProcessSMS(index);
      }
    }
  }
}

// Helper function to extract SMS index from "+CMTI" response
int getSMSIndex(String response) {
  int commaIndex = response.indexOf(",");
  if (commaIndex != -1) {
    return response.substring(commaIndex + 1).toInt();
  }
  return -1; // Invalid index
}

// // Helper function to read and process SMS at a given index
// void readAndProcessSMS(int index) {
//   Serial.print("Reading SMS at index ");
//   Serial.println(index);

//   // Send AT command to read SMS
//   gsmSerial.print("AT+CMGR=");
//   gsmSerial.println(index);
//   delay(1000);

//   // Read the response from the GSM module
//   String smsContent = "";
//   while (gsmSerial.available()) {
//     char c = gsmSerial.read();
//     smsContent += c;
//     delay(10);
//   }

//   Serial.println("SMS Content: ");
//   Serial.println(smsContent);

//   // Check if the SMS starts with "LOC" and process it
//   if (smsContent.indexOf("LOC") != -1) {
//     parseAndSaveLOC(smsContent);
//   }
// }

void readAndProcessSMS(int index) {
  Serial.print("Reading SMS at index ");
  Serial.println(index);

  // Send AT command to read SMS
  
  gsmSerial.print("AT+CMGF=1\r\n");
  gsmSerial.print("AT+CMGL=\"REC UNREAD\"\r\n");
  // gsmSerial.print("AT+CMGR=");
  // gsmSerial.println(index);
  delay(500);

  // Read the response from the GSM module
  String smsContent = "";
  unsigned long startTime = millis();
  
  // Read until a timeout or until the GSM module stops sending data
  while (millis() - startTime < 1000) { // 2-second timeout
    while (gsmSerial.available()) {
      char c = gsmSerial.read();
      smsContent += c;
      startTime = millis(); // Reset timeout when data is received
    }
  }

  Serial.println("SMS Content: ");
  Serial.println(smsContent);

  // Check if the SMS starts with "LOC" and process it
  if (smsContent.indexOf("LOC") != -1) {
    parseAndSaveLOC(smsContent);
  } else {
    Serial.println("No LOC command found in SMS.");
  }
}

void parseAndSaveLOC(String sms) {
  // Extract latitude and longitude from SMS
  sms.trim();
  sms.remove(0, 3);  // Remove "LOC"
  int commaIndex = sms.indexOf(',');
   int locIndex = sms.indexOf("LOC");
  if (commaIndex > 0) {
    String latStr = sms.substring(locIndex + 3, commaIndex);
    String lonStr = sms.substring(commaIndex + 1);

    long latitude = latStr.toFloat() * 1000000;
    long longitude = lonStr.toFloat() * 1000000;

    // Save to EEPROM
    updateEEPROM(latitude, longitude);
    savedLatitude = latitude;
    savedLongitude = longitude;

    updateLCD("SMS Updated:", "EEPROM Saved");
    delay(2000);
  } else {
    updateLCD("SMS Error:", "Invalid LOC");
    delay(2000);
  }
}
