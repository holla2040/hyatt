#ifndef INSPECT_H
#define INSPECT_H
#include "hyatt.h"
    
struct inspect {
    char label[CONTROLPANEL_MACROLABELWIDTH];
};    
    
struct inspect inspects[CONTROLPANEL_SELECTIONCOUNTMAX] = {
    {"Point"},
    {"Point"},
    {"Measure"},
    {"Circle"},
    {"Circle"},
    {"Skew"},
    {"Go"},
    {"Go"},
    {"Clear"}
};
    
#endif
