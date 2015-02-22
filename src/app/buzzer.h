//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * Buzzer: audible buzzer controller
 */
 
#if !defined(__BUZZER_H__)
#define __BUZZER_H__

#include "lpc_types.h"

class Buzzer {
    
    public:
        
        Buzzer(uint8_t gpio);
        
        static void Initialise();
        
        void On();
        void Off();
        void Beep();
        void Beeps();
        
    private:
        enum Mode {
            OFF,
            CONTINUOUS,
            BEEP,
            BEEPS
        };
        
        void Update();
    
        uint8_t     gpio_;
        uint8_t     flag_;
        uint8_t     state_;
        Mode        mode_;
        uint32_t    delay_;
        
        friend void BuzzerInterruptHandler();
};

#endif // #if !defined(__BUZZER_H__)
