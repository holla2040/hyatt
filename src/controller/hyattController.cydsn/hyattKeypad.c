#include "hyattKeypad.h"

CY_ISR(keyHandler) {
    keyPending = 1;
    Pin_IO_INT_ClearInterrupt();
}

void hyattKeypad_loop() {
    if (keyPending) {

    }
}

