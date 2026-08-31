#include "motors.h"
#include "fsl_pwm.h"
#include "peripherals.h"
#include <stdlib.h>

void Motors_InitHardware(void)
{
    SetLeftMotorSpeed(0);
    SetRightMotorSpeed(0);
}

void SetLeftMotorSpeed(int8_t speed)
{
    uint8_t dutyA, dutyB;
    if (speed >= 0)
    {
        dutyB = 100U;
        dutyA = 100U - speed;
    }
    else
    {
        dutyA = 100U;
        dutyB = 100U + speed;
    }
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM1, FLEXPWM0_SM1_A, kPWM_SignedEdgeAligned, dutyA);
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM1, FLEXPWM0_SM1_B, kPWM_SignedEdgeAligned, dutyB);
    PWM_SetPwmLdok(FLEXPWM0_PERIPHERAL, 1U << FLEXPWM0_SM1, true);
}

void SetRightMotorSpeed(int8_t speed)
{
    uint8_t dutyA, dutyB;
    if (speed >= 0)
    {
        dutyB = 100U;
        dutyA = 100U - speed;
    }
    else
    {
        dutyA = 100U;
        dutyB = 100U + speed;
    }
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM2, FLEXPWM0_SM2_A, kPWM_SignedEdgeAligned, dutyA);
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM2, FLEXPWM0_SM2_B, kPWM_SignedEdgeAligned, dutyB);
    PWM_SetPwmLdok(FLEXPWM0_PERIPHERAL, 1U << FLEXPWM0_SM2, true);
}
