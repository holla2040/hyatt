#ifndef INSPECT_H
#define INSPECT_H
#include "hyatt.h"
    
struct inspect {
    char label[CONTROLPANEL_LABELWIDTH];
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

float inspectPoints[2][2];
float inspectCirclePoints[3][2]; // x,y * 3
float inspectLength,inspectAngle;

    
#endif
