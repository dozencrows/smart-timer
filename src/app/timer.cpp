/*
 * Implementation of Timer class
 */
 
#include "timer.h"

#include "stdio.h"
#include "LPC8xx.h"
#include "util/lcd.h"

#define MAX_TIMER_INSTANCES 2
#define MRT_TIMER           1

//----------------------------------------------------------------------------------------
// Interrupt handling
//

static int      timer_instance_count = 0;
static Timer*   timer_instances[MAX_TIMER_INSTANCES];

void TimerInterruptHandler(void) {
    for (int i = 0; i < timer_instance_count; i++) {
        timer_instances[i]->Tick();
    }
}

extern "C" void MRT_IRQHandler(void) {
    LPC_MRT->Channel[MRT_TIMER].STAT = 0x1 ; /* clr interupt request */
    TimerInterruptHandler();
}

//----------------------------------------------------------------------------------------
// Class implementation
//

Timer::Timer() {
    current_hours_      = 0;
    current_minutes_    = 0;
    current_seconds_    = 0;

    start_hours_        = 0;
    start_minutes_      = 0;
    start_seconds_      = 0;
    
    x_ = 0;
    y_ = 0;

    state_ = STOPPED;
    updated_ = false;
    
    if (timer_instance_count < MAX_TIMER_INSTANCES) {
        timer_instances[timer_instance_count++] = this;
    }
}

void Timer::Initialise() {
    // Set up a repeating 1 second timer interrupt on MRT channel 0
    LPC_MRT->Channel[MRT_TIMER].CTRL    = 0x01;
    LPC_MRT->Channel[MRT_TIMER].INTVAL  = SystemCoreClock | (1 << 31);
    NVIC_EnableIRQ(MRT_IRQn);
}

void Timer::SetCoords(uint8_t x, uint8_t y) {
    x_ = x;
    y_ = y;
}

void Timer::SetStartTime(uint8_t hours, uint8_t minutes, uint8_t seconds) {
    start_hours_    = hours;
    start_minutes_  = minutes;
    start_seconds_  = seconds;
}

void Timer::Start() {
    state_ = RUNNING;
}

void Timer::Stop() {
    state_ = PAUSED;
}

void Timer::Reset() {
    current_hours_      = start_hours_;
    current_minutes_    = start_minutes_;
    current_seconds_    = start_seconds_;
    updated_            = true;
    state_              = STOPPED;
}

void Timer::Tick() {
    if (state_ == RUNNING) {
        if (current_seconds_ > 0) {
            current_seconds_--;
        }
        else if (current_minutes_ > 0) {
            current_seconds_ = 59;
            current_minutes_--;
        }
        else if (current_hours_ > 0) {
            current_seconds_ = 59;
            current_minutes_ = 59;
            current_hours_--;
        }
        
        if (current_seconds_ == current_minutes_ == current_hours_ == 0) {
            state_ = ALARM;
        }
        
        updated_ = true;
    }
}

void Timer::Update() {    
    if (updated_) {
        char time_text[8];
        
        time_text[0] = current_hours_ + '0';
        time_text[1] = ':';
        time_text[2] = (current_minutes_ / 10) + '0';
        time_text[3] = (current_minutes_ % 10) + '0';
        time_text[4] = ':';
        time_text[5] = (current_seconds_ / 10) + '0';
        time_text[6] = (current_seconds_ % 10) + '0';
        time_text[7] = '\0';
        
        lcdMoveTo(x_, y_);
        lcdPuts(time_text);
        updated_ = false;
    }
}

