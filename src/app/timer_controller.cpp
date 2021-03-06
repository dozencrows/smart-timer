//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * TimerController implementation
 */
 
#include "timer_controller.h"

#include "buzzer.h"
#include "backlight.h"

#define BUTTON_H        0x08
#define BUTTON_M        0x01
#define BUTTON_S        0x02
#define BUTTON_START    0x04

TimerController::TimerController(Buzzer& buzzer, Backlight& backlight) : buzzer_(buzzer), backlight_(backlight), timer1_(*this), timer2_(*this){
    last_buttons_ = 0;
    timer1_.SetCoords(0, 0);
    timer1_.Reset();
    timer2_.SetCoords(9, 0);
    timer2_.Reset();
}

void TimerController::Update() {
    timer1_.Update();
    timer2_.Update();
}

void TimerController::ForceUpdate() {
    timer1_.ForceUpdate();
    timer2_.ForceUpdate();
}

extern void errorWithCode(const char* msg, int code);

void TimerController::ProcessButtons(uint8_t button_state) {
    uint8_t buttons_changed = button_state ^ last_buttons_;
    
    ProcessTimerButtons(button_state >> 4, buttons_changed >> 4, timer1_);
    ProcessTimerButtons(button_state & 0xf, buttons_changed & 0xf, timer2_);
    
    backlight_.DelayedOff(BACKLIGHT_ON_TIME_MS);
    
    last_buttons_ = button_state;
}

void TimerController::ProcessTimerButtons(uint8_t button_state, uint8_t buttons_changed, Timer& timer) {
    uint8_t buttons_pressed = button_state & buttons_changed;
    
    if (buttons_pressed) {
        buzzer_.Beep();
        if (backlight_.IsOn()) {
            if (buttons_pressed & BUTTON_START) {
                timer.ToggleStartStop();
            }
            else if (buttons_pressed & (BUTTON_H|BUTTON_M|BUTTON_S)) {
                uint8_t time_buttons = button_state & (BUTTON_H|BUTTON_M|BUTTON_S);
                if (time_buttons != BUTTON_H && time_buttons != BUTTON_M && time_buttons != BUTTON_S) {
                    timer.Clear();
                }
                else {
                    switch(time_buttons) {
                        case BUTTON_H:
                            timer.AddHour();
                            break;
                        case BUTTON_M:
                            timer.AddMinute();
                            break;
                        case BUTTON_S:
                            timer.AddSecond();
                            break;
                    }
                }
            }
        }
        else {
            backlight_.On();
        }
    }    
}

void TimerController::Notify(Timer& timer, Notification notification) {
    switch(notification) {
        case ALARM_START:
            buzzer_.Beeps();
            backlight_.On();
            break;
        case ALARM_STOP:
            buzzer_.Off();
            break;
    }
}

