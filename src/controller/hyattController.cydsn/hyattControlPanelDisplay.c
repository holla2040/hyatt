#include <stdlib.h>
#include <FS.h>
#include "hyatt.h"
#include "action.h"

#define lcdAddr   0x27

#define DISPLAYSLOWUPDATEINTERVAL 250
#define DISPLAYFASTUPDATEINTERVAL 100
#define FILENAMEMAX 32

const char watch[] = " \xa5"; // a5 is a center dot, see lcd char set in manual
uint8_t watchCount;

extern parser_block_t gc_block;

char selections[CONTROLPANEL_SELECTIONCOUNTMAX][CONTROLPANEL_SELECTIONWIDTH] = {};

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

char filelist[CONTROLPANEL_SELECTIONCOUNTMAX][FILENAMEMAX];

void hyattControlPanelDisplayInit() {
    LCD_Start(lcdAddr,20,4,0);

    LCD_SetCursor(0,0);

    hyattTimeoutDisplaySlowUpdate = 0;
    hyattTimeoutDisplayFastUpdate = 0;
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

        LCD_SetCursor(15,0);
        gc_state.modal.units ?  LCD_PutString("INCH"):LCD_PutString("MM  ");

        LCD_SetCursor(0,3);
        lastBlock[20] = 0; // clip lastBlock to display width
        sprintf(buf,"%-20s",lastBlock);
        LCD_PutString(buf);

        LCD_SetCursor(18,1);
        (gc_block.modal.spindle & SPINDLE_ENABLE_CW) ? LCD_PutString("S"): LCD_PutString(" ");

        LCD_SetCursor(19,1);
        (gc_state.modal.coolant & COOLANT_MIST_ENABLE) ? LCD_PutString("A"): LCD_PutString(" ");

        LCD_SetCursor(12,2);
        sprintf(buf,"%4d",(uint16_t)gc_state.feed_rate);
        LCD_PutString(buf);

        LCD_SetCursor(17,2);
        sprintf(buf,"%-3d",sys.f_override);
        LCD_PutString(buf);

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
        for (idx=0; idx< N_AXIS; idx++) {
            LCD_SetCursor(1,idx);
            if (bit_istrue(settings.flags,BITFLAG_REPORT_INCHES)) {
                sprintf(buf,"%9.4f",print_position[idx]*INCH_PER_MM);
            } else {
                sprintf(buf,"%9.3f",print_position[idx]);
            }
            LCD_PutString(buf);
        }

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
    LCD_SetCursor(0,0);     LCD_PutString("Action Select - Knob");

    actionsLoad();
    selectionsDisplay();

    LCD_SetCursor(0,1);
    LCD_Blink();

    wheel0 = wheelDecoder_GetCounter();
    hyattControlPanelState = CONTROLPANEL_SELECT_ACTION;
}

void actionExecute(int8_t i){
    LCD_NoBlink();
    LCD_Clear();
    LCD_SetCursor(4,1);
    LCD_PutString(actions[i].label);
    LCD_SetCursor(4,2);
    LCD_PutString(actions[i].block);
    CyDelay(2000);
    wheelDecoder_SetCounter(wheel0);
    hyattControlPanelState = CONTROLPANEL_IDLE_SETUP;
    grblBlockSend(actions[i].block);
}

void hyattControlPanelDisplayAction() {
    int16_t i = abs(wheel0 - wheelDecoder_GetCounter()) % CONTROLPANEL_SELECTIONCOUNTMAX;
    int x,y;
    x = (i / 3) * 7;
    y = (i % 3) + 1;
    LCD_SetCursor(x,y);

    if (!(FEED_OVERRIDE_Read() & 0x01)) actionExecute(i);
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
    LCD_SetCursor(0,0);     LCD_PutString("Load Select Feed Off");

    filelistGet();
    selectionsDisplay();

    LCD_SetCursor(0,1);
    LCD_Blink();

    wheel0 = wheelDecoder_GetCounter();
    hyattControlPanelState = CONTROLPANEL_SELECT_LOAD;
}

void hyattControlPanelDisplayLoad() {
    int16_t i = abs(wheel0 - wheelDecoder_GetCounter()) % CONTROLPANEL_SELECTIONCOUNTMAX;
    int x,y;
    x = (i / 3) * 7;
    y = (i % 3) + 1;
    LCD_SetCursor(x,y);

    if ((FEED_OVERRIDE_Read() & FEED_OVERRIDE_OFF )) {
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
        LCD_PutString("Loaded in HOLD");
        CyDelay(4000);
        wheelDecoder_SetCounter(wheel0);
        hyattControlPanelState = CONTROLPANEL_IDLE_SETUP;

        hyattSenderSend(filelist[i]);
    }
}

/* ============ load ================ */




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
    }
}
