#include "turn_signals.h"
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include <stdio.h>

/* ========================================================================== */
/* Types and Enums                                                            */
/* ========================================================================== */
typedef enum {
    kSTATE_OFF = 0,
    kSTATE_TURN_LEFT = 1,
    kSTATE_TURN_RIGHT = 2
} TurnSignalState;

typedef enum {
    kBLINK_PHASE_OFF = 0,
    kBLINK_PHASE_ON = 1
} BlinkPhase;

/* ========================================================================== */
/* Internal Variables                                                         */
/* ========================================================================== */
static uint32_t freqHighHz = 1750;
static uint32_t freqLowHz = 1000;
static bool useHighFreq = true;
static uint32_t pauseTimeMs = 100;
static uint32_t buzzTimeMs = 300;
static volatile uint32_t elapsedMs = 0;
static volatile bool buzzerTick = false;
static volatile bool timerTick = false;
static TurnSignalState currentState = kSTATE_OFF;
static BlinkPhase blinkPhase = kBLINK_PHASE_OFF;

/* ========================================================================== */
/* Internal Functions                                                         */
/* ========================================================================== */
static void SetAllSignalsOff(void)
{
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, FLEXPWM0_SM0_A, kPWM_SignedEdgeAligned, 0U);
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, kPWM_PwmX, kPWM_SignedEdgeAligned, 0U);
    
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM1, kPWM_PwmX, kPWM_SignedEdgeAligned, 0U);
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM2, kPWM_PwmX, kPWM_SignedEdgeAligned, 100U);
    
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, FLEXPWM0_SM0_B, kPWM_SignedEdgeAligned, 0U);
    PWM_SetPwmLdok(FLEXPWM0_PERIPHERAL, (1U << FLEXPWM0_SM0) | (1U << FLEXPWM0_SM1) | (1U << FLEXPWM0_SM2), true);
    
    /* Left GPIOs */
    GPIO_PinWrite(BOARD_INITPINS_LED_FRONT_LEFT_BLUE_GPIO, BOARD_INITPINS_LED_FRONT_LEFT_BLUE_GPIO_PIN, 0U);
    GPIO_PinWrite(BOARD_INITPINS_LED_FRONT_LEFT_RED_GPIO, BOARD_INITPINS_LED_FRONT_LEFT_RED_GPIO_PIN, 0U);
    GPIO_PinWrite(BOARD_INITPINS_LED_BACK_LEFT_RED_GPIO, BOARD_INITPINS_LED_BACK_LEFT_RED_GPIO_PIN, 0U);

    /* Right GPIOs */
    GPIO_PinWrite(BOARD_INITPINS_LED_FRONT_RIGHT_BLUE_GPIO, BOARD_INITPINS_LED_FRONT_RIGHT_BLUE_GPIO_PIN, 0U);
    GPIO_PinWrite(BOARD_INITPINS_LED_FRONT_RIGHT_RED_GPIO, BOARD_INITPINS_LED_FRONT_RIGHT_RED_GPIO_PIN, 0U);
    GPIO_PinWrite(BOARD_INITPINS_LED_BACK_RIGHT_RED_GPIO, BOARD_INITPINS_LED_BACK_RIGHT_RED_GPIO_PIN, 0U);
}

static void SetTurnSignalLeftActive(void)
{
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, FLEXPWM0_SM0_A, kPWM_SignedEdgeAligned, 90U);
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, kPWM_PwmX, kPWM_SignedEdgeAligned, 10U);
    PWM_SetPwmLdok(FLEXPWM0_PERIPHERAL, 1U << FLEXPWM0_SM0, true);
    
    GPIO_PinWrite(BOARD_INITPINS_LED_FRONT_LEFT_BLUE_GPIO, BOARD_INITPINS_LED_FRONT_LEFT_BLUE_GPIO_PIN, 1U);
    GPIO_PinWrite(BOARD_INITPINS_LED_FRONT_LEFT_RED_GPIO, BOARD_INITPINS_LED_FRONT_LEFT_RED_GPIO_PIN, 0U);
    GPIO_PinWrite(BOARD_INITPINS_LED_BACK_LEFT_RED_GPIO, BOARD_INITPINS_LED_BACK_LEFT_RED_GPIO_PIN, 0U);
}

static void SetTurnSignalLeftInactive(void)
{
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, FLEXPWM0_SM0_A, kPWM_SignedEdgeAligned, 100U);
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, kPWM_PwmX, kPWM_SignedEdgeAligned, 0U);
    PWM_SetPwmLdok(FLEXPWM0_PERIPHERAL, 1U << FLEXPWM0_SM0, true);
    
    GPIO_PinWrite(BOARD_INITPINS_LED_FRONT_LEFT_BLUE_GPIO, BOARD_INITPINS_LED_FRONT_LEFT_BLUE_GPIO_PIN, 1U);
    GPIO_PinWrite(BOARD_INITPINS_LED_FRONT_LEFT_RED_GPIO, BOARD_INITPINS_LED_FRONT_LEFT_RED_GPIO_PIN, 1U);
    GPIO_PinWrite(BOARD_INITPINS_LED_BACK_LEFT_RED_GPIO, BOARD_INITPINS_LED_BACK_LEFT_RED_GPIO_PIN, 1U);
}

static void SetTurnSignalRightActive(void)
{
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM2, kPWM_PwmX, kPWM_SignedEdgeAligned, 10U);
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM1, kPWM_PwmX, kPWM_SignedEdgeAligned, 10U);
    PWM_SetPwmLdok(FLEXPWM0_PERIPHERAL, (1U << FLEXPWM0_SM1) | (1U << FLEXPWM0_SM2), true);

    GPIO_PinWrite(BOARD_INITPINS_LED_FRONT_RIGHT_BLUE_GPIO, BOARD_INITPINS_LED_FRONT_RIGHT_BLUE_GPIO_PIN, 1U);
    GPIO_PinWrite(BOARD_INITPINS_LED_FRONT_RIGHT_RED_GPIO, BOARD_INITPINS_LED_FRONT_RIGHT_RED_GPIO_PIN, 0U);
    GPIO_PinWrite(BOARD_INITPINS_LED_BACK_RIGHT_RED_GPIO, BOARD_INITPINS_LED_BACK_RIGHT_RED_GPIO_PIN, 0U);
}

static void SetTurnSignalRightInactive(void)
{
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM2, kPWM_PwmX, kPWM_SignedEdgeAligned, 0U);
    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM1, kPWM_PwmX, kPWM_SignedEdgeAligned, 0U);
    PWM_SetPwmLdok(FLEXPWM0_PERIPHERAL, (1U << FLEXPWM0_SM1) | (1U << FLEXPWM0_SM2), true);

    GPIO_PinWrite(BOARD_INITPINS_LED_FRONT_RIGHT_BLUE_GPIO, BOARD_INITPINS_LED_FRONT_RIGHT_BLUE_GPIO_PIN, 1U);
    GPIO_PinWrite(BOARD_INITPINS_LED_FRONT_RIGHT_RED_GPIO, BOARD_INITPINS_LED_FRONT_RIGHT_RED_GPIO_PIN, 1U);
    GPIO_PinWrite(BOARD_INITPINS_LED_BACK_RIGHT_RED_GPIO, BOARD_INITPINS_LED_BACK_RIGHT_RED_GPIO_PIN, 1U);
}

/* ========================================================================== */
/* Public Functions                                                           */
/* ========================================================================== */
void TurnSignals_InitHardware(void)
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

void TurnSignals_Tick(void)
{
    elapsedMs += 100;
    
    if (elapsedMs >= 500)
    {
        elapsedMs = 0;
        timerTick = true;
    }
    
    buzzerTick = true;
}

void SignalLeft(void)
{
    SetAllSignalsOff();
    currentState = kSTATE_TURN_LEFT;
    blinkPhase = kBLINK_PHASE_ON;
    timerTick = true;
    elapsedMs = 0;
}

void SignalRight(void)
{
    SetAllSignalsOff();
    currentState = kSTATE_TURN_RIGHT;
    blinkPhase = kBLINK_PHASE_ON;
    timerTick = true;
    elapsedMs = 0;
}

void SignalOff(void)
{
    currentState = kSTATE_OFF;
    SetAllSignalsOff();
    elapsedMs = 0;
}

void ProcessBlinker(void)
{
    if (timerTick)
    {
        timerTick = false;
        
        if (currentState == kSTATE_TURN_LEFT)
        {
            if (blinkPhase == kBLINK_PHASE_ON)
            {
                SetTurnSignalLeftActive();
                blinkPhase = kBLINK_PHASE_OFF;
            }
            else
            {
                SetTurnSignalLeftInactive();
                blinkPhase = kBLINK_PHASE_ON;
            }
        }
        else if (currentState == kSTATE_TURN_RIGHT)
        {
            if (blinkPhase == kBLINK_PHASE_ON)
            {
                SetTurnSignalRightActive();
                blinkPhase = kBLINK_PHASE_OFF;
            }
            else
            {
                SetTurnSignalRightInactive();
                blinkPhase = kBLINK_PHASE_ON;
            }
        }
    }
}

static void UpdateSM0FrequencyAndDuty(uint32_t freq)
{
    pwm_signal_param_t pwmBConfig = {
        .pwmChannel = kPWM_PwmB,
        .dutyCyclePercent = 50U,
        .level = kPWM_HighTrue,
        .faultState = kPWM_PwmFaultState0,
        .pwmchannelenable = true,
        .deadtimeValue = 0U
    };
    
    /* Change frequency for the entire Submodule 0 and set Channel B to 50% */
    PWM_SetupPwm(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, &pwmBConfig, 1U, kPWM_SignedEdgeAligned, freq, FLEXPWM0_SM0_SM_CLK_SOURCE_FREQ_HZ);
    
    /* Re-apply duty cycles for channels A and X because the submodule period (VAL1) has changed */
    if (currentState == kSTATE_TURN_LEFT)
    {
        if (blinkPhase == kBLINK_PHASE_OFF) /* Currently active visual phase */
        {
            PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, FLEXPWM0_SM0_A, kPWM_SignedEdgeAligned, 90U);
            PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, kPWM_PwmX, kPWM_SignedEdgeAligned, 10U);
        }
        else /* Currently inactive visual phase */
        {
            PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, FLEXPWM0_SM0_A, kPWM_SignedEdgeAligned, 100U);
            PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, kPWM_PwmX, kPWM_SignedEdgeAligned, 0U);
        }
    }
    else
    {
        /* Right turn or OFF state: left LEDs on SM0 should be off */
        PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, FLEXPWM0_SM0_A, kPWM_SignedEdgeAligned, 0U);
        PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, kPWM_PwmX, kPWM_SignedEdgeAligned, 0U);
    }
    
    PWM_SetPwmLdok(FLEXPWM0_PERIPHERAL, 1U << FLEXPWM0_SM0, true);
}

void ProcessBuzzer(void)
{
    if (buzzerTick)
    {
        buzzerTick = false;
        
        if (currentState == kSTATE_OFF)
        {
            /* Ensure buzzer stays quiet when signals are off */
            PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, FLEXPWM0_SM0_B, kPWM_SignedEdgeAligned, 0U);
            PWM_SetPwmLdok(FLEXPWM0_PERIPHERAL, 1U << FLEXPWM0_SM0, true);
            useHighFreq = true; /* Reset so the first buzz is always high */
        }
        else
        {
            /* Timing sequence: 100ms pause -> 300ms buzz -> 100ms pause */
            if (elapsedMs >= pauseTimeMs && elapsedMs < (pauseTimeMs + buzzTimeMs))
            {
                if (elapsedMs == pauseTimeMs)
                {
                    /* Start of the buzz phase: set alternating frequency */
                    uint32_t freq = useHighFreq ? freqHighHz : freqLowHz;
                    UpdateSM0FrequencyAndDuty(freq);
                    useHighFreq = !useHighFreq; /* Toggle for the next buzz */
                }
                else 
                {
                    /* Keep buzzing at current freq with 50% duty */
                    PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, FLEXPWM0_SM0_B, kPWM_SignedEdgeAligned, 50U);
                    PWM_SetPwmLdok(FLEXPWM0_PERIPHERAL, 1U << FLEXPWM0_SM0, true);
                }
            }
            else
            {
                PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, FLEXPWM0_SM0, FLEXPWM0_SM0_B, kPWM_SignedEdgeAligned, 0U);
                PWM_SetPwmLdok(FLEXPWM0_PERIPHERAL, 1U << FLEXPWM0_SM0, true);
            }
        }
    }
}
