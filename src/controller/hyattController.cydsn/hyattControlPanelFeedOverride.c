#include "project.h"
#include "hyatt.h"
#include <stdlib.h>

uint8_t f_overrideLast;
#define FEEDTHRESHOLD 3

uint32_t timeoutFeedOverrideUpdate;
#define WHEELFEEDOVERRIDEINTERVAL 250


CY_ISR(feedOverrideHandler) {
    CyDelay(2); // need delay for off pin to settle
    (FEED_OVERRIDE_Read() & FEED_OVERRIDE_OFF) ? system_set_exec_state_flag(EXEC_FEED_HOLD) : system_set_exec_state_flag(EXEC_CYCLE_START);
    FEED_OVERRIDE_ClearInterrupt();
}

void hyattControlPanelFeedOverrideInit() {
    ADC_Start();
    ADC_StartConvert();
    
    AMUX_Select(AMUX_FEED_OVERRIDE_IN);
    timeoutFeedOverrideUpdate = 0;
    
    f_overrideLast = 255;
    FeedOverrideISR_StartEx(feedOverrideHandler);
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
    // uint8_t v = FEED_OVERRIDE_Read();
    // (v & FEED_OVERRIDE_OFF) ? system_set_exec_state_flag(EXEC_FEED_HOLD) : system_set_exec_state_flag(EXEC_CYCLE_START);

    // uint8_t newfo = (200*ADC_GetResult16())/4096;
    // this results in newfo being increments of 10
    uint8_t newfo = 10*round(20.0*ADC_GetResult16()/4096.0);
    if (abs(newfo - f_overrideLast) > FEEDTHRESHOLD) {
        feedOverrideSet(newfo);
    }

    if (hyattTicks > timeoutFeedOverrideUpdate) {
        feedOverrideSet(newfo);
        timeoutFeedOverrideUpdate = hyattTicks + WHEELFEEDOVERRIDEINTERVAL;
    }
}
