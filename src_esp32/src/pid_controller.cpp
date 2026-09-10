#include "pid_controller.h"
#include <Arduino.h>

#define SETPOINT 235
#define BASE_SPEED 30
#define MAX_SPEED 80
#define MIN_SPEED -40
#define MIN_ACTIVE_SPEED 29 // Minimum PWM speed to break static friction
#define K_TUNING 0.85

volatile TurnDirection next_turn_direction = TURN_NONE;
bool emergency_stop = false;

int TURN_SPEED = 30;

// Intersection Turn Constants (time in ms)
unsigned long FORWARD_DURATION = 2165;

unsigned long LEFT_PHASE1_DURATION = 2550;
unsigned long LEFT_PHASE2_DURATION = 540;

unsigned long RIGHT_PHASE1_DURATION = 1615;
unsigned long RIGHT_PHASE2_DURATION = 475 * K_TUNING;

// State tracking for intersection sequences
bool is_turning = false;
unsigned long turn_start_time = 0;
bool intersection_parked = false; // New: Software Parking Brake

// PID Constants (Ultra-Smooth Offline Tuning)
const float Kp = 0.05;  // Reduced slightly further to limit max steering strength
const float Kd = 0.01;   // Increased dampening to prevent oscillation
const float Ki = 0.0;

int previous_error = 0;
float integral = 0;

int last_left_speed = 0;
int last_right_speed = 0;
bool persistent_line_lost = false;

void initPID() {
    last_left_speed = BASE_SPEED;
    last_right_speed = BASE_SPEED;
    previous_error = 0;
    integral = 0;
    persistent_line_lost = false;
}

void updatePID(int current_x, bool line_lost, bool intersection_stop) {
    // --- BOOT DIAGNOSTIC SEQUENCE ---
    static bool boot_sequence_active = true;
    static unsigned long boot_start_time = 0;
    unsigned long current_time = millis();
    
    if (boot_sequence_active) {
        if (boot_start_time == 0) {
            boot_start_time = current_time; // Mark the exact millisecond the first frame arrives
        }
        
        // For exactly 1 second after booting, halt the motors and flash the left turn signal!
        if (current_time - boot_start_time < 1000) {
            Serial.printf("0,0,1,0\n");
            Serial.flush();
            initPID(); // Keep the PID memory fresh so error doesn't build up while parked
            return;    // Exit immediately so no other logic runs
        } else {
            boot_sequence_active = false; // Boot complete, never run this block again unless reset!
        }
    }
    // --------------------------------
    
    if (emergency_stop) {
        Serial.printf("0,0,0,0\n");
        Serial.flush();
        initPID(); // Keep PID fresh while parked
        return; 
    }

    if (line_lost) {
        persistent_line_lost = true;
    }

    // 1. Latch the parking brake the millisecond an intersection is seen!
    if (intersection_stop && !is_turning) {
        intersection_parked = true;
    }

    // 2. Check if the parking brake is currently engaged
    if (intersection_parked && !is_turning) {
        if (next_turn_direction == TURN_NONE) {
            // Wait for AI certitude > threshold OR a manual BLE command 
            Serial.printf("0,0,0,0\n");
            Serial.flush();
            initPID(); // Wipe derivative/integral error while parked
            return; 
        } else {
            // Command queued! Release the parking brake and start the turn!
            intersection_parked = false;
            is_turning = true;
            turn_start_time = millis();
        }
    }

    // Execute active turn sequence
    if (is_turning) {
        unsigned long elapsed = current_time - turn_start_time;
        int left_speed = BASE_SPEED - 5;
        int right_speed = BASE_SPEED;
        int left_signal = 0;
        int right_signal = 0;

        if (next_turn_direction == TURN_FORWARD) {
            if (elapsed < FORWARD_DURATION) {
                // Forward (no signals)
            } else {
                is_turning = false;
                next_turn_direction = TURN_NONE;
            }
        }
        else if (next_turn_direction == TURN_LEFT) {
            left_signal = 1;
            if (elapsed < LEFT_PHASE1_DURATION) {
                // Forward
            } else if (elapsed < LEFT_PHASE1_DURATION + LEFT_PHASE2_DURATION) {
                left_speed = -TURN_SPEED;
                right_speed = TURN_SPEED;
            } else {
                is_turning = false;
                next_turn_direction = TURN_NONE;
                left_signal = 0;
            }
        }
        else if (next_turn_direction == TURN_RIGHT) {
            right_signal = 1;
            if (elapsed < RIGHT_PHASE1_DURATION) {
                // Forward
            } else if (elapsed < RIGHT_PHASE1_DURATION + RIGHT_PHASE2_DURATION) {
                left_speed = TURN_SPEED;
                right_speed = -TURN_SPEED;
            } else {
                is_turning = false;
                next_turn_direction = TURN_NONE;
                right_signal = 0;
            }
        }
        
        if (is_turning) {
            // Send the hardcoded turn command and block PID
            Serial.printf("%d,%d,%d,%d\n", left_speed, right_speed, left_signal, right_signal);
            Serial.flush();
            initPID(); // Keep PID fresh so it doesn't build error while turning
            return;
        }
    }

    int left_speed = BASE_SPEED;
    int right_speed = BASE_SPEED;

    if (line_lost) {
        // If the car was stopped (0,0) due to booting up or just finishing an intersection turn,
        // we must creep forward at BASE_SPEED to find the line. Otherwise it will never start!
        if (last_left_speed == 0 && last_right_speed == 0) {
            left_speed = BASE_SPEED;
            right_speed = BASE_SPEED;
        } else {
            // Otherwise, lock the motors to the exact speeds from the last valid frame
            left_speed = last_left_speed;
            right_speed = last_right_speed;
        }
    } else {
        static bool first_run = true;
        int error = current_x - SETPOINT;
        
        // Prevent Derivative Kick on the very first frame
        if (first_run) {
            previous_error = error;
            first_run = false;
        }
        
        integral += error;
        float derivative = error - previous_error;
        
        float correction = (Kp * error) + (Ki * integral) + (Kd * derivative);
        previous_error = error;

        // Steering: if error is positive (line is to the right), turn right -> Left motor speeds up, Right slows down.
        left_speed = BASE_SPEED + (int)correction;
        right_speed = BASE_SPEED - (int)correction;
    }

    // -------------------------------------------------------------
    // KINEMATIC DEADZONE ELIMINATION
    // Preserves the PID turning radius by applying the deadzone 
    // adjustment equally to both wheels!
    // -------------------------------------------------------------
    int midpoint = MIN_ACTIVE_SPEED / 2;
    
    // Check Left Wheel Deadzone
    if (left_speed > 0 && left_speed < MIN_ACTIVE_SPEED) {
        int adjustment = (left_speed <= midpoint) ? -left_speed : (MIN_ACTIVE_SPEED - left_speed);
        left_speed += adjustment;
        right_speed += adjustment; // Preserve differential
    }
    else if (left_speed < 0 && left_speed > -MIN_ACTIVE_SPEED) {
        int adjustment = (left_speed >= -midpoint) ? -left_speed : (-MIN_ACTIVE_SPEED - left_speed);
        left_speed += adjustment;
        right_speed += adjustment; 
    }

    // Check Right Wheel Deadzone
    if (right_speed > 0 && right_speed < MIN_ACTIVE_SPEED) {
        int adjustment = (right_speed <= midpoint) ? -right_speed : (MIN_ACTIVE_SPEED - right_speed);
        right_speed += adjustment;
        left_speed += adjustment; // Preserve differential
    }
    else if (right_speed < 0 && right_speed > -MIN_ACTIVE_SPEED) {
        int adjustment = (right_speed >= -midpoint) ? -right_speed : (-MIN_ACTIVE_SPEED - right_speed);
        right_speed += adjustment;
        left_speed += adjustment; 
    }

    // -------------------------------------------------------------
    // SAFETY CLAMPING (Applied last to ensure physical hardware limits)
    // -------------------------------------------------------------
    if (left_speed > MAX_SPEED) left_speed = MAX_SPEED;
    if (left_speed < MIN_SPEED) left_speed = MIN_SPEED;
    
    if (right_speed > MAX_SPEED) right_speed = MAX_SPEED;
    if (right_speed < MIN_SPEED) right_speed = MIN_SPEED;

    // Save for memory state
    last_left_speed = left_speed;
    last_right_speed = right_speed;

    // Send UART command to MCXA153
    // Format: <LeftMotor>,<RightMotor>,<LeftTurnSignal>,<RightTurnSignal>\n
    int left_sig = persistent_line_lost ? 1 : 0;
    // Serial.printf("%d,%d,%d,0\n", left_speed, right_speed, left_sig);
	Serial.printf("%d,%d,0,0\n", left_speed, right_speed);
    Serial.flush();
}
