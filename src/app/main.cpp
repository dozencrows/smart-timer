// I2C master, reads out an attached RTC chip on I2C address 0x68.

#include "stdio.h"
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

#define LOOP_STEP_MS        64
#define BUZZER_GPIO         5

#define MCP23017_I2C_ADDR   0x20

#define MCP23017_IODIRA     0x00
#define MCP23017_IODIRB     0x01
#define MCP23017_GPIOA      0x12
#define MCP23017_GPIOB      0x13
#define MCP23017_IPOLA      0x02
#define MCP23017_IPOLB      0x03
#define MCP23017_GPPUA      0x0c
#define MCP23017_GPPUB      0x0d

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

static uint8_t i2cReadRegister (uint8_t addr, uint8_t reg) {
    uint8_t buf [4];

    I2C_PARAM_T param;
    I2C_RESULT_T result;

    buf[0] = (addr << 1) | 1;
    buf[1] = reg;

    param.num_bytes_send  = 2;
    param.num_bytes_rec   = 2;
    param.buffer_ptr_send = param.buffer_ptr_rec = buf;
    param.stop_flag       = 1;

    if (LPC_I2CD_API->i2c_master_tx_rx_poll(ih, &param, &result) != LPC_OK)
        error("i2c_master_tx_rx_poll");

    return buf[1];
}

static void i2cWriteRegister(uint8_t addr, uint8_t reg, uint8_t val) {
    uint8_t buf [4];
    
    I2C_PARAM_T param;
    I2C_RESULT_T result;

    buf[0] = (addr << 1) | 1;
    buf[1] = reg;
    buf[2] = val;

    /* Setup parameters for transfer */
    param.num_bytes_send  = 3;
    param.num_bytes_rec   = 0;
    param.buffer_ptr_send = buf;
    param.buffer_ptr_rec  = NULL;
    param.stop_flag       = 1;

    if (LPC_I2CD_API->i2c_master_transmit_poll(ih, &param, &result) != LPC_OK)
        error("i2c_master_transmit_poll");
}

int main () { 
    timersInit();
    serial.init(LPC_USART0, 115200);
    delayMs(100);
    i2cSetup();
    
    LPC_SWM->PINENABLE0 |= 1<<6;            // disable RESET to allow GPIO_5
    
    Buzzer::Initialise();
    Timer::Initialise();
    Backlight::Initialise();

    i2cWriteRegister(MCP23017_I2C_ADDR, MCP23017_IODIRB, 0xff);  // 0-7: input
    i2cWriteRegister(MCP23017_I2C_ADDR, MCP23017_GPPUB, 0xff);   // 0-7: pull-up
    i2cWriteRegister(MCP23017_I2C_ADDR, MCP23017_IPOLB, 0xff);   // 0-7: invert

    lcdInit();
    
    Backlight backlight;
    Buzzer buzzer(BUZZER_GPIO);
    TimerController timer_controller(buzzer, backlight);
    
    mrt_interrupt_control(true);
    
    while (true) {
        uint8_t buttons = i2cReadRegister(MCP23017_I2C_ADDR, MCP23017_GPIOB);
        timer_controller.ProcessButtons(buttons);
        timer_controller.Update();
        delayMs(LOOP_STEP_MS);
    }
}

