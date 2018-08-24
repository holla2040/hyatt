/* This blinks LED 1A, top left */

/* uint8_t ms included below so debugging with locals/watch is possible */
/* printf doesn't go anywhere, or does it? */
/* using bank=0 */

/* references
    https://youtu.be/6ZnSUP0L4HE
*/

#include <stdio.h>
#include "project.h"

#define U3Addr   0x20
#define U4Addr   0x21

#define IODIRA   0x00
#define IOPOLA   0x02
#define GPINTENA 0x04
#define DEFVALA  0x06
#define INTCONA  0x08
#define IOCONA   0x0A
#define GPPUA    0x0C
#define INTFA    0x0E
#define INTCAPA  0x10
#define GPIOA    0x12
#define OLATA    0x14

#define IODIRB   0x01
#define GPIOB    0x13

uint8_t pushes;
uint8_t pushDisplayCount;


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

uint8_t testI2CAddress(uint8 address) {
    uint8_t status;
	status = MCP23017_MasterSendStart(address,1);
	MCP23017_MasterSendStop();
    status &= MCP23017_MSTR_ERR_LB_NAK;
	if(status) {
        return 0;    //	Status indicates error, so no address response
	}
    return 1;               //	Status shows no error
}
//***********************************************************************************//


CY_ISR(pushHandler) {
    pushes++;
    pushDisplayCount = 1;
    Pin_IO_INT_ClearInterrupt();
}

void pushDisplay() {
    pushDisplayCount = 0;
    regWrite(U3Addr,GPIOB,0x00); // led off
    CyDelay(200);
    for (uint8_t i = 0; i < pushes; i++) {
        regWrite(U3Addr,GPIOB,0x01); 
        CyDelay(50);
        regWrite(U3Addr,GPIOB,0x00); 
        CyDelay(50);
    }
}

#define FLASHDURATION 250

int main(void) {   
    int i;
    uint8_t k,l1,l2;
    CyGlobalIntEnable;
    
    pushes = 0;
    l1 = 0;
    l2 = 0;
       
    Pin_IO_INT_Int_StartEx(pushHandler);
    MCP23017_Start();
    IO_RESET_Write(0);
    CyDelay(1);
    IO_RESET_Write(1);
    
    
/*
    uint8_t address;
    for (address = 0; address < 128; address++) {
     testI2CAddress(address); // use debugger to see found devices
    }
*/
   
    regWrite(U3Addr,IODIRA,  0x77);         // set port A to input
    regWrite(U3Addr,IOPOLA,  0x77);         // invert logic on port A, 1 means switch pressed
    regWrite(U3Addr,GPPUA,   0x77);         // add pull-up to all port A
    regWrite(U3Addr,GPINTENA,0x77);         // enable interrupt on port A changes
    regWrite(U3Addr,DEFVALA, 0x00);         // 
    regWrite(U3Addr,INTCONA, 0x77);         // Change from DEFVAL, fires on button release
    regWrite(U3Addr,IOCONA,  0x77);         // INT pins connected, open-drain output 
    
    regWrite(U3Addr,IODIRB,  0x88);         // bit 0 to output
    regWrite(U3Addr,GPIOB,   0x77);         // led on
    
    regWrite(U4Addr,IODIRB,  0x88);         // bit 0 to output
    regWrite(U4Addr,GPIOB,   0x77);         // led on
 
    for(i=0;i<10;i++) {
        regWrite(U3Addr,GPIOB,0x77); // leds on
        regWrite(U4Addr,GPIOB,0x77); // leds on
        CyDelay(20);
        regWrite(U3Addr,GPIOB,0x00); // leds off
        regWrite(U4Addr,GPIOB,0x00); // leds off
        CyDelay(20);
    }
    
    regWrite(U3Addr,GPIOB,0x01);
    regWrite(U4Addr,GPIOB,0x01);
    CyDelay(FLASHDURATION);
    regWrite(U3Addr,GPIOB,0x02);
    regWrite(U4Addr,GPIOB,0x02);
    CyDelay(FLASHDURATION);
    regWrite(U3Addr,GPIOB,0x04);
    regWrite(U4Addr,GPIOB,0x04);
    CyDelay(FLASHDURATION);
    regWrite(U3Addr,GPIOB,0x10);
    regWrite(U4Addr,GPIOB,0x10);
    CyDelay(FLASHDURATION);
    regWrite(U3Addr,GPIOB,0x20);
    regWrite(U4Addr,GPIOB,0x20);
    CyDelay(FLASHDURATION);
    regWrite(U3Addr,GPIOB,0x40);
    regWrite(U4Addr,GPIOB,0x40);
    CyDelay(FLASHDURATION);
    regWrite(U3Addr,GPIOB,0x00);
    regWrite(U4Addr,GPIOB,0x00);
    CyDelay(FLASHDURATION);
    
    for(;;) {
        k = regRead(U3Addr,GPIOA)&0x07;
        if (k) {
            k |= k << 4;
            l1 ^= k;
            regWrite(U3Addr,GPIOB,l1);
            
            l2++;
            regWrite(U4Addr,GPIOB,l2);
            CyDelay(250);
       }
        /*
        l1 ^= (regRead(U3Addr,GPIOA)&k);
        regWrite(U3Addr,GPIOB,l1);
        l2 ^= regRead(U4Addr,GPIOA);
        regWrite(U4Addr,GPIOB,l2);
        */
        
/*
        // polling - 
        while (regRead(U3Addr,GPIOA) & 0x01) {};
        if (pushDisplayCount) {
            pushDisplay();
        }
*/
    }
}
