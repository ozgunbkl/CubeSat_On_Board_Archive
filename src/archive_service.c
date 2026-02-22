#include "archive_service.h"
#include "commands.h"
#include "fdir_service.h"
#include "time_service.h"
#include "utils.h"
#include <string.h>

// Simulated Non-Volatile Memory
static uint8_t archive_memory[ARCHIVE_SIZE_BYTES];
static uint16_t write_index = 0;

void Archive_Init(void){
    write_index = 0;
    // In a real FSW, we might clear the memory or scan for the last entry
    memset(archive_memory, 0, ARCHIVE_SIZE_BYTES);
}

ArchiveStatus_t Archive_WriteRecord(uint16_t id, const uint8_t *data, uint16_t len){
    // 1. Parameter validation
    if(data == NULL || len > MAX_RECORD_PAYLOAD){
        return ARCHIVE_ERR_PARAM;
    }

    // 2. Bound checking (Will the whole struct fit?)
    if(write_index + sizeof(ArchiveRecord_t) > ARCHIVE_SIZE_BYTES){
        FaultReport_t memory_fault = {
            .source = SRC_ARCHIVE,
            .severity = FAULT_WARNING,
            .fault_code = 0xAF01, // Flash Full Code
            .timestamp = Time_GetSeconds()
        };
        FDIR_ReportFault(memory_fault);
        return ARCHIVE_ERR_FULL;
    }

    // 3. Record construction (The "Envelope")
    ArchiveRecord_t new_record;
    new_record.record_id = id;
    new_record.length = len;

    // Clear the payload buffer and copy actual data
    memset(new_record.payload, 0, MAX_RECORD_PAYLOAD);
    memcpy(new_record.payload, data, len);

    // 4. Integrity protection
    // Note : I only CRC the payload for this project
    new_record.crc = utils_crc16(new_record.payload, len);
    
    // 5. Sequential write (Deterministic)
    memcpy(&archive_memory[write_index], &new_record, sizeof(ArchiveRecord_t));
    write_index += sizeof(ArchiveRecord_t);

    return ARCHIVE_OK;

}

uint16_t Archive_GetUsage(void){
    return write_index;
}


ArchiveStatus_t Archive_ReadRecord(uint16_t index, ArchiveRecord_t *out_record){
    // 1. Parameter validation
    if(out_record == NULL){
        return ARCHIVE_ERR_PARAM;
    }

    // 2. Check if the index exists (is it within the written bounds?)
    uint16_t offset = index * sizeof(ArchiveRecord_t);
    if(offset + sizeof(ArchiveRecord_t) > write_index){
        return ARCHIVE_ERR_NOT_FOUND;
    }

    // 3. Extract the record from simulated memory
    // I copy the raw bytes from my 'flash' back into a struct format
    memcpy(out_record, &archive_memory[offset], sizeof(ArchiveRecord_t));

    // 4. Integrity validation (The 'space' check)
    // Recalculate CRC on the read data to ensure no bit-flips occurred
    uint16_t calculated_crc = utils_crc16(out_record->payload, out_record->length);
    
    if (calculated_crc != out_record->crc) {
        FaultReport_t archive_fault = {
            .source = SRC_ARCHIVE,
            .severity = FAULT_CRITICAL,
            .fault_code = 0x01, // CRC Mismatch
            .timestamp = Time_GetSeconds()
        };
        FDIR_ReportFault(archive_fault);
        
        return ARCHIVE_ERR_CRC; // Corruption detected!
    }

    return ARCHIVE_OK;
}

/**
 * @brief Test Hook: Manually corrupts a byte in memory to simulate radiation.
 * @param byte_index The exact byte in the 4096-byte array to flip.
 */
void Archive_CorruptMemoryForTest(uint16_t byte_index) {
    if (byte_index < ARCHIVE_SIZE_BYTES) {
        // XOR with 0xFF flips all 8 bits in that byte
        archive_memory[byte_index] ^= 0xFF; 
    }
}


void ARCHIVE_ProcessCommand(const uint8_t* payload, uint16_t len) {
    if (len < 1) return;

    uint8_t command_id = payload[0];

    switch (command_id) {
        case ARCHIVE_CMD_GET_BY_INDEX:
            // Corrected: Strict check for exactly 3 bytes (ID + 2-byte Index)
            if (len == 3) {
                uint16_t target_index = (payload[1] << 8) | payload[2];
                
                ArchiveRecord_t record;
                ArchiveStatus_t status = Archive_ReadRecord(target_index, &record);

                if (status == ARCHIVE_OK) {
                    printf("ARCHIVE: Read success at index %d.\n", target_index);
                    // TM_SendArchiveReport(record.payload, record.length);
                } else {
                    printf("ARCHIVE ERROR: Read failed (Status: %d)\n", status);
                }
            } else {
                // Helpful for Ground Station debugging
                FaultReport_t cmd_fault = {
                    .source = SRC_ARCHIVE,
                    .severity = FAULT_WARNING,
                    .fault_code = 0xAF02, // Error code for "Wrong Command Length"
                    .timestamp = Time_GetSeconds()
                };
                FDIR_ReportFault(cmd_fault);
                printf("ARCHIVE ERROR: GET_INDEX expects 3 bytes, got %d\n", len);
            }
            break;

        case ARCHIVE_CMD_WIPE_ALL:
            // Corrected: Strict check for safety key and length
            if (len == 2) {
                if (payload[1] == 0xAA) {
                    printf("ARCHIVE: !!! MEMORY WIPE INITIATED !!!\n");
                    write_index = 0; 
                    memset(archive_memory, 0, ARCHIVE_SIZE_BYTES);
                } else {
                    FaultReport_t security_fault = {
                        .source = SRC_ARCHIVE,
                        .severity = FAULT_WARNING,
                        .fault_code = 0xAF03, // Error code for "Invalid Security Key"
                        .timestamp = Time_GetSeconds()
                    };
                    FDIR_ReportFault(security_fault);
                    printf("ARCHIVE REJECTED: Invalid safety key 0x%02X\n", payload[1]);
                }
            } else {
                printf("ARCHIVE ERROR: WIPE expects 2 bytes, got %d\n", len);
            }
            break;

        default:
            FaultReport_t unknown_fault = {
                .source = SRC_ARCHIVE,
                .severity = FAULT_WARNING,
                .fault_code = 0xAF04,
                .timestamp = Time_GetSeconds()
            };
            FDIR_ReportFault(unknown_fault);
            printf("ARCHIVE: Unknown Command ID 0x%02X\n", command_id);
            break;
    }
}