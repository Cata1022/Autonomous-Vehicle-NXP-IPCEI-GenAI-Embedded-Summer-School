#ifndef TURN_SIGNALS_H_
#define TURN_SIGNALS_H_

#include <stdint.h>
#include <stdbool.h>

/* Initializes the hardware components (PWMs, etc.) needed for turn signals */
void TurnSignals_InitHardware(void);

/* Triggers a timing tick for the state machine (call from SysTick) */
void TurnSignals_Tick(void);

/* Public API for controlling signals */
void SignalLeft(void);
void SignalRight(void);
void SignalOff(void);

/* Process the blinking logic (call from main loop) */
void ProcessBlinker(void);

/* Process the buzzer logic (call from main loop) */
void ProcessBuzzer(void);

#endif /* TURN_SIGNALS_H_ */
