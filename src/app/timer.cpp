/*
 * Implementation of Timer class
 */
 
#include "timer.h"

#include "stdio.h"
#include "LPC8xx.h"
#include "util/lcd.h"

#define MAX_TIMER_INSTANCES     2
#define MRT_TIMER               1
#define TIME_TEXT_BUFFER_LEN    8
#define MAX_HOURS               9
#define MAX_MINUTES             59
#define MAX_SECONDS             59

//----------------------------------------------------------------------------------------
// Utilities
//
void Time2DigitsToAscii(uint8_t byte, char* str) {
    uint8_t digit_0 = '0';
    
    while (byte > 9) {
        byte -= 10;
        digit_0++;
    }
    
    str[0] = digit_0;
    str[1] = byte + '0';
}

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
    current_time_.all   = 0;
    start_time_.all     = 0;
    
    x_ = 0;
    y_ = 0;

    state_ = STOPPED;
    updated_ = false;
    visible_ = true;
    
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

void Timer::ToggleStartStop() {
    if (state_ == ALARM) {
        Reset();
    }
    else if (state_ != RUNNING) {
        if (current_time_.all > 0) {
            state_ = RUNNING;
        }
    }
    else {
        state_ = STOPPED;
    }
}

void Timer::Clear() {
    start_time_.all = 0;
    Reset();
}

void Timer::Reset() {
    current_time_.all   = start_time_.all;
    updated_            = true;
    state_              = STOPPED;
    visible_            = true;
}

void Timer::AddHour() {
    AddTime(1, 0, 0);
}

void Timer::AddMinute() {
    AddTime(0, 1, 0);
}

void Timer::AddSecond() {
    AddTime(0, 0, 1);
}

void Timer::AddTime(uint8_t hours, uint8_t minutes, uint8_t seconds) {
    if (state_ == RUNNING) {
        state_ = STOPPED;
    }
    else if (state_ == ALARM) {
        Reset();
        state_ = STOPPED;
    }
    else {
        current_time_.hours += hours;
        current_time_.minutes += minutes;
        current_time_.seconds += seconds;
        
        while (current_time_.hours > MAX_HOURS) {
            current_time_.hours -= MAX_HOURS + 1;
        }
        while (current_time_.minutes > MAX_MINUTES) {
            current_time_.minutes -= MAX_MINUTES + 1;
        }
        while (current_time_.seconds > MAX_SECONDS) {
            current_time_.seconds -= MAX_SECONDS + 1;
        }
        
        start_time_.all = current_time_.all;
        updated_ = true;
    }
}

void Timer::Tick() {
    if (state_ == RUNNING) {
        if (current_time_.seconds > 0) {
            current_time_.seconds--;
        }
        else if (current_time_.minutes > 0) {
            current_time_.seconds = 59;
            current_time_.minutes--;
        }
        else if (current_time_.hours > 0) {
            current_time_.seconds = 59;
            current_time_.minutes = 59;
            current_time_.hours--;
        }
        
        if (current_time_.all == 0) {
            state_ = ALARM;
        }
        
        updated_ = true;
    }
    else if (state_ == ALARM) {
        visible_ = !visible_;
        updated_ = true;
    }
}

void Timer::Update() {    
    if (updated_) {
        char time_text[TIME_TEXT_BUFFER_LEN];

        if (visible_) {        
            time_text[0] = current_time_.hours + '0';
            time_text[1] = ':';
            Time2DigitsToAscii(current_time_.minutes, time_text + 2);
            time_text[4] = ':';
            Time2DigitsToAscii(current_time_.seconds, time_text + 5);
        }
        else {
            uint32_t* blank_text = (uint32_t*)time_text;
            blank_text[0] = 0x20202020;
            blank_text[1] = 0x20202020;
        }
        
        time_text[7] = '\0';
        lcdMoveTo(x_, y_);
        lcdPuts(time_text);
        updated_ = false;
    }
}

