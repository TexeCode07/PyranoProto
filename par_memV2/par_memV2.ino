#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

#define RE 8
#define DE 7
#define CS_PIN 4
#define BUTTON_PIN 16  // Push button connected here

const byte pyranometer[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};
byte values[8];

SoftwareSerial mod(2, 3);
File logFile;

bool logging = false;   // Initially, logging is OFF
unsigned long startTime = 0; // Timer reset point
int fileIndex = 1;
bool lastButtonState = HIGH;

void setup() {
    Serial.begin(9600);
    mod.begin(4800);
    pinMode(RE, OUTPUT);
    pinMode(DE, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP); // Enable internal pull-up resistor
    
    if (!SD.begin(CS_PIN)) {
        Serial.println("SD card initialization failed!");
        return;
    }
    
    createNewLogFile();
    delay(1000);
}

void loop() {
    checkButtonPress();
    logSensorData();
    delay(3000);
}

void createNewLogFile() {
    char filename[20];
    do {
        sprintf(filename, "sensorread%02d.csv", fileIndex);
        fileIndex++;
    } while (SD.exists(filename));
    
    logFile = SD.open(filename, FILE_WRITE);
    if (logFile) {
        logFile.println("Millis,Solar Radiation (W/m^2)");
        logFile.close();
        Serial.print("Logging to: ");
        Serial.println(filename);
    } else {
        Serial.println("Error creating file!");
    }
}

void checkButtonPress() {
    bool buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == LOW && lastButtonState == HIGH) { // Detect falling edge (press and release)
        logging = !logging;  // Toggle logging state
        if (logging) {
            Serial.println("Logging started...");
            startTime = millis();  // Reset time reference
        } else {
            Serial.println("Logging stopped.");
        }
        delay(200);  // Debounce delay
    }
    lastButtonState = buttonState;
}

void logSensorData() {
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    delay(10);
    mod.write(pyranometer, sizeof(pyranometer));
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
    delay(10);

    unsigned long readStartTime = millis();
    while (mod.available() < 7 && millis() - readStartTime < 1000) {
        delay(1);
    }
    
    byte index = 0;
    while (mod.available() && index < 8) {
        values[index] = mod.read();
        Serial.print(values[index], HEX);
        Serial.print(" ");
        index++;
    }
    Serial.println();
    
    if (index >= 5) {
        int Solar_Radiation = (values[3] << 8) | values[4];
        
        Serial.print("Solar Radiation: ");
        Serial.print(Solar_Radiation);
        Serial.println(" W/m^2");
        
        if (logging) {  // Only log when logging is active
            char filename[20];
            sprintf(filename, "sensorread%02d.csv", fileIndex - 1);
            logFile = SD.open(filename, FILE_WRITE);
            
            if (logFile) {
                logFile.print(millis() - startTime);  // Time since logging started
                logFile.print(",");
                logFile.println(Solar_Radiation);
                logFile.close();
            } else {
                Serial.println("Error writing to file!");
            }
        }
    }
}
