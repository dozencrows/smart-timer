/* Timers module - various useful timers for LPC8xx using Multi-Rate timer */

#include "LPC8xx.h"
#include "stdio.h"

void timersInit() {
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<10);    // enable MRT clock
    LPC_SYSCON->PRESETCTRL &= ~(1<<7);       // reset MRT
    LPC_SYSCON->PRESETCTRL |=  (1<<7);
    
    LPC_MRT->Channel[2].CTRL = (0x01 << 1); //MRT2 one-shot mode
    LPC_MRT->Channel[3].CTRL = (0x02 << 1); //MRT3 bus stall mode , no interrupts
}

void delayMs(int milliseconds) {
    LPC_MRT->Channel[2].INTVAL = (((SystemCoreClock / 250L) * milliseconds) >> 2) - 286;

    while (LPC_MRT->Channel[2].STAT & 0x02)
        ; //wait while running
}

void delayUs(int microseconds) {
    int clk = (SystemCoreClock / 1000000) * microseconds;
    
    if (clk > 3) {
        LPC_MRT->Channel[3].INTVAL = (clk-3);
    }
}

