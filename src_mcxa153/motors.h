#ifndef MOTORS_H_
#define MOTORS_H_

#include <stdint.h>

void Motors_InitHardware(void);
void SetLeftMotorSpeed(int8_t speed);
void SetRightMotorSpeed(int8_t speed);

#endif /* MOTORS_H_ */
