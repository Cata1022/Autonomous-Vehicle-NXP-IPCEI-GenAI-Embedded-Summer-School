#ifndef PWM_CONTROL_H
#define PWM_CONTROL_H

#include <stdint.h>

void PwmControl_InitTurnSignals(void);

void PwmControl_SetLeftTurnSignalPWM(uint8_t dutyA, uint8_t dutyX);
void PwmControl_SetRightTurnSignalPWM(uint8_t duty1X, uint8_t duty2X);
void PwmControl_SetAllSignalsOffPWM(void);

void PwmControl_SetBuzzerDuty(uint8_t dutyB);
void PwmControl_SetBuzzerFrequencyAndDuty(uint32_t freqHz, uint8_t dutyB);

/* Helper to reload specific submodule configuration during blinker phase swaps */
void PwmControl_ReloadLeftTurnPWM(void);
void PwmControl_ReloadRightTurnPWM(void);

#endif /* PWM_CONTROL_H */
