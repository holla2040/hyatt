/* references
    backpack uses NXP PCF8574
    https://www.electroschematics.com/12459/arduino-i2c-lcd-backpack-introductory-tutorial/
    https://youtu.be/gz-BboycONk
    
    https://medium.com/@ubidefeo/arduino-cheap-i2c-wire-lcd-backpack-4335927e6887
    
*/

#include <stdio.h>
#include "project.h"
#include "LCD.h"

#define lcdAddr   0x27

int main(void) {
    uint16_t count = 0;
    char status[100];
    CyGlobalIntEnable;
    
    I2C_Start();
    LCD_Start(lcdAddr,20,4,0);

    LCD_PutString("lcd2004");
    LCD_SetCursor(1,1);
    LCD_PutString("lcd");
    LCD_SetCursor(2,2);
    LCD_PutString("20");
    LCD_SetCursor(3,3);
    LCD_PutString("04");

    for(;;) {
        LCD_SetCursor(15,0);
        sprintf(status,"%d",count++);
        LCD_PutString(status);
        CyDelay(250);
    }
}
