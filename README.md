# Design-car — ESP32 MG90S Steering Controller

A minimal, responsive steering controller for a small RC car built around the
MG90S micro servo. Two push buttons drive the servo to a left or right target
position — either over Bluetooth Classic with two ESP32s, or wired on a single
board with the lowest possible latency.

The project exists for one reason: the original firmware ramped the servo one
degree every 15 ms, so a 90° command took 1.35 s. This version removes the
software ramp entirely and commands the target position in a single step, then
lets the servo move at its own mechanical speed.

---

## Contents

| Sketch | Boards | Use it when |
| --- | --- | --- |
| [`WIRED_MG90S`](WIRED_MG90S/WIRED_MG90S.ino) | 1 × ESP32 | **Start here.** Lowest latency, simplest wiring. |
| [`SENDER_RC_Carv1`](SENDER_RC_Carv1/SENDER_RC_Carv1.ino) + [`RECIVER_RC_CARv1`](RECIVER_RC_CARv1/RECIVER_RC_CARv1.ino) | 2 × ESP32 | You need the buttons off-board (handheld remote). |

Full wiring, calibration and tuning notes live in **[SERVO_SETUP.md](SERVO_SETUP.md)**.

---

## Hardware

- 2 × ESP32 with **Bluetooth Classic** for the wireless pair — an original
  ESP32-WROOM-32 works; ESP32-S3 / C3 / C6 **do not** (Bluetooth Classic is
  absent, and both sketches fail to compile with an explicit `#error`).
- 1 × MG90S micro servo.
- 2 × momentary push buttons (internal pull-ups, no external resistors needed).
- A regulated external **5 V** supply for the servo — 2 A is a reasonable
  starting point, subject to the servo's stall current.
- Common ground between the servo supply and the ESP32.

Total BOM is roughly a couple of ESP32 dev boards, one MG90S, two buttons and a
5 V supply.

---

## How it behaves

- **Left click** → target the CCW position. **Right click** → target the CW
  position (directions as viewed looking at the output shaft).
- **Release**, **both buttons**, and **loss of Bluetooth** all hold the last
  selected target. Clicking does not centre the servo and does not stop it
  part-way.
- **On boot** the servo starts at centre (1500 µs) and stays there.
- **Repeated clicks** in the same direction are no-ops once that target is
  already reached.
- Buttons are debounced for **5 ms**; the wireless link sends on change and
  falls back to a **50 ms heartbeat** so a dropped packet can't wedge the car.

> An MG90S is a **positional** servo — it cannot rotate continuously. These
> sketches are not intended for a continuous-rotation conversion.

---

## Quick start (wired, single board)

1. Arduino IDE → install the **Espressif ESP32** board package and the
   **ESP32Servo** library.
2. Open `WIRED_MG90S/WIRED_MG90S.ino` (keep the sketch in its same-named
   folder) and upload.
3. Wire it up:

| Signal | ESP32 pin |
| --- | --- |
| Left button | GPIO25 → GND |
| Right button | GPIO26 → GND |
| Servo signal (orange/yellow) | GPIO18 |
| Servo power (red) | external 5 V supply + |
| Servo ground (brown/black) | supply − **and** ESP32 GND |

4. Power up **unloaded** and confirm: left button → CCW, right button → CW.

## Quick start (wireless, two boards)

1. Flash `SENDER_RC_Carv1` to the button board, `RECIVER_RC_CARv1` to the servo
   board.
2. The sender boots advertising as `ESP32_Sensor` and auto-connects to the
   receiver named `ESP32_Servo`, retrying every 1000 ms. No pairing step and no
   hard-coded MAC address.
3. Wire buttons to GPIO25/26 on the sender, servo signal to GPIO18 on the
   receiver, and keep the external 5 V servo supply with a common ground.

---

## Calibration and tuning

| Constant | File | Default | Effect |
| --- | --- | --- | --- |
| `REVERSE` | receiver / wired | `false` | Flips direction if your servo or linkage moves the wrong way. |
| `MIN_PULSE_US` / `MAX_PULSE_US` | receiver / wired | `1000` / `2000` | Travel limits. Narrower than the original 500–2400 µs range on purpose. |
| `DEBOUNCE_MS` | sender / wired | `5` | Button debounce window. |
| `HEARTBEAT_MS` | sender | `50` | Re-send interval when the command is unchanged. |
| `RECONNECT_MS` | sender | `1000` | Bluetooth reconnect retry interval. |

Adjust `MIN_PULSE_US` / `MAX_PULSE_US` **only** after checking the servo's
supported range and your linkage travel. Back off if the servo buzzes or presses
against a mechanical stop.

**Power notes:** never power the servo from a GPIO or the ESP32 3.3 V pin, and
avoid tying the external 5 V rail to USB power. If the supply still dips, use
shorter leads and add a 470–1000 µF capacitor (≥ 10 V) across the servo power
near the servo — with correct polarity. A capacitor does not replace an
adequate supply.

---

## Troubleshooting

| Symptom | Likely cause |
| --- | --- |
| `#error Bluetooth Classic is not enabled` | Board is an ESP32-S3/C3. Use an original ESP32. |
| Sender never connects | Receiver not powered, or Bluetooth name changed. |
| Servo jitters or browns out | Servo fed from the ESP32 instead of a separate 5 V supply. |
| Movement direction is inverted | Set `REVERSE = true` in the receiver or wired sketch. |
| Movement still feels slow | It is the servo's own speed limit, not the code. Use a servo with a faster sec/60° rating at your voltage. |
| Servo buzzes at an end position | Travel range too wide for the linkage — narrow `MIN/MAX_PULSE_US`. |

---

## Verification

Software simulations exercise debounce, prompt sends, heartbeat behaviour,
left/right targets, release, repeated commands, and batched click/release input.
They do **not** verify the real ESP32 libraries, the radio link, wiring,
physical direction, or servo speed. Always compile/upload with your installed
board package and test both directions unloaded before reconnecting a load.

---

## License

MIT — see [LICENSE](LICENSE).
