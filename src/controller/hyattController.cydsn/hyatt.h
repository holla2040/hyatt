/* hyatt */
#ifndef controller_h
#define controller_h
#include <stdint.h>
    
enum AXIS{X=1,Y,Z,A};

uint8_t keyPending;

#define U3Addr      0x20
#define U4Addr      0x21

#define IOA_DIR     0x00
#define IOA_POL     0x02
#define IOA_GPINTEN 0x04
#define IOA_DEFVAL  0x06
#define IOA_INTCON  0x08
#define IOA_CON     0x0A
#define IOA_GPPU    0x0C
#define IOA_INTF    0x0E
#define IOA_INTCAP  0x10
#define IOA_GPIOA   0x12
#define IOA_OLAT    0x14

#define IOB_DIR     0x01
#define IOB_POL     0x03
#define IOB_GPINTEN 0x05
#define IOB_DEFVAL  0x07
#define IOB_INTCON  0x09
#define IOB_CON     0x0B
#define IOB_GPPU    0x0D
#define IOB_INTF    0x0F
#define IOB_INTCAP  0x11
#define IOB_GPIOB   0x13
#define IOB_OLAT    0x15

struct {
    uint8_t     axisSelected;
    uint8_t     feedOverride;
    uint8_t     jogWheelDelta;
    uint8_t     jogWheelDirection;
    uint8_t     jogWheelSpeed;
} controllerConfig;
    
#endif
