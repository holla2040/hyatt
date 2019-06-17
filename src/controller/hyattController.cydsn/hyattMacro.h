#ifndef MACROS_H
#define MACROS_H
#include "hyatt.h"
    
struct macro {
    char label[CONTROLPANEL_MACROLABELWIDTH];
    char block[50];
};    
    
struct macro macros[CONTROLPANEL_SELECTIONCOUNTMAX] = {
    {"X0Y0","G28G91Z0\nX0Y0G0G90"},
    {"HOME","$H"},
    {"G28","G28G91Z0\nG28"},
    {"TCX0Y0","G10L20P1X0Y0"},
    {"TCX0","G10L20P1X0"},
    {"TCY0","G10L20P1Y0"},
    {"G28Set","G28.1"},
    {"G54","G54"},
    {"G55","G55"},
};
    
#endif
