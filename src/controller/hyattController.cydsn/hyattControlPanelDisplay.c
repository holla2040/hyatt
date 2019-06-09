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
uint8_t operationType;
enum {OPERATIONBEFORE,OPERATIONSINGLE,OPERATIONAFTER};

char selections[CONTROLPANEL_SELECTIONCOUNTMAX][CONTROLPANEL_SELECTIONWIDTH] = {};
extern char filelist[CONTROLPANEL_SELECTIONCOUNTMAX][FILENAMEMAX];
extern float fileXMin,fileXMax,fileYMin,fileYMax;
extern uint32_t fileSize;
extern uint32_t fileOpSeeks[CONTROLPANEL_SELECTIONCOUNTMAX+1];


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
    int x,y;
    x = (i / 3) * 7;
    y = (i % 3) + 1;
    LCD_SetCursor(x,y);

    if (enterCount) {
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
    int x,y;
    x = (i / 3) * 7;
    y = (i % 3) + 1;
    LCD_SetCursor(x,y);

    if (enterCount) {
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
        CyDelay(200);
        wheelDecoder_SetCounter(wheel0);
        hyattControlPanelState = CONTROLPANEL_IDLE_SETUP;
        grblBlockSend(macros[i].block);
    }
}
/* ============ macro end ================ */


/* ============ file ================ */
void hyattControlPanelDisplayFileSetup() {
    LCD_Clear();
    LCD_SetCursor(0,0);     LCD_PutString("File");

    hyattFilelistGet();
    selectionsDisplay();

    LCD_SetCursor(0,1);
    LCD_Blink();

    wheel0 = wheelDecoder_GetCounter();
    hyattControlPanelState = CONTROLPANEL_SELECT_FILE;
    enterCount = 0;
}

void hyattControlPanelDisplayFileDisplay() {
    strcpy(selections[0],"Load");  strcpy(selections[3],"");     strcpy(selections[6],"Before");
    strcpy(selections[1],"NW");    strcpy(selections[4],"NE");   strcpy(selections[7],"Single");
    strcpy(selections[2],"SW");    strcpy(selections[5],"SE");   strcpy(selections[8],"After");

    LCD_Clear();
    LCD_SetCursor(0,0);     LCD_PutString(filelist[fileSelectedIndex]);
    LCD_SetCursor(0,1);     LCD_PutString("scanning ");
    hyattFileScan(filelist[fileSelectedIndex]);
    LCD_SetCursor(0,1);     LCD_PutString("         ");

    selectionsDisplay();

    LCD_SetCursor(0,1);
    LCD_Blink();

    wheel0 = wheelDecoder_GetCounter();
    enterCount = 0;
    hyattControlPanelState = CONTROLPANEL_SELECT_FILE_ACTION;
}

void hyattControlPanelDisplayFile() {
    int16_t i = abs(wheel0 - wheelDecoder_GetCounter()) % CONTROLPANEL_SELECTIONCOUNTMAX;
    int x,y;
    x = (i / 3) * 7;
    y = (i % 3) + 1;

    LCD_SetCursor(x,y);
    if (enterCount) {
        fileSelectedIndex = i;
        hyattControlPanelDisplayFileDisplay();
    }
}

void hyattControlPanelDisplayFileAction() {
    char buf[50];
    int16_t i = abs(wheel0 - wheelDecoder_GetCounter()) % CONTROLPANEL_SELECTIONCOUNTMAX;
    int x,y;
    x = (i / 3) * 7;
    y = (i % 3) + 1;
    LCD_SetCursor(x,y);

    if (enterCount) {
        switch(i) {
            // using menu layout from above
            case 0: 
                hyattFileSend(filelist[fileSelectedIndex],0,fileSize);
                hyattControlPanelState = CONTROLPANEL_IDLE_SETUP;
                break;
            case 3:  
                break;
            case 6: 
                operationType = OPERATIONBEFORE;
                hyattControlPanelState = CONTROLPANEL_SELECT_FILE_OPERATION_SETUP;
                break;

            case 1:
                sprintf(buf,"G1 F2500 X%f Y%f",fileXMin,fileYMax);
                grblBlockSend(buf);
                break;
            case 4: 
                sprintf(buf,"G1 F2500 X%f Y%f",fileXMax,fileYMax);
                grblBlockSend(buf);
                break;
            case 7:
                operationType = OPERATIONSINGLE;
                hyattControlPanelState = CONTROLPANEL_SELECT_FILE_OPERATION_SETUP;
                break;

            case 2:
                sprintf(buf,"G1 F2500 X%f Y%f",fileXMin,fileYMin);
                grblBlockSend(buf);
                break;
            case 5:
                sprintf(buf,"G1 F2500 X%f Y%f",fileXMax,fileYMin);
                grblBlockSend(buf);
                break;
            case 8: 
                operationType = OPERATIONAFTER;
                hyattControlPanelState = CONTROLPANEL_SELECT_FILE_OPERATION_SETUP;
                break;
        }
        enterCount = 0;
    }    

    if (hyattTicks > hyattTimeoutDisplayUpdate) { // forces zdisplay update during movement
        hyattCurrentPosition();
        hyattZDisplayLoop();
        hyattTimeoutDisplayUpdate = hyattTicks + DISPLAYUPDATECYCLEINTERVAL;
    }
}

void hyattControlPanelDisplayFileOperationSetup() {
    LCD_Clear();
    LCD_SetCursor(0,0);     
    switch (operationType) {
        case OPERATIONBEFORE:
            LCD_PutString("File Op Before");
            break;
        case OPERATIONSINGLE:
            LCD_PutString("File Op Single");
            break;
        case OPERATIONAFTER:
            LCD_PutString("File Op After");
            break;
    }
    LCD_SetCursor(0,1);  
    LCD_PutString("scanning");

    hyattFileOperationsGet(filelist[fileSelectedIndex]);
    LCD_SetCursor(0,1);  
    LCD_PutString("        ");

    selectionsDisplay();

    LCD_SetCursor(0,1);
    LCD_Blink();

    wheel0 = wheelDecoder_GetCounter();
    hyattControlPanelState = CONTROLPANEL_SELECT_FILE_OPERATION_SELECT;
    enterCount = 0;
}

void hyattControlPanelDisplayFileOperationSelect() {
    int16_t i = abs(wheel0 - wheelDecoder_GetCounter()) % CONTROLPANEL_SELECTIONCOUNTMAX;
    int x,y;
    x = (i / 3) * 7;
    y = (i % 3) + 1;
    LCD_SetCursor(x,y);

    if (enterCount) {
        switch (operationType) {
            case OPERATIONBEFORE:
                hyattFileSend(filelist[fileSelectedIndex],0,fileOpSeeks[i]);
                hyattControlPanelState = CONTROLPANEL_IDLE_SETUP;
                break;
            case OPERATIONSINGLE:
                hyattFileSend(filelist[fileSelectedIndex],fileOpSeeks[i],fileOpSeeks[i+1]);
                hyattControlPanelState = CONTROLPANEL_IDLE_SETUP;
                break;
            case OPERATIONAFTER:
                hyattFileSend(filelist[fileSelectedIndex],fileOpSeeks[i+1],fileSize);
                hyattControlPanelState = CONTROLPANEL_IDLE_SETUP;
                break;
        }
        enterCount = 0;
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
        case CONTROLPANEL_SELECT_FILE_OPERATION_SETUP:
            hyattControlPanelDisplayFileOperationSetup();
            break;
        case CONTROLPANEL_SELECT_FILE_OPERATION_SELECT:
            hyattControlPanelDisplayFileOperationSelect();
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
