/*

 Module for controlling LCDs via I2C backpack from Bitsbox
 
 I2C backpack:
    PCF8574 port expander chip.
    Pins from Arduino info page: 
    
        en rw rs d4 d5 d6 d7 bl
        2  1  0  4  5  6  7  3

    I2C address would appear to be 0x27
    
 LCD controller:
    Should be set to 4-bit interface mode
    
*/

#include "stdio.h"

#include "lpc_types.h"
#include "romapi_8xx.h"

#include "timers.h"
#include "lcd.h"

// ---------------------------------------------------------------------------
// External functions in other modules
//

extern I2C_HANDLE_T* ih;
extern void error(const char* msg);
extern void errorWithCode(const char* msg, int code);

// ---------------------------------------------------------------------------
// Backpack control
//

#define I2C_ADDR        0x27
#define PIN_EN          2
#define PIN_RW          1
#define PIN_RS          0
#define PIN_D4          4
#define PIN_D5          5
#define PIN_D6          6
#define PIN_D7          7
#define PIN_BACKLIGHT   3

#define __EN            (1 << PIN_EN)
#define __RW            (1 << PIN_RW)
#define __RS            (1 << PIN_RS)
#define __BL            (1 << PIN_BACKLIGHT)

static void i2cWrite(uint8_t addr, uint8_t value) {
    uint8_t buf [4];
    
    I2C_PARAM_T param;
    I2C_RESULT_T result;

    buf[0] = (addr << 1) | 0;
    buf[1] = value;
    
    /* Setup parameters for transfer */
    param.num_bytes_send  = 2;
    param.num_bytes_rec   = 0;
    param.buffer_ptr_send = buf;
    param.buffer_ptr_rec  = NULL;
    param.stop_flag       = 1;

    ErrorCode_t err = LPC_I2CD_API->i2c_master_transmit_poll(ih, &param, &result);
    if (err != LPC_OK)
        errorWithCode("lcd:i2c_master_transmit_poll", err);
    
} 

// ---------------------------------------------------------------------------
// LCD control
//

#define WRITE_MODE_CMD      0
#define WRITE_MODE_DATA     1

// LCD Commands
#define LCD_CLEARDISPLAY        0x01
#define LCD_RETURNHOME          0x02
#define LCD_ENTRYMODESET        0x04
#define LCD_DISPLAYCONTROL      0x08
#define LCD_CURSORSHIFT         0x10
#define LCD_FUNCTIONSET         0x20
#define LCD_SETCGRAMADDR        0x40
#define LCD_SETDDRAMADDR        0x80

// flags for function set
#define LCD_8BITMODE            0x10
#define LCD_4BITMODE            0x00
#define LCD_2LINE               0x08
#define LCD_1LINE               0x00
#define LCD_5x10DOTS            0x04
#define LCD_5x8DOTS             0x00

// flags for display entry mode
#define LCD_ENTRYRIGHT          0x00
#define LCD_ENTRYLEFT           0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off and cursor control
#define LCD_DISPLAYON           0x04
#define LCD_DISPLAYOFF          0x00
#define LCD_CURSORON            0x02
#define LCD_CURSOROFF           0x00
#define LCD_BLINKON             0x01
#define LCD_BLINKOFF            0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE         0x08
#define LCD_CURSORMOVE          0x00
#define LCD_MOVERIGHT           0x04
#define LCD_MOVELEFT            0x00

#define LCD_CLEAR_TIME_US       2000

static uint8_t backlight_state = 0;

void lcdSetBacklight(int value) {
    if (value) {
        backlight_state = __BL;
    }
    else {
        backlight_state = 0;
    }
    
    i2cWrite(I2C_ADDR, backlight_state);
}

static const uint8_t backpack_lut[] = {
    0x00,
    1 << PIN_D4,
    1 << PIN_D5,
    1 << PIN_D4 | 1 << PIN_D5,
    1 << PIN_D6,
    1 << PIN_D6 | 1 << PIN_D4,
    1 << PIN_D6 | 1 << PIN_D5,
    1 << PIN_D6 | 1 << PIN_D5 | 1 << PIN_D4,
    1 << PIN_D7,
    1 << PIN_D7 | 1 << PIN_D4,
    1 << PIN_D7 | 1 << PIN_D5,
    1 << PIN_D7 | 1 << PIN_D4 | 1 << PIN_D5,
    1 << PIN_D7 | 1 << PIN_D6,
    1 << PIN_D7 | 1 << PIN_D6 | 1 << PIN_D4,
    1 << PIN_D7 | 1 << PIN_D6 | 1 << PIN_D5,
    1 << PIN_D7 | 1 << PIN_D6 | 1 << PIN_D5 | 1 << PIN_D4,
};

static void lcdWriteNybble(uint8_t value, uint8_t mode) {
    uint8_t backpack_value = backpack_lut[value & 0xf];
    
    if ( mode == WRITE_MODE_DATA )
    {
      mode = __RS;
    }

    backpack_value |= mode | backlight_state;

    i2cWrite(I2C_ADDR, backpack_value | __EN);
    i2cWrite(I2C_ADDR, backpack_value & ~__EN);   
}

static void lcdWriteByte(uint8_t value, uint8_t mode) {
    lcdWriteNybble(value >> 4, mode);
    lcdWriteNybble(value & 0x0f, mode);
}

void lcdInit() {
    // Bring all pins to 0 via I2C
    i2cWrite(I2C_ADDR, 0);
        
    // Ensure we meet minimum 40ms wait between power crossing 2.7V and
    // sending of first command
    delayMs(100);
    
    lcdWriteNybble(0x03, WRITE_MODE_CMD);
    delayMs(5);

    // second try
    lcdWriteNybble(0x03, WRITE_MODE_CMD);
    delayUs(150);

    // third go!
    lcdWriteNybble(0x03, WRITE_MODE_CMD);
    delayUs(150);

    // finally, set to 4-bit interface
    lcdWriteNybble(0x02, WRITE_MODE_CMD);
    delayUs(150);
    
    // Set up display mode
    uint8_t display_function = LCD_2LINE | LCD_5x8DOTS;    
    lcdWriteByte(LCD_FUNCTIONSET | display_function, WRITE_MODE_CMD);
    delayUs(60);
    
    uint8_t display_control = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;;
    lcdWriteByte(LCD_DISPLAYCONTROL | display_control, WRITE_MODE_CMD);

    lcdClear();    
    
    uint8_t display_mode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    lcdWriteByte(LCD_ENTRYMODESET | display_mode, WRITE_MODE_CMD);
}

void lcdClear() {
    lcdWriteByte(LCD_CLEARDISPLAY, WRITE_MODE_CMD);
    delayUs(LCD_CLEAR_TIME_US);
}

void lcdPuts(const char* s) {
    while (*s) {
        lcdWriteByte(*s++, WRITE_MODE_DATA);
    }
}

void lcdMoveTo(int x, int y) {
    uint8_t addr = (x & 0x0f) + (y & 0x01) * 0x40;
    lcdWriteByte(LCD_SETDDRAMADDR | addr, WRITE_MODE_CMD);
}

