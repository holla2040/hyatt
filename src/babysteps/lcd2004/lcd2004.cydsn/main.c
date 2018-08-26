/* references
    backpack uses NXP PCF8574
    https://www.electroschematics.com/12459/arduino-i2c-lcd-backpack-introductory-tutorial/
    https://youtu.be/gz-BboycONk
    
    https://medium.com/@ubidefeo/arduino-cheap-i2c-wire-lcd-backpack-4335927e6887
    
    add m library gcc command line -lm
    change heap size to 0x200 in cydwr System
    add linker command line -u_printf_float
*/

#include <stdio.h>
#include "project.h"
#include <math.h>
#include "LCD.h"

#define lcdAddr   0x27

int main(void) {
    char status[100];
    double t = 0.5;
    
    CyGlobalIntEnable;
    
    I2C_Start();
    LCD_Start(lcdAddr,20,4,0);

    LCD_SetCursor(0,0);
    LCD_Clear();
    LCD_SetCursor(0,0);     LCD_PutString("X");
    LCD_SetCursor(0,1);     LCD_PutString("Y");
    LCD_SetCursor(0,2);     LCD_PutString("Z");
    LCD_SetCursor(0,3);     LCD_PutString("G54");
    LCD_SetCursor(12,0);    LCD_PutString("RUNNING");
    LCD_SetCursor(12,1);    LCD_PutString("M0 M5 M9");
    LCD_SetCursor(12,2);    LCD_PutString("T4 F750");
    LCD_SetCursor(12,3);    LCD_PutString("G1/21/90");

    for(;;) {
        LCD_SetCursor(2,0);
        sprintf(status,"%9.4f",500.0*cos(t)+500.0);
        LCD_PutString(status);
        
        LCD_SetCursor(2,1);
        sprintf(status,"%9.4f",500.0*sin(t)+500.0);
        LCD_PutString(status);
        
        LCD_SetCursor(2,2);
        sprintf(status,"%9.4f",50.0*sin(t + 3.00)-50.0);
        LCD_PutString(status);
        
        CyDelay(250);
        t += 0.001;
    }
}
