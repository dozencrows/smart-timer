/*
 * TimerController implementation
 */
 
#include "timer_controller.h"

#define BUTTON_H        0x08
#define BUTTON_M        0x04
#define BUTTON_S        0x02
#define BUTTON_START    0x01

TimerController::TimerController() {
    last_buttons = 0;
}

void TimerController::Initialise() {
    timer1_.SetCoords(0, 0);
    timer1_.Reset();
    timer2_.SetCoords(9, 0);
    timer2_.Reset();
}

void TimerController::Update() {
    timer1_.Update();
    timer2_.Update();
}

void TimerController::ProcessButtons(uint8_t button_state) {
    uint8_t buttons_changed = button_state ^ last_buttons;
    
    ProcessTimerButtons(button_state >> 4, buttons_changed >> 4, timer1_);
    ProcessTimerButtons(button_state & 0xf, buttons_changed & 0xf, timer2_);
    
    last_buttons = button_state;
}

void TimerController::ProcessTimerButtons(uint8_t button_state, uint8_t buttons_changed, Timer& timer) {
    uint8_t buttons_pressed = button_state & buttons_changed;
    
    if (buttons_pressed) {
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
}

