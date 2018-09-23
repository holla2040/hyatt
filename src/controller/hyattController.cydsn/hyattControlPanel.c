#include "project.h"
#include "hyatt.h"

void hyattControlPanelInit() {    
    hyattControlPanelFeedOverrideInit();
    hyattControlPanelKeypadInit();
    hyattControlPanelDisplayInit();
    hyattControlPanelWheelInit();
    hyattSelectInit();
}

void hyattControlPanelLoop() {
    hyattControlPanelKeypadLoop();
    hyattControlPanelDisplayLoop();
    hyattControlPanelFeedOverrideLoop();
    
    if (sys.state & (STATE_IDLE | STATE_JOG)) {
        hyattControlPanelWheelLoop();
        hyattSelectLoop();
    }
}


