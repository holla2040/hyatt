#include "hyatt.h"

extern uint32_t hyattTicks;

#define lcdAddr   0x27
#define DISPLAYUPDATEINTERVAL 10

const char watch[] = " \xa5";

uint32_t timeoutDisplaySlowUpdate;
uint32_t timeoutDisplayFastUpdate;



void hyattControlPanelDisplayInit() {
    LCD_Start(lcdAddr,20,4,0);
    
    LCD_SetCursor(0,0);
    LCD_Clear();
    LCD_SetCursor(0,0);     LCD_PutString("X");
    LCD_SetCursor(0,1);     LCD_PutString("Y");
    LCD_SetCursor(0,2);     LCD_PutString("Z");
    LCD_SetCursor(0,3);     LCD_PutString("G.. G. G.. G.. F..");
    LCD_SetCursor(12,0);    LCD_PutString("state");
    LCD_SetCursor(12,1);    LCD_PutString("M0 M5 M9");
    
    
    timeoutDisplaySlowUpdate = 0;
    timeoutDisplayFastUpdate = 0;
}

uint8_t watchCount;

void hyattControlPanelDisplayLoop() {
   char buf[100];
    if (hyattTicks > timeoutDisplaySlowUpdate) {
        LCD_SetCursor(12,0);
        switch(sys.state) {
            case STATE_IDLE:        LCD_PutString("Idle   "); break;
            case STATE_CYCLE:       LCD_PutString("Run    "); break;
            case STATE_HOLD:        LCD_PutString("Hold   "); break;
            case STATE_HOMING:      LCD_PutString("Home   "); break;
            case STATE_ALARM:       LCD_PutString("Alarm  "); break;
            case STATE_CHECK_MODE:  LCD_PutString("Check  "); break;
            case STATE_SAFETY_DOOR: LCD_PutString("Door   "); break;
            default:                LCD_PutString("?      "); break;
        };
        LCD_SetCursor(19,0);
        LCD_Write(watch[(++watchCount)%strlen(watch)]);
        
        
        LCD_SetCursor(1,3);
        sprintf(buf,"%d",54+gc_state.modal.coord_select);
        LCD_PutString(buf);

        LCD_SetCursor(5,3);
        sprintf(buf,"%d",gc_state.modal.motion);
        LCD_PutString(buf);

        LCD_SetCursor(8,3);
        sprintf(buf,"%d",21-gc_state.modal.units);
        LCD_PutString(buf);

        LCD_SetCursor(12,3);
        sprintf(buf,"%d",90+gc_state.modal.distance);
        LCD_PutString(buf);

        LCD_SetCursor(16,3);
        sprintf(buf,"%-4d",(uint16_t)gc_state.feed_rate);
        LCD_PutString(buf);
        
        
        timeoutDisplaySlowUpdate = hyattTicks + 500;
    }

    if (hyattTicks > timeoutDisplayFastUpdate) {
        float wco[N_AXIS];
        if (bit_isfalse(settings.status_report_mask,BITFLAG_RT_STATUS_POSITION_TYPE) || (sys.report_wco_counter == 0) ) {
            for (idx=0; idx< N_AXIS; idx++) {
                // Apply work coordinate offsets and tool length offset to current position.
                wco[idx] = gc_state.coord_system[idx]+gc_state.coord_offset[idx];
                if (idx == TOOL_LENGTH_OFFSET_AXIS) { wco[idx] += gc_state.tool_length_offset; }
/*
                if (bit_isfalse(settings.status_report_mask,BITFLAG_RT_STATUS_POSITION_TYPE)) {
                    print_position[idx] -= wco[idx];
                }
*/
                LCD_SetCursor(1,idx);
                sprintf(buf,"%10.4f",wco[idx]);
                LCD_PutString(buf);

            }
        }

        timeoutDisplayFastUpdate = hyattTicks + 20;
    }
}
