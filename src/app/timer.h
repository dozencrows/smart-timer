/*
 * Timer: a basic count-down timer built on a state machine
 *
 */
#if !defined(__TIMER_H__)
#define __TIMER_H__

#include "lpc_types.h"

class TimerController;

class Timer {
    public:
    
        enum State {
            STOPPED = 0,
            RUNNING,
            ALARM
        };
    
        Timer(TimerController& controller);
        
        static void Initialise();
        
        void SetCoords(uint8_t x, uint8_t y);
        void ToggleStartStop();
        void Clear();
        void Reset();
        void Update();
        void AddHour();
        void AddMinute();
        void AddSecond();
        
        bool IsStopped() { return state_ == STOPPED; }

    private:
        void AddTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
        void Tick();
        
        union TimeVal {
            struct {
                uint8_t hours;
                uint8_t minutes;
                uint8_t seconds;
                uint8_t pad;
            };
            uint32_t all;
        };
        
        TimerController& controller_;
        TimeVal current_time_;
        TimeVal start_time_;
        
        uint8_t x_;
        uint8_t y_;
        
        State state_;
        
        bool updated_;
        bool visible_;
        
        friend void TimerInterruptHandler(void);
};

#endif // if !defined(__TIMER_H__)

