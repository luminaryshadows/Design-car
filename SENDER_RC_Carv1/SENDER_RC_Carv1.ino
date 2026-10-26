#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth Classic is not enabled! Select an original ESP32 board.
#endif

BluetoothSerial SerialBT;

const int LEFT_PIN = 25;
const int RIGHT_PIN = 26;
const char* RECEIVER_NAME = "ESP32_Servo";
const unsigned long DEBOUNCE_MS = 5;
const unsigned long HEARTBEAT_MS = 50;
const unsigned long RECONNECT_MS = 1000;

char sampledCommand = 'S';
char stableCommand = 'S';
char sentCommand = '\0';
unsigned long sampleChangedAt = 0;
unsigned long lastSentAt = 0;
unsigned long lastConnectAttempt = 0;
bool firstConnectAttempt = true;
bool wasConnected = false;

char readButtons() {
  const bool left = digitalRead(LEFT_PIN) == LOW;
  const bool right = digitalRead(RIGHT_PIN) == LOW;
  if (left == right) return 'S'; // Neither or both: keep the last position.
  return left ? 'L' : 'R';
}

void setup() {
  Serial.begin(115200);
  pinMode(LEFT_PIN, INPUT_PULLUP); // Each button connects its GPIO to GND.
  pinMode(RIGHT_PIN, INPUT_PULLUP);
  SerialBT.begin("ESP32_Sensor", true);
}

void loop() {
  unsigned long now = millis();
  if (!SerialBT.connected(0)) {
    wasConnected = false;
    if (firstConnectAttempt || now - lastConnectAttempt >= RECONNECT_MS) {
      firstConnectAttempt = false;
      // Name discovery/connect may block, but only while disconnected.
      SerialBT.connect(RECEIVER_NAME);
      lastConnectAttempt = millis();
    }
    delay(1);
    return;
  }

  if (!wasConnected) {
    wasConnected = true;
    sampledCommand = readButtons();
    stableCommand = 'S';
    sentCommand = '\0';
    sampleChangedAt = now;
  }

  const char current = readButtons();
  if (current != sampledCommand) {
    sampledCommand = current;
    sampleChangedAt = now;
  }
  if (now - sampleChangedAt >= DEBOUNCE_MS) {
    stableCommand = sampledCommand;
  }

  // Send changes immediately after debounce; keep the existing L/R/S protocol.
  if (stableCommand != sentCommand || now - lastSentAt >= HEARTBEAT_MS) {
    if (SerialBT.write((uint8_t)stableCommand) == 1) {
      sentCommand = stableCommand;
      lastSentAt = now;
    }
  }
  delay(1); // Yield to ESP32 Bluetooth/RTOS without a 50 ms input delay.
}
