/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"

#define IOADDR 0x20

int main(void)
{
    uint8_t wBuf[1] = {0};
    uint8_t rBuf[1] = {0};
    
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    MCP23017_Start();
    
    MCP23017_MasterWriteBuf(IOADDR,wBuf,1,MCP23017_MODE_NO_STOP);
    while ((MCP23017_MasterStatus() & MCP23017_MSTAT_WR_CMPLT)==0) {};
    MCP23017_MasterReadBuf(IOADDR,rBuf,1,MCP23017_MODE_REPEAT_START);
    while ((MCP23017_MasterStatus() & MCP23017_MSTAT_RD_CMPLT)==0) {};
    for(;;)
    {
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
