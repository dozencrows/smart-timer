//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * Button input reader using I2C GPIO extender
 */

#if !defined(__BUTTONINPUT_H__)
#define __BUTTONINPUT_H__

#include "lpc_types.h"

class ButtonInput {
    public:

        ButtonInput(uint8_t i2c_addr);
    
        static void Initialise();
        
        uint8_t GetButtonStates();
        bool HasButtonStateChanged();
        void DiscardNextState();
        
    private:
        uint8_t i2c_addr_;
        uint8_t button_state_;
};

#endif

