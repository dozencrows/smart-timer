/*
 * Timer Controller: object that manages the interaction between the UI and the timers
 */
 
#if !defined(__TIMERCONTROLLER_H__)
#define __TIMERCONTROLLER_H__

#include "timer.h"

class Buzzer;

class TimerController {
    public:
        enum Notification {
            ALARM_START,
            ALARM_STOP
        };
        
        TimerController(Buzzer& buzzer);
        
        void Update();
        void ProcessButtons(uint8_t button_state);
        void Notify(Timer& timer, Notification notification);
        
    private:
    
        void ProcessTimerButtons(uint8_t button_state, uint8_t buttons_changed, Timer& timer);
        
        Buzzer& buzzer_;
        Timer   timer1_;
        Timer   timer2_;
        uint8_t last_buttons;
};

#endif // #if !defined(__TIMERCONTROLLER_H__)
