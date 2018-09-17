/* hyatt */

#include "hyattController.h"

CY_ISR(keyHandler) {
    keyPending = 1;
    Pin_IO_INT_ClearInterrupt();
}


void hyatt_init() {
    controllerConfig.axisSelected = X;
}

void hyatt_loop() {
    if (keyPending) {

    }
}
