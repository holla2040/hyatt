#ifndef MACROS_H
#define MACROS_H
#include "hyatt.h"
    
struct action {
    char label[CONTROLPANEL_ACTIONLABELWIDTH];
    char block[50];
};    
    
struct action actions[CONTROLPANEL_SELECTIONCOUNTMAX] = {
    {"X0Y0","X0 Y0 G1F2500"},
    {"HOME","$H"},
    {"X10Y10","X100 Y100 G1F2500"},
    {"",""},
    {"",""},
    {"",""},
    {"",""},
    {"",""},
    {"",""}
};
    
#endif
