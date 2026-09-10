# Autonomous Car

## 1. Introduction

This project consists of a self-driving rover that pairs an **ESP32-S3 CAM** (vision and AI) with the **NXP FRDM-MCXA153** (bare-metal motor control and signaling). The rover strictly follows the lines of the road using computer vision processed on the ESP32. Initial STOP/GO commands are issued using Bluetooth Low Energy (BLE) via the ESP32.

The purpose is to demonstrate a distributed embedded processing architecture for edge AI robotics. It is useful for students because it covers PWM motor control, UART inter-MCU communication, and PID control loops — all within a tangible, demonstrable physical system.

## 2. General Description

### 2.1 Project Summary

- **Project Name:** Autonomous Car
- **Short Summary:** A self-driving rover pairing an ESP32-S3 CAM for vision/AI with the NXP FRDM-MCXA153 for differential motor control.
- **Main Objective:** Navigate autonomously in a controlled environment by tracking lane lines strictly using the camera.
- **Intended Users:** Third-year Computer Science students.
- **Operating Environment:** Controlled indoor environment with clear track markings.
- **Selected Scope:** Recommended (core motor logic on NXP + UART vision/steering commands from ESP32).
- **Main Behavior:** ESP32-S3 processes camera feeds for lane keeping, receives BLE START/STOP commands, and sends steering/speed/signal commands via UART to the NXP FRDM-MCXA153. The NXP drives the motors via a DRV8833 driver and controls the turn signals.
- **Inputs:** Camera images and BLE commands (via ESP32).
- **Outputs:** PWM signals for motor speed/direction (DRV8833) and turn signals (PWM/GPIO).
- **Out of Scope:** GPS navigation; SLAM; Public road driving.

### 2.2 Feature Tiers

| Tier | Description | Main Features | Extra Components | Main Risks | Suitability |
|---|---|---|---|---|---|
| Core | Basic motor chassis using NXP FRDM-MCXA153 | DC motor control via PWM and DRV8833; Turn signal management | DRV8833; DC Motors; 18650 cells | Battery voltage limits | Suitable (Beginner) |
| Recommended | Adds ESP32-S3 CAM for computer vision | Camera-based lane tracking; UART protocol between MCUs; BLE STOP/GO | ESP32-S3 CAM | UART sync | Suitable (Intermediate) |
| Advanced | Adds TinyML traffic sign recognition on ESP32 | Edge AI / TinyML inference; Dual-core ESP32 processing | None extra | Task starvation | Risky but Optional |

### 2.3 Scenarios

| ID | Scenario | Description |
|---|---|---|
| SC-001 | Startup | NXP initializes PWM drivers for DRV8833 and establishes interrupt-driven UART buffer. ESP32 boots, calibrates camera, and waits for BLE GO command. |
| SC-002 | Normal Operation | ESP32 tracks lane and sends continuous UART commands. NXP translates to left/right PWM for DRV8833 differential drive and manages turn signal blinking/buzzing. |
| SC-003 | BLE STOP | User sends a STOP command via Bluetooth to the ESP32. ESP32 commands the NXP to halt all motors. |
| SC-004 | Traffic Sign Override | (Advanced) ESP32 detects traffic signs via a custom TinyML CNN. Once a turn direction is identified with high confidence, it locks the direction. The vehicle detects the physical intersection via camera pixel volume, automatically executes a hardcoded pivot maneuver, and resumes PID tracking. |
| SC-005 | Communication Loss | NXP detects UART timeout/corruption. System ignores malformed packets. |
| SC-006 | Debugging | Student connects USB serial console to NXP. Reads parsed UART packets and motor execution states in real time. |

### 2.4 User Stories

| ID | User Story |
|---|---|
| US-001 | As a student, I need the NXP FRDM-MCXA153 to handle bare-metal motor control and complex turn signal sequencing without blocking the main logic loop. |
| US-002 | As a developer, I need to send robust, low-latency differential drive commands over UART so that the ESP32 vision logic can steer the chassis instantly. |
| US-003 | As an instructor, I need to review test evidence (videos, serial logs) so that I can verify the rover meets acceptance criteria. |
| US-004 | As a user, I need a BLE Start/Stop interface via the ESP32 so that I can safely command the rover wirelessly. |

### 2.5 Use Case Diagram

```mermaid
flowchart LR
    Student[Student]
    Instructor[Instructor / Reviewer]
    Vehicle[Vehicle / Rover]

    UC1((Initialize System))
    UC2((Connect BLE))
    UC3((Start Autonomous Mode))
    UC4((Follow Lane))
    UC5((Signal Turns))
    UC6((Recognize Traffic Sign))
    UC7((View Debug Logs))
    UC8((Review Test Evidence))

    Student --> UC1
    Student --> UC2
    Student --> UC3
    Student --> UC7
    UC3 --> UC4
    UC4 --> UC5
    UC4 --> UC6
    Vehicle --> UC4
    Vehicle --> UC5
    Instructor --> UC8
```

### 2.6 Hardware and Software Block Diagram

```mermaid
flowchart TD
    subgraph Power
        Battery["2x 18650 Cells (8.4V)"]
        Regulator[5V / 3.3V Step-Down]
        Battery --> Regulator
        Battery --> |Raw 8.4V| DRV8833
    end

    subgraph Perception
        Camera[OV3660 Camera]
    end

    subgraph Controllers
        ESP32[ESP32-S3 CAM]
        NXP[NXP FRDM-MCXA153]
    end

    subgraph Actuation
        DRV8833[DRV8833 Motor Driver]
        Motors[Left / Right DC Motors]
        TurnSignals[LEDs & Buzzer]
    end

    subgraph UI
        BLE[BLE Interface]
    end

    Regulator --> |3.3V| NXP
    Regulator --> |3.3V| ESP32
    Camera --> ESP32
    BLE --> ESP32
    ESP32 <--> |UART 3.3V| NXP
    NXP --> |PWM| DRV8833
    DRV8833 --> Motors
    NXP --> |PWM/GPIO| TurnSignals
```

**Power Flow:** 2x 18650 cells (8.4V) feed the DRV8833 motor power input directly and are stepped down to 3.3V/5V for the MCUs and camera.

**Data/Control Flow:** Camera / BLE → ESP32 (vision processing and command parsing) → UART → NXP (motor mapping) → DRV8833 → Motors.

## 3. Hardware Design

### 3.1 Bill of Materials

| # | Component | Qty | Tier | Purpose | Likely Interface | Voltage / Power Notes | Risks / Checks |
|---|---:|---:|---|---|---|---|---|
| 1 | NXP FRDM-MCXA153 | 1 | Core | Main execution controller | MCU | 3.3V logic | Mandatory board |
| 2 | DRV8833 Motor Driver | 1 | Core | Drive DC motors (differential) | PWM | Separate motor power rail; shared GND | Heat dissipation |
| 3 | DC Motors | 2 | Core | Propulsion (left/right) | Via DRV8833 | High current | Inductive spikes |
| 4 | Turn Signal LEDs | 4+ | Core | Turning visual feedback | GPIO/PWM | 3.3V | Current limiting resistor |
| 5 | Buzzer | 1 | Core | Turning audio feedback | PWM | 3.3V | Current limiting resistor |
| 6 | ESP32-S3 CAM (OV3660) | 1 | Recommended | Vision, BLE, and AI | UART to NXP | 3.3V logic; power hungry | Separate toolchain |
| 7 | 18650 Battery Cells | 2 | Core | Main power source | Battery holder | 8.4V nominal | Over-discharge protection |
| 8 | Step-Down Regulator LM2596 | 1 | Core | Logic power supply | VCC/GND | 8.4V → 5V/3.3V | Check max output current |

### 3.2 Hardware Block Diagram Description

The **NXP FRDM-MCXA153** is the central execution controller. It receives regulated 3.3V power from a step-down converter fed by two 18650 cells. The battery also directly supplies raw 8.4V voltage to the **DRV8833 Motor Driver**. The NXP drives the DRV8833 using 4 PWM signals (IN1/IN2 per motor) utilizing a Slow-Decay PWM strategy for both speed and direction control.

For the recommended tier, the **ESP32-S3 CAM** connects via 3.3V UART (TX/RX cross-connected, shared ground). The ESP32 handles all vision processing, receives BLE start/stop commands, and sends parsed steering, speed, and turn signal commands to the NXP.

**Voltage Compatibility:** Both ESP32 and NXP operate at 3.3V logic — no level shifting needed for UART. The DRV8833 logic inputs accept 3.3V signals directly from the NXP.

### 3.3 Pin Allocation Draft

| Component | Tier | Signal | Required MCU Capability | Suggested Pin / Capability | Voltage Level | Direction | Interface | Verification Needed |
|---|---|---|---|---|---|---|---|---|
| DRV8833 L-IN1 | Core | Left Motor PWM A | FLEXPWM0_SM1_A | PORT3_8 (Pin 42) | 3.3V | Output | PWM | Confirmed in Code |
| DRV8833 L-IN2 | Core | Left Motor PWM B | FLEXPWM0_SM1_B | PORT3_9 (Pin 41) | 3.3V | Output | PWM | Confirmed in Code |
| DRV8833 R-IN1 | Core | Right Motor PWM A| FLEXPWM0_SM2_A | PORT3_10 (Pin 40)| 3.3V | Output | PWM | Confirmed in Code |
| DRV8833 R-IN2 | Core | Right Motor PWM B| FLEXPWM0_SM2_B | PORT3_11 (Pin 39)| 3.3V | Output | PWM | Confirmed in Code |
| Buzzer | Core | Turn Audio Alert | FLEXPWM0_SM0_B | PORT3_7 (Pin 43) | 3.3V | Output | PWM | Confirmed in Code |
| Turn LED L | Core | Left Blink Visual| FLEXPWM0_SM0_A/X| Various GPIO Pins| 3.3V | Output | PWM/GPIO | Confirmed in Code |
| Turn LED R | Core | Right Blink Vis. | FLEXPWM0_SM1_X/2_X| Various GPIO Pins| 3.3V | Output | PWM/GPIO | Confirmed in Code |
| ESP32 UART TX | Recommended | Data to NXP | LPUART1 RX | PORT1_8 | 3.3V | Input | UART | Confirmed in Code |
| ESP32 UART RX | Recommended | Data from NXP| LPUART1 TX | PORT1_9 | 3.3V | Output | UART | Confirmed in Code |

### 3.4 Electrical Schematics

![image](hardware/schematic.png)

## 4. Software Design

### 4.1 Development Environment

- **NXP FRDM-MCXA153:** VS Code with NXP extension (CMake based).
- **ESP32-S3:** PlatformIO.
- **SDK:** MCUXpresso SDK (bare-metal drivers for UART, PWM, GPIO); ESP32 Camera Driver.
- **Language:** C (NXP) and C++ (ESP32).
- **Debugging:** On-board MCU-Link debugger; Serial console via USB.

### 4.2 Firmware Architecture

- **Architecture Style:** Bare-metal superloop with interrupt-driven UART.
- **Reason:** Minimizes complexity while ensuring UART commands are buffered without blocking the critical obstacle-detection loop.

**Main Modules:**

| Module | Responsibility |
|---|---|
| Motor Driver | Configure FLEXPWM channels; set left/right motor speed using slow-decay PWM via DRV8833 |
| Turn Signals | Manage blinking timing, hardware PWM frequency modulation, and buzzer sequencing |
| UART Command Parser | Receive ESP32 packets via 256-byte ISR ring buffer; parse safely ignoring garbage |
| Command Execution | Execute parsed motor and turn signal commands on the physical hardware |
| Debug/Logging | Print sensor values, motor duties, and system state to USB serial console |
| Vision Processing (ESP32) | Capture frames (QVGA, Grayscale), crop ROIs, perform thresholding, calculate lane centroids and tripwire pixel volume |
| PID Controller (ESP32) | Calculate differential motor speeds, apply deadzone compensation, and manage intersection turn logic/timing |
| ML Inference (ESP32) | Feed cropped ROIs into the custom Edge Impulse CNN classifier, update confidence thresholds, and predict turn directions |
| Web Server (ESP32) | Host a diagnostic HTTP endpoint to live-stream the exact 96x96 ROI being fed into the neural network |
| BLE Interface (ESP32) | Handle wireless start/stop overriding commands via Bluetooth Low Energy |

**Startup Sequence:** Initialize system clocks → Configure GPIO for Turn Signals → Configure FLEXPWM for DRV8833 and Buzzer → Configure LPUART1 (RX FIFO Watermark=0) → Start SysTick timer → Enter main loop.

**Main Loop Flow:** Drain UART ring buffer → Parse valid packets (`<L>,<R>,<SL>,<SR>\n`) → Process Blinker State Machine → Process Buzzer State Machine → Repeat.

**Interrupt Handling:** LPUART1 ISR triggers on every single byte (Watermark 0) and places it into a 256-byte circular buffer to prevent hardware overruns.

**Error Handling / Safe-State:** The UART parser rigidly ignores all characters except digits, commas, and minus signs, ensuring garbage/noise does not corrupt motor states. Default power-on state is stopped (0,0,0,0).

**Configuration Constants:** `RING_BUFFER_SIZE` (256).

### 4.3 Main Algorithms and Data Structures

- **Blinker State Machine:** States: `kSTATE_OFF`, `kSTATE_TURN_LEFT`, `kSTATE_TURN_RIGHT`. Transitions driven by UART commands and `SysTick` 100ms intervals.
- **Slow-Decay PWM Strategy:** To operate the DRV8833 efficiently, the motor driver maps speeds (`-100` to `100`) to IN1 and IN2 duties where one channel is driven HIGH and the other is modulated, effectively creating active braking when coasting.
- **UART Packet Parsing:** Simple framed protocol with start byte, payload (speed, steering), checksum, end byte. Circular buffer in ISR.
- **Regions of Interest (ROI) (ESP32 side):** To optimize processing time, the QVGA (320x240) camera feed is segmented. A bottom slice (320x40) is used as a full-width tripwire for intersections and line tracking, while a specific fixed patch (96x96) is fed to the ML model.
- **PID Lane Keeping (ESP32 side):** The bottom ROI is converted to grayscale, thresholded, and scanned for the tracking line to compute the line's centroid. The PID controller continuously adjusts the differential motor speeds based on the centroid error from the setpoint. It incorporates memory for "lost line" states (maintaining previous speeds or creeping forward) and features a kinematic deadzone elimination step to overcome static friction on the physical track.
- **Intersection Detection & Execution (ESP32 side):** By analyzing the volume of black pixels in the tripwire ROI, the vehicle detects intersection blocks. Upon detection, it latches a parking brake. If an ML turn direction was locked in prior to the intersection, it executes a hardcoded turn sequence (timed forward/pivot maneuvers). If no turn is locked in, the vehicle halts and awaits a manual BLE command.
- **TinyML Inference (Advanced, ESP32 side):** A Custom Keras CNN (3-layer Grayscale) replaced earlier, heavier models. Initial attempts with a MobileNetV2 (float32) ran too slowly (over 1s latency), and an int8 quantized version crashed the ESP-NN hardware accelerator because the tensor arena exceeded internal SRAM limits, forcing it into PSRAM (which causes vector instruction panics on the ESP32). The final ultra-lean model fits entirely in internal SRAM, achieving high inference speeds (<10ms). However, the model still occasionally struggles with identifying signs in real-world scenarios due to harsh lighting variations and severe hardware/memory limitations restricting model depth.

### 4.4 Functional Requirements Summary

| ID | Tier | Requirement | Priority | Verification | Acceptance Criterion |
|---|---|---|---|---|---|
| FR-001 | Core | The system shall use the NXP FRDM-MCXA153 as the main controller platform. | Must | Inspection | NXP board is present and controls DRV8833 |
| FR-002 | Core | The system shall control two DC motors via DRV8833 using differential steering (4 PWM). | Must | Demonstration | Motors spin correctly for forward, left, right |
| FR-003 | Core | The system shall provide visual and audio turn signal feedback. | Must | Test | LEDs and buzzer activate correctly |
| FR-004 | Recommended | The system shall receive BLE Start/Stop commands via the ESP32. | Must | Demonstration | BLE command toggles motor enable/disable |
| FR-005 | Recommended | The system shall receive robust motor and signal targets via UART from the ESP32. | Must | Test | UART command alters motor speeds instantly |
| FR-006 | Advanced | When a STOP override command is received via UART, the system shall halt motors for a configured duration. | Should | Test | ESP32 STOP command overrides normal drive |

### 4.5 Non-Functional Requirements Summary

| ID | Tier | Category | Requirement | Metric / Threshold | Verification |
|---|---|---|---|---|---|
| NFR-001 | Core | Safety / HW | All components shall be electrically compatible with 3.3V logic or use interface circuitry. | Max 3.3V on GPIO | Voltage measurement |
| NFR-002 | Core | Timing | UART interrupt must be serviced instantly without hardware overruns. | 0 `[OR]` errors | Console logs |
| NFR-003 | Core | Power | DRV8833 motor power rail shall be isolated from logic rail (shared ground only). | 0 logic brownouts | Oscilloscope |
| NFR-004 | Core | Safety | System shall default to stopped state on power-on. | PWM=0 on boot | Demonstration |
| NFR-005 | Core | Reliability | UART parser shall strictly filter invalid ASCII characters. | 100% valid parsed ints| Test |
| NFR-006 | Core | Privacy | All vision processing shall be local on ESP32. | No network traffic | Inspection |
| NFR-007 | Core | Memory | NXP firmware shall fit within MCU Flash and SRAM limits. | Within datasheet limits | Build output check |
| NFR-008 | Core | Usability | Serial debug console shall print human-readable sensor and motor state. | Readable output | Demonstration |

### 4.6 Test Plan Summary

| Test ID | Requirement | Tier | Test Type | Expected Result | Evidence |
|---|---|---|---|---|---|
| TC-001 | FR-001 | Core | Inspection | NXP FRDM-MCXA153 is the primary board controlling DRV8833 | Photo |
| TC-002 | FR-002 | Core | Demonstration | Motors spin forward, reverse, left, right correctly | Video |
| TC-003 | FR-003 | Core | Test | Turn signals blink and buzz at correct intervals | Video |
| TC-004 | FR-004 | Core | Demonstration | BLE command toggles rover on/off | Video |
| TC-005 | FR-005 | Recommended | Test | UART command from ESP32 changes motor speeds instantly | Serial log |
| TC-006 | FR-006 | Advanced | Test | STOP override halts motors for configured duration | Video; serial log |

### 4.7 Traceability Summary

| User Story | Requirement(s) | Test Case(s) | Evidence | Gap |
|---|---|---|---|---|
| US-001 | FR-001, FR-003 | TC-001, TC-003 | Photo, Video | None |
| US-002 | FR-002, FR-005 | TC-002, TC-005 | Video, Serial log | None |
| US-003 | All FRs | TC-001 to TC-005 | All evidence types | None |
| US-004 | FR-004 | TC-004 | Video | None |

## 5. Risk Matrix

| ID | Category | Tier | Severity | Probability | Impact | Mitigation
|---|---|---|---|---|---|---|
| R-001 | Hardware | Core | High | High | UART RX FIFO holding packets hostage | Set FIFO Watermark to 0
| R-002 | Safety | Core | Medium | Medium | Rover runs off limits | BLE kill switch; speed limits; test on blocks first
| R-003 | Technical | Recommended | Medium | Low | UART message corruption causing erratic movement | ASCII character filtering; strict packet parsing
| R-004 | Technical | Recommended | Medium | Medium | Camera lighting/framing causing poor lane extraction | Controlled lighting; tunable thresholds
| R-005 | Advanced | Advanced | Low | High | TinyML model training takes too long | Limit to less classes
| R-006 | Memory | Advanced | Medium | Medium | ESP32 SRAM exhaustion if PSRAM misconfigured | Verify PSRAM init in ESP-IDF config

## 6. Review Checklist

- [X] FRDM-MCXA153 confirmed as mandatory board
- [X] FRDM-MCXA153 pinout checked against pin allocation draft
- [X] Voltage compatibility checked (DRV8833 logic inputs)
- [X] Current limits checked (GPIO drive strength for LEDs/Buzzer)
- [X] Power budget checked (Battery → Regulator → MCUs; Battery → DRV8833 → Motors)
- [X] External modules checked (DRV8833, ESP32-S3 CAM)
- [X] Advanced components approved or removed (TinyML on ESP32)
- [X] Sensor/actuator interfaces confirmed
- [X] Firmware architecture approved (bare-metal superloop + UART ISR)
- [X] Timing and memory constraints reviewed
- [X] Test plan reviewed
- [X] Traceability reviewed
- [X] Safety/privacy/security risks reviewed
- [X] AI assumptions accepted or rejected
- [X] Implementation allowed to start

## 7. Obtained Results

1. All the hardware aspects are implemented
2. The MCXA153 board is fully programmed, it correctly controls:

	- the DC motors

	- the headlights (LEDs)

	- the turn signals (buzzer + LED color)

	- the UART communcication
3. The ESP32-S3-CAM successfully:
	- communicates motor controls and turn signal commands with the MCXA153
	- tracks the road using the PID algorithm
    - detects intersections	 
    - recognises traffic signs using a TinyML classifier model
5. Attached below is a picture of the vehicle on the custom built track:

![image](hardware/vehicle.jpg)

6. Video of the fully functional car:

[![Demo](hardware/video_thumbnail.jpg)](https://drive.google.com/file/d/1B0jAqS5aXTMIYaHL0EXGuWcjN_YQXrFH/view?usp=sharing)

## 8. Conclusions

The distributed embedded architecture successfully showcased how a robust, bare-metal microcontroller (NXP MCXA153) can manage hard real-time tasks like motor actuation, while a secondary processor (ESP32-S3 CAM) handles intensive computer vision and TinyML workloads. 

**What worked well:**
- The UART communication protocol proved extremely resilient. Hardware overruns were eliminated by using an interrupt-driven circular buffer on the MCXA153.
- The dual-core FreeRTOS architecture on the ESP32 effectively decoupled the ML inference from the PID line-tracking, ensuring the motors were always updated with real-time centroid data even while the neural network was processing.
- The use of precise Regions of Interest (ROIs) dramatically reduced pixel processing overhead, allowing the PID controller to operate at a stable, unthrottled loop rate.

**What was difficult & What would be improved:**
- The physical track introduced static friction challenges, which required implementing a kinematic deadzone eliminator in the PID algorithm to ensure the car could reliably start moving.
- **Machine Learning Hardware Deficit:** The ESP32-S3's internal SRAM (512KB) and ESP-NN vector hardware accelerator proved to be a severe bottleneck. The hardware accelerator crashes if the tensor arena is placed in the external PSRAM. Because the camera buffers and OS stacks consume over 150KB of internal SRAM, the ML model's arena was strictly limited to under 60KB. 
- Early attempts to use a MobileNetV2 (float32) were impossibly slow (>1 second per frame), and its quantized int8 version caused hardware panics due to memory allocation constraints. 
- The final implementation successfully deployed an ultra-lean 3-layer Custom Keras CNN (grayscale, utilizing strided convolutions) that fit entirely within internal SRAM, achieving lightning-fast inference speeds. However, this heavily restricted model depth resulted in diminished accuracy. The model still occasionally struggles with identifying signs in real-world scenarios due to harsh lighting variations. Future versions would benefit from a more powerful inference edge board (like a Coral Edge TPU or Raspberry Pi) or a dedicated SRAM-heavy ML microcontroller to allow deeper networks without sacrificing hardware acceleration.

**How Gen AI helped:**
Generative AI acted as an expert pair programmer throughout the development process. It successfully diagnosed the obscure memory allocation crash involving the ESP-NN hardware vector instructions and PSRAM. It also designed the architectural refactor, optimized the PID deadzone compensation, and architected the custom Keras CNN architecture to bypass the memory limits while retaining hardware acceleration.
