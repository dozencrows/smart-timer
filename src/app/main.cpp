// I2C master, reads out an attached RTC chip on I2C address 0x68.

#include "stdio.h"
#include "string.h"
#include "serial.h"

#include "lpc_types.h"
#include "romapi_8xx.h"

#include "util/timers.h"
#include "util/lcd.h"
#include "util/mrt_interrupt.h"
#include "util/mcp.h"

#include "timer_controller.h"
#include "buzzer.h"
#include "backlight.h"

#define LOOP_STEP_MS        64
#define BUZZER_GPIO         5

#define MCP_I2C_ADDR        0x20

uint32_t SystemMainClock = 12000000;
uint32_t SystemCoreClock = 12000000;

const char hexdigit[] = "0123456789abcdef";

uint32_t i2cBuffer [24];
I2C_HANDLE_T* ih;

void error(const char* msg) {
    printf("\n**ERROR: %s\n", msg);
}

void errorWithCode(const char* msg, int code) {
    printf("\n**ERROR: %s (%d)\n", msg, code);
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

static bool buttonIRQ = false;

extern "C" void PININT0_IRQHandler(void) {
    NVIC_ClearPendingIRQ(PININT0_IRQn);
    if (LPC_PIN_INT->FALL & 1) {
        LPC_PIN_INT->FALL = 1;
        buttonIRQ = true;
    }
}

int main () { 
    timersInit();
    serial.init(LPC_USART0, 115200);
    delayMs(100);
    i2cSetup();
    
    LPC_SWM->PINENABLE0 |= 1<<6;            // disable RESET to allow GPIO_5

    // Set up PIO0_1 (pin 5) for GPIO input. 
    // On reset:
    // * its PINENABLE0 bit is 1 so no special function is active - no change needed
    // * GPIO means no movable function should be assigned to this pin - no change needed
    // * Input GPIO means 0 in bit for direction register - no change needed
        
    Buzzer::Initialise();
    Timer::Initialise();
    Backlight::Initialise();

    mcpWriteRegister(MCP_I2C_ADDR, MCP23008_IODIR, 0xff);   // 0-7: input
    mcpWriteRegister(MCP_I2C_ADDR, MCP23008_GPPU, 0xff);    // 0-7: pull-up
    mcpWriteRegister(MCP_I2C_ADDR, MCP23008_IPOL, 0xff);    // 0-7: invert
    mcpWriteRegister(MCP_I2C_ADDR, MCP23008_GPINTEN, 0xff); // 0-7: interrupt on change. Interrupt pin is active low

    // Configure pin interrupt for PIO0_1
    LPC_SYSCON->PINTSEL[0]      = 1;        // Pin interrupt 0 from PIO0_1
    LPC_PIN_INT->ISEL           = 0;        // Level sensitive (1 bit per pin interrupt)
    LPC_PIN_INT->IENF           = 1;        // Falling level   (1 bit per pin interrupt)
    LPC_SYSCON->SYSAHBCLKCTRL  |= 1<<6;   // Turn on clock to pin interrupts block (already 1 after reset)
    NVIC_EnableIRQ(PININT0_IRQn);           // Enable pin interrupt in NVIC (#24)

    lcdInit();
    
    Backlight       backlight;
    Buzzer          buzzer(BUZZER_GPIO);
    TimerController timer_controller(buzzer, backlight);
    
    mrt_interrupt_control(true);
    backlight.DelayedOff(BACKLIGHT_ON_TIME_MS);
    
    while (true) {
        if (buttonIRQ) {
            buttonIRQ = false;
            uint8_t buttons = mcpReadRegister(MCP_I2C_ADDR, MCP23008_GPIO);
            timer_controller.ProcessButtons(buttons);
        }
        
        timer_controller.Update();
        __WFI();
    }
}

