#include "turn_signals.h"
#include "pwm_control.h"
#include "board.h"
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
    PwmControl_SetAllSignalsOffPWM();
    
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
    PwmControl_SetLeftTurnSignalPWM(90U, 10U);
    PwmControl_ReloadLeftTurnPWM();
    
    GPIO_PinWrite(BOARD_INITPINS_LED_FRONT_LEFT_BLUE_GPIO, BOARD_INITPINS_LED_FRONT_LEFT_BLUE_GPIO_PIN, 1U);
    GPIO_PinWrite(BOARD_INITPINS_LED_FRONT_LEFT_RED_GPIO, BOARD_INITPINS_LED_FRONT_LEFT_RED_GPIO_PIN, 0U);
    GPIO_PinWrite(BOARD_INITPINS_LED_BACK_LEFT_RED_GPIO, BOARD_INITPINS_LED_BACK_LEFT_RED_GPIO_PIN, 0U);
}

static void SetTurnSignalLeftInactive(void)
{
    PwmControl_SetLeftTurnSignalPWM(100U, 0U);
    PwmControl_ReloadLeftTurnPWM();
    
    GPIO_PinWrite(BOARD_INITPINS_LED_FRONT_LEFT_BLUE_GPIO, BOARD_INITPINS_LED_FRONT_LEFT_BLUE_GPIO_PIN, 1U);
    GPIO_PinWrite(BOARD_INITPINS_LED_FRONT_LEFT_RED_GPIO, BOARD_INITPINS_LED_FRONT_LEFT_RED_GPIO_PIN, 1U);
    GPIO_PinWrite(BOARD_INITPINS_LED_BACK_LEFT_RED_GPIO, BOARD_INITPINS_LED_BACK_LEFT_RED_GPIO_PIN, 1U);
}

static void SetTurnSignalRightActive(void)
{
    PwmControl_SetRightTurnSignalPWM(10U, 10U);
    PwmControl_ReloadRightTurnPWM();

    GPIO_PinWrite(BOARD_INITPINS_LED_FRONT_RIGHT_BLUE_GPIO, BOARD_INITPINS_LED_FRONT_RIGHT_BLUE_GPIO_PIN, 1U);
    GPIO_PinWrite(BOARD_INITPINS_LED_FRONT_RIGHT_RED_GPIO, BOARD_INITPINS_LED_FRONT_RIGHT_RED_GPIO_PIN, 0U);
    GPIO_PinWrite(BOARD_INITPINS_LED_BACK_RIGHT_RED_GPIO, BOARD_INITPINS_LED_BACK_RIGHT_RED_GPIO_PIN, 0U);
}

static void SetTurnSignalRightInactive(void)
{
    PwmControl_SetRightTurnSignalPWM(0U, 0U);
    PwmControl_ReloadRightTurnPWM();

    GPIO_PinWrite(BOARD_INITPINS_LED_FRONT_RIGHT_BLUE_GPIO, BOARD_INITPINS_LED_FRONT_RIGHT_BLUE_GPIO_PIN, 1U);
    GPIO_PinWrite(BOARD_INITPINS_LED_FRONT_RIGHT_RED_GPIO, BOARD_INITPINS_LED_FRONT_RIGHT_RED_GPIO_PIN, 1U);
    GPIO_PinWrite(BOARD_INITPINS_LED_BACK_RIGHT_RED_GPIO, BOARD_INITPINS_LED_BACK_RIGHT_RED_GPIO_PIN, 1U);
}

/* ========================================================================== */
/* Public Functions                                                           */
/* ========================================================================== */
void TurnSignals_InitHardware(void)
{
    PwmControl_InitTurnSignals();
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
    /* Change frequency for the entire Submodule 0 and set Channel B to 50% */
    PwmControl_SetBuzzerFrequencyAndDuty(freq, 50U);
    
    /* Re-apply duty cycles for channels A and X because the submodule period (VAL1) has changed */
    if (currentState == kSTATE_TURN_LEFT)
    {
        if (blinkPhase == kBLINK_PHASE_OFF) /* Currently active visual phase */
        {
            PwmControl_SetLeftTurnSignalPWM(90U, 10U);
        }
        else /* Currently inactive visual phase */
        {
            PwmControl_SetLeftTurnSignalPWM(100U, 0U);
        }
    }
    else
    {
        /* Right turn or OFF state: left LEDs on SM0 should be off */
        PwmControl_SetLeftTurnSignalPWM(0U, 0U);
    }
    
    PwmControl_ReloadLeftTurnPWM();
}

void ProcessBuzzer(void)
{
    if (buzzerTick)
    {
        buzzerTick = false;
        
        if (currentState == kSTATE_OFF)
        {
            /* Ensure buzzer stays quiet when signals are off */
            PwmControl_SetBuzzerDuty(0U);
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
                    PwmControl_SetBuzzerDuty(50U);
                }
            }
            else
            {
                PwmControl_SetBuzzerDuty(0U);
            }
        }
    }
}
