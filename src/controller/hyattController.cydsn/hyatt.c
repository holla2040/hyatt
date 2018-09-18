#include "project.h"
#include "hyatt.h"

int32_t  wheelCountLast;

/* 24HZ interrupt */
CY_ISR(isrHyattMainHandler) {
    hyattControlPanelKeypadTick();
}

void hyattInit() {
    isrHyatt_StartEx(isrHyattMainHandler);
    wheelEncoder_Start();
    
    hyattControlPanelKeypadInit();

    hyattStatus.axisSelected = AXISSELECTED_X;
    hyattStatus.jogWheelStepSize = JOGWHEELSTEPSIZE_SMALL;
}

void hyattLoop() {
    hyattControlPanelKeypadLoop();
}
