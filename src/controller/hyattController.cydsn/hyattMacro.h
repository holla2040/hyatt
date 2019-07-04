#ifndef MACROS_H
#define MACROS_H
#include "hyatt.h"
    
struct macro {
    char label[CONTROLPANEL_LABELWIDTH];
    char block[50];
};    
    
struct macro macros[CONTROLPANEL_SELECTIONCOUNTMAX] = {
    {"X0Y0","G53Z-3\nG54X0Y0G0G90"},
    {"HOME","$H"},
    {"G28","G28G91Z0\nG28"},

    {"G30","G30G91Z0\nG30"}, 
    {"",""},
    {"",""},

    {"G28Set","G28.1"},
    {"G30Set","G30.1"},
    {"",""}
};
    
#endif
