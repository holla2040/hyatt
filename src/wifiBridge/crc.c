#include <stdio.h>
#include <stdint.h>

#define CRC_POLY_16   0xA001
#define CRC_START_16  0x0000

uint16_t crc_tab16[256];

void init_crc16_tab( void ) {
    uint16_t i;
    uint16_t j;
    uint16_t crc;
    uint16_t c;

    for (i=0; i<256; i++) {
        crc = 0;
        c   = i;
        for (j=0; j<8; j++) {
            if ( (crc ^ c) & 0x0001 ) crc = ( crc >> 1 ) ^ CRC_POLY_16;
            else                      crc =   crc >> 1;

            c = c >> 1;
        }

        crc_tab16[i] = crc;
    }
}  

uint16_t crc_16( const unsigned char *input_str, size_t num_bytes ) {
    uint16_t crc;
    const unsigned char *ptr;
    size_t a;

    crc = CRC_START_16;
    ptr = input_str;

    if ( ptr != NULL ) for (a=0; a<num_bytes; a++) {

        crc = (crc >> 8) ^ crc_tab16[ (crc ^ (uint16_t) *ptr++) & 0x00FF ];
    }

    return crc;

}  /* crc_16 */
