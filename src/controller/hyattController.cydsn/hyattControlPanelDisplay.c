#include <stdlib.h>
#include "hyatt.h"

#define lcdAddr   0x27

#define DISPLAYSLOWUPDATEINTERVAL 250
#define DISPLAYFASTUPDATEINTERVAL 100

const char watch[] = " \xa5"; // a5 is a center dot, see lcd char set in manual
uint8_t watchCount;
int16_t wheel0;

extern parser_block_t gc_block;

#define SELECTIONCOUNTMAX 9
uint8_t selectionCount;
#define SELECTIONWIDTH 7
    // 6 chars and null
uint8_t selection;
char selections[SELECTIONCOUNTMAX][SELECTIONWIDTH] = {};

void selectionsClear() {
    for (int i = 0; i < SELECTIONCOUNTMAX; i++) {
        strcpy(selections[i],"      ");
    }
}

void selectionsDisplay() {
    int x,y;
    for (int i = 0; i < SELECTIONCOUNTMAX; i++) {
        x = (i / 3) * 7;
        y = (i % 3) + 1;
        LCD_SetCursor(x,y);
        LCD_PutString(selections[i]);
    }
}

void hyattControlPanelDisplayInit() {
    LCD_Start(lcdAddr,20,4,0);

    LCD_SetCursor(0,0);

    hyattTimeoutDisplaySlowUpdate = 0;
    hyattTimeoutDisplayFastUpdate = 0;

    selection = 0;
    selectionsClear();
}

void hyattControPanelDisplayIdleSetup() {
    LCD_Clear();
    LCD_SetCursor(0,0);     LCD_PutString("X");
    LCD_SetCursor(0,1);     LCD_PutString("Y");
    LCD_SetCursor(0,2);     LCD_PutString("Z");
    LCD_SetCursor(11,2);    LCD_PutString("F");
    LCD_SetCursor(16,2);    LCD_PutString("/");
    LCD_SetCursor(11,0);    LCD_PutString("G");
    hyattControlPanelState = CONTROLPANEL_IDLE;
}

void hyattControPanelDisplayIdle() {
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
void hyattControPanelDisplayActionSetup() {
    LCD_Clear();
    LCD_SetCursor(0,0);     LCD_PutString("Action Select");

    selectionsClear();

    strcpy(selections[0],"X0Y0");
    strcpy(selections[1],"HOME");
    strcpy(selections[2],"PARK");
    strcpy(selections[3],"FOUR");
    strcpy(selections[4],"FIVE");
    strcpy(selections[5],"SIX");
    strcpy(selections[6],"SEVEN");
    strcpy(selections[7],"EIGHT");
    strcpy(selections[8],"NINE");

    selectionsDisplay();

    LCD_SetCursor(0,1);
    LCD_Blink();

    wheel0 = wheelDecoder_GetCounter();

    hyattControlPanelState = CONTROLPANEL_SELECT_ACTION;
}

void actionExecute(int8_t i){
    LCD_NoBlink();
    LCD_Clear();
    LCD_SetCursor(7,1);
    LCD_PutString(selections[i]);
    CyDelay(2000);
    hyattControlPanelState = CONTROLPANEL_IDLE_SETUP;
}

void hyattControPanelDisplayAction() {
    int16_t i = abs(wheel0 - wheelDecoder_GetCounter()) % SELECTIONCOUNTMAX;
    int x,y;
    x = (i / 3) * 7;
    y = (i % 3) + 1;
    LCD_SetCursor(x,y);

    if (!(FEED_OVERRIDE_Read() & 0x01)) actionExecute(i);
}

void hyattControPanelDisplayLoadSetup() {
    LCD_Clear();
    LCD_SetCursor(0,0);     LCD_PutString("Load Select");
    hyattControlPanelState = CONTROLPANEL_SELECT_LOAD;
}

void hyattControPanelDisplayLoad() {
}

void hyattControlPanelDisplayLoop() {
    switch (hyattControlPanelState) {
        case CONTROLPANEL_IDLE_SETUP:
            hyattControPanelDisplayIdleSetup();
            break;
        case CONTROLPANEL_IDLE:
            hyattControPanelDisplayIdle();
            break;
        case CONTROLPANEL_SELECT_ACTION_SETUP:
            hyattControPanelDisplayActionSetup();
            break;
        case CONTROLPANEL_SELECT_ACTION:
            hyattControPanelDisplayAction();
            break;
        case CONTROLPANEL_SELECT_LOAD_SETUP:
            hyattControPanelDisplayLoadSetup();
            break;
        case CONTROLPANEL_SELECT_LOAD:
            hyattControPanelDisplayLoad();
            break;
    }
}
