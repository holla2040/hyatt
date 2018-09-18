#include <stdint.h>
#include "project.h"

void i2cRegWrite(uint8_t addr, uint8_t reg, uint8_t value) {
    uint8_t wBuf[2] = {0};
    wBuf[0] = reg;
    wBuf[1] = value;
    I2C_MasterWriteBuf(addr,(uint8_t *)wBuf,2,I2C_MODE_COMPLETE_XFER);
    while ((I2C_MasterStatus() & I2C_MSTAT_WR_CMPLT)==0) {};
}    

uint8_t i2cRegRead(uint8_t addr, uint8_t reg) {
    uint8_t wBuf[1] = {0};
    uint8_t rBuf[1] = {0};
    wBuf[0] = reg;
    I2C_MasterWriteBuf(addr,(uint8_t *)wBuf,1,I2C_MODE_NO_STOP);
    while ((I2C_MasterStatus() & I2C_MSTAT_WR_CMPLT)==0) {};
    I2C_MasterReadBuf(addr,(uint8_t *)rBuf,1,I2C_MODE_REPEAT_START);
    while ((I2C_MasterStatus() & I2C_MSTAT_RD_CMPLT)==0) {};
    return rBuf[0];
}

void i2cBufWrite(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len) {
    I2C_MasterWriteBuf(addr,(uint8_t *)&reg,1,I2C_MODE_NO_STOP);
    while ((I2C_MasterStatus() & I2C_MSTAT_WR_CMPLT)==0) {};
    I2C_MasterWriteBuf(addr,buf,len,I2C_MODE_COMPLETE_XFER);
    while ((I2C_MasterStatus() & I2C_MSTAT_WR_CMPLT)==0) {};
}

void i2c_init() {
    I2C_Start();
    IO_RESET_Write(0);
    CyDelay(1);
    IO_RESET_Write(1);
}
