/*
 * Backlight controller implementation
 */
 
#include "backlight.h"

#include "LPC8xx.h"
#include "util/lcd.h"
#include "util/mrt_interrupt.h"

//----------------------------------------------------------------------------------------
// Interrupt handling
//
#define BACKLIGHT_MRT_TIMER   0

void BacklightInterruptHandler(void) {
    lcdSetBacklight(0);    
}

//----------------------------------------------------------------------------------------
// Class implementation
//

void Backlight::Initialise() {
    mrt_interrupt_set_timer_callback(BACKLIGHT_MRT_TIMER, BacklightInterruptHandler);
}

Backlight::Backlight() {
}

void Backlight::On() {
    lcdSetBacklight(1);
}

void Backlight::DelayedOff(uint32_t delay_ms) {
    LPC_MRT->Channel[BACKLIGHT_MRT_TIMER].CTRL    = 0x03;
    LPC_MRT->Channel[BACKLIGHT_MRT_TIMER].INTVAL  = (FIXED_CLOCK_RATE_HZ / 1000) * delay_ms | (1 << 31);
}

