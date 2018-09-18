/* hyatt */
#ifndef controller_h
#define controller_h
#include <stdint.h>
    
#define AXISSELECTED_X      0x0001
#define AXISSELECTED_Y      0x0002
#define AXISSELECTED_Z      0x0004
    
#define JOGWHEELSTEPSIZE_SMALL  0x0010
#define JOGWHEELSTEPSIZE_MEDIUM 0x0020
#define JOGWHEELSTEPSIZE_LARGE  0x0040

struct {
    uint8_t     axisSelected;
    uint8_t     feedOverride;
    uint8_t     jogWheelDelta;
    uint8_t     jogWheelDirection;
    uint8_t     jogWheelStepSize;
} controllerConfig;
    

void hyattControllerInit();
void hyattControllerLoop();


#endif
