#ifndef MACROS_H
#define MACROS_H
#include "hyatt.h"
    
struct action {
    char label[CONTROLPANEL_ACTIONLABELWIDTH];
    char block[50];
};    
    
struct action actions[CONTROLPANEL_SELECTIONCOUNTMAX] = {
    {"X0Y0Z0","X0 Y0 Z0 G1F2500"},
    {"HOME","$H"},
    {"X10Y10","X100 Y100 G1F2500"},
    {"MAXG55","G55G1F2500X780Y750"},
    {"",""},
    {"",""},
    {"",""},
    {"",""},
    {"",""}
};
    
#endif
