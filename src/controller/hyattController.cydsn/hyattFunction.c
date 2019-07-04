#include "project.h"
#include "hyatt.h"

void laserZeroAxisXY() {
    char buf[50];

    sprintf(buf,"G10L20P%dX%f",gc_state.modal.coord_select+1,settings.hyatt_zero_offset_x);
    grblBlockSend(buf);
    sprintf(buf,"G10L20P%dY%f",gc_state.modal.coord_select+1,settings.hyatt_zero_offset_y);
    grblBlockSend(buf);
}

void laserZeroOffsetSet() {
    char buf[50];
    sprintf(buf,"$33=%f",x);
    grblBlockSend(buf);
    sprintf(buf,"$34=%f",y);
    grblBlockSend(buf);
}

