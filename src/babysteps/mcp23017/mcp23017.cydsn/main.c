/* This blinks LED 1A, top left */

/* uint8_t ms included below so debugging with locals/watch is possible */
/* printf doesn't go anywhere, or does it? */
/* using bank=0 */

#include <stdio.h>
#include "project.h"

#define IOADDR 0x20

#define IODIRA 0x00
#define IOPOLA 0x02

#define IODIRB 0x01
#define GPIOA  0x12
#define GPIOB  0x13
#define GPPUA  0x0C

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

    regWrite(IOADDR,IODIRA,0xFF);   // set port A to input
    regWrite(IOADDR,IOPOLA,0xFF);   // invert logic on port A, 1 means switch pressed
    regWrite(IOADDR,GPPUA,0xFF);    // add pull-up to all port A
    regWrite(IOADDR,IODIRB,0xFE);   // bit 0 to output
    regWrite(IOADDR,GPIOB,0x01);    // led on

    for(;;) {
        regWrite(IOADDR,GPIOB,0x01); // led on
        CyDelay(500);
        regWrite(IOADDR,GPIOB,0x00); // led off
        CyDelay(500);
        
        while (regRead(IOADDR,GPIOA) & 0x01) {};
    }
}
