/* MCP23017 GPIO Extender Module */
#if !defined(__MCP23017_H__)
#define __MCP23017_H__

#include "lpc_types.h"

#define MCP23017_IODIRA     0x00
#define MCP23017_IODIRB     0x01
#define MCP23017_GPIOA      0x12
#define MCP23017_GPIOB      0x13
#define MCP23017_IPOLA      0x02
#define MCP23017_IPOLB      0x03
#define MCP23017_GPPUA      0x0c
#define MCP23017_GPPUB      0x0d

extern uint8_t mcp23017ReadRegister (uint8_t addr, uint8_t reg);
extern void mcp23017WriteRegister(uint8_t addr, uint8_t reg, uint8_t val);


#endif // #if !defined(__MCP23017_H__)

