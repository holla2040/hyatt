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
    
    if (sys.state == STATE_IDLE | sys.state == STATE_JOG) {  // can't just OR these STATE_IDLE = 0
        hyattControlPanelWheelLoop();
        hyattSelectLoop();
    }
}


