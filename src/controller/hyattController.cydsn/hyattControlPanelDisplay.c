#include "hyatt.h"

#define lcdAddr   0x27

#define DISPLAYSLOWUPDATEINTERVAL 250
#define DISPLAYFASTUPDATEINTERVAL 100

const char watch[] = " \xa5";

extern parser_block_t gc_block;


void hyattControlPanelDisplayInit() {
    LCD_Start(lcdAddr,20,4,0);
    
    LCD_SetCursor(0,0);
    LCD_Clear();
    LCD_SetCursor(0,0);     LCD_PutString("X");
    LCD_SetCursor(0,1);     LCD_PutString("Y");
    LCD_SetCursor(0,2);     LCD_PutString("Z");
    LCD_SetCursor(10,2);    LCD_PutString("F");
    
    hyattTimeoutDisplaySlowUpdate = 0;
    hyattTimeoutDisplayFastUpdate = 0;
}

uint8_t watchCount;

void hyattControlPanelDisplayLoop() {
   char buf[100];
    if (hyattTicks > hyattTimeoutDisplaySlowUpdate) {
        LCD_SetCursor(10,1);
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
              
        LCD_SetCursor(10,0);
        sprintf(buf,"%d",54+gc_state.modal.coord_select);
        LCD_PutString(buf);

        LCD_SetCursor(13,0);
        gc_state.modal.units ?  LCD_PutString("INCH"):LCD_PutString("MM  ");

/*
        LCD_SetCursor(8,3);
        gc_state.modal.motion ?  LCD_PutString("LINEAR"):LCD_PutString("RAPID ");

        LCD_SetCursor(15,3);
        gc_state.modal.distance ?  LCD_PutString("INCRE"):LCD_PutString("ABSOL");
*/
        
        LCD_SetCursor(0,3);
        lastBlock[20] = 0; // clip lastBlock to display width
        sprintf(buf,"%-20s",lastBlock);
        LCD_PutString(buf);
        
        LCD_SetCursor(18,1);
        (gc_block.modal.spindle == SPINDLE_ENABLE_CW) ? LCD_PutString("S"): LCD_PutString(" ");

        LCD_SetCursor(19,1);
        (gc_block.modal.coolant == COOLANT_MIST_ENABLE) ? LCD_PutString("A"): LCD_PutString(" ");

        LCD_SetCursor(11,2);
        sprintf(buf,"%-4d",(uint16_t)gc_state.feed_rate);
        LCD_PutString(buf);

        LCD_SetCursor(16,2);
        sprintf(buf,"%3d%%",sys.f_override);
        LCD_PutString(buf);
        
        
        


/*
        LCD_SetCursor(5,3);
        sprintf(buf,"%d",gc_state.modal.motion);
        LCD_PutString(buf);

        LCD_SetCursor(8,3);
        sprintf(buf,"%d",21-gc_state.modal.units);
        LCD_PutString(buf);

        LCD_SetCursor(12,3);
        sprintf(buf,"%d",90+gc_state.modal.distance);
        LCD_PutString(buf);
*/

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
                sprintf(buf,"%7.3f",print_position[idx]*INCH_PER_MM);
            } else {
                sprintf(buf,"%8.2f",print_position[idx]);
            }
            LCD_PutString(buf);
        }

        hyattTimeoutDisplayFastUpdate = hyattTicks + DISPLAYFASTUPDATEINTERVAL;
    }
}
