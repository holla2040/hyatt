/* This blinks LED 1A, top left */

/* uint8_t ms included below so debugging with locals/watch is possible */
/* printf doesn't go anywhere, or does it? */

#include <stdio.h>
#include "project.h"

#define IOADDR 0x20
#define IODIRB 0x01
#define GPIOB  0x13

void regWrite(uint8_t addr, uint8_t reg, uint8_t value) {
    uint8_t wBuf[2] = {0};
    wBuf[0] = reg;
    wBuf[1] = value;
    MCP23017_MasterWriteBuf(addr,(uint8_t *)wBuf,2,MCP23017_MODE_COMPLETE_XFER);
    while ((MCP23017_MasterStatus() & MCP23017_MSTAT_WR_CMPLT)==0) {};
}    

uint8_t regRead(uint8_t addr, uint8_t reg) {
    uint8_t wBuf[1] = {0};
    uint8_t rBuf[1] = {0};
    wBuf[0] = reg;
    MCP23017_MasterWriteBuf(addr,(uint8_t *)wBuf,1,MCP23017_MODE_NO_STOP);
    while ((MCP23017_MasterStatus() & MCP23017_MSTAT_WR_CMPLT)==0) {};
    MCP23017_MasterReadBuf(addr,(uint8_t *)rBuf,1,MCP23017_MODE_REPEAT_START);
    while ((MCP23017_MasterStatus() & MCP23017_MSTAT_RD_CMPLT)==0) {};
    return rBuf[0];
}

int main(void)
{
    CyGlobalIntEnable;
    
    MCP23017_Start();
    
    regWrite(IOADDR,IODIRB,0xFE); // bit 0 to output
    for(;;) {
        regWrite(IOADDR,GPIOB,0x01); // led on
        CyDelay(500);
        regWrite(IOADDR,GPIOB,0x00); // led off
        CyDelay(500);
    }
}
