#include "utils.h"

uint16_t utils_crc16(const uint8_t *data, size_t len)
{
    uint16_t crc = 0;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i]; // Simple XOR placeholder
    }
    return crc;
}