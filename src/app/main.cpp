//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "string.h"
#include "serial.h"

#include "lpc_types.h"
#include "romapi_8xx.h"

#include "util/timers.h"
#include "util/lcd.h"
#include "util/mrt_interrupt.h"

#include "timer_controller.h"
#include "buzzer.h"
#include "backlight.h"
#include "button_input.h"

#define LOOP_STEP_MS        64
#define BUZZER_GPIO         5
#define INPUT_I2C_ADDR      0x20

uint32_t SystemMainClock = FIXED_CLOCK_RATE_HZ;
uint32_t SystemCoreClock = FIXED_CLOCK_RATE_HZ;

const char hexdigit[] = "0123456789abcdef";

uint32_t i2cBuffer [24];
I2C_HANDLE_T* ih;

void error(const char* msg) {
    puts("\n**ERROR: ");
    puts(msg);
    putchar('\n');
}

void errorWithCode(const char* msg, int code) {
    puts("\n**ERROR: ");
    puts(msg);
    putchar(' ');
    
    for (int i = 24; i >= 0; i-=4) {
        putchar(hexdigit[(code >> i) & 0xf]);
    }
    putchar('\n');
}

static void i2cSetup () {
    LPC_SWM->PINENABLE0 |= 3<<2;            // disable SWCLK and SWDIO

    LPC_SWM->PINASSIGN7 = 0x02FFFFFF;       // SDA on P2, pin 4
    LPC_SWM->PINASSIGN8 = 0xFFFFFF03;       // SCL on P3, pin 3
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<5);    // enable I2C clock

    ih = LPC_I2CD_API->i2c_setup(LPC_I2C_BASE, i2cBuffer);
    if (ih == NULL)
        error("i2c_setup");
        
    if (LPC_I2CD_API->i2c_set_bitrate(ih, 12000000, 100000) != LPC_OK)
        error("i2c_set_bitrate");

    if (LPC_I2CD_API->i2c_set_timeout(ih, 100000) != LPC_OK)
        error("i2c_set_timeout");
}

static void configureLowPowerPins() {
    // Set PIO0_10 and 11 as low outputs, as they float.
    // This saves some power
    LPC_GPIO_PORT->DIR0 |= 3 << 10;
    LPC_GPIO_PORT->CLR0  = 3 << 10;
}

static void deepSleep() {
    LPC_SYSCON->STARTERP0   = 0x01; // pin interrupt 0 will wakeup 
    LPC_PMU->PCON           = 0x01; // select deep sleep
    SCB->SCR                = SCB_SCR_SLEEPDEEP_Msk;
    __WFI();
    LPC_PMU->PCON           = 0;
    SCB->SCR                = 0;
}

static void powerDown() {
    LPC_SYSCON->STARTERP0   = 0x01; // pin interrupt 0 will wakeup 
    LPC_PMU->PCON           = 0x02; // select power down
    SCB->SCR                = SCB_SCR_SLEEPDEEP_Msk;
    __WFI();
    LPC_PMU->PCON           = 0;
    SCB->SCR                = 0;
}

static void lcdPowerOn() {
    LPC_GPIO_PORT->B0[0] = 1;
}

static void lcdPowerOff() {
    LPC_GPIO_PORT->B0[0] = 0;
}

static void initLcdPowerSwitch() {
    LPC_SWM->PINASSIGN0 |= 0xff00;          // ensure pin 8 is not assigned to UART RXD  
    LPC_GPIO_PORT->DIR0 |= 1;               // set GPIO 0 output (to control LCD power)
    lcdPowerOn();
}

int main () {
    configureLowPowerPins(); 
    timersInit();
    serial.init(LPC_USART0, FIXED_UART_BAUD_RATE);
    delayMs(100);
    puts("Smart Timer");
    i2cSetup();
    
    LPC_SWM->PINENABLE0 |= 1<<6;            // disable RESET to allow GPIO_5 (for buzzer)
    initLcdPowerSwitch();
        
    Buzzer::Initialise();
    Timer::Initialise();
    Backlight::Initialise();
    ButtonInput::Initialise();

    lcdInit();
    
    Backlight       backlight;
    Buzzer          buzzer(BUZZER_GPIO);
    TimerController timer_controller(buzzer, backlight);
    ButtonInput     button_input(INPUT_I2C_ADDR);
    
    mrt_interrupt_control(true);
    backlight.DelayedOff(BACKLIGHT_ON_TIME_MS);
    
    while (true) {
        while (button_input.HasButtonStateChanged()) {
            timer_controller.ProcessButtons(button_input.GetButtonStates());
        }

        timer_controller.Update();
        
        if (!button_input.HasButtonStateChanged()) {
            if (timer_controller.IsIdle() && !backlight.IsOn()) {
                lcdPowerOff();
                powerDown();

                lcdPowerOn();
                delayMs(10);
                
                if (button_input.HasButtonStateChanged()) {
                    timer_controller.ProcessButtons(button_input.GetButtonStates());
                }
                
                lcdInit();
                timer_controller.ForceUpdate();
            }
            else {
                __WFI();
            }
        }
    }
}

