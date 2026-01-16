#include "unity.h"
#include "archive_service.h"
#include <string.h>

void setUp(void){
    Archive_Init();
}

void tearDown(void){
    // Nothing to clean up after each test
}

void test_Archive_WriteAndReadMatch(void){
    // 1. Prepare dummy data (e.g. simulated battey voltage)
    uint16_t sensor_id = 0xAA;
    uint8_t sensor_data[] = {0x12, 0x34, 0x56};
    uint16_t data_len = sizeof(sensor_data);

    // 2. Write to archive
    ArchiveStatus_t status = Archive_WriteRecord(sensor_id, sensor_data, data_len);

    TEST_ASSERT_EQUAL(ARCHIVE_OK, status);

    // 3. Read it back from Index 0
    ArchiveRecord_t result;
    status = Archive_ReadRecord(0, &result);
    TEST_ASSERT_EQUAL(ARCHIVE_OK, status);

    // 4. Verify the bits did not change!
    TEST_ASSERT_EQUAL(sensor_id, result.record_id);
    TEST_ASSERT_EQUAL(data_len, result.length);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(sensor_data, result.payload, data_len);

}

void test_Archive_ReportFullWhenMemoryExceeded(void){
    uint8_t dummy_data[10] = {0};
    ArchiveStatus_t status;

    // 1. Calculate how many records fit in 4096 bytes
    uint16_t capacity = ARCHIVE_SIZE_BYTES / sizeof(ArchiveRecord_t);

    // 2. Fill the archive to the brim
    for(int i = 0; i < capacity; i++){
        status = Archive_WriteRecord(i, dummy_data, 10);
        TEST_ASSERT_EQUAL(ARCHIVE_OK, status);
    }

    // 3. This next write should be the one that fails
    status = Archive_WriteRecord(99, dummy_data, 10);
    TEST_ASSERT_EQUAL(ARCHIVE_ERR_FULL, status);

}

void test_Archive_DetectsRadiationCorruption(void) {
    uint16_t id = 0xBC;
    uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    
    // 1. Write a healthy record
    Archive_WriteRecord(id, data, 4);

    // 2. Simulate Radiation: Corrupt a byte in the payload area of record 0
    // In my struct, payload starts after record_id (2) and length (2) = byte 4
    Archive_CorruptMemoryForTest(4); 

    // 3. Try to Read
    ArchiveRecord_t result;
    ArchiveStatus_t status = Archive_ReadRecord(0, &result);

    // 4. Verification: It MUST return a CRC error
    TEST_ASSERT_EQUAL(ARCHIVE_ERR_CRC, status);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_Archive_WriteAndReadMatch);
    RUN_TEST(test_Archive_ReportFullWhenMemoryExceeded);
    RUN_TEST(test_Archive_DetectsRadiationCorruption);

    return UNITY_END();
}