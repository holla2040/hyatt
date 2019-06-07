#include "project.h"
#include <stdio.h>
#include "RC65X.h"
#include "hyatt.h"

uint16_t button;

char * keyLabel(uint16_t v) {
    switch (v) {
        // case RC65X_KEYTVINPUT:           return "KEYTVINPUT";   break;
        case RC65X_KEYPWR:               return "KEYPWR";       break;
        // case RC65X_KEYON:                return "KEYON";        break;
        case RC65X_KEYFORMAT:            return "KEYFORMAT";    break;
        // case RC65X_KEYOFF:               return "KEYOFF";       break;7
        case RC65X_KEYPREVIOUS:          return "KEYPREVIOUS";  break;
        case RC65X_KEYREPLAY:            return "KEYREPLAY";    break;
        case RC65X_KEYSTOP:              return "KEYSTOP";      break;
        case RC65X_KEYADVANCE:           return "KEYADVANCE";   break;
        case RC65X_KEYNEXT:              return "KEYNEXT";      break;
        case RC65X_KEYPLAY:              return "KEYPLAY";      break;
        case RC65X_KEYPAUSE:             return "KEYPAUSE";     break;
        case RC65X_KEYRECORD:            return "KEYRECORD";    break;
        case RC65X_KEYGUIDE:             return "KEYGUIDE";     break;
        case RC65X_KEYACTIVE:            return "KEYACTIVE";    break;
        case RC65X_KEYLIST:              return "KEYLIST";      break;
        case RC65X_KEYEXIT:              return "KEYEXIT";      break;
        case RC65X_KEYUP:                return "KEYUP";        break;
        case RC65X_KEYRIGHT:             return "KEYRIGHT";     break;
        case RC65X_KEYDOWN:              return "KEYDOWN";      break;
        case RC65X_KEYLEFT:              return "KEYLEFT";      break;
        case RC65X_KEYSELECT:            return "KEYSELECT";    break;
        // case RC65X_KEYSELECT_UP:         return "KEYSELECT";    break;
        case RC65X_KEYBACK:              return "KEYBACK";      break;
        case RC65X_KEYMENU:              return "KEYMENU";      break;
        case RC65X_KEYINFO:              return "KEYINFO";      break;
        case RC65X_KEYRED:               return "KEYRED";       break;
        case RC65X_KEYGREEN:             return "KEYGREEN";     break;
        case RC65X_KEYYELLOW:            return "KEYYELLOW";    break;
        case RC65X_KEYBLUE:              return "KEYBLUE";      break;
        // case RC65X_KEYVOLUMEUP:       return "KEYVOLUMEUP";  break;
        // case RC65X_KEYVOLUMEDOWN:     return "KEYVOLUMEDOWN";break;
        case RC65X_KEYCHANUP:            return "KEYCHANUP";    break;
        case RC65X_KEYCHANDOWN:          return "KEYCHANDOWN";  break;
        // case RC65X_KEYMUTE:           return "KEYMUTE";      break;
        case RC65X_KEYPREV:              return "KEYPREV";      break;
        case RC65X_KEY1:                 return "KEY1";         break;
        case RC65X_KEY2:                 return "KEY2";         break;
        case RC65X_KEY3:                 return "KEY3";         break;
        case RC65X_KEY4:                 return "KEY4";         break;
        case RC65X_KEY5:                 return "KEY5";         break;
        case RC65X_KEY6:                 return "KEY6";         break;
        case RC65X_KEY7:                 return "KEY7";         break;
        case RC65X_KEY8:                 return "KEY8";         break;
        case RC65X_KEY9:                 return "KEY9";         break;
        case RC65X_KEYDASH:              return "KEYDASH";      break;
        case RC65X_KEY0:                 return "KEY0";         break;
        case RC65X_KEYENTER:             return "KEYENTER";     break;
    }
    return ""; // could be the default buttons for tv only or junk
}

void isr_IR_Handler() {
    button = irDecoderShiftReg_ReadRegValue();
    irDecoderShiftReg_WriteRegValue(0x00);
    irDurationTimer_ReadStatusRegister(); // clears interrupt
}

void hyattControlPanelIRInit(void) {
    irDecoderShiftReg_Start();
    irDurationTimer_Start();
        
    isr_IR_StartEx(isr_IR_Handler);
}

void hyattControlPanelIRLoop(void) {
    if (button) {
        if (sys.state == STATE_ALARM) {
            if (button == RC65X_KEYPWR) {
                grblBlockSend("$X");
            }
        }
        
        if (sys.state == STATE_CYCLE) {
            if (button == RC65X_KEYSTOP) {
                hyattFileSenderState = FILESENDERSTATE_IDLE;
                return;
            }
        }
        
        if (button == RC65X_KEYSELECT) {
            switch (sys.state) {
                case STATE_HOLD:
                    system_set_exec_state_flag(EXEC_CYCLE_START);
                    break;
                case STATE_IDLE:
                case STATE_JOG:
                case STATE_CYCLE:
                    system_set_exec_state_flag(EXEC_FEED_HOLD);
                    break;
            }
        }

        if (sys.state == STATE_IDLE) {
                switch (button) {
                    case RC65X_KEYPREVIOUS:
                        grblBlockSend("$H");
                        break;
                   case RC65X_KEYUP:
                        grblBlockSend("Y1G91G1F2500");
                        break;
                    case RC65X_KEYDOWN:
                        grblBlockSend("Y-1G91G1F2500");
                        break;
                    case RC65X_KEYLEFT:
                        grblBlockSend("X-1G91G1F2500");
                        break;
                    case RC65X_KEYRIGHT:
                        grblBlockSend("X1G91G1F2500");
                        break;
                    case RC65X_KEYEXIT:
                        grblBlockSend("Z1G91G1F2500");
                        break;
                    case RC65X_KEYINFO:
                        grblBlockSend("Z-1G91G1F2500");
                        break;
                    case RC65X_KEYFORMAT:
                        unitToggle();
                        break;
                    case RC65X_KEYRECORD:
                        axisZero();
                        break;
                    case RC65X_KEYPLAY:
                        hyattFileSend(INSERTFN);
                        break;
                    default:
                        hyattControlPanelDisplayMDIKey(button);
                }
            }
        button = 0;
    }
}
