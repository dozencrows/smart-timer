//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * LCD Backlight controller
 */
 
#if !defined(__BACKLIGHT_H__)
#define __BACKLIGHT_H__

#include "lpc_types.h"

class Backlight {
    public:
    
        Backlight();
        
        static void Initialise();
        
        void On();
        void DelayedOff(uint32_t delay_ms);
        bool IsOn();
        
    private:
    
        friend void BacklightInterruptHandler(void);
};

#endif // #if !defined(__BACKLIGHT_H__) 
