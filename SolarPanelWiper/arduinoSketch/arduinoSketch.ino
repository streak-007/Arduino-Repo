#include <SoftwareSerial.h>
#include <Wire.h>
#include <RTClib.h>

// Define pins
#define RELAY_CW  2    // Relay for clockwise motion
#define RELAY_CCW 3    // Relay for counterclockwise motion
#define PUMP 4         // Pump relay
#define LIMIT_CW 5     // Limit switch for CW end
#define LIMIT_CCW 6    // Limit switch for CCW end
#define BLUETOOTH_RX 7 // Bluetooth RX
#define BLUETOOTH_TX 8 // Bluetooth TX

SoftwareSerial bluetooth(BLUETOOTH_RX, BLUETOOTH_TX);
RTC_DS3231 rtc;
bool automaticMode = true; // Auto mode enabled by default
bool running = false;

void setup() {
    pinMode(RELAY_CW, OUTPUT);
    pinMode(RELAY_CCW, OUTPUT);
    pinMode(PUMP, OUTPUT);
    pinMode(LIMIT_CW, INPUT_PULLUP);
    pinMode(LIMIT_CCW, INPUT_PULLUP);
    
    // Set relays and pump to HIGH by default
    digitalWrite(RELAY_CW, HIGH);
    digitalWrite(RELAY_CCW, HIGH);
    digitalWrite(PUMP, HIGH); // Keep pump on by default
    
    bluetooth.begin(9600);
    Serial.begin(9600);
    Serial.println("Initializing system...");
    bluetooth.println("Initializing system...");
    
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        bluetooth.println("Couldn't find RTC");
    }

    // Print status of automatic mode on startup
    printAutoModeStatus();

    // Print available commands
    printHelp();
}

void loop() {
    if (bluetooth.available()) {
        String command = bluetooth.readStringUntil('\n');
        command.trim();
        Serial.print("Received command: ");
        Serial.println(command);
        bluetooth.print("Received command: ");
        bluetooth.println(command);
        
        if (command == "CLEAN") {
            Serial.println("Starting manual clean...");
            bluetooth.println("Starting manual clean...");
            manualClean();
        } else if (command == "AUTO_ON") {
            Serial.println("Automatic mode enabled.");
            bluetooth.println("Automatic mode enabled.");
            automaticMode = true;
            printAutoModeStatus(); // Print status after change
        } else if (command == "AUTO_OFF") {
            Serial.println("Automatic mode disabled.");
            bluetooth.println("Automatic mode disabled.");
            automaticMode = false;
            printAutoModeStatus(); // Print status after change
        } else if (command == "TIME") {
            DateTime now = rtc.now();
            String timeString = "Current Time: " + String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());
            Serial.println(timeString);
            bluetooth.println(timeString);
        } else if (command == "HELP") {
            printHelp();
        }
    }
    
    if (automaticMode) {
        DateTime now = rtc.now();
        if ((now.hour() == 6 || now.hour() == 18) && now.minute() == 0 && !running) {
            Serial.println("Starting automatic clean...");
            bluetooth.println("Starting automatic clean...");
            running = true;
            manualClean();
        } else if ((now.hour() == 6 || now.hour() == 18) && now.minute() > 1) {
            running = false;
        }
    }
}

void manualClean() {
    Serial.println("Moving clockwise with pump ON...");
    bluetooth.println("Moving clockwise with pump ON...");
    digitalWrite(RELAY_CW, LOW);
    digitalWrite(PUMP, LOW);
    while (digitalRead(LIMIT_CW)) {
        delay(100);
    }
    Serial.println("Clockwise limit reached. Stopping and turning off pump...");
    bluetooth.println("Clockwise limit reached. Stopping and turning off pump...");
    digitalWrite(RELAY_CW, HIGH);
    digitalWrite(PUMP, HIGH);
    delay(500);
    
    Serial.println("Moving counterclockwise...");
    bluetooth.println("Moving counterclockwise...");
    digitalWrite(RELAY_CCW, LOW);
    while (digitalRead(LIMIT_CCW)) {
        delay(100);
    }
    Serial.println("Counterclockwise limit reached. Stopping...");
    bluetooth.println("Counterclockwise limit reached. Stopping...");
    digitalWrite(RELAY_CCW, HIGH);
}

void printAutoModeStatus() {
    String status = automaticMode ? "enabled" : "disabled";
    Serial.println("Automatic mode is " + status);
    bluetooth.println("Automatic mode is " + status);
}

void printHelp() {
    String helpMessage = "Available Commands:\n";
    helpMessage += "1. CLEAN - Start manual clean process\n";
    helpMessage += "2. AUTO_ON - Enable automatic mode\n";
    helpMessage += "3. AUTO_OFF - Disable automatic mode\n";
    helpMessage += "4. TIME - Get the current time from RTC\n";
    helpMessage += "5. HELP - Show this help message\n";
    
    Serial.println(helpMessage);
    bluetooth.println(helpMessage);
}
