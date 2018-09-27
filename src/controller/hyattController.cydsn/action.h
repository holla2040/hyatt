#ifndef MACROS_H
#define MACROS_H
#include "hyatt.h"
    
struct action {
    char label[CONTROLPANEL_ACTIONLABELWIDTH];
    char block[50];
};    
    
struct action actions[CONTROLPANEL_SELECTIONCOUNTMAX] = {
    {"X0Y0","$J=X0Y0F5000G1"},
    {"HOME","$H"},
    {"X10Y10","$J=X10Y10F5000G1"},
    {"",""},
    {"",""},
    {"",""},
    {"",""},
    {"",""},
    {"",""}
};
    
#endif
