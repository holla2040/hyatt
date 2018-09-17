/* hyatt */
#ifndef controller_h
#define controller_h
#include <stdint.h>
    
enum {AXISSELECTEDX=1,AXISSELECTEDY,AXISSELECTEDZ};

uint32_t millis();

struct {
    uint8_t     axisSelected;
    uint8_t     feedOverride;
    uint8_t     jogWheelDelta;
    uint8_t     jogWheelDirection;
    uint8_t     jogWheelSpeed;
} controllerConfig;
    

void hyattControllerInit();
void hyattControllerLoop();


#endif
