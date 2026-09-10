#include "pwm_control.h"
#include "fsl_pwm.h"
#include "peripherals.h"
#include <stdbool.h>

void PwmControl_InitTurnSignals(void)
{
    /* Fix the fault map explicitly just in case*/
    PWM_SetupFaultDisableMap(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, FLEXPWM0_SM0_A, kPWM_faultchannel_0, 0U);
    PWM_SetupFaultDisableMap(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, kPWM_PwmX, kPWM_faultchannel_0, 0U);
    
    PWM_SetupFaultDisableMap(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM1, kPWM_PwmX, kPWM_faultchannel_0, 0U);
    PWM_SetupFaultDisableMap(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM2, kPWM_PwmX, kPWM_faultchannel_0, 0U);

    pwm_signal_param_t sm0Config[3] = {
        { .pwmChannel = kPWM_PwmA, .dutyCyclePercent = 0U, .level = kPWM_HighTrue, .faultState = kPWM_PwmFaultState0, .pwmchannelenable = true, .deadtimeValue = 0U },
        { .pwmChannel = kPWM_PwmB, .dutyCyclePercent = 0U, .level = kPWM_HighTrue, .faultState = kPWM_PwmFaultState0, .pwmchannelenable = true, .deadtimeValue = 0U },
        { .pwmChannel = kPWM_PwmX, .dutyCyclePercent = 0U, .level = kPWM_HighTrue, .faultState = kPWM_PwmFaultState0, .pwmchannelenable = true, .deadtimeValue = 0U }
    };
    PWM_SetupPwm(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, sm0Config, 3U, kPWM_SignedEdgeAligned, FLEXPWM0_SM0_COUNTER_FREQ_HZ, FLEXPWM0_SM0_SM_CLK_SOURCE_FREQ_HZ);

    pwm_signal_param_t sm1Config[3] = {
        { .pwmChannel = kPWM_PwmA, .dutyCyclePercent = 100U, .level = kPWM_HighTrue, .faultState = kPWM_PwmFaultState0, .pwmchannelenable = true, .deadtimeValue = 0U },
        { .pwmChannel = kPWM_PwmB, .dutyCyclePercent = 100U, .level = kPWM_HighTrue, .faultState = kPWM_PwmFaultState0, .pwmchannelenable = true, .deadtimeValue = 0U },
        { .pwmChannel = kPWM_PwmX, .dutyCyclePercent = 0U, .level = kPWM_HighTrue, .faultState = kPWM_PwmFaultState0, .pwmchannelenable = true, .deadtimeValue = 0U }
    };
    PWM_SetupPwm(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM1, sm1Config, 3U, kPWM_SignedEdgeAligned, FLEXPWM0_SM1_COUNTER_FREQ_HZ, FLEXPWM0_SM1_SM_CLK_SOURCE_FREQ_HZ);

    pwm_signal_param_t sm2Config[3] = {
        { .pwmChannel = kPWM_PwmA, .dutyCyclePercent = 100U, .level = kPWM_HighTrue, .faultState = kPWM_PwmFaultState0, .pwmchannelenable = true, .deadtimeValue = 0U },
        { .pwmChannel = kPWM_PwmB, .dutyCyclePercent = 100U, .level = kPWM_HighTrue, .faultState = kPWM_PwmFaultState0, .pwmchannelenable = true, .deadtimeValue = 0U },
        { .pwmChannel = kPWM_PwmX, .dutyCyclePercent = 100U, .level = kPWM_HighTrue, .faultState = kPWM_PwmFaultState0, .pwmchannelenable = true, .deadtimeValue = 0U }
    };
    PWM_SetupPwm(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM2, sm2Config, 3U, kPWM_SignedEdgeAligned, FLEXPWM0_SM2_COUNTER_FREQ_HZ, FLEXPWM0_SM2_SM_CLK_SOURCE_FREQ_HZ);

    /* Start Timers and Load logic for all Submodules */
    PWM_SetPwmLdok(FLEXPWM0_PERIPHERAL, (1U << FLEXPWM0_SM0) | (1U << FLEXPWM0_SM1) | (1U << FLEXPWM0_SM2), true);
    PWM_StartTimer(FLEXPWM0_PERIPHERAL, (1U << FLEXPWM0_SM0) | (1U << FLEXPWM0_SM1) | (1U << FLEXPWM0_SM2));
}

void PwmControl_SetLeftTurnSignalPWM(uint8_t dutyA, uint8_t dutyX)
{
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, FLEXPWM0_SM0_A, kPWM_SignedEdgeAligned, dutyA);
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, kPWM_PwmX, kPWM_SignedEdgeAligned, dutyX);
}

void PwmControl_SetRightTurnSignalPWM(uint8_t duty1X, uint8_t duty2X)
{
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM1, kPWM_PwmX, kPWM_SignedEdgeAligned, duty1X);
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM2, kPWM_PwmX, kPWM_SignedEdgeAligned, duty2X);
}

void PwmControl_SetAllSignalsOffPWM(void)
{
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, FLEXPWM0_SM0_A, kPWM_SignedEdgeAligned, 0U);
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, kPWM_PwmX, kPWM_SignedEdgeAligned, 0U);
    
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM1, kPWM_PwmX, kPWM_SignedEdgeAligned, 0U);
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM2, kPWM_PwmX, kPWM_SignedEdgeAligned, 100U);
    
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, FLEXPWM0_SM0_B, kPWM_SignedEdgeAligned, 0U);
    PWM_SetPwmLdok(FLEXPWM0_PERIPHERAL, (1U << FLEXPWM0_SM0) | (1U << FLEXPWM0_SM1) | (1U << FLEXPWM0_SM2), true);
}

void PwmControl_SetBuzzerDuty(uint8_t dutyB)
{
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, FLEXPWM0_SM0_B, kPWM_SignedEdgeAligned, dutyB);
    PWM_SetPwmLdok(FLEXPWM0_PERIPHERAL, 1U << FLEXPWM0_SM0, true);
}

void PwmControl_SetBuzzerFrequencyAndDuty(uint32_t freqHz, uint8_t dutyB)
{
    pwm_signal_param_t pwmBConfig = {
        .pwmChannel = kPWM_PwmB,
        .dutyCyclePercent = dutyB,
        .level = kPWM_HighTrue,
        .faultState = kPWM_PwmFaultState0,
        .pwmchannelenable = true,
        .deadtimeValue = 0U
    };
    
    /* Change frequency for the entire Submodule 0 and set Channel B */
    PWM_SetupPwm(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, &pwmBConfig, 1U, kPWM_SignedEdgeAligned, freqHz, FLEXPWM0_SM0_SM_CLK_SOURCE_FREQ_HZ);
}

void PwmControl_ReloadLeftTurnPWM(void)
{
    PWM_SetPwmLdok(FLEXPWM0_PERIPHERAL, 1U << FLEXPWM0_SM0, true);
}

void PwmControl_ReloadRightTurnPWM(void)
{
    PWM_SetPwmLdok(FLEXPWM0_PERIPHERAL, (1U << FLEXPWM0_SM1) | (1U << FLEXPWM0_SM2), true);
}
