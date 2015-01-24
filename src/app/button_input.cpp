/*
 * GPIO extender button input implementation
 *
 * Sets up MCP23008 expander for all pins to be inputs with pull-ups.
 * Assumes MCP23008 interrupt pin is connected to PIO0_1 (pin 5 on LPC810)
 * PIO0_1 is left in default post-reset state:
 *  - its PINENABLE0 bit is 1 so no special function is active - no change needed
 *  - GPIO means no movable function should be assigned to this pin - no change needed
 *  - Input GPIO means 0 in bit for direction register - no change needed
 */
 
#include "button_input.h"

#include "LPC8xx.h"
#include "util/mcp.h"
#include "util/timers.h"

#define POST_READ_DELAY_MS  8

static volatile int buttonIRQCount = 0;

extern "C" void PININT0_IRQHandler(void) {
    NVIC_ClearPendingIRQ(PININT0_IRQn);
    if (LPC_PIN_INT->FALL & 1) {
        LPC_PIN_INT->FALL = 1;
        buttonIRQCount++;
    }
}

void ButtonInput::Initialise() {
    // Configure pin interrupt for PIO0_1
    LPC_SYSCON->PINTSEL[0]      = 1;        // Pin interrupt 0 from PIO0_1
    LPC_PIN_INT->ISEL           = 0;        // Level sensitive (1 bit per pin interrupt)
    LPC_PIN_INT->IENF           = 1;        // Falling level   (1 bit per pin interrupt)
    LPC_SYSCON->SYSAHBCLKCTRL  |= 1<<6;     // Turn on clock to pin interrupts block (already 1 after reset)
    NVIC_EnableIRQ(PININT0_IRQn);           // Enable pin interrupt in NVIC (#24)
}

ButtonInput::ButtonInput(uint8_t i2c_addr) : i2c_addr_(i2c_addr), button_state_(0) {
    mcpWriteRegister(i2c_addr_, MCP23008_IODIR, 0xff);   // 0-7: input
    mcpWriteRegister(i2c_addr_, MCP23008_GPPU, 0xff);    // 0-7: pull-up
    mcpWriteRegister(i2c_addr_, MCP23008_IPOL, 0xff);    // 0-7: invert
    mcpWriteRegister(i2c_addr_, MCP23008_GPINTEN, 0xff); // 0-7: interrupt on change. Interrupt pin is active low
}

uint8_t ButtonInput::GetButtonStates() {
    if (buttonIRQCount > 0) {
        __disable_irq();
        buttonIRQCount--;
        __enable_irq();
        button_state_ = mcpReadRegister(i2c_addr_, MCP23008_GPIO);
        delayMs(POST_READ_DELAY_MS);     // short delay to try to avoid I2C errors
    }
    
    return button_state_;
}

bool ButtonInput::HasButtonStateChanged() {
    return buttonIRQCount > 0;
}

