#include "project.h"
#include "hyatt.h"

void hyattControlPanelInit() {    
    hyattControlPanelFeedOverrideInit();
    hyattControlPanelKeypadInit();
    hyattControlPanelDisplayInit();
    hyattControlPanelWheelInit();
}

void hyattControlPanelLoop() {
    hyattControlPanelKeypadLoop();
    hyattControlPanelDisplayLoop();
    hyattControlPanelFeedOverrideLoop();
    
    if ((sys.state == STATE_IDLE) || (sys.state == STATE_JOG)) {
        hyattControlPanelWheelLoop();
    }
}


