#include <stdint.h>
#include "project.h"
#include "hyatt.h"
#include "serial.h"
#include "grbl.h"

/* using bank=0 */

uint8_t  keyPending;
uint16_t keyIndicator;

uint32_t timeoutKeypadUpdate;
#define KEYPADUPDATEINTERVAL 100

extern parser_block_t gc_block;

CY_ISR(keyHandler) {
    keyPending = 1;
    Pin_IO_INT_ClearInterrupt();
}

void hyattControlPanelIndicatorUpdate() {
    i2cRegWrite(KEYPAD_ROW12_ADDR,IOB_GPIO,     keyIndicator    &0xFF);
    CyDelay(50);
    i2cRegWrite(KEYPAD_ROW34_ADDR,IOB_GPIO,    (keyIndicator>>8)&0xFF);
};    

void hyattControlPanelKeypadInit() {
    Pin_IO_INT_Int_StartEx(keyHandler);
    
    i2cRegWrite(KEYPAD_ROW12_ADDR,IOA_DIR,     0x77);         // set port A to input
    i2cRegWrite(KEYPAD_ROW12_ADDR,IOA_POL,     0x77);         // invert logic on port A, 1 means switch pressed
    i2cRegWrite(KEYPAD_ROW12_ADDR,IOA_GPPU,    0x77);         // add pull-up to all port A
    i2cRegWrite(KEYPAD_ROW12_ADDR,IOA_GPINTEN, 0x77);         // enable interrupt on port A changes
    i2cRegWrite(KEYPAD_ROW12_ADDR,IOA_DEFVAL,  0x00);         // 
    i2cRegWrite(KEYPAD_ROW12_ADDR,IOA_INTCON,  0x77);         // Change from DEFVAL, fires on button release
    i2cRegWrite(KEYPAD_ROW12_ADDR,IOA_CON,     0x77);         // INT pins connected, open-drain output 
    
    i2cRegWrite(KEYPAD_ROW12_ADDR,IOB_DIR,     0x88);         // led pins to output
    i2cRegWrite(KEYPAD_ROW12_ADDR,IOB_GPIO,    0x77);         // leds on
    


    i2cRegWrite(KEYPAD_ROW34_ADDR,IOA_DIR,     0x77);         // set port A to input
    i2cRegWrite(KEYPAD_ROW34_ADDR,IOA_POL,     0x77);         // invert logic on port A, 1 means switch pressed
    i2cRegWrite(KEYPAD_ROW34_ADDR,IOA_GPPU,    0x77);         // add pull-up to all port A
    i2cRegWrite(KEYPAD_ROW34_ADDR,IOA_GPINTEN, 0x77);         // enable interrupt on port A changes
    i2cRegWrite(KEYPAD_ROW34_ADDR,IOA_DEFVAL,  0x00);         // 
    i2cRegWrite(KEYPAD_ROW34_ADDR,IOA_INTCON,  0x77);         // Change from DEFVAL, fires on button release
    i2cRegWrite(KEYPAD_ROW34_ADDR,IOA_CON,     0x77);         // INT pins connected, open-drain output 
    
    i2cRegWrite(KEYPAD_ROW34_ADDR,IOB_DIR,     0x88);         // led pins to output
    i2cRegWrite(KEYPAD_ROW34_ADDR,IOB_GPIO,    0x77);         // leds on

    CyDelay(50);
        
    keyIndicator = 0x0001;
    for (uint8_t i = 0;i < 16;i++) {
        hyattControlPanelIndicatorUpdate();
        keyIndicator = keyIndicator << 1;
        CyDelay(2);
    }
}

void hyattControlPanelKeypadLoop() {
    uint16_t key;
    int c;
    char buf[30];
    if (keyPending) {
        key = 0x00;
        if (i2cRegRead(KEYPAD_ROW34_ADDR,IOA_INTF)) {
            key  |= i2cRegRead(KEYPAD_ROW34_ADDR,IOA_INTCAP)<<8;
        }
        if (i2cRegRead(KEYPAD_ROW12_ADDR,IOA_INTF)) {
            key |= i2cRegRead(KEYPAD_ROW12_ADDR,IOA_INTCAP);
        }
        
        // mist key is immediate
        if (key == KEY_MIST) {
            system_set_exec_accessory_override_flag(EXEC_COOLANT_MIST_OVR_TOGGLE);
            hyattTimeoutKeypadUpdate = hyattTicks + 100;
        }

        if (sys.state == STATE_IDLE) {
            switch(key) {
                case KEY_X:
                    hyattAxisSelected = AXISSELECTED_X;
                    break;
                case KEY_Y:
                    hyattAxisSelected = AXISSELECTED_Y;
                    break;
                case KEY_Z:
                    hyattAxisSelected = AXISSELECTED_Z;
                    break;
                case KEY_SLOW:
                    hyattWheelStepSize = WHEELSTEPSIZE_SMALL;
                    break;
                case KEY_MEDIUM:
                    hyattWheelStepSize = WHEELSTEPSIZE_MEDIUM;
                    break;
                case KEY_FAST:
                    hyattWheelStepSize = WHEELSTEPSIZE_LARGE;
                    break;
                case KEY_COORDSELECT:
                    c = gc_state.modal.coord_select + 1;
                    if (c > 3) c = 0;
                    sprintf(buf,"G%d",54+c);
                    grblBlockSend(buf);
                    break;
                case KEY_UNIT:
                        (gc_block.modal.units == UNITS_MODE_INCHES) ? grblBlockSend("G21"):grblBlockSend("G20");
                    break;
                case KEY_AXISZERO:
                        sprintf(buf,"G10L20P%d_0",gc_state.modal.coord_select+1);
                        buf[8] = selectedAxisLetter();
                        grblBlockSend(buf);
                    break;
                case KEY_SPINDLE:
                        (gc_block.modal.spindle == SPINDLE_DISABLE) ? grblBlockSend("M3"):grblBlockSend("M5");
                    break;
                default:
                    keyIndicator = key & 0xFF88;
            }
        }
        hyattTimeoutDisplaySlowUpdate = 0;
        keyPending = 0;
    }
    
    if (hyattTicks > hyattTimeoutKeypadUpdate) {
        // clearing 23017 interrupt
        i2cRegRead(KEYPAD_ROW12_ADDR, IOA_GPIO);
        i2cRegRead(KEYPAD_ROW34_ADDR, IOA_GPIO);
        
        keyIndicator = 0x0000; // all off
        keyIndicator |= hyattAxisSelected | hyattWheelStepSize;
        keyIndicator |= (gc_block.modal.units?0:1) << 10;
        keyIndicator |= (gc_block.modal.spindle == SPINDLE_ENABLE_CW ?1:0) << 13;
        keyIndicator |= (gc_block.modal.coolant == COOLANT_MIST_ENABLE ?1:0) << 14;
        
        hyattControlPanelIndicatorUpdate();
        
        // FEED_OVERRIDE_BTN_Write(!FEED_OVERRIDE_BTN_Read()); // blinks the 059 LED
        
        if (sys.state == STATE_IDLE) {
            hyattTimeoutKeypadUpdate = hyattTicks + KEYPADUPDATEINTERVAL;
        } else {
            hyattTimeoutKeypadUpdate = hyattTicks + 2000;
        }
     }
}

