#!/usr/bin/env node
const {
    calcCRC16,
    CRC16_POLYNOME,
    CRC16_INITIAL,
    CRC16_XOR_OUT,
    unsigned
} = require('./crc16.js');

[
    function test1() {
        //              calcCRC16(array, polynome=CRC16_POLYNOME, initial=CRC16_INITIAL, xorOut=CRC16_XOR_OUT, reverseIn=CRC16_REV_IN, reverseOut=CRC16_REV_OUT) {
        let crc = calcCRC16(['C','?','!'].map(c => c.charCodeAt(0)),
            CRC16_POLYNOME,
            CRC16_INITIAL,
            CRC16_XOR_OUT,
            true,
            true
        );
        console.log(crc, crc.toString(16), 0x6C31 === crc?'pass':'fail' );
    },
		function test2() {
			['�','m','�','n','T','␃']
		}
].forEach(f=>f());
