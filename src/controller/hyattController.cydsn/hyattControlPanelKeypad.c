#include <stdint.h>
#include "project.h"
#include "hyatt.h"
#include "serial.h"
#include "grbl.h"

/* using bank=0 */

uint8_t  keyPending;
uint16_t keyIndicator;

extern uint32_t hyattTicks;
extern uint32_t timeoutDisplaySlowUpdate;

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
    for (uint8_t i = 0;i < 15;i++) {
        hyattControlPanelIndicatorUpdate();
        keyIndicator = keyIndicator << 1;
        CyDelay(10);
    }
}

void grblMessage(char *block) {
    for (uint8_t i = 0; i < strlen(block); i++) {
        rx_handler(block[i]);                                                                                                            
    }
}

uint32_t timeoutKeypadUpdate;

void hyattControlPanelKeypadLoop() {
    uint16_t key;
    char buf[30];
    if (keyPending) {
        key = 0x00;
        if (i2cRegRead(KEYPAD_ROW34_ADDR,IOA_INTF)) {
            key  |= i2cRegRead(KEYPAD_ROW34_ADDR,IOA_INTCAP)<<8;
        }
        if (i2cRegRead(KEYPAD_ROW12_ADDR,IOA_INTF)) {
            key |= i2cRegRead(KEYPAD_ROW12_ADDR,IOA_INTCAP);
        }
        switch(key) {
            case KEY_X:
                hyattStatus.axisSelected = AXISSELECTED_X;
                break;
            case KEY_Y:
                hyattStatus.axisSelected = AXISSELECTED_Y;
                break;
            case KEY_Z:
                hyattStatus.axisSelected = AXISSELECTED_Z;
                break;
            case KEY_SLOW:
                hyattStatus.jogWheelStepSize = JOGWHEELSTEPSIZE_SMALL;
                break;
            case KEY_MEDIUM:
                hyattStatus.jogWheelStepSize = JOGWHEELSTEPSIZE_MEDIUM;
                break;
            case KEY_FAST:
                hyattStatus.jogWheelStepSize = JOGWHEELSTEPSIZE_LARGE;
                break;
            case KEY_COORDSELECT:
                if (sys.state == STATE_IDLE) {
                    int c = gc_state.modal.coord_select + 1;
                    if (c > 3) c = 0;
                    sprintf(buf,"G%d\n",54+c);
                    grblMessage(buf);
                }
                break;
            case 0x4100:
                // strcpy(l,"$J=G91G21?1F1000\n");
                // l[9] = hyattStatus.axisSelected;
                break;
            case KEY_UNIT:
                if (sys.state == STATE_IDLE) {
                    (gc_block.modal.units == UNITS_MODE_INCHES) ? grblMessage("G21\n"):grblMessage("G20\n");
                }
                break;
            case KEY_AXISZERO:
                if (sys.state == STATE_IDLE) {
                    sprintf(buf,"G10L20P%d_0\n",gc_state.modal.coord_select+1);
                    if (hyattStatus.axisSelected == AXISSELECTED_X) buf[8] = 'X';
                    if (hyattStatus.axisSelected == AXISSELECTED_Y) buf[8] = 'Y';
                    if (hyattStatus.axisSelected == AXISSELECTED_Z) buf[8] = 'Z';
                    grblMessage(buf);
                }
                break;
            case KEY_SPINDLE:
                (gc_block.modal.spindle == SPINDLE_DISABLE) ? grblMessage("M3\n"):grblMessage("M5\n");
                break;
            case KEY_MIST:
                (gc_block.modal.coolant == COOLANT_DISABLE) ? grblMessage("M7\n"):grblMessage("M9\n");
                break;
            default:
                keyIndicator = key & 0xFF88;
        }
        timeoutDisplaySlowUpdate = 0;
        keyPending = 0;
    }
    
    if (hyattTicks > timeoutKeypadUpdate) {
        // clearing 23017 interrupt
        i2cRegRead(KEYPAD_ROW12_ADDR, IOA_GPIO);
        i2cRegRead(KEYPAD_ROW34_ADDR, IOA_GPIO);
        
        keyIndicator = 0x0000; // all off
        keyIndicator |= hyattStatus.axisSelected | hyattStatus.jogWheelStepSize;
        keyIndicator |= (gc_block.modal.units?0:1) << 10;
        keyIndicator |= (gc_block.modal.spindle == SPINDLE_ENABLE_CW ?1:0) << 13;
        keyIndicator |= (gc_block.modal.coolant == COOLANT_MIST_ENABLE ?1:0) << 14;
        
        hyattControlPanelIndicatorUpdate();
        
        FEED_OVERRIDE_BTN_Write(!FEED_OVERRIDE_BTN_Read()); // blinks the 059 LED
        // system_set_exec_state_flag(EXEC_STATUS_REPORT);
        
        timeoutKeypadUpdate = hyattTicks + 50;
     }
}

