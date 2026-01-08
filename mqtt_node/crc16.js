/*
 * CRC-16/MODBUS
 * Adapted from https://github.com/hayschan/esp-crc-suite
 */
const unsigned = _=>_>>>0;
const int16 = _=>_&0xFFFF;
const int8 = _=>_&0xFF;
const uint8 =_=>unsigned(int8(_));
const uint16 =_=>unsigned(int16(_));
const HIGH = _=>unsigned(_)&0xFF;
const LOW = _=>_>>>8;

//Forms: Normal 0x8005, reversed 0xA001, reciprocal 0x4003, and Reversed reciprocal 0xC002
const POLYNOME_NORMAL = 0x8005;
const POLYNOME_REVERSED = 0xA001;
const POLYNOME_RECIPROCAL = 0x4003;
const POLYNOME_REVERSED_RECIPROCAL = 0xC002;

const CRC16_POLYNOME = POLYNOME_REVERSED;
const CRC16_INITIAL  = 0xFFFF;
const CRC16_XOR_OUT  = 0x0000;
const CRC16_REV_IN   = true;
const CRC16_REV_OUT  = true;

function reverse8bits(x) {
    x = (((x & 0xAA) >>> 1) | uint8((x & 0x55) << 1));
    x = (((x & 0xCC) >>> 2) | uint8((x & 0x33) << 2));
    x =          ((x >>> 4) | uint8(x << 4));
    return x;
}

function reverse16bits(x) {
    x = (((x & 0xAAAA) >>> 1) | uint16((x & 0x5555) << 1));
    x = (((x & 0xCCCC) >>> 2) | uint16((x & 0x3333) << 2));
    x = (((x & 0xF0F0) >>> 4) | uint16((x & 0x0F0F) << 4));
    x = (( x >>> 8) | uint16(x << 8));
    return x;
}

/*
 * Function to calculate the CRC-16 using the MODBUS algorithm
 */
function CRC16(polynome=CRC16_POLYNOME, initial=CRC16_INITIAL, xorOut=CRC16_XOR_OUT, reverseIn=CRC16_REV_IN, reverseOut=CRC16_REV_OUT) {
    const self = this;
    this.polynome = polynome;
    this.initial = initial;
    this.xorOut = xorOut;
    this.reverseIn = reverseIn;
    this.reverseOut = reverseOut;
    this.crc;
    this.count;
    this.restart = restart;
    this.add_array = add_array;

    this.restart();

    this.reset = (polynome, initial, xorOut, reverseIn, reverseOut) => {
        this.polynome = polynome;
        this.initial = initial;
        this.xorOut = xorOut;
        this.reverseIn = reverseIn;
        this.reverseOut = reverseOut;
        this.restart();
    }
    this.getCount = () => {
        return this.count;
    }
    this.add = (b) => {
        this.count++;
        add_byte(b);
    }
    this.calc = () => {
        let rv = this.crc;//uint16_t
        if(this.reverseOut) {
            rv = reverse16bits(rv);
        }
        rv = rv ^ this.xorOut;
        return rv;
    }
    this.getCRC = () => {
        return this.calc();
    }

    function restart() {
        self.crc = self.initial;
        self.count = 0;
    }
    function add_array(array) {
        self.count += array.length;
        for(let i=0; i<array.length; i++) {
            add_byte(array[i]);
        }
    }
    function add_byte(b) {
        if(self.reverseIn) {
            b = reverse8bits(b);
        }
        self.crc = self.crc ^ (b << 8);
        for(let i=8; i; i--) {
            if(self.crc & (1<<15)) {
                self.crc = uint16(self.crc << 1) ^ self.polynome;
            } else {
                self.crc = uint16(self.crc << 1);
            }
        }
    }
}

function calcCRC16(array, polynome=CRC16_POLYNOME, initial=CRC16_INITIAL, xorOut=CRC16_XOR_OUT, reverseIn=CRC16_REV_IN, reverseOut=CRC16_REV_OUT) {
    const crc = new CRC16(polynome, initial, xorOut, reverseIn, reverseOut);
    crc.add_array(array);
    return crc.calc();
}

module.exports = { CRC16, calcCRC16, reverse16bits, reverse8bits, unsigned, HIGH, LOW, CRC16_POLYNOME, CRC16_INITIAL, CRC16_XOR_OUT, CRC16_REV_IN, CRC16_REV_OUT};
