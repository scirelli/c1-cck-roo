#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <CRC.h>

#define MSG_ADC_SZ 9 //Total adc there can be. Old HAM 6 more IR. New one has 3 plus some other ADC

// Remember to keep this struct a multiple of 2 or 4, add padding if needed.
typedef struct {
    uint16_t crc;                 //2
    uint8_t adc_data[MSG_ADC_SZ]; //9
    uint8_t gpio;                 //1
} outgoingMsg_t;                  //=12 bytes

uint16_t calcCRC();


#endif /* _MESSAGE_H_ */
