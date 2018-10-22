
#include "project.h"
#define KEYPAD_ROW12_ADDR      0x20                                                                                                                       
#define KEYPAD_ROW34_ADDR      0x21
#define DISPLAY_ADDR           0x27

int main(void){
    uint8_t error;
    
    CyGlobalIntEnable; 
    uart_Start();
    uart_PutString("\x1B[2J\x1b[H"); // clear screen
    CyDelay(500);
    uart_PutString("i2cScan begin\n");

    i2c_Start();
    
    error = i2c_MasterSendStart(KEYPAD_ROW12_ADDR,0);
    if (error == i2c_MSTR_NO_ERROR) {
        uart_PutString("KEYPAD_ROW12 found\n");
    }
    if (error != i2c_MSTR_NO_ERROR) {
        uart_PutString("KEYPAD_ROW12 not found\n");
    }
    i2c_MasterSendStop();
    
    error = i2c_MasterSendStart(KEYPAD_ROW34_ADDR,0);
    if (error == i2c_MSTR_NO_ERROR) {
        uart_PutString("KEYPAD_ROW34 found\n");
    }
    if (error != i2c_MSTR_NO_ERROR) {
        uart_PutString("KEYPAD_ROW34 not found\n");
    }
    i2c_MasterSendStop();

    
    
    error = i2c_MasterSendStart(DISPLAY_ADDR,0);
    if (error == i2c_MSTR_NO_ERROR) {
        uart_PutString("DISPLAY      found\n");
    }
    if (error != i2c_MSTR_NO_ERROR) {
        uart_PutString("DISPLAY      not found\n");
    }
    i2c_MasterSendStop();
    
    
    uart_PutString("i2cScan done\n");
    
    for(;;) {
    
    }
}
