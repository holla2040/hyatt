#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include "hyatt.h"
    
struct function {
    char label[CONTROLPANEL_LABELWIDTH];
    void (*fPtr)();
};    
    
struct function functions[CONTROLPANEL_SELECTIONCOUNTMAX] = {
    {"L X0Y0",laserZeroAxisXY},
    {"LasSet",},
    {"",},

    {"",},
    {"",},
    {"",},

    {"",},
    {"",},
    {"",}
};
    
#endif
