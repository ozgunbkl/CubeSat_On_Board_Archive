#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Calculates a 16-bit checksum for data integrity.
 * @note This is a temporary implementation for the Archive Project.
 */
uint16_t utils_crc16(const uint8_t *data, size_t len);

#endif