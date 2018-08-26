#include <stdio.h>
#include "project.h"

uint16_t count;
uint8_t btn;
uint8_t off;

CY_ISR(FeedOverrideHandler) {
    uint8_t v = FEED_OVERRIDE_Read();
    btn = !((v & 0x01) == 0x01);
    off = ((v & 0x02) == 0x02);
    FEED_OVERRIDE_ClearInterrupt();
}

int main(void){
    char status[100];
    
    CyGlobalIntEnable;

    Serial_Start();
    Serial_PutString("\x1B[2J"); // home
    Serial_PutString("feedOverride main\n");
    
    FeedOverrideADC_Start();
    FeedOverrideADC_StartConvert();
    
    FeedOverrideISR_StartEx(FeedOverrideHandler);
        
    for(;;) {
        Serial_PutString("\x1B[0;0f\x1B[K"); // home
        sprintf(status,"cnt:  %d\nknob: %d     \nbtn:  %d\noff:  %d\n\n",count++,(200*FeedOverrideADC_GetResult16())/4096,btn,off);
        Serial_PutString(status);
        CyDelay(100);
    }
}