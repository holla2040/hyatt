#include <stdint.h>
#include "project.h"

void i2cRegWrite(uint8_t addr, uint8_t reg, uint8_t value) {
    I2C_MasterSendStart(addr, 0);
    I2C_MasterWriteByte(reg);
    I2C_MasterWriteByte(value);
    I2C_MasterSendStop();

    
    
/*    
    uint8_t wBuf[2] = {0};
    wBuf[0] = reg;
    wBuf[1] = value;
    I2C_MasterWriteBuf(addr,(uint8_t *)wBuf,2,I2C_MODE_COMPLETE_XFER);
    while ((I2C_MasterStatus() & I2C_MSTAT_WR_CMPLT)==0) {};
*/
}    

uint8_t i2cRegRead(uint8_t addr, uint8_t reg) {
    uint8_t rv;
    I2C_MasterSendStart(addr, I2C_WRITE_XFER_MODE);
    I2C_MasterWriteByte(reg);
    I2C_MasterSendRestart(addr,I2C_READ_XFER_MODE);
    rv = I2C_MasterReadByte(I2C_NAK_DATA);
    I2C_MasterSendStop();
    return rv;
 
/*
    uint8_t wBuf[1] = {0};
    uint8_t rBuf[1] = {0};
    wBuf[0] = reg;
    I2C_MasterWriteBuf(addr,(uint8_t *)wBuf,1,I2C_MODE_NO_STOP);
    while ((I2C_MasterStatus() & I2C_MSTAT_WR_CMPLT)==0) {};
    I2C_MasterReadBuf(addr,(uint8_t *)rBuf,1,I2C_MODE_REPEAT_START);
    while ((I2C_MasterStatus() & I2C_MSTAT_RD_CMPLT)==0) {};
    return rBuf[0];
*/
}

/* functional but not used
void i2cBufWrite(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len) {
    I2C_MasterWriteBuf(addr,(uint8_t *)&reg,1,I2C_MODE_NO_STOP);
    while ((I2C_MasterStatus() & I2C_MSTAT_WR_CMPLT)==0) {};
    I2C_MasterWriteBuf(addr,buf,len,I2C_MODE_COMPLETE_XFER);
    while ((I2C_MasterStatus() & I2C_MSTAT_WR_CMPLT)==0) {};
}
*/