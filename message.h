#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <CRC16.h>

#define ADC_COUNT 9

// Remember to keep this struct a multiple of 2 or 4, add padding if needed.
typedef struct {
    uint16_t crc;                //2
    uint8_t adc_data[ADC_COUNT]; //9
    uint8_t gpio;                //1
} outgoingMsg_t;                 //=12 bytes

uint16_t calcCRC();


#endif /* _MESSAGE_H_ */
