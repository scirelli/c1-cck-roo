#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <CRC.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#define LIMIT_SWITCH_FRONT_BIT      0
#define LIMIT_SWITCH_REAR_BIT       1
#define DOOR_SWITCH_OPEN_BIT        2
#define DOOR_SWITCH_CLOSE_BIT       3
#define PIXEL_POWER_GOOD_BIT        4
#define LCD_POWER_GOOD_BIT          5
#define SYSTEM_5V_POWER_GOOD_BIT    6

#define MSG_ADC_SZ 9 //Total adc there can be. Old HAM 6 more IR. New one has 3 plus some other ADC
#define GPIO_SZ    1
#define ERROR_FLAGS_SZ 0
#define PAYLOAD_SZ (MSG_ADC_SZ + GPIO_SZ + ERROR_FLAGS_SZ)

// Remember to keep this struct a multiple of 2 or 4, add padding if needed.
typedef struct {
    uint16_t crc;                 //2
    uint8_t adc_data[MSG_ADC_SZ]; //9
    uint8_t gpio;                 //1
} outgoingMsg_t;                  //=12 bytes

uint16_t calcCRC();
outgoingMsg_t* msg_insertCRC(outgoingMsg_t *msg);

#endif /* _MESSAGE_H_ */
