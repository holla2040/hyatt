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
    char buffer[400];
    int i,k;
    double t = 0.5;
    uint8_t err,s,c;
    
    CyGlobalIntEnable;
    
    I2C_Start();
    LCD_Start(lcdAddr,20,4,0);

    
    
    LCD_Clear();

    LCD_SetCursor(0,0); 
    sprintf(status,"%.4f",500.0*cos(t)+500.0);
    LCD_PutString(status);

    i = 0;
    for (uint8_t j = 0;j < strlen(status);j++) {
        c = status[j] & 0xF0; 
        buffer[i++] = c | En;
        buffer[i++] = c & ~En;
        c = status[j] << 4;
        buffer[i++] = c | En;
        buffer[i++] = c & ~En;
    }
    
    LCD_SetCursor(0,1);     
    err = I2C_MasterWriteBuf(lcdAddr,(uint8_t *)buffer,10,I2C_MODE_COMPLETE_XFER);
    CyDelay(100);
    I2C_MasterSendStop();
    s = I2C_MasterClearStatus();

    LCD_SetCursor(0,3); 
    sprintf(status,"0x%02X",err);
    LCD_PutString(status);

    LCD_SetCursor(10,3); 
    sprintf(status,"0x%02X",s);
    LCD_PutString(status);


    
    LCD_SetCursor(0,2); 
    sprintf(status,"%9.4f",500.0*cos(t)+500.0);
    LCD_PutString(status);

    LCD_SetCursor(10,0); 
    err = I2C_MasterSendStart(lcdAddr+1,0);
    I2C_MasterSendStop();
    sprintf(status,"0x%02X",err);
    LCD_PutString(status);

    k = 0;
    LCD_Clear();
//    LCD_PutString("X");
    for (;;) {
        //sprintf(status,"%d",(k++)%10);
        sprintf(status,"Four score and seven years ago our fathers brought forth on this continent, a na");
        //sprintf(status,"Four score and seven years ago our ");
        i = 0;
        for (uint8_t j = 0;j < strlen(status);j++) {
            c = (status[j] & 0xF0) | (LCD_BACKLIGHT | 0x01); 
            buffer[i++] = c | En;
            buffer[i++] = c & ~En;
            c = (status[j] << 4) | (LCD_BACKLIGHT | 0x01);
            buffer[i++] = c | En;
            buffer[i++] = c & ~En;
        }

        err = I2C_MasterWriteBuf(lcdAddr,(uint8_t *)buffer,160,I2C_MODE_COMPLETE_XFER);
        I2C_MasterSendStop();
        while ((I2C_MasterStatus() & I2C_MSTAT_WR_CMPLT)==0) {};  

        CyDelayUs(10);
        err = I2C_MasterWriteBuf(lcdAddr,(uint8_t *)&buffer[160],160,I2C_MODE_COMPLETE_XFER);
        I2C_MasterSendStop();
//        LCD_PutString(status);
        
      CyDelay(1000);
    }

    
/*    
      
    
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
*/
}
