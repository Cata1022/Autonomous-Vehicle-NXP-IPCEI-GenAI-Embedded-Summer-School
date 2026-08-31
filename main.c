#include "board.h"
#include "app.h"
#include "fsl_debug_console.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "turn_signals.h"
#include "motors.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

/* ========================================================================== */
/* Global Variables                                                           */
/* ========================================================================== */
#define RING_BUFFER_SIZE 256
volatile char rxRingBuffer[RING_BUFFER_SIZE];
volatile uint16_t rxRingHead = 0;
volatile uint16_t rxRingTail = 0;

/* ========================================================================== */
/* Interrupt Handlers                                                         */
/* ========================================================================== */
void LPUART1_IRQHandler(void)
{
    uint32_t status = LPUART_GetStatusFlags(LPUART1);
    
    if (status & (kLPUART_RxOverrunFlag | kLPUART_NoiseErrorFlag | kLPUART_FramingErrorFlag | kLPUART_ParityErrorFlag)) {
        LPUART_ClearStatusFlags(LPUART1, kLPUART_RxOverrunFlag | kLPUART_NoiseErrorFlag | kLPUART_FramingErrorFlag | kLPUART_ParityErrorFlag);
    }
    
    if (status & kLPUART_RxDataRegFullFlag) {
        uint8_t ch = LPUART_ReadByte(LPUART1);
        uint16_t nextHead = (rxRingHead + 1) % RING_BUFFER_SIZE;
        if (nextHead != rxRingTail) {
            rxRingBuffer[rxRingHead] = ch;
            rxRingHead = nextHead;
        }
    }
}

void LPUART0_SignalEvent(uint32_t event)
{
    (void)event;
}

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

    /* Start 100ms Tick for the Blinker state machine */
    SysTick_Config(SystemCoreClock / 10U);
    
    /* Force RX FIFO Watermark to 0 to prevent hardware tail-holding */
    LPUART_SetRxFifoWatermark(LPUART1, 0);
    
    /* Enable LPUART1 RX Interrupts */
    LPUART_EnableInterrupts(LPUART1, kLPUART_RxDataRegFullInterruptEnable | kLPUART_RxOverrunInterruptEnable);
    EnableIRQ(LPUART1_IRQn);
}

void ProcessUart(void)
{
    static char lineBuffer[64];
    static uint8_t lineIndex = 0;
    static int last_ls = 0, last_rs = 0, last_lsig = 0, last_rsig = 0;
    
    while (rxRingTail != rxRingHead)
    {
        char ch = rxRingBuffer[rxRingTail];
        rxRingTail = (rxRingTail + 1) % RING_BUFFER_SIZE;
        
        if (ch == '\r' || ch == '\n')
        {
            lineBuffer[lineIndex] = '\0';
            
            if (lineIndex > 0)
            {
                PRINTF("RAW: '%s'\r\n", lineBuffer);
                char *tok1 = strtok(lineBuffer, ",");
                char *tok2 = strtok(NULL, ",");
                char *tok3 = strtok(NULL, ",");
                char *tok4 = strtok(NULL, "\r\n");
                
                if (tok1 && tok2 && tok3 && tok4)
                {
                    int ls = atoi(tok1);
                    int rs = atoi(tok2);
                    int l_sig = atoi(tok3);
                    int r_sig = atoi(tok4);
                    
                    bool changed = false;
                    
                    if (ls != last_ls) { SetLeftMotorSpeed((int8_t)ls); last_ls = ls; changed = true; }
                    if (rs != last_rs) { SetRightMotorSpeed((int8_t)rs); last_rs = rs; changed = true; }
                    
                    if (l_sig != last_lsig || r_sig != last_rsig) {
                        if (l_sig == 1) SignalLeft();
                        else if (r_sig == 1) SignalRight();
                        else SignalOff();
                        last_lsig = l_sig;
                        last_rsig = r_sig;
                        changed = true;
                    }
                    
                    if (changed) {
                        PRINTF("CMD EXEC: L:%c%d R:%c%d SL:%d SR:%d\r\n", 
                               ls < 0 ? '-' : ' ', abs(ls), 
                               rs < 0 ? '-' : ' ', abs(rs), 
                               l_sig, r_sig);
                    }
                }
                else
                {
                    PRINTF("PARSE FAIL: '%s'\r\n", lineBuffer);
                }
            }
            lineIndex = 0;
        }
        else if (ch == ',' || ch == '-' || (ch >= '0' && ch <= '9'))
        {
            if (lineIndex < sizeof(lineBuffer) - 1)
            {
                lineBuffer[lineIndex++] = ch;
            }
        }
    }
}

/* ========================================================================== */
/* Main function                                                              */
/* ========================================================================== */
int main(void)
{
    InitHardware();

    while (1)
    {
        ProcessUart();
        ProcessBlinker();
        ProcessBuzzer();
    }
}
