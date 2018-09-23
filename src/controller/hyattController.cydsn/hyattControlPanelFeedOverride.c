#include "project.h"
#include "hyatt.h"
#include <stdlib.h>

#define FEED_OVERRIDE_BTN 0x01
#define FEED_OVERRIDE_OFF 0x02

uint8_t f_overrideLast;
#define FEEDTHRESHOLD 3

CY_ISR(feedOverrideHandler) {
    uint8_t v = FEED_OVERRIDE_Read();
    hyattFeedOverrideButton = !((v & FEED_OVERRIDE_BTN) ? 1 : 0);
    hyattFeedOverrideOff    =  ((v & FEED_OVERRIDE_OFF) ? 1 : 0);
    
    FEED_OVERRIDE_ClearInterrupt();
}

void hyattControlPanelFeedOverrideInit() {
    ADC_Start();
    ADC_StartConvert();
    
    AMUX_Select(AMUX_FEED_OVERRIDE_IN);
    
    f_overrideLast = 255;
    // FeedOverrideISR_StartEx(feedOverrideHandler);
}

void feedOverrideSet(uint8_t v) {
    v = min(v,MAX_FEED_RATE_OVERRIDE);
    v = max(v,MIN_FEED_RATE_OVERRIDE);
    f_overrideLast = sys.f_override = v;
      
    sys.report_ovr_counter = 0; // Set to report change immediately
    plan_update_velocity_profile_parameters();
    plan_cycle_reinitialize();
}

void hyattControlPanelFeedOverrideLoop() {
    uint8_t v = FEED_OVERRIDE_Read();
    (v & FEED_OVERRIDE_OFF) ? system_set_exec_state_flag(EXEC_FEED_HOLD) : system_set_exec_state_flag(EXEC_CYCLE_START);
    
    uint8_t newfo = (200*ADC_GetResult16())/4096;
    if (abs(newfo - f_overrideLast) > FEEDTHRESHOLD) {
        feedOverrideSet(newfo);
    }
}