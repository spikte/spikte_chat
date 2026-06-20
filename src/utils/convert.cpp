#include "../../include/utils/convert.hpp"

uint16_t bytesToU16(uint8_t *bytes) {
    uint16_t res;

    res = 0;
    for(int i = 0; i < 2; i++) {
        res |= (uint16_t)bytes[i] << (8 - i * 8);
    }

    return res;
}
uint32_t bytesToU32(uint8_t *bytes) {
    uint32_t res;

    res = 0;
    for(int i = 0; i < 4; i++) {
        res |= (uint32_t)bytes[i] << (24 - i * 8);
    }

    return res;
}
uint64_t bytesToU64(uint8_t *bytes) {
    uint64_t res;

    res = 0;
    for(int i = 0; i < 8; i++) {
        res |= (uint64_t)bytes[i] << (56 - i * 8);
    }

    return res;
}
