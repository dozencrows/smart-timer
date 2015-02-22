//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/* MCP23008 & MCP23017 GPIO Extender Module */
#if !defined(__MCP_H__)
#define __MCP_H__

#include "lpc_types.h"

#define MCP23017_IODIRA     0x00
#define MCP23017_IODIRB     0x01
#define MCP23017_GPIOA      0x12
#define MCP23017_GPIOB      0x13
#define MCP23017_IPOLA      0x02
#define MCP23017_IPOLB      0x03
#define MCP23017_GPPUA      0x0c
#define MCP23017_GPPUB      0x0d

#define MCP23008_IODIR      0x00
#define MCP23008_IPOL       0x01
#define MCP23008_GPINTEN    0x02
#define MCP23008_DEFVAL     0x03
#define MCP23008_INTCON     0x04
#define MCP23008_IOCON      0x05
#define MCP23008_GPPU       0x06
#define MCP23008_INTF       0x07
#define MCP23008_INTCAP     0x08
#define MCP23008_GPIO       0x09
#define MCP23008_OLAT       0x0A

extern uint8_t mcpReadRegister (uint8_t addr, uint8_t reg);
extern void mcpWriteRegister(uint8_t addr, uint8_t reg, uint8_t val);


#endif // #if !defined(__MCP_H__)

