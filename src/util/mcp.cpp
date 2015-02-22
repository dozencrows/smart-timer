//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * Implementation of MCP23008 and MCP23017 GPIO expander interfacing
 */
 
#include "stdio.h"
#include "mcp.h"
#include "romapi_8xx.h"

extern void error(const char*);

extern I2C_HANDLE_T* ih;

uint8_t mcpReadRegister (uint8_t addr, uint8_t reg) {
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

void mcpWriteRegister(uint8_t addr, uint8_t reg, uint8_t val) {
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

