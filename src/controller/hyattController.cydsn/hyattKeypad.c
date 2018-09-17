#include <stdint.h>
#include "project.h"
#include "hyattKeypad.h"
#include "hyattI2C.h"
#include "hyattController.h"

/* using bank=0 */

uint8_t keyPending;
uint16_t keyIndicator;

uint16_t key;

uint16_t keyPadIndicatorCount;
#define KEYINDICATORUPDATEINTERVAL 23

CY_ISR(keyHandler) {
    keyPending = 1;
    Pin_IO_INT_ClearInterrupt();
}

void indicatorUpdate() {
    i2cRegWrite(KEYPAD_UPPER_ADDR,IOB_GPIO,     keyIndicator    &0xFF);
    CyDelay(50);
    i2cRegWrite(KEYPAD_UPPER_ADDR,IOB_GPIO,    (keyIndicator>>8)&0xFF);
};    

void hyattKeypadInit() {
    i2c_init();
    Pin_IO_INT_Int_StartEx(keyHandler);
    
    i2cRegWrite(KEYPAD_UPPER_ADDR,IOA_DIR,     0x77);         // set port A to input
    i2cRegWrite(KEYPAD_UPPER_ADDR,IOA_POL,     0x77);         // invert logic on port A, 1 means switch pressed
    i2cRegWrite(KEYPAD_UPPER_ADDR,IOA_GPPU,    0x77);         // add pull-up to all port A
    i2cRegWrite(KEYPAD_UPPER_ADDR,IOA_GPINTEN, 0x77);         // enable interrupt on port A changes
    i2cRegWrite(KEYPAD_UPPER_ADDR,IOA_DEFVAL,  0x00);         // 
    i2cRegWrite(KEYPAD_UPPER_ADDR,IOA_INTCON,  0x77);         // Change from DEFVAL, fires on button release
    i2cRegWrite(KEYPAD_UPPER_ADDR,IOA_CON,     0x77);         // INT pins connected, open-drain output 
    
    i2cRegWrite(KEYPAD_UPPER_ADDR,IOB_DIR,     0x88);         // led pins to output
    i2cRegWrite(KEYPAD_UPPER_ADDR,IOB_GPIO,    0x77);         // leds on
    


    i2cRegWrite(KEYPAD_UPPER_ADDR,IOA_DIR,     0x77);         // set port A to input
    i2cRegWrite(KEYPAD_UPPER_ADDR,IOA_POL,     0x77);         // invert logic on port A, 1 means switch pressed
    i2cRegWrite(KEYPAD_UPPER_ADDR,IOA_GPPU,    0x77);         // add pull-up to all port A
    i2cRegWrite(KEYPAD_UPPER_ADDR,IOA_GPINTEN, 0x77);         // enable interrupt on port A changes
    i2cRegWrite(KEYPAD_UPPER_ADDR,IOA_DEFVAL,  0x00);         // 
    i2cRegWrite(KEYPAD_UPPER_ADDR,IOA_INTCON,  0x77);         // Change from DEFVAL, fires on button release
    i2cRegWrite(KEYPAD_UPPER_ADDR,IOA_CON,     0x77);         // INT pins connected, open-drain output 
    
    i2cRegWrite(KEYPAD_UPPER_ADDR,IOB_DIR,     0x88);         // led pins to output
    i2cRegWrite(KEYPAD_UPPER_ADDR,IOB_GPIO,    0x77);         // leds on

    CyDelay(100);
        
    keyIndicator = 0x0001;
    for (uint8_t i = 0;i < 15;i++) {
        indicatorUpdate();
        keyIndicator = keyIndicator << 1;
        CyDelay(100);
    }
}

void hyattKeypadLoop() {
    if (keyPending) {
        key  = i2cRegRead(KEYPAD_UPPER_ADDR,IOA_INTCAP)<<8;
        key |= i2cRegRead(KEYPAD_UPPER_ADDR,IOA_INTCAP);
        switch(key) {
            case KEY_X:
                controllerConfig.axisSelected = AXISSELECTEDX;
                break;
            case KEY_Y:
                controllerConfig.axisSelected = AXISSELECTEDY;
                break;
            case KEY_Z:
                controllerConfig.axisSelected = AXISSELECTEDZ;
                break;
        }

        keyPending = 0;
    }
    
    if (keyPadIndicatorCount > KEYINDICATORUPDATEINTERVAL) {
        
        // clearing 23017 interrupt
        i2cRegRead(KEYPAD_UPPER_ADDR, IOA_GPIO);
        i2cRegRead(KEYPAD_UPPER_ADDR, IOA_GPIO);
            
        keyIndicator = keyIndicator & ~(KEY_X|KEY_Y|KEY_Z);
        switch (controllerConfig.axisSelected) {
            case AXISSELECTEDX:
                keyIndicator |= KEY_X;
                break;
            case AXISSELECTEDY:
                keyIndicator |= KEY_Y;
                break;
            case AXISSELECTEDZ:
                keyIndicator |= KEY_Z;
                break;
        }
        indicatorUpdate();
        
        FEED_OVERRIDE_BTN_Write(!FEED_OVERRIDE_BTN_Read());
        
        keyPadIndicatorCount = 0;
    }
}

