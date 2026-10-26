# Fast MG90S controller

Flash SENDER_RC_Carv1/SENDER_RC_Carv1.ino to the button ESP32 and
RECIVER_RC_CARv1/RECIVER_RC_CARv1.ino to the servo ESP32. Use an original
ESP32 with Bluetooth Classic (for example ESP32-WROOM-32), not an ESP32-S3/C3.
Install the Espressif ESP32 board package and the ESP32Servo library in Arduino IDE.
Keep each sketch inside its same-named folder.

Left clicks select the CCW target; right clicks select the CW target, viewed
looking at the output shaft. Release, both buttons, and loss of Bluetooth hold
the last selected target. The servo starts at center on boot. A repeated click
in the same direction does not move farther once that target has been reached.
A normal MG90S is a positional servo: it cannot rotate continuously. These
sketches are not intended for a continuous-rotation conversion.

## Wiring

- Sender: left button between GPIO25 and GND, right between GPIO26 and GND.
  Internal pull-ups are enabled; no external pull-ups are required for short wiring.
- Receiver: servo signal (usually orange/yellow) to GPIO18.
- Servo positive (usually red) to a regulated external 5 V supply; a supply rated
  for 2 A is a reasonable choice for one servo, subject to its actual stall current.
- Servo ground (usually brown/black) to supply negative AND receiver ESP32 GND.
- Power the ESP32 normally, for example over USB. Do not power the servo from
  a GPIO or the ESP32 3.3 V pin. Avoid tying external 5 V to the USB power rail.
- If supply dips remain, use shorter power leads and optionally a 470–1000 uF
  capacitor rated at least 10 V across servo power near the servo, with correct
  polarity. A capacitor does not replace an adequate supply.

## Speed and calibration

The original receiver took 15 ms per degree (1.35 s for 90 degrees), and the
sender sampled only every 50 ms. The revised code removes that software ramp,
sends button changes after 5 ms of debounce, and commands the target in one step.
Bluetooth scheduling, the 50 Hz servo pulse frame, load, and motor speed still
limit response; code cannot guarantee an instantaneous or exact movement time.
Do not raise the PWM refresh frequency to speed up an analog MG90S.

Begin unloaded: left should move CCW, right CW. If reversed, set REVERSE = true
in the RECEIVER sketch (or the wired sketch). The 1000–2000 us starting range is
intentionally narrower than the original 500–2400 us range. Actual angles vary.
Adjust MIN_PULSE_US/MAX_PULSE_US only after checking your servo's supported range
and linkage travel; back off if it buzzes or presses against a stop.

For still lower input latency, use WIRED_MG90S/WIRED_MG90S.ino on ONE ESP32.
Connect both buttons to that board's GPIO25/GPIO26 and GND, and the servo signal
to its GPIO18; retain the external servo supply and common ground. This sketch
replaces the Bluetooth pair, keeps the same click behavior, and removes radio
latency. It does not increase the servo's mechanical speed. If properly powered
and unloaded movement is still too slow, use a servo with a faster rated seconds
per 60 degrees at the operating voltage and enough torque for the load.

## Verification

Software simulations exercise debounce, prompt sends, heartbeat behavior,
left/right targets, release, repeated commands, and batched click/release input.
They do not verify the real ESP32 libraries, radio, wiring, physical direction,
or servo speed. Compile/upload with your installed ESP32 board package and
ESP32Servo library, then test both directions unloaded before reconnecting a load.
