#include <ESP32Servo.h>

// Optional single-ESP32 controller: buttons and servo on the same board.
const int LEFT_PIN = 25;
const int RIGHT_PIN = 26;
const int SERVO_PIN = 18;
const bool REVERSE = false;
const int MIN_PULSE_US = 1000;
const int CENTER_PULSE_US = 1500;
const int MAX_PULSE_US = 2000;
const unsigned long DEBOUNCE_MS = 5;
Servo myServo;
char sampledCommand = 'S';
char stableCommand = 'S';
unsigned long sampleChangedAt = 0;
int targetPulseUs = CENTER_PULSE_US;

void setup() {
  pinMode(LEFT_PIN, INPUT_PULLUP);
  pinMode(RIGHT_PIN, INPUT_PULLUP);
  myServo.setPeriodHertz(50);
  myServo.attach(SERVO_PIN, MIN_PULSE_US, MAX_PULSE_US);
  myServo.writeMicroseconds(targetPulseUs);
}

void loop() {
  const unsigned long now = millis();
  const bool left = digitalRead(LEFT_PIN) == LOW;
  const bool right = digitalRead(RIGHT_PIN) == LOW;
  const char current = left == right ? 'S' : (left ? 'L' : 'R');
  if (current != sampledCommand) {
    sampledCommand = current;
    sampleChangedAt = now;
  }
  if (now - sampleChangedAt >= DEBOUNCE_MS && stableCommand != sampledCommand) {
    stableCommand = sampledCommand;
    if (stableCommand != 'S') {
      bool ccw = stableCommand == 'L';
      if (REVERSE) ccw = !ccw;
      const int nextTarget = ccw ? MAX_PULSE_US : MIN_PULSE_US;
      if (nextTarget != targetPulseUs) {
        targetPulseUs = nextTarget;
        myServo.writeMicroseconds(targetPulseUs);
      }
    }
  }
  delay(1);
}
