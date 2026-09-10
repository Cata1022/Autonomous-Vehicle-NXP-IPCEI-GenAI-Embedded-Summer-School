#include "vehicle_control.h"

void sendVehicleCommand(int8_t leftMotor, int8_t rightMotor, uint8_t leftSignal, uint8_t rightSignal) {
    // Constrain the motor values to -100 to 100
    if (leftMotor < -100) leftMotor = -100;
    if (leftMotor > 100) leftMotor = 100;
    if (rightMotor < -100) rightMotor = -100;
    if (rightMotor > 100) rightMotor = 100;

    // Constrain the signal values to 0 or 1
    leftSignal = (leftSignal > 0) ? 1 : 0;
    rightSignal = (rightSignal > 0) ? 1 : 0;

    // Send the command in the exact format required by the MCXA153
    Serial.printf("%d,%d,%d,%d\n", leftMotor, rightMotor, leftSignal, rightSignal);
    
    // Ensure the hardware buffer empties immediately
    Serial.flush();
}
