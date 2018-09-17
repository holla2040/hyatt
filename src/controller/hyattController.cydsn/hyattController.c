/* hyatt */
#include "project.h"
#include "hyattController.h"
#include "hyattKeypad.h"

uint16_t keyPadIndicatorCount;

CY_ISR(isrHyattHandler) {
    keyPadIndicatorCount++;
}


void hyattControllerInit() {
    isrHyatt_StartEx(isrHyattHandler);
    
    hyattKeypadInit();

    controllerConfig.axisSelected = AXISSELECTEDX;
}

void hyattControllerLoop() {
    hyattKeypadLoop();
}

uint32_t millis() {
    return 0;
}