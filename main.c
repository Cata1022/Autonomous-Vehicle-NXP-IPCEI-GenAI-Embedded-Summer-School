#include "board.h"
#include "app.h"
#include "fsl_debug_console.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "turn_signals.h"
#include "motors.h"
#include "uart_comm.h"

/* ========================================================================== */
/* Interrupt Handlers                                                         */
/* ========================================================================== */
void SysTick_Handler(void)
{
    TurnSignals_Tick();
}

/* ========================================================================== */
/* Application Functions                                                      */
/* ========================================================================== */
void InitHardware(void)
{
    BOARD_InitHardware();
    BOARD_InitDebugConsole();

    /* Initialize Turn Signals (PWM submodules & Pins) */
    TurnSignals_InitHardware();

    /* Initialize Motors to hard-stop state */
    Motors_InitHardware();

    /* Initialize UART Communication */
    UartComm_InitHardware();

    /* Start 100ms Tick for the Blinker state machine */
    SysTick_Config(SystemCoreClock / 10U);
}

/* ========================================================================== */
/* Main function                                                              */
/* ========================================================================== */
int main(void)
{
    InitHardware();

    while (1)
    {
        UartComm_Process();
        ProcessBlinker();
        ProcessBuzzer();
    }
}
