#ifndef MACROS_H
#define MACROS_H
#include "hyatt.h"
    
struct action {
    char label[CONTROLPANEL_ACTIONLABELWIDTH];
    char block[50];
};    
    
struct action actions[CONTROLPANEL_SELECTIONCOUNTMAX] = {
    {"X0Y0","$J=X0Y0F2500"},
    {"HOME","$H"},
    {"X10Y10","$J=X100Y10F100"},
    {"",""},
    {"",""},
    {"",""},
    {"",""},
    {"",""},
    {"",""}
};
    
#endif
