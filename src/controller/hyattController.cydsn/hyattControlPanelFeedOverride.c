#include "project.h"
#include "hyatt.h"

#define FEED_OVERRIDE_BTN 0x01
#define FEED_OVERRIDE_OFF 0x02

CY_ISR(feedOverrideHandler) {
    uint8_t v = FEED_OVERRIDE_Read();
    hyattFeedOverrideButton = !((v & FEED_OVERRIDE_BTN) == FEED_OVERRIDE_BTN);
    hyattFeedOverrideOff    =  ((v & FEED_OVERRIDE_OFF) == FEED_OVERRIDE_OFF);
    FEED_OVERRIDE_ClearInterrupt();
}

void hyattControlPanelFeedOverrideInit() {
    ADC_Start();
    ADC_StartConvert();
    
    FeedOverrideISR_StartEx(feedOverrideHandler);
}
