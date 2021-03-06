#ifndef MACROS_H
#define MACROS_H
#include "hyatt.h"
    
struct macro {
    char label[CONTROLPANEL_LABELWIDTH];
    char block[50];
};    
    
struct macro macros[CONTROLPANEL_SELECTIONCOUNTMAX] = {
    {"X0Y0","G90G53Z-3\nG54G0G90X0Y0"},
    {"HOME","$H"},
    {"",""},

    {"G28","G21G90G53Z-3\nG28G91X0Y0\nG90"},
    {"G30","G21G90G53Z-3\nG30G91X0Y0\nG90"},
    {"",""},

    {"G28Set","G28.1"},
    {"G30Set","G30.1"},
    {"",""}
};
    
#endif
