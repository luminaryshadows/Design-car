#include "BluetoothSerial.h"
#include <ESP32Servo.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth Classic is not enabled! Select an original ESP32 board.
#endif

BluetoothSerial SerialBT;
Servo myServo;

const int SERVO_PIN = 18;
// Directions are viewed looking at the output shaft. Flip if your servo or
// linkage moves the opposite way. Increasing pulse width is CCW by default.
const bool REVERSE = false;

// Conservative starting range; actual travel varies between MG90S units.
// Calibrate with the linkage unloaded. Do not drive against mechanical stops.
const int MIN_PULSE_US = 1000;
const int CENTER_PULSE_US = 1500;
const int MAX_PULSE_US = 2000;
int targetPulseUs = CENTER_PULSE_US;

void applyCommand(char command) {
  // S (release/both buttons) holds the selected target. It does not cancel a
  // click or stop halfway. The servo also holds its target if Bluetooth drops.
  if (command != 'L' && command != 'R') return;
  bool ccw = command == 'L';
  if (REVERSE) ccw = !ccw;
  const int nextTarget = ccw ? MAX_PULSE_US : MIN_PULSE_US;
  if (nextTarget != targetPulseUs) {
    targetPulseUs = nextTarget;
    // One target update: the servo moves at its own available mechanical speed.
    myServo.writeMicroseconds(targetPulseUs);
  }
}

void setup() {
  Serial.begin(115200);
  myServo.setPeriodHertz(50); // Standard analog-servo refresh; not a speed knob.
  myServo.attach(SERVO_PIN, MIN_PULSE_US, MAX_PULSE_US);
  myServo.writeMicroseconds(targetPulseUs);
  SerialBT.begin("ESP32_Servo");
}

void loop() {
  // Apply every command: a queued L followed by S must still register the click.
  while (SerialBT.available()) {
    applyCommand((char)SerialBT.read());
  }
  delay(1);
}
