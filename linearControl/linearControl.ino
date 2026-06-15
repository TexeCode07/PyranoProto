#include <AccelStepper.h>

// Define stepper motor
#define STEP_PIN 4
#define DIR_PIN 3
#define ENABLE_PIN 2
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// Define limit switches
#define LIMIT_LEFT  5
#define LIMIT_RIGHT 6

// Define push button
#define BUTTON_PIN 14

// Variables to track state
bool movingToRight = true;  // Start by moving toward right limit
bool buttonPressed = false;

void setup() {
    pinMode(LIMIT_LEFT, INPUT_PULLUP);
    pinMode(LIMIT_RIGHT, INPUT_PULLUP);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, LOW);  // Enable the driver (LOW = active)

    stepper.setMaxSpeed(1000);  // Adjust speed (steps per second)
    stepper.setAcceleration(500);  // Adjust acceleration
}

void loop() {
    // Move to limit switch on startup
    if (movingToRight) {
        stepper.setSpeed(500);  // Move right
        while (digitalRead(LIMIT_RIGHT) == HIGH) {
            stepper.runSpeed();
        }
    } else {
        stepper.setSpeed(-500);  // Move left
        while (digitalRead(LIMIT_LEFT) == HIGH) {
            stepper.runSpeed();
        }
    }

    // Stop motor when limit switch is hit
    stepper.stop();
    digitalWrite(ENABLE_PIN, HIGH); // Disable driver to save power
    delay(500);

    // Wait for button press to move in the opposite direction
    while (digitalRead(BUTTON_PIN) == HIGH);
    delay(100);  // Debounce

    // Enable the driver before moving
    digitalWrite(ENABLE_PIN, LOW); // Enable driver
    movingToRight = !movingToRight;
}
