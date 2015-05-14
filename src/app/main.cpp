//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

// Port & pin usage:
//	Pin	Port	Dir	    Usage			        Pull	Default
//	-----------------------------------------------------------------
//	1	PIO0_5	In	    Reset			        Up	    Reset (in)
//	2	PIO0_4	Out	    Buzzer on/off, UART TXD	Up	    PIO0_4 (in), UART TXD in ISP
//	3	SCL	    Out	    I2C			            Up	    SWDIO
//	4	SDA	    In/Out	I2C			            Up	    SWCLK
//	5	PIO0_1	In	    Button interrupt	    Up	    PIO0_1 (in), ISP if low on reset
//	6	VDD		        Power
//	7	VSS		        Ground
//	8	PIO0_0	Out	    LCD power		        Up	    PIO0_0 (in), UART RXD in ISP
//

// Programming mode means:
//  Switch LCD power connection to UART RXD (and force actual power switch to high)
//  Switch Buzzer connection to UART TXD

// Debugging mode means
//  Switch Buzzer connection to UART TXD

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


// Set this define to add debugging aids in code, and configure UART
// output to replace buzzer control
//#define DEBUG

#define LOOP_STEP_MS        64
#define BUZZER_GPIO         4
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

    // Keep internal pull-ups on I2C pins, as this works and is less
    // power draining when LCD off and in low power state
    
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
#if defined(DEBUG)
    // Ensure UART TXD is on 
    LPC_SWM->PINASSIGN0 &= 0xffffff00;
    LPC_SWM->PINASSIGN0 |= 0x00000004;
#else
    LPC_SWM->PINASSIGN0 |= 0xff;
#endif

    configureLowPowerPins(); 
    timersInit();
    serial.init(LPC_USART0, FIXED_UART_BAUD_RATE);
    delayMs(100);
    puts("Smart Timer");
    i2cSetup();
    
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

