Autonomous Vehicle Cam

This file represents the main specs and rules of this project that you, as the coding, assistant must follow

The project's goal is to build an autonomous vehicle using a double uC configuration, one NXP FRDM MCXA153, already
programmed and this ESP32-S3-CAM module that is to be programmed. The main job of the esp32 is to process
images captured by the mounted OV3660 camera, process them according to user prompts and send the results (represented
by motor controls + left/right signals) via UART to the MCXA153 according to the instructions listed below and only
to them strictly

ESP32-S3-CAM Module
Camera Configuration & Sensor (OV3660)

Sensor Model: OV3660, mounted on an OEM ESP32-S3-CAM board.

! Analyze but never modify the platformio.ini file or any other board configuration setting. If necessary, announce it clearly
and wait for user input before acting on such matter !

ESP32-to-MCXA UART Communication Specification
1. Hardware Configuration

Baud Rate: 115200 bps
Logic Level: 3.3V (Direct connection TX-to-RX, RX-to-TX. No level shifters required).
Same serial as the debug one (Serial.printf(...))

2. Packet Data Format The MCXA expects a strict Comma-Separated Values (CSV) string terminated by a newline character (\n or 0x0A).

Syntax: <LeftMotor>,<RightMotor>,<LeftTurnSignal>,<RightTurnSignal>\n
LeftMotor / RightMotor: Integer from -100 (full reverse) to 100 (full forward).
LeftTurnSignal / RightTurnSignal: Integer 0 (Off) or 1 (On).
Examples:
Forward max, no signals: 100,100,0,0\n
Pivot left, left signal on: -50,50,1,0\n
Full stop, right signal on: 0,0,0,1\n
3. Parsing Rules & Constraints (Critical for the ESP32 Coder)

No Padding or Spaces: Do not add spaces or align the columns. Send 9,9,0,0\n, NOT  09, 09, 0, 0\n.
Strict Filtering: The MCXA firmware implements an aggressive hardware filter. It physically discards any character that is not a digit (0-9), a minus sign (-), a comma (,), or a newline (\n).
Carriage Returns: The ESP32 Serial.println() sends \r\n. The MCXA will ignore the \r, but to save bandwidth, it is highly recommended to use Serial.print("100,100,0,0\n"); instead.

! Add no extra messages, no debug information, absolutely nothing except messages in the format described above !

4. Timing & Latency (The "No Delay" Rule)

Zero-Latency Hardware: The MCXA has been explicitly programmed with a FIFO Watermark of 0. This means the absolute microsecond the ESP32 sends the \n byte, the MCXA fires a hardware interrupt and immediately parses the packet.
No Artificial Delays Needed: The ESP32 DOES NOT need an artificial delay(50) after transmitting. Previous delays were a workaround for hardware buffering which has now been destroyed.
Serial.flush(): The ESP32 may use Serial.flush() to ensure its own TX buffer is empty before moving on, but do not inject vTaskDelay or delay() specifically for the UART link.
Transmission Rate: The MCXA has a 256-byte circular ring buffer and a main superloop running at roughly a million times a second. The ESP32 should send data at a stable framerate (e.g., 20 Hz to 50 Hz).
Do not put the ESP32 in a tight while(1) loop that sends data at 10,000 Hz, or you will overflow the 256-byte ring buffer on the MCXA.
Continuous streaming (e.g., sending 100,100,0,0\n twenty times a second) is perfectly fine. The MCXA saves its last known state and intentionally skips recalculating the PWM timers if the incoming command matches the current physical state.
5. Boot Synchronization

When the MCXA powers on, its internal state assumes 0,0,0,0.
If the ESP32 boots up and wants the car to remain stopped, it does not strictly need to send 0,0,0,0\n. However, the moment the ESP32 calculates a movement or signal command (e.g., L: 0, R: 0, SL: 1), it must immediately dispatch the full string (0,0,1,0\n).
You may use the following commands (roughly) but adapt knowing all the information above:
Serial.printf("%d,%d,%d,%d\n", left_motor, right_motor, l, r);
Serial.flush();

Design the code as modular as possible, do not put everything in main or loop(), create files when needed for the various
functionlities of the code, proper functions for every task, no magic numbers, etc.
