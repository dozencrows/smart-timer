//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * Implementation of buzzer controller
 */
 
#include "buzzer.h"

#include "LPC8xx.h"

#define BUZZER_CONTINUOUS_TONE

#define MAX_BUZZER_INSTANCES    1
#define BUZZER_MASK             1
#define BEEP_LENGTH_MS          125

#if defined(BUZZER_CONTINUOUS_TONE)
#define INT_PERIOD_MS           25
#define INT_RATE                (1000 / INT_PERIOD_MS)
#define BEEP_DELAY              (BEEP_LENGTH_MS / INT_PERIOD_MS)
#else
#define BUZZER_FREQ_HZ          2000
#define INT_RATE                (BUZZER_FREQ_HZ * 2)
#define BEEP_DELAY              ((INT_RATE / 1000) * BEEP_LENGTH_MS)
#endif

#define SYSTICK_COUNTER         (FIXED_CLOCK_RATE_HZ / INT_RATE)

//----------------------------------------------------------------------------------------
// Interrupt handling
//

static int buzzer_instance_count = 0;
static Buzzer* buzzer_instances[MAX_BUZZER_INSTANCES];

void BuzzerInterruptHandler()
{
    for (int i = 0; i < buzzer_instance_count; i++) {
        buzzer_instances[i]->Update();
    }
}

extern "C" void SysTick_Handler () {
    BuzzerInterruptHandler();
}

//----------------------------------------------------------------------------------------
// Class implementation
//

void Buzzer::Initialise() {
    //SysTick_Config(SystemCoreClock/INT_RATE);
}

Buzzer::Buzzer(uint8_t gpio) : gpio_(gpio){
    flag_   = 0;
    state_  = 0;
    mode_   = OFF;
    delay_  = 0;

    LPC_GPIO_PORT->DIR0 |= 1 << gpio_;

    if (buzzer_instance_count < MAX_BUZZER_INSTANCES) {
        buzzer_instances[buzzer_instance_count++] = this;
    }
}

void Buzzer::Update() {
    switch(mode_) {
        case BEEP:
            delay_--;
            if (!delay_) {
                Off();
            }
            break;
            
        case BEEPS:
            delay_--;
            if (!delay_) {
                flag_ ^= BUZZER_MASK;
                delay_ = BEEP_DELAY;
            }
            break;
    }
    
#if defined(BUZZER_CONTINUOUS_TONE)
    LPC_GPIO_PORT->B0[gpio_] = flag_;
#else
    state_ ^= BUZZER_MASK;
    LPC_GPIO_PORT->B0[gpio_] = state_ & flag_;
#endif
}

void Buzzer::On() {
    flag_ = BUZZER_MASK;
    mode_ = CONTINUOUS;
    SysTick_Config(SYSTICK_COUNTER);
}

void Buzzer::Off() {
    SysTick->CTRL = 0;
    flag_ = 0;
    mode_ = OFF;
    LPC_GPIO_PORT->B0[gpio_] = 0;
}

void Buzzer::Beep() {
    flag_   = BUZZER_MASK;
    delay_  = BEEP_DELAY;
    mode_   = BEEP;
    SysTick_Config(SYSTICK_COUNTER);
}

void Buzzer::Beeps() {
    flag_   = BUZZER_MASK;
    delay_  = BEEP_DELAY;
    mode_   = BEEPS;
    SysTick_Config(SYSTICK_COUNTER);
}

