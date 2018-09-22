#include "project.h"
#include "hyatt.h"

void hyattControlPanelInit() {    
    hyattControlPanelFeedOverrideInit();
    hyattControlPanelDisplayInit();
    hyattControlPanelKeypadInit();
    hyattControlPanelWheelInit();
}

void hyattControlPanelLoop() {
    hyattControlPanelKeypadLoop();
    hyattControlPanelDisplayLoop();
    hyattControlPanelWheelLoop();
}


