#include "project.h"
#include "hyatt.h"

#define STATE_CHANGE_INTERVAL 500
uint32_t timeoutStateUpdate;

void hyattControlPanelInit() {
    hyattControlPanelFeedOverrideInit();
    hyattControlPanelKeypadInit();
    hyattControlPanelDisplayInit();
    hyattControlPanelWheelInit();
    hyattControlPanelIRInit();
    timeoutStateUpdate = 0;
    hyattControlPanelState = CONTROLPANEL_IDLE_SETUP;
}

void hyattControlPanelLoop() {
    hyattControlPanelKeypadLoop();
    hyattControlPanelDisplayLoop();
    hyattControlPanelFeedOverrideLoop();
    hyattControlPanelIRLoop();

    if ((sys.state == STATE_IDLE) | (sys.state == STATE_JOG)) {  // can't just OR these STATE_IDLE = 0
        if (hyattControlPanelState == CONTROLPANEL_IDLE) hyattControlPanelWheelLoop();
    }

    if (hyattTicks > timeoutStateUpdate) {
        timeoutStateUpdate = hyattTicks + STATE_CHANGE_INTERVAL;
    }
}
