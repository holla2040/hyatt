/* hyatt */

#include "hyattController.h"


void hyattController_init() {
    hyattKeypad_init();

    controllerConfig.axisSelected = X;
}

void hyattController_loop() {
    hyattKeypad_loop();
}
