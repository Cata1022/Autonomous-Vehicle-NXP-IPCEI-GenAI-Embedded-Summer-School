#ifndef VEHICLE_CONTROL_H
#define VEHICLE_CONTROL_H

#include <Arduino.h>

/**
 * @brief Sends standard command to MCXA153 via Serial
 * 
 * @param leftMotor -100 to 100
 * @param rightMotor -100 to 100
 * @param leftSignal 0 or 1
 * @param rightSignal 0 or 1
 */
void sendVehicleCommand(int8_t leftMotor, int8_t rightMotor, uint8_t leftSignal, uint8_t rightSignal);

#endif // VEHICLE_CONTROL_H
