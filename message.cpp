#include "message.h"

const uint8_t START_CODON[] = { 0xCA, 0xFE, 0xBA, 0xBE };
const uint8_t END_CODON[] = { 0xDE, 0xAD, 0xBE, 0xEF };

uint16_t calcCRC(outgoingMsg_t msg) {
    // CRC16 crc(
    //     CRC16_MODBUS_POLYNOME,
    //     CRC16_MODBUS_INITIAL,
    //     CRC16_MODBUS_XOR_OUT,
    //     CRC16_MODBUS_REV_IN,
    //     CRC16_MODBUS_REV_OUT
    // );
    // for (unsigned int i=0; i<sizeof(msg); i++) {
    //     crc.add(((uint8_t*)msg)[i]);
    // }
    return calcCRC16(
        (uint8_t*)&msg,
        sizeof(msg),
        CRC16_MODBUS_POLYNOME,
        CRC16_MODBUS_INITIAL,
        CRC16_MODBUS_XOR_OUT,
        CRC16_MODBUS_REV_IN,
        CRC16_MODBUS_REV_OUT,
        CRC_YIELD_DISABLED
    );
}
