#include "uart_comm.h"
#include "fsl_lpuart.h"
#include "motors.h"
#include "turn_signals.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define RING_BUFFER_SIZE 256
volatile char rxRingBuffer[RING_BUFFER_SIZE];
volatile uint16_t rxRingHead = 0;
volatile uint16_t rxRingTail = 0;

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

void UartComm_InitHardware(void)
{
    /* Force RX FIFO Watermark to 0 to prevent hardware tail-holding */
    LPUART_SetRxFifoWatermark(LPUART1, 0);
    
    /* Enable LPUART1 RX Interrupts */
    LPUART_EnableInterrupts(LPUART1, kLPUART_RxDataRegFullInterruptEnable | kLPUART_RxOverrunInterruptEnable);
    EnableIRQ(LPUART1_IRQn);
}

void UartComm_Process(void)
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
                    
                    if (ls != last_ls) { SetLeftMotorSpeed((int8_t)ls); last_ls = ls; }
                    if (rs != last_rs) { SetRightMotorSpeed((int8_t)rs); last_rs = rs; }
                    
                    if (l_sig != last_lsig || r_sig != last_rsig) {
                        if (l_sig == 1) SignalLeft();
                        else if (r_sig == 1) SignalRight();
                        else SignalOff();
                        last_lsig = l_sig;
                        last_rsig = r_sig;
                    }
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
