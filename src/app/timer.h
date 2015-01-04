/*
 * Timer: a basic count-down timer built on a state machine
 *
 */
#if !defined(__TIMER_H__)
#define __TIMER_H__

#include "lpc_types.h"

class Timer {
    public:
    
        enum State {
            STOPPED = 0,
            RUNNING,
            PAUSED,
            ALARM
        };
    
        Timer();
        
        static void Initialise();
        
        void SetCoords(uint8_t x, uint8_t y);
        void SetStartTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
        void Start();
        void Stop();
        void Reset();
        void Update();
    
    private:
        void Tick();
        
        uint8_t current_hours_;
        uint8_t current_minutes_;
        uint8_t current_seconds_;

        uint8_t start_hours_;
        uint8_t start_minutes_;
        uint8_t start_seconds_;
        
        uint8_t x_;
        uint8_t y_;
        
        State state_;
        
        bool updated_;
        
        friend void TimerInterruptHandler(void);
};

#endif // if !defined(__TIMER_H__)

