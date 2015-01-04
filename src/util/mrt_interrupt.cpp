/*
 * Managing MRT interrupts
 */
 
#include "mrt_interrupt.h"

#include "LPC8xx.h"

#define MRT_CHANNEL_COUNT   4

static void (*mrt_callbacks[MRT_CHANNEL_COUNT])() = { 0, 0, 0, 0 };

void mrt_interrupt_control(bool enable) {
    if (enable) {
        NVIC_EnableIRQ(MRT_IRQn);
    }
    else {
        NVIC_DisableIRQ(MRT_IRQn);
    }
}

void mrt_interrupt_set_timer_callback(int channel, void (*callback)()) {
    mrt_callbacks[channel] = callback;    
}

extern "C" void MRT_IRQHandler(void) {
    for (int i = 0; i < MRT_CHANNEL_COUNT; i++) {
        if (LPC_MRT->Channel[i].STAT & 0x1) {
            LPC_MRT->Channel[i].STAT = 0x1 ; /* clr interrupt request */
            if (mrt_callbacks[i]) {
                mrt_callbacks[i]();
            }
        }
    }
}

