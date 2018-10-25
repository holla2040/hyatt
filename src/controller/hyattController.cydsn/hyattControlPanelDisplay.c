#include <stdlib.h>
#include <FS.h>
#include "hyatt.h"
#include "action.h"
#include "RC65X.h"

#define DISPLAYSLOWUPDATEINTERVAL 500
#define DISPLAYFASTUPDATEINTERVAL 250
#define FILENAMEMAX 32
#define MDIBLOCKLEN 50

const char watch[] = " \xa5"; // a5 is a center dot, see lcd char set in manual
uint8_t watchCount;
char mdiBlock[MDIBLOCKLEN];
extern parser_block_t gc_block;

char selections[CONTROLPANEL_SELECTIONCOUNTMAX][CONTROLPANEL_SELECTIONWIDTH] = {};
char filelist[CONTROLPANEL_SELECTIONCOUNTMAX][FILENAMEMAX];

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

    hyattTimeoutDisplaySlowUpdate = 0;
    hyattTimeoutDisplayFastUpdate = 0;

    uartZDisplay_Start();
}

void hyattControlPanelDisplayIdleSetup() {
    LCD_Clear();
    LCD_NoBlink();
    LCD_SetCursor(0,0);     LCD_PutString("X");
    LCD_SetCursor(0,1);     LCD_PutString("Y");
    LCD_SetCursor(0,2);     LCD_PutString("Z");
    LCD_SetCursor(11,2);    LCD_PutString("F");
    LCD_SetCursor(16,2);    LCD_PutString("/");
    LCD_SetCursor(11,0);    LCD_PutString("G");
    hyattControlPanelState = CONTROLPANEL_IDLE;
}

void hyattControlPanelDisplayIdle() {
    char buf[100];
    if (hyattTicks > hyattTimeoutDisplaySlowUpdate) {
        LCD_SetCursor(11,1);
        switch(sys.state) {
            case STATE_IDLE:        LCD_PutString("IDLE "); break;
            case STATE_CYCLE:       LCD_PutString("RUN  "); break;
            case STATE_HOLD:        LCD_PutString("HOLD "); break;
            case STATE_HOMING:      LCD_PutString("HOME "); break;
            case STATE_ALARM:       LCD_PutString("ALARM"); break;
            case STATE_CHECK_MODE:  LCD_PutString("CHECK"); break;
            case STATE_SAFETY_DOOR: LCD_PutString("DOOR "); break;
            case STATE_JOG:         LCD_PutString("JOG  "); break;
            default:                LCD_PutString("?    "); break;
        };
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


        LCD_SetCursor(12,2);
        sprintf(buf,"%4d",(uint16_t)gc_state.feed_rate);
        LCD_PutString(buf);

        LCD_SetCursor(17,2);
        sprintf(buf,"%-3d",sys.f_override);
        LCD_PutString(buf);

        sprintf(buf,"F%4d/%-3d%%",(uint16_t)gc_state.feed_rate,sys.f_override);
        hyattZDisplaySet("f",buf);

        LCD_SetCursor(0,3);
        lastBlock[20] = 0; // clip lastBlock to display width
        sprintf(buf,"%-20s",lastBlock);
        LCD_PutString(buf);
        hyattZDisplaySet("st",lastBlock);

        hyattTimeoutDisplaySlowUpdate = hyattTicks + DISPLAYSLOWUPDATEINTERVAL;
    }

    if (hyattTicks > hyattTimeoutDisplayFastUpdate) {
        uint8_t idx;
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


/* ============ actions ================ */
void actionsLoad() {
    // selections should be all "       ", no
    selectionsClear();
    for (int i = 0; i < CONTROLPANEL_SELECTIONCOUNTMAX; i++) {
        if (strlen(actions[i].label)) strcpy(selections[i],actions[i].label);
    }
}

void hyattControlPanelDisplayActionSetup() {
    LCD_Clear();
    LCD_SetCursor(0,0);     LCD_PutString("Actions - Feed Sel");

    actionsLoad();
    selectionsDisplay();

    LCD_SetCursor(0,1);
    LCD_Blink();

    wheel0 = wheelDecoder_GetCounter();
    hyattControlPanelState = CONTROLPANEL_SELECT_ACTION;
}

void hyattControlPanelDisplayAction() {
    int16_t i = abs(wheel0 - wheelDecoder_GetCounter()) % CONTROLPANEL_SELECTIONCOUNTMAX;
    int x,y,f;
    x = (i / 3) * 7;
    y = (i % 3) + 1;
    LCD_SetCursor(x,y);

    f = FEED_OVERRIDE_Read();
    if ((f & FEED_OVERRIDE_OFF) | !(f & FEED_OVERRIDE_BTN)) {
        LCD_NoBlink();
        LCD_Clear();
        LCD_SetCursor(0,0);
        LCD_PutString("SEND");
        LCD_SetCursor(5,0);
        LCD_PutString(actions[i].label);
        LCD_SetCursor(0,1);
        LCD_PutString(actions[i].block);
        LCD_SetCursor(0,3);
        LCD_PutString("Loaded");
        CyDelay(2000);
        wheelDecoder_SetCounter(wheel0);
        hyattControlPanelState = CONTROLPANEL_IDLE_SETUP;
        grblBlockSend(actions[i].block);
    }

}
/* ============ actions end ================ */


/* ============ load ================ */
void filelistGet() {
    uint8_t i = 0;
    FS_FIND_DATA fd;
    char fn[32];

    FS_Mount("");
    selectionsClear();
    if (FS_FindFirstFile(&fd, "", fn, sizeof(fn)) == 0) {
        do {
            if (!(fd.Attributes & FS_ATTR_DIRECTORY)) {
                if (strlen(fn)){
                    strcpy(filelist[i],fn);
                    strncpy(selections[i],fn,CONTROLPANEL_SELECTIONWIDTH-1);
                    for (uint8_t j = 0; j < strlen(selections[i]); j++) {
                        if (selections[i][j] == '.') selections[i][j] = 0x00;
                    }
                } else {
                    strcpy(filelist[i],"");
                }
                i++;
                if (i == CONTROLPANEL_SELECTIONCOUNTMAX) break; // there's more than 9 files on sd
            }
        } while (FS_FindNextFile (&fd));
    }
    FS_FindClose(&fd);
    FS_Unmount("");

}

void hyattControlPanelDisplayLoadSetup() {
    LCD_Clear();
    LCD_SetCursor(0,0);     LCD_PutString("Load - Feed Select");

    filelistGet();
    selectionsDisplay();

    LCD_SetCursor(0,1);
    LCD_Blink();

    wheel0 = wheelDecoder_GetCounter();
    hyattControlPanelState = CONTROLPANEL_SELECT_LOAD;
}

void hyattControlPanelDisplayLoad() {
    int16_t i = abs(wheel0 - wheelDecoder_GetCounter()) % CONTROLPANEL_SELECTIONCOUNTMAX;
    int x,y,f;
    x = (i / 3) * 7;
    y = (i % 3) + 1;
    LCD_SetCursor(x,y);
    f = FEED_OVERRIDE_Read();
    if ((f & FEED_OVERRIDE_OFF) | !(f & FEED_OVERRIDE_BTN)) {
        FS_FILE *file;
        char buf[10];
        LCD_NoBlink();
        LCD_Clear();
        LCD_SetCursor(0,0);
        LCD_PutString("LOAD");
        LCD_SetCursor(5,0);
        LCD_PutString(filelist[i]);
        file = FS_FOpen(filelist[i], "r");
        sprintf(buf,"%ld",FS_GetFileSize(file));
        LCD_SetCursor(0,1);
        LCD_PutString("SIZE");
        LCD_SetCursor(5,1);
        LCD_PutString(buf);
        LCD_SetCursor(0,3);
        LCD_PutString("Loaded");
        CyDelay(2000);
        wheelDecoder_SetCounter(wheel0);
        hyattControlPanelState = CONTROLPANEL_IDLE_SETUP;

        hyattSenderSend(filelist[i]);
    }
}

/* ============ load ================ */

/* ============ MDI ================ */
void hyattControlPanelDisplayMDISetup() {
    LCD_Clear();
    LCD_SetCursor(0,0);
    LCD_PutString("MDI");
    strcpy(mdiBlock,"");
    LCD_SetCursor(0,1);
    LCD_Write('>');
    hyattControlPanelState = CONTROLPANEL_SELECT_MDI;
}

void hyattControlPanelDisplayMDI() {
}

void hyattControlPanelDisplayMDIKey(uint16_t key) {
    if (hyattControlPanelState == CONTROLPANEL_SELECT_MDI) {
        if (strlen(mdiBlock) >= MDIBLOCKLEN) {
            strcpy(mdiBlock,"");
        }
        switch (key) {
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
            case RC65X_KEYBACK:
                hyattControlPanelState = CONTROLPANEL_IDLE_SETUP;
                return;
                break;
            case RC65X_KEYENTER:
                hyattControlPanelState = CONTROLPANEL_IDLE_SETUP;
                strcat(mdiBlock,"G1F1000");
                grblBlockSend(mdiBlock);
                return;
                break;
            case RC65X_KEYPREV:
                mdiBlock[strlen(mdiBlock)-1] = 0x00;
                break;
        }
        LCD_SetCursor(1,1);
        LCD_PutString(mdiBlock);
        LCD_PutString("    ");  // clear line, needed for prev
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
        case CONTROLPANEL_SELECT_ACTION_SETUP:
            hyattControlPanelDisplayActionSetup();
            break;
        case CONTROLPANEL_SELECT_ACTION:
            hyattControlPanelDisplayAction();
            break;
        case CONTROLPANEL_SELECT_LOAD_SETUP:
            hyattControlPanelDisplayLoadSetup();
            break;
        case CONTROLPANEL_SELECT_LOAD:
            hyattControlPanelDisplayLoad();
            break;
        case CONTROLPANEL_SELECT_MDI_SETUP:
            hyattControlPanelDisplayMDISetup();
            break;
        case CONTROLPANEL_SELECT_MDI:
            hyattControlPanelDisplayMDI();
            break;

    }
}

void hyattZDisplayCommand(char *command) {
    char line[100];
    sprintf(line,"%s\xff\xff\xff",command);
    uartZDisplay_PutString(line);
}

void hyattZDisplaySet(char *attr,char *value) {
    char line[100];
    sprintf(line,"%s.txt=\"%s\"\xff\xff\xff",attr,value);
    uartZDisplay_PutString(line);
}
