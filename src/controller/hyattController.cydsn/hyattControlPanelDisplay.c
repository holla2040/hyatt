#include <stdlib.h>
#include <FS.h>
#include "hyatt.h"
#include "hyattMacro.h"
#include "hyattInspect.h"
#include "RC65X.h"

#define DISPLAYUPDATEIDLEINTERVAL 100
#define DISPLAYUPDATECYCLEINTERVAL 100

#define MDIBLOCKLEN 50

#define BACKLIGHTON_DATA 0x09  //(LCD_BACKLIGHT | 0x01)

const char watch[] = " \xa5"; // a5 is a center dot, see lcd char set in manual
uint8_t watchCount;
char mdiBlock[MDIBLOCKLEN];
extern parser_block_t gc_block;

char status[100];
char buffer[400];
char *bptr,*sptr;
uint8_t secondHalf;
uint32_t hyattTimeoutDisplayUpdate;
float inspectPoints[2][2];
float inspectCirclePoints[3][2]; // x,y * 3
float inspectLength,inspectAngle;
uint8_t fileSelectedIndex;

char selections[CONTROLPANEL_SELECTIONCOUNTMAX][CONTROLPANEL_SELECTIONWIDTH] = {};
extern char filelist[CONTROLPANEL_SELECTIONCOUNTMAX][FILENAMEMAX];
extern uint8_t fileSelectIndex;
extern float fileXMin,fileXMax,fileYMin,fileYMax;

char *stateString() {
    switch (sys.state) {
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

char *alarmString(uint8_t alarmid) {
    switch (alarmid) {
        case 1: return "hard limit";
        case 2: return "soft limit";
        case 3: return "abort cycle";
        case 4: return "probe initial";
        case 5: return "probe contact";
        case 6: return "homing reset";
        case 7: return "homing door";
        case 8: return "homing pulloff";
        case 9: return "homing approach";
    }
    return "none";
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
    I2C_MasterWriteBuf(DISPLAY2004_ADDR,(uint8_t *)buffer,6,I2C_MODE_COMPLETE_XFER);
    while ((I2C_MasterStatus() & I2C_MSTAT_WR_CMPLT)==0) {}; // wait until command is sent
}

void selectionsClear() {
    for (int i = 0; i < CONTROLPANEL_SELECTIONCOUNTMAX; i++) {
        strcpy(selections[i],"      ");
    }
}

void selectionsDisplay() {
    int x,y;
    for (int i = 0; i < CONTROLPANEL_SELECTIONCOUNTMAX; i++) {
        x = (i / 3) * 7;
        y = (i % 3) + 1;
        LCD_SetCursor(x,y);
        LCD_PutString(selections[i]);
    }
}

void hyattControlPanelDisplayInit() {
    LCD_Start(DISPLAY2004_ADDR,20,4,0);
    LCD_SetCursor(0,0);
    strcpy(lastBlock,"HOMING REQUIRED");
}

void hyattControlPanelDisplayIdleSetup() {
    LCD_Clear();
    LCD_NoBlink();
    hyattControlPanelState = CONTROLPANEL_IDLE;
}

void hyattCurrentPosition() {
    uint8_t idx;
    int32_t current_position[N_AXIS]; // Copy current state of the system position variable
    float print_position[N_AXIS];
    float wco[N_AXIS];

    memcpy(current_position,sys_position,sizeof(sys_position));
    system_convert_array_steps_to_mpos(print_position,current_position);

    bptr = buffer;
    sptr = status;

    if (bit_isfalse(settings.status_report_mask,BITFLAG_RT_STATUS_POSITION_TYPE) || (sys.report_wco_counter == 0) ) {
        for (idx=0; idx< N_AXIS; idx++) {
            // Apply work coordinate offsets and tool length offset to current position.
            wco[idx] = gc_state.coord_system[idx]+gc_state.coord_offset[idx];
            if (idx == TOOL_LENGTH_OFFSET_AXIS) { wco[idx] += gc_state.tool_length_offset; }
            if (bit_isfalse(settings.status_report_mask,BITFLAG_RT_STATUS_POSITION_TYPE)) {
                print_position[idx] -= wco[idx];
            }

        }
    }

    // x,y,z are floats defined in hyatt.h
    
    x = print_position[0];
    y = print_position[1];
    z = print_position[2];
    
    if (bit_istrue(settings.flags,BITFLAG_REPORT_INCHES)) {
        x *= INCH_PER_MM;
        y *= INCH_PER_MM;
        z *= INCH_PER_MM;
    }     
}

void hyattControlPanelDisplayIdle() {
    uint8_t c,i2cStatus;
    i2cStatus = I2C_MasterStatus();

    if (secondHalf) {
        if (i2cStatus & I2C_MSTAT_WR_CMPLT) {
            I2C_MasterWriteBuf(DISPLAY2004_ADDR,(uint8_t *)&buffer[160],160,I2C_MODE_COMPLETE_XFER); // write the 2nd half, line 1 and 
            if (sys.state == STATE_CYCLE ) {
                hyattTimeoutDisplayUpdate = hyattTicks + DISPLAYUPDATECYCLEINTERVAL;
            } else {
                hyattTimeoutDisplayUpdate = hyattTicks + DISPLAYUPDATEIDLEINTERVAL;
            }
            secondHalf = 0;
        }
        return;
    }
        
    if (hyattTicks > hyattTimeoutDisplayUpdate) {
        if (i2cStatus & I2C_MSTAT_ERR_MASK) I2C_MasterClearStatus();

        hyattCurrentPosition();
        
        home(); // sets DDRAM address to 0, which is upper corner
        if (sys.state == STATE_ALARM) {
            strcpy(lastBlock,alarmString(sys_rt_exec_alarm));
        }

        if (strlen(mdiBlock)) {
            sprintf(lastBlock,"> %s                     ",mdiBlock);
        }

        lastBlock[20] = 0; // clip the last block parsed and planned
        
        sprintf(status,"X%9.4f G%02d %4s%cZ%9.4f F%4d/%-3dY%9.4f %5s  %c%c%-20s",
            x,54+gc_state.modal.coord_select,gc_state.modal.units?"INCH":"MM  ",watch[(++watchCount)%2],
            z,(uint16_t)(gc_state.feed_rate > 9999?9999:gc_state.feed_rate),sys.f_override,
            y,stateString(),(gc_block.modal.spindle & SPINDLE_ENABLE_CW)?'S':' ',gc_state.modal.coolant & COOLANT_MIST_ENABLE?'A':' ',
            lastBlock
        );

        do {
            c = (*sptr & 0xF0) | BACKLIGHTON_DATA; 
            *bptr++ = c | En;
            *bptr++ = c & ~En;
            c = (*sptr << 4) | BACKLIGHTON_DATA;
            *bptr++ = c | En;
            *bptr++ = c & ~En;
        } while (*sptr++);
        
        I2C_MasterClearStatus(); // just in case someone left i2c in a bad way, naughty naughty
        I2C_MasterWriteBuf(DISPLAY2004_ADDR,(uint8_t *)buffer,160,I2C_MODE_COMPLETE_XFER); // write the 1st half, line 0 and 2

        secondHalf = 1;
        hyattZDisplayUpdate = 0;
    }
}

/* ============ inspect ================ */
void inspectsLoad() {
    // selections should be all "       ", no
    selectionsClear();
    for (int i = 0; i < CONTROLPANEL_SELECTIONCOUNTMAX; i++) {
        if (strlen(inspects[i].label)) strcpy(selections[i],inspects[i].label);
    }
}

void hyattControlPanelDisplayInspectSetup() {
    LCD_Clear();
    LCD_SetCursor(0,0); 

    inspectsLoad();
    selectionsDisplay();

    LCD_SetCursor(0,0);
    LCD_Blink();

    wheel0 = wheelDecoder_GetCounter();
    hyattControlPanelState = CONTROLPANEL_SELECT_INSPECT;
    enterCount = 0;
}

void hyattControlPanelDisplayInspect() {
    int16_t i = abs(wheel0 - wheelDecoder_GetCounter()) % CONTROLPANEL_SELECTIONCOUNTMAX;
    int x,y,f;
    x = (i / 3) * 7;
    y = (i % 3) + 1;
    LCD_SetCursor(x,y);

    f = FEED_OVERRIDE_Read();
    if ((f & FEED_OVERRIDE_OFF) | !(f & FEED_OVERRIDE_BTN) | enterCount) {
        hyattCurrentPosition();
        switch (i) {
            case 1: // point 1
                inspectPoints[0][0] = x;
                inspectPoints[0][1] = y;
                break;
            case 2: // point 2
                inspectPoints[1][0] = x;
                inspectPoints[1][1] = y;
                break;
            case 3: // measure
                break;
        };

/*
            CyDelay(2000);
        wheelDecoder_SetCounter(wheel0);
*/
    }
}

void hyattControlPanelDisplayInspectResult() {

}
/* ============ inspect end ================ */

/* ============ macro ================ */
void macrosLoad() {
    // selections should be all "       ", no
    selectionsClear();
    for (int i = 0; i < CONTROLPANEL_SELECTIONCOUNTMAX; i++) {
        if (strlen(macros[i].label)) strcpy(selections[i],macros[i].label);
    }
}

void hyattControlPanelDisplayMacroSetup() {
    LCD_Clear();
    LCD_SetCursor(0,0);     LCD_PutString("Macros");

    macrosLoad();
    selectionsDisplay();

    LCD_SetCursor(0,1);
    LCD_Blink();

    wheel0 = wheelDecoder_GetCounter();
    hyattControlPanelState = CONTROLPANEL_SELECT_MACRO;
    enterCount = 0;
}

void hyattControlPanelDisplayMacro() {
    int16_t i = abs(wheel0 - wheelDecoder_GetCounter()) % CONTROLPANEL_SELECTIONCOUNTMAX;
    int x,y,f;
    x = (i / 3) * 7;
    y = (i % 3) + 1;
    LCD_SetCursor(x,y);

    f = FEED_OVERRIDE_Read();
    if ((f & FEED_OVERRIDE_OFF) | !(f & FEED_OVERRIDE_BTN) | enterCount) {
        LCD_NoBlink();
        LCD_Clear();
        LCD_SetCursor(0,0);
        LCD_PutString("SEND");
        LCD_SetCursor(5,0);
        LCD_PutString(macros[i].label);
        LCD_SetCursor(0,1);
        LCD_PutString(macros[i].block);
        LCD_SetCursor(0,3);
        LCD_PutString("Loaded");
        CyDelay(2000);
        wheelDecoder_SetCounter(wheel0);
        hyattControlPanelState = CONTROLPANEL_IDLE_SETUP;
        grblBlockSend(macros[i].block);
    }
}
/* ============ macro end ================ */


/* ============ file ================ */
/* file action layout
    All     Op      From Op
    NW      NE
    SW      SE
*/



void hyattControlPanelDisplayFileSetup() {
    LCD_Clear();
    LCD_SetCursor(0,0);     LCD_PutString("File Ops");

    hyattFilelistGet();
    selectionsDisplay();

    LCD_SetCursor(0,1);
    LCD_Blink();

    wheel0 = wheelDecoder_GetCounter();
    hyattControlPanelState = CONTROLPANEL_SELECT_FILE;
    enterCount = 0;
}


void hyattControlPanelDisplayFile() {
    int16_t i = abs(wheel0 - wheelDecoder_GetCounter()) % CONTROLPANEL_SELECTIONCOUNTMAX;
    int x,y,f;
    x = (i / 3) * 7;
    y = (i % 3) + 1;
    LCD_SetCursor(x,y);
    f = FEED_OVERRIDE_Read();
    if ((f & FEED_OVERRIDE_OFF) | !(f & FEED_OVERRIDE_BTN) | enterCount) {
        fileSelectedIndex = i;
        strcpy(selections[0],"Load");   strcpy(selections[3],"Op"); strcpy(selections[6],"Ops");
        strcpy(selections[1],"Perim");  strcpy(selections[4],"NW"); strcpy(selections[7],"NE");
        strcpy(selections[2],"");       strcpy(selections[5],"SW"); strcpy(selections[8],"SE");


        LCD_Clear();
        LCD_SetCursor(0,0);     LCD_PutString(filelist[i]);

        selectionsDisplay();

        LCD_SetCursor(0,1);
        LCD_Blink();

        wheel0 = wheelDecoder_GetCounter();
        hyattControlPanelState = CONTROLPANEL_SELECT_FILE_ACTION;
        enterCount = 0;
        fileSelectIndex = i;
    }
}

void hyattControlPanelDisplayFileAction() {
// xxx
    char buf[100];
    int16_t i = abs(wheel0 - wheelDecoder_GetCounter()) % CONTROLPANEL_SELECTIONCOUNTMAX;
    int x,y,f;
    x = (i / 3) * 7;
    y = (i % 3) + 1;
    LCD_SetCursor(x,y);
    f = FEED_OVERRIDE_Read();
    if ((f & FEED_OVERRIDE_OFF) | !(f & FEED_OVERRIDE_BTN) | enterCount) {
        switch(i) {
            // use menu layout from above

            case 0: 
                hyattFileSend(filelist[fileSelectedIndex]);
                break;
            case 3: 
                break;
            case 6: 
                break;

            case 1:
                LCD_Clear();
                LCD_SetCursor(0,0);     
                LCD_PutString(filelist[fileSelectedIndex]);
                LCD_SetCursor(0,1);     
                LCD_PutString("perimeter scan");
                hyattFilePerimeter(filelist[fileSelectedIndex]);
                hyattControlPanelDisplayFileSetup();
                hyattControlPanelDisplayFile();
                break;
            case 4: 
                sprintf(buf,"G1 F2500 X%f Y%f",fileXMin,fileYMax);
                grblBlockSend(buf);
                break;
            case 7:
                sprintf(buf,"G1 F2500 X%f Y%f",fileXMax,fileYMax);
                grblBlockSend(buf);
                break;

            case 2:
                break;
            case 5:
                break;
                sprintf(buf,"G1 F2500 X%f Y%f",fileXMin,fileYMin);
                grblBlockSend(buf);
                break;
            case 8: 
                sprintf(buf,"G1 F2500 X%f Y%f",fileXMax,fileYMin);
                grblBlockSend(buf);
                break;
        }
        enterCount = 0;
    }    
    if (hyattTicks > hyattTimeoutDisplayUpdate) { // forces zdisplay update during movement
        sprintf(lastBlock,"%f,%f %f,%f",fileXMin,fileYMin,fileXMax,fileYMax);
        hyattZDisplayUpdate = 0;
        hyattZDisplayLoop();
    }
}

/* ============ file ================ */

/* ============ MDI ================ */
void hyattControlPanelDisplayMDI() {
}

void hyattControlPanelDisplayMDIKey(uint16_t key) {
    if (strlen(mdiBlock) >= MDIBLOCKLEN) {
        strcpy(mdiBlock,"");
    }
    switch (key) {
        case RC65X_KEYMENU:
            strcpy(mdiBlock,"");
            strcpy(lastBlock,"");
            break;
        case RC65X_KEY0:
            strcat(mdiBlock,"0");
            break;
        case RC65X_KEY1:
            strcat(mdiBlock,"1");
            break;
        case RC65X_KEY2:
            strcat(mdiBlock,"2");
            break;
        case RC65X_KEY3:
            strcat(mdiBlock,"3");
            break;
        case RC65X_KEY4:
            strcat(mdiBlock,"4");
            break;
        case RC65X_KEY5:
            strcat(mdiBlock,"5");
            break;
        case RC65X_KEY6:
            strcat(mdiBlock,"6");
            break;
        case RC65X_KEY7:
            strcat(mdiBlock,"7");
            break;
        case RC65X_KEY8:
            strcat(mdiBlock,"8");
            break;
        case RC65X_KEY9:
            strcat(mdiBlock,"9");
            break;
        case RC65X_KEYDASH:
            strcat(mdiBlock,"-");
            break;
        case RC65X_KEYMUTE:
            strcat(mdiBlock,".");
            break;
        case RC65X_KEYRED:
            strcat(mdiBlock,"X");
            break;
        case RC65X_KEYGREEN:
            strcat(mdiBlock,"Y");
            break;
        case RC65X_KEYYELLOW:
            strcat(mdiBlock,"Z");
            break;
        case RC65X_KEYCHANUP:
            strcat(mdiBlock,"G");
            break;
        case RC65X_KEYCHANDOWN:
            strcat(mdiBlock,"F");
            break;
        case RC65X_KEYENTER:
            grblBlockSend(mdiBlock);
            break;
        case RC65X_KEYPREV:
            if (strlen(mdiBlock)>1) {
                mdiBlock[strlen(mdiBlock)-1] = 0x00;
            } else {
                strcpy(mdiBlock,"");
                strcpy(lastBlock,"");
            }
            break;
    }
}
/* ============ MDI ================ */


void hyattControlPanelDisplayLoop() {
    switch (hyattControlPanelState) {
        case CONTROLPANEL_IDLE_SETUP:
            hyattControlPanelDisplayIdleSetup();
            break;
        case CONTROLPANEL_IDLE:
            hyattControlPanelDisplayIdle();
            break;
        case CONTROLPANEL_SELECT_MACRO_SETUP:
            hyattControlPanelDisplayMacroSetup();
            break;
        case CONTROLPANEL_SELECT_MACRO:
            hyattControlPanelDisplayMacro();
            break;
        case CONTROLPANEL_SELECT_FILE_SETUP:
            hyattControlPanelDisplayFileSetup();
            break;
        case CONTROLPANEL_SELECT_FILE:
            hyattControlPanelDisplayFile();
            break;
        case CONTROLPANEL_SELECT_FILE_ACTION:
            hyattControlPanelDisplayFileAction();
            break;
        case CONTROLPANEL_SELECT_INSPECT_SETUP:
            hyattControlPanelDisplayInspectSetup();
            break;
        case CONTROLPANEL_SELECT_INSPECT:
            hyattControlPanelDisplayInspect();
            break;
        case CONTROLPANEL_SELECT_INSPECT_RESULT:
            hyattControlPanelDisplayInspectResult();
            break;
    }
}

/*
void hyattControlPanelDisplayIdleOld() {
    char buf[100];
    if (hyattTicks > hyattTimeoutDisplaySlowUpdate) {
        // if (sys.state == STATE_CYCLE) return;
        
        if (sys.state != STATE_CYCLE) {
            LCD_SetCursor(11,1);
            LCD_PutString(stateString());

            LCD_SetCursor(19,0);
            LCD_Write(watch[(++watchCount)%strlen(watch)]);

            LCD_SetCursor(12,0);
            sprintf(buf,"%d",54+gc_state.modal.coord_select);
            LCD_PutString(buf);
            sprintf(buf,"G%d",54+gc_state.modal.coord_select);
            hyattZDisplaySet("c",buf);

            LCD_SetCursor(15,0);
            if (gc_state.modal.units) {
                LCD_PutString("INCH");
                hyattZDisplaySet("u","INCH");
            } else {
                LCD_PutString("MM  ");
                hyattZDisplaySet("u","MM");
            }

            LCD_SetCursor(18,1);
            if (gc_block.modal.spindle & SPINDLE_ENABLE_CW) {
                LCD_PutString("S");
                hyattZDisplaySet("s","SPINDLE");
            } else {
                LCD_PutString(" ");
                hyattZDisplaySet("s","");
            }

            LCD_SetCursor(19,1);
            if (gc_state.modal.coolant & COOLANT_MIST_ENABLE) {
                LCD_PutString("A");
                hyattZDisplaySet("a","AIR");
            } else {
                LCD_PutString(" ");
                hyattZDisplaySet("a","");
            }


            sprintf(buf,"F%4d/%-3d%%",(uint16_t)gc_state.feed_rate,sys.f_override);
            hyattZDisplaySet("f",buf);

            LCD_SetCursor(0,3);
            lastBlock[20] = 0; // clip lastBlock to display width
            sprintf(buf,"%-20s",lastBlock);
            LCD_PutString(buf);

        LCD_SetCursor(12,2);
        sprintf(buf,"%4d",(uint16_t)gc_state.feed_rate);
        LCD_PutString(buf);

        LCD_SetCursor(17,2);
        sprintf(buf,"%-3d",sys.f_override);
        LCD_PutString(buf);

        }
        hyattZDisplaySet("st",lastBlock);

        hyattTimeoutDisplaySlowUpdate = hyattTicks + DISPLAYSLOWUPDATEINTERVAL;
    }

    if (hyattTicks > hyattTimeoutDisplayFastUpdate) {
        uint8_t idx;
        // if (sys.state == STATE_CYCLE) return;
        int32_t current_position[N_AXIS]; // Copy current state of the system position variable
        memcpy(current_position,sys_position,sizeof(sys_position));
        float print_position[N_AXIS];
        system_convert_array_steps_to_mpos(print_position,current_position);
        float wco[N_AXIS];

        if (bit_isfalse(settings.status_report_mask,BITFLAG_RT_STATUS_POSITION_TYPE) || (sys.report_wco_counter == 0) ) {
            for (idx=0; idx< N_AXIS; idx++) {
                // Apply work coordinate offsets and tool length offset to current position.
                wco[idx] = gc_state.coord_system[idx]+gc_state.coord_offset[idx];
                if (idx == TOOL_LENGTH_OFFSET_AXIS) { wco[idx] += gc_state.tool_length_offset; }
                if (bit_isfalse(settings.status_report_mask,BITFLAG_RT_STATUS_POSITION_TYPE)) {
                    print_position[idx] -= wco[idx];
                }

            }
        }
        hyattZDisplayCommand("ref_stop");
        for (idx=0; idx< N_AXIS; idx++) {
            char attr[2] = {0,0};
            double v = print_position[idx];
            LCD_SetCursor(1,idx);
            if (bit_istrue(settings.flags,BITFLAG_REPORT_INCHES)) {
                v *= INCH_PER_MM;
            }
            sprintf(buf,"%9.3f",v);
            LCD_PutString(buf);
            attr[0] = (char)('x'+idx);

            hyattZDisplaySet(attr,buf);
        }
        hyattZDisplayCommand("ref_star");
        hyattTimeoutDisplayFastUpdate = hyattTicks + DISPLAYFASTUPDATEINTERVAL;
    }
}
*/
/*
        FS_FILE *file;
        char buf[10];
        LCD_NoBlink();
        LCD_Clear();
        LCD_SetCursor(0,0);
        LCD_PutString("FILE");
        LCD_SetCursor(5,0);
        LCD_PutString(filelist[i]);
        file = FS_FOpen(filelist[i], "r");
        sprintf(buf,"%ld",FS_GetFileSize(file));
        LCD_SetCursor(0,1);
        LCD_PutString("SIZE");
        LCD_SetCursor(5,1);
        LCD_PutString(buf);
        LCD_SetCursor(0,3);
        LCD_PutString("File loaded");
        CyDelay(2000);
        wheelDecoder_SetCounter(wheel0);
        hyattControlPanelState = CONTROLPANEL_IDLE_SETUP;
*/

