#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <util/crc16.h>

#define IR_PIN_LIST  {(IR_1_PIN), (IR_2_PIN), (IR_3_PIN), (IR_4_PIN), (IR_5_PIN), (IR_6_PIN)}
#define ALL_ADC_PINS {(IR_1_PIN), (IR_2_PIN), (IR_3_PIN), (IR_4_PIN), (IR_5_PIN), (IR_6_PIN), (AMBIENT_LIGHT_SENSE_PIN),(MOTOR_OVERCURRENT_SENSE_PIN),(SYSTEM_12V_OVERCURRENT_SENSE_PIN)}
#define ADC_COUNT 9

typedef struct {
    uint16_t crc;                //2
    uint8_t adc_data[ADC_COUNT]; //9
    uint8_t gpio;                //1
} outgoingMsg_t;                 //=12 bytes

const uint8_t START_CODON[] = { 0xCA, 0xFE, 0xBA, 0xBE };
const uint8_t END_CODON[] = { 0xDE, 0xAD, 0xBE, 0xEF };

uint16_t calcCRC();


#endif /* _MESSAGE_H_ */
