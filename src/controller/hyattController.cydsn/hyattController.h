/* hyatt */
#ifndef controller_h
#define controller_h
#include <stdint.h>
    
enum AXIS{X=1,Y,Z,A};

struct {
    uint8_t     axisSelected;
    uint8_t     feedOverride;
    uint8_t     jogWheelDelta;
    uint8_t     jogWheelDirection;
    uint8_t     jogWheelSpeed;
} controllerConfig;
    
#endif
