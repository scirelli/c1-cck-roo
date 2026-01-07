#include "message.h"

const uint8_t START_CODON[] = { 0xCA, 0xFE, 0xBA, 0xBE };
const uint8_t END_CODON[] = { 0xDE, 0xAD, 0xBE, 0xEF };


uint16_t calcCRC() {
  uint16_t crc = 0;
  for (unsigned int i=0; i<CONTENT_SZ<<1; i++) {
    crc = _crc16_update (crc, ((uint8_t*)msg)[i]);
  }
  return crc;
}
