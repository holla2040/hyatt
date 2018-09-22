#include "project.h"
#include "hyatt.h"

#define FEED_OVERRIDE_BTN 0x01
#define FEED_OVERRIDE_OFF 0x02

CY_ISR(feedOverrideHandler) {
    uint8_t v = FEED_OVERRIDE_Read();
    hyattFeedOverrideButton = !((v & FEED_OVERRIDE_BTN) ? 1 : 0);
    hyattFeedOverrideOff    =  ((v & FEED_OVERRIDE_OFF) ? 1 : 0);
    
     
    
    FEED_OVERRIDE_ClearInterrupt();
}

void hyattControlPanelFeedOverrideInit() {
    ADC_Start();
    ADC_StartConvert();
    
    // FeedOverrideISR_StartEx(feedOverrideHandler);
}

void hyattControlPanelFeedOverrideLoop() {
    uint8_t v = FEED_OVERRIDE_Read();
    (v & FEED_OVERRIDE_OFF) ? system_set_exec_state_flag(EXEC_FEED_HOLD) : system_set_exec_state_flag(EXEC_CYCLE_START);
}