/*
 * Timer Controller: object that manages the interaction between the UI and the timers
 */
 
#if !defined(__TIMERCONTROLLER_H__)
#define __TIMERCONTROLLER_H__

#include "timer.h"

class TimerController {
    public:
        TimerController();
        
        void Initialise();
        void Update();
        void ProcessButtons(uint8_t button_state);
        
    private:
    
        void ProcessTimerButtons(uint8_t button_state, uint8_t buttons_changed, Timer& timer);
    
        Timer   timer1_;
        Timer   timer2_;
        uint8_t last_buttons;
};

#endif // #if !defined(__TIMERCONTROLLER_H__)
