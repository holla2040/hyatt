#ifndef MACROS_H
#define MACROS_H
#include "hyatt.h"
    
struct macro {
    char label[CONTROLPANEL_MACROLABELWIDTH];
    char block[50];
};    
    
struct macro macros[CONTROLPANEL_SELECTIONCOUNTMAX] = {
    // {"X0Y0Z1","X0Y0Z1G1F2500"},
    {"X0Y0Z1","X0Y0Z1G0"},
    {"HOME","$H"},
    {"G28","G28"},
    {"",""},
    {"",""},
    {"G28Set","G28.1"},
    {"G54","G54"},
    {"G55","G55"},
    {"G56","G56"}
};
    
#endif
