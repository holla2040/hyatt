#ifndef hyattKeypad_h
#define hyattKeypad_h
    
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
#define IOA_GPIO    0x12
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
#define IOB_GPIO    0x13
#define IOB_OLAT    0x15

#define KEY_X 0x0001
#define KEY_Y 0x0002
#define KEY_Z 0x0004
    
void hyattKeypadInit();
void hyattKeypadLoop();
void indicatorUpdate();

#endif