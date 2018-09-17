#ifndef i2c_h
#define i2c_h
#include <stdint.h>    
 
void i2c_init();
void i2cRegWrite(uint8_t addr, uint8_t reg, uint8_t value);
uint8_t i2cRegRead(uint8_t addr, uint8_t reg);

void i2cBufWrite(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len);

#endif