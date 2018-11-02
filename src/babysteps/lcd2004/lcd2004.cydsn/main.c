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
const char watch[] = " \xa5"; // a5 is a center dot, see lcd char set in manual
uint8_t watchCount;

uint8_t coord = 0;
uint8_t unit  = 1;
uint8_t state = 0;
uint8_t spindle = 1;
uint8_t coolant = 0;
uint16_t feed = 234;
uint8_t  override = 37;

#define ONDATA 0x09  //(LCD_BACKLIGHT | 0x01)

char *stateString() { // copied from hyattControlPanelDisplay
    switch (state) {
        case 0x00: return "IDLE ";
        case 0x01: return "ALARM";
        case 0x02: return "CHECK";
        case 0x04: return "HOME ";
        case 0x08: return "RUN  ";
        case 0x10: return "HOLD ";
        case 0x20: return "JOG  ";
        case 0x40: return "DOOR ";
        case 0x80: return "SLEEP";
    }
    return 0;
}

void home() {
    // need to write (LCD_SETDDRAMADDR(0x80) + col row), see LCD_SetCursor(0,0)
    char buffer[] = {
        0x88, // LCD_SETDDRAMADDR_high + backlight
        0x8C, // above + en
        0x88, // above
        0x08, // LCD_SETDDRAMADDR_low + backlight
        0x0C, // above + en
        0x08
    };
    I2C_MasterWriteBuf(lcdAddr,(uint8_t *)buffer,6,I2C_MODE_COMPLETE_XFER);
    while ((I2C_MasterStatus() & I2C_MSTAT_WR_CMPLT)==0) {}; // wait until command is sent
}

int main(void) {
    char status[100];
    char buffer[400];
    char *bptr,*sptr;
    double t = 0.5;
    uint8_t err,c;
    double x,y,z;
    char last[21];
    
    CyGlobalIntEnable;
    
    I2C_Start();
    LCD_Start(lcdAddr,20,4,0);

    LCD_Clear();
    CyDelay(1);

     for (;;) {
        bptr = buffer;
        sptr = status;
        x = 500.0*cos(t)+500.0;
        y = 500.0*sin(t)+500.0;
        z = 50.0*cos(t) + 50.0;
        
        home(); // sets DDRAM address to 0, which is upper corner

        TIMING_Write(0);

        do {
            c = (*sptr & 0xF0) | ONDATA; 
            *bptr++ = c | En;
            *bptr++ = c & ~En;
            c = (*sptr << 4) | ONDATA;
            *bptr++ = c | En;
            *bptr++ = c & ~En;
        } while (*sptr++);
        
        TIMING_Write(1);
        I2C_MasterSendStop(); // just in case someone left i2c in a bad way, naughty naughty
        err = I2C_MasterWriteBuf(lcdAddr,(uint8_t *)buffer,160,I2C_MODE_COMPLETE_XFER); // write the 1st half, line 0 and 2
        
        while ((I2C_MasterStatus() & I2C_MSTAT_WR_CMPLT)==0) {};
        CyDelayUs(10); // display needs this for a internal refresh or something
        err = I2C_MasterWriteBuf(lcdAddr,(uint8_t *)&buffer[160],160,I2C_MODE_COMPLETE_XFER); // write the 2nd half, line 1 and 3

        snprintf(last,20,"G0X%-.4fY%-.4f",x,y); // format this outside the timing loop, grbl's parser generates this as string
        CyDelay(250); // lcd refresh interval
        t += 0.001;
    }

/*
        i = 0;
        for (uint8_t j = 0;j < strlen(status);j++) {
            c = (status[j] & 0xF0) | (LCD_BACKLIGHT | 0x01); 
            buffer[i++] = c | En;
            buffer[i++] = c & ~En;
            c = (status[j] << 4) | (LCD_BACKLIGHT | 0x01);
            buffer[i++] = c | En;
            buffer[i++] = c & ~En;
        }
*/

      
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


/*
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
*/
