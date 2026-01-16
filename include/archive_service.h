#ifndef ARCHIVE_SERVICE_H
#define ARCHIVE_SERVICE_H

#include <stdint.h>
#include <stddef.h>
#include "utils.h"

#define ARCHIVE_SIZE_BYTES 4096
#define MAX_RECORD_PAYLOAD 128

void Archive_CorruptMemoryForTest(uint16_t byte_index);

typedef enum {
    ARCHIVE_OK = 0,
    ARCHIVE_ERR_FULL,
    ARCHIVE_ERR_CRC,
    ARCHIVE_ERR_PARAM,
    ARCHIVE_ERR_NOT_FOUND
} ArchiveStatus_t;

/**
 * @brief Internal representation of a stored record.
 * This is the "envelope" that protects the raw data.
 */
typedef struct {
    uint16_t record_id;
    uint16_t length;
    uint8_t  payload[MAX_RECORD_PAYLOAD];
    uint16_t crc;
} ArchiveRecord_t;

/* --- Deterministic API --- */

// Initializes the archive pointer (simulated boot-up)
void Archive_Init(void);

// Stores a new record in the sequential buffer
ArchiveStatus_t Archive_WriteRecord(uint16_t id, const uint8_t *data, uint16_t len);

// Reads a record back by index (for later downlink integration)
ArchiveStatus_t Archive_ReadRecord(uint16_t index, ArchiveRecord_t *out_record);

// Returns current memory usage status
uint16_t Archive_GetUsage(void);

#endif