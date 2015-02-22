//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * Timer Controller: object that manages the interaction between the UI and the timers
 */
 
#if !defined(__TIMERCONTROLLER_H__)
#define __TIMERCONTROLLER_H__

#include "timer.h"

#define BACKLIGHT_ON_TIME_MS   2000

class Buzzer;
class Backlight;

class TimerController {
    public:
        enum Notification {
            ALARM_START,
            ALARM_STOP
        };
        
        TimerController(Buzzer& buzzer, Backlight& backlight);
        
        void Update();
        void ProcessButtons(uint8_t button_state);
        void Notify(Timer& timer, Notification notification);
        void ForceUpdate();
        
        bool IsIdle() { return timer1_.IsStopped() && timer2_.IsStopped(); }
        
    private:
    
        void ProcessTimerButtons(uint8_t button_state, uint8_t buttons_changed, Timer& timer);
        
        Buzzer&     buzzer_;
        Backlight&  backlight_;
        Timer       timer1_;
        Timer       timer2_;
        uint8_t     last_buttons_;
};

#endif // #if !defined(__TIMERCONTROLLER_H__)
