#include "project.h"
#include "hyatt.h"

uint32_t hyattTicks;

/* 24HZ interrupt */
CY_ISR(isrHyattMainHandler) {
    hyattTicks++;
}

void hyattInit() {
    isrHyatt_StartEx(isrHyattMainHandler);
    wheelEncoder_Start();
    
    I2C_Start();   
    CyDelay(50);
    
    IO_RESET_Write(0); // reset the IO Expanders
    CyDelay(1);
    IO_RESET_Write(1);
    
    hyattControlPanelDisplayInit();
    hyattControlPanelKeypadInit();
    
    hyattStatus.axisSelected = AXISSELECTED_X;
    hyattStatus.jogWheelStepSize = JOGWHEELSTEPSIZE_SMALL;
}

void hyattLoop() {
    hyattControlPanelKeypadLoop();
    hyattControlPanelDisplayLoop();
}
