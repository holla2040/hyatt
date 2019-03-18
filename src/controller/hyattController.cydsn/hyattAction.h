#ifndef MACROS_H
#define MACROS_H
#include "hyatt.h"
    
struct action {
    char label[CONTROLPANEL_ACTIONLABELWIDTH];
    char block[50];
};    
    
struct action actions[CONTROLPANEL_SELECTIONCOUNTMAX] = {
    {"X0Y0Z0","X0Y0Z0G1F2500"},
    {"HOME","$H"},
    {"G28 GO","G28"},
    {"",""},
    {"",""},
    {"G28Set","G28.1"},
    {"G54","G54"},
    {"G55","G55"},
    {"G56","G56"}
};
    
#endif
