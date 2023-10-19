// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#include <cstdint>
#else
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "azure_c_shared_utility/uuid.h"

static TEST_MUTEX_HANDLE g_testByTest;

#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_stdint.h"

#include "azure_c_shared_utility/memory_data.h"

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)
static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(memory_data_ut)

TEST_SUITE_INITIALIZE(a)
{
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    (void)umocktypes_stdint_register_types();
}

TEST_SUITE_CLEANUP(b)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
}

TEST_FUNCTION_INITIALIZE(c)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(d)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* read_uint8_t */

/*Tests_SRS_MEMORY_DATA_02_041: [ read_uint8_t shall write in destination the byte at source ]*/
TEST_FUNCTION(read_uint8_t_succeeds)
{
    ///arrange
    size_t size = sizeof(uint8_t);
    unsigned char* source = (unsigned char*)malloc(size);
    uint64_t expectedValue = 0;
    size_t i;
    uint8_t destination = 0;
    ASSERT_IS_NOT_NULL(source);
    for (i = 0; i < size; i++)
    {
        source[i] = (unsigned char)(i + 1);
        expectedValue = (expectedValue << 8) + source[i];
    }

    ///act
    read_uint8_t(source, &destination);

    ///assert
    ASSERT_ARE_EQUAL(uint8_t, expectedValue, destination);

    ///clean
    free(source);
}

/* read_uint16_t */

/*Tests_SRS_MEMORY_DATA_02_042: [ read_uint16_t shall write in destination the bytes at source MSB first and return. ]*/
TEST_FUNCTION(read_uint16_t_succeeds)
{
    ///arrange
    size_t size = sizeof(uint16_t);
    unsigned char* source = (unsigned char*)malloc(size);
    uint64_t expectedValue = 0;
    uint16_t destination = 0;
    size_t i;
    ASSERT_IS_NOT_NULL(source);
    for (i = 0; i < size; i++)
    {
        source[i] = (unsigned char)(i + 1);
        expectedValue = (expectedValue << 8) + source[i];
    }

    ///act
    read_uint16_t(source, &destination);

    ///assert
    ASSERT_ARE_EQUAL(uint16_t, expectedValue, destination);

    ///clean
    free(source);
}

/* read_uint32_t */

/*Tests_SRS_MEMORY_DATA_02_043: [ read_uint32_t shall write in destination the bytes at source MSB first. ]*/
TEST_FUNCTION(read_uint32_t_succeeds)
{
    ///arrange
    size_t size = sizeof(uint32_t);
    unsigned char* source = (unsigned char*)malloc(size);
    uint64_t expectedValue = 0;
    size_t i;
    uint32_t destination = 0;
    ASSERT_IS_NOT_NULL(source);
    for (i = 0; i < size; i++)
    {
        source[i] = (unsigned char)(i + 1);
        expectedValue = (expectedValue << 8) + source[i];
    }

    ///act
    read_uint32_t(source, &destination);

    ///assert
    ASSERT_ARE_EQUAL(uint32_t, expectedValue, destination);

    ///clean
    free(source);
}

/* read_uint64_t */
/*Tests_SRS_MEMORY_DATA_02_044: [ read_uint64_t shall write in destination the bytes at source MSB first. ]*/
TEST_FUNCTION(read_uint64_t_succeeds)
{
    ///arrange
    size_t size = sizeof(uint64_t);
    unsigned char* source = (unsigned char*)malloc(size);
    uint64_t expectedValue = 0;
    size_t i;
    uint64_t destination = 0;
    ASSERT_IS_NOT_NULL(source);
    for (i = 0; i < size; i++)
    {
        source[i] = (unsigned char)(i + 1);
        expectedValue = (expectedValue << 8) + source[i];
    }

    ///act
    read_uint64_t(source, &destination);

    ///assert
    ASSERT_ARE_EQUAL(uint64_t, expectedValue, destination);

    ///clean
    free(source);
}

/* write_uint8_t */

/*Tests_SRS_MEMORY_DATA_02_050: [ write_uint8_t shall write in destination the byte of value. ]*/
TEST_FUNCTION(write_uint8_t_succeeds)
{
    ///arrange
    uint8_t value = 0;
    unsigned char destination[8] = { 0 };
    size_t i;
    size_t j;

    for (i = 0; i < sizeof(uint8_t); i++)
    {
        value = (value << 8) + ((uint8_t)i + 1);
    }

    ///act
    write_uint8_t(destination, value);

    ///assert
    for (j = 0; j < sizeof(uint8_t); j++)
    {
        ASSERT_ARE_EQUAL(uint8_t, j+1, destination[j]);
    }

}

/* write_uint16_t */

/*Tests_SRS_MEMORY_DATA_02_051: [ write_uint16_t shall write in destination the bytes of value MSB first. ]*/
TEST_FUNCTION(write_uint16_t_succeeds)
{
    ///arrange
    uint16_t value = 0;
    unsigned char destination[8] = { 0 };
    size_t i;
    size_t j;
    for (i = 0; i < sizeof(uint16_t); i++)
    {
        value = (value << 8) + ((uint16_t)i + 1);
    }

    ///act
    write_uint16_t(destination, value);

    ///assert
    for (j = 0; j < sizeof(uint16_t); j++)
    {
        ASSERT_ARE_EQUAL(uint16_t, j + 1, destination[j]);
    }

}

/* write_uint32_t */

/*Tests_SRS_MEMORY_DATA_02_052: [ write_uint32_t shall write in destination the bytes of value MSB first. ]*/
TEST_FUNCTION(write_uint32_t_succeeds)
{
    ///arrange
    uint32_t value = 0;
    size_t i;
    size_t j;
    unsigned char destination[8] = { 0 };
    for (i = 0; i < sizeof(uint32_t); i++)
    {
        value = (value << 8) + ((uint32_t)i + 1);
    }

    ///act
    write_uint32_t(destination, value);

    ///assert
    for (j = 0; j < sizeof(uint32_t); j++)
    {
        ASSERT_ARE_EQUAL(uint32_t, j + 1, destination[j]);
    }

}

/* write_uint64_t */

/*Tests_SRS_MEMORY_DATA_02_053: [ write_uint64_t shall write in destination the bytes of value MSB first. ]*/
TEST_FUNCTION(write_uint64_t_succeeds)
{
    ///arrange
    uint64_t value = 0;
    size_t i;
    size_t j;
    unsigned char destination[8] = { 0 };
    for (i = 0; i < sizeof(uint64_t); i++)
    {
        value = (value << 8) + ((uint64_t)i + 1);
    }

    ///act
    write_uint64_t(destination, value);

    ///assert
    for (j = 0; j < sizeof(uint64_t); j++)
    {
        ASSERT_ARE_EQUAL(uint64_t, j + 1, destination[j]);
    }

}

/* write_int8_t */

/*Tests_SRS_MEMORY_DATA_02_054: [ write_int8_t shall write at destination the byte of value. ]*/
TEST_FUNCTION(write_int8_t_succeeds)
{
    ///arrange
    int8_t value = 0;
    size_t i;
    size_t j;
    unsigned char destination[8] = { 0 };
    for (i = 0; i < sizeof(int8_t); i++)
    {
        value = (value << 8) + ((int8_t)i + 1);
    }

    ///act
    write_int8_t(destination, value);

    ///assert
    for (j = 0; j < sizeof(int8_t); j++)
    {
        ASSERT_ARE_EQUAL(int8_t, j + 1, destination[j]);
    }
}

/*Tests_SRS_MEMORY_DATA_02_054: [ write_int8_t shall write at destination the byte of value. ]*/
TEST_FUNCTION(write_int8_t_succeeds_2)
{
    ///arrange
    int8_t value = INT8_MIN;

    unsigned char destination[8] = { 0 };

    ///act
    write_int8_t(destination, value);

    ///assert
    ASSERT_ARE_EQUAL(uint8_t, 0x80, destination[0]);

}

/* write_int16_t */

/*Tests_SRS_MEMORY_DATA_02_055: [ write_int16_t shall write at destination the bytes of value starting with MSB. ]*/
TEST_FUNCTION(write_int16_t_succeeds)
{
    ///arrange
    int16_t value = 0;
    unsigned char destination[8] = { 0 };
    size_t i;
    size_t j;
    for (i = 0; i < sizeof(int16_t); i++)
    {
        value = (value << 8) + ((int16_t)i + 1);
    }

    ///act
    write_int16_t(destination, value);

    ///assert
    for (j = 0; j < sizeof(int16_t); j++)
    {
        ASSERT_ARE_EQUAL(int16_t, j + 1, destination[j]);
    }

}

/*Tests_SRS_MEMORY_DATA_02_055: [ write_int16_t shall write at destination the bytes of value starting with MSB. ]*/
TEST_FUNCTION(write_int16_t_succeeds_2)
{
    ///arrange
    int16_t value = INT16_MIN + 1; /*0x8001*/
    unsigned char destination[8] = { 0 };

    ///act
    write_int16_t(destination, value);

    ///assert
    ASSERT_ARE_EQUAL(uint8_t, 0x80, destination[0]);
    ASSERT_ARE_EQUAL(uint8_t, 1,    destination[1]);
}

/* write_int32_t */

/*Tests_SRS_MEMORY_DATA_02_056: [ write_int32_t shall write at destination the bytes of value starting with MSB ]*/
TEST_FUNCTION(write_int32_t_succeeds)
{
    ///arrange
    int32_t value = INT32_MIN + (1<<16)+(2<<8)+3;

    unsigned char destination[8] = { 0 };

    ///act
    write_int32_t(destination, value);

    ///assert
    ASSERT_ARE_EQUAL(uint8_t, 0x80, destination[0]);
    ASSERT_ARE_EQUAL(uint8_t, 1, destination[1]);
    ASSERT_ARE_EQUAL(uint8_t, 2, destination[2]);
    ASSERT_ARE_EQUAL(uint8_t, 3, destination[3]);
}

/*Tests_SRS_MEMORY_DATA_02_057: [ write_int64_t shall write at destination the bytes of value starting with MSB. ]*/
TEST_FUNCTION(write_int64_t_succeeds)
{
    ///arrange
    int64_t value = 0;
    unsigned char destination[8] = { 0 };
    size_t i;
    size_t j;
    for (i = 0; i < sizeof(int64_t); i++)
    {
        value = (value << 8) + ((int64_t)i + 1);
    }

    ///act
    write_int64_t(destination, value);

    ///assert
    for (j = 0; j < sizeof(int64_t); j++)
    {
        ASSERT_ARE_EQUAL(int64_t, j + 1, destination[j]);
    }

}

/*Tests_SRS_MEMORY_DATA_02_057: [ write_int64_t shall write at destination the bytes of value starting with MSB. ]*/
TEST_FUNCTION(write_int64_t_succeeds_2)
{
    ///arrange
    int64_t value = INT64_MIN + ((int64_t)1 << 48) + ((int64_t)2 << 40) + ((int64_t)3 << 32) + ((int64_t)4 << 24) + ((int64_t)5 << 16) + ((int64_t)6 << 8) + (int64_t)7;
    unsigned char destination[8] = { 0 };

    ///act
    write_int64_t(destination, value);

    ///assert
    ASSERT_ARE_EQUAL(uint64_t, 0x80, destination[0]);
    ASSERT_ARE_EQUAL(uint64_t, 1, destination[1]);
    ASSERT_ARE_EQUAL(uint64_t, 2, destination[2]);
    ASSERT_ARE_EQUAL(uint64_t, 3, destination[3]);
    ASSERT_ARE_EQUAL(uint64_t, 4, destination[4]);
    ASSERT_ARE_EQUAL(uint64_t, 5, destination[5]);
    ASSERT_ARE_EQUAL(uint64_t, 6, destination[6]);
    ASSERT_ARE_EQUAL(uint64_t, 7, destination[7]);
}

/* write_uuid_t */

/*Tests_SRS_MEMORY_DATA_02_058: [ write_uuid_t shall write at destination the bytes of value ]*/
TEST_FUNCTION(write_uuid_t_succeeds)
{
    ///arrange
    const UUID_T value = { 0x42, 0x43, 0x42, 0x43, 0x42, 0x43, 0x42, 0x43, 0x42, 0x43, 0x42, 0x43, 0x42, 0x43, 0x42, 0x43 };
    unsigned char destination[sizeof(UUID_T)] = { 0 };
    char* expected_bytes_string;
    char* actual_bytes_string;

    ///act
    write_uuid_t(destination, value);

    ///assert
    expected_bytes_string = UUID_to_string(&value);
    actual_bytes_string = UUID_to_string((const UUID_T*)&destination[0]);
    ASSERT_ARE_EQUAL(char_ptr, expected_bytes_string, actual_bytes_string);
    free(expected_bytes_string);
    free(actual_bytes_string);
}

/*Tests_SRS_MEMORY_DATA_02_058: [ write_uuid_t shall write at destination the bytes of value ]*/
TEST_FUNCTION(write_uuid_t_succeeds_2)
{
    ///arrange
    const UUID_T value = { 0xAA, 0x00, 0xAB, 0x01, 0xAC, 0x02, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8, 0xF7, 0xF6, 0xF5 };
    unsigned char destination[sizeof(UUID_T)] = { 0 };
    char* expected_bytes_string;
    char* actual_bytes_string;

    ///act
    write_uuid_t(destination, value);

    ///assert
    expected_bytes_string = UUID_to_string(&value);
    actual_bytes_string = UUID_to_string((const UUID_T*)&destination[0]);
    ASSERT_ARE_EQUAL(char_ptr, expected_bytes_string, actual_bytes_string);
    free(expected_bytes_string);
    free(actual_bytes_string);
}

/* read_int8_t */

/*Tests_SRS_MEMORY_DATA_02_045: [ read_int8_t shall write in destination the signed byte at source. ]*/
TEST_FUNCTION(read_int8_t_positive)
{
    ///arrange
    unsigned char source[1] = { 1 };
    int8_t destination;

    ///act
    read_int8_t(source, &destination);

    ///assert
    ASSERT_ARE_EQUAL(int8_t, 1, destination);

    ///clean
}

/*Tests_SRS_MEMORY_DATA_02_045: [ read_int8_t shall write in destination the signed byte at source. ]*/
TEST_FUNCTION(read_int8_t_negative)
{
    ///arrange
    unsigned char source[1] = { 0xFF };
    int8_t destination;

    ///act
    read_int8_t(source, &destination);

    ///assert
    ASSERT_ARE_EQUAL(int8_t, -1, destination);

    ///clean
}

/* read_int16_t */

/*Tests_SRS_MEMORY_DATA_02_046: [ read_int16_t shall write in destination the bytes at source MSB first. ]*/
TEST_FUNCTION(read_int16_t_positive)
{
    ///arrange
    unsigned char source[2] = { 0x1 , 0x2};
    int16_t destination;

    ///act
    read_int16_t(source, &destination);

    ///assert
    ASSERT_ARE_EQUAL(int16_t, (1<<8)+2, destination);

    ///clean
}

/*Tests_SRS_MEMORY_DATA_02_046: [ read_int16_t shall write in destination the bytes at source MSB first. ]*/
TEST_FUNCTION(read_int16_t_negative)
{
    ///arrange
    unsigned char source[2] = { 0x80 , 0x01 };
    int16_t destination;

    ///act
    read_int16_t(source, &destination);

    ///assert
    ASSERT_ARE_EQUAL(int16_t, INT16_MIN+1, destination);

    ///clean
}

/* read_int32_t */

/*Tests_SRS_MEMORY_DATA_02_047: [ read_int32_t shall write in destination the bytes at source MSB first. ]*/
TEST_FUNCTION(read_int32_t_positive)
{
    ///arrange
    unsigned char source[4] = { 0x1 , 0x2, 0x3, 0x4};
    int32_t destination;

    ///act
    read_int32_t(source, &destination);

    ///assert
    ASSERT_ARE_EQUAL(int32_t, (1 << 24) + (2 << 16) + (3 << 8) + 4, destination);

    ///clean
}

/*Tests_SRS_MEMORY_DATA_02_047: [ read_int32_t shall write in destination the bytes at source MSB first. ]*/
TEST_FUNCTION(read_int32_t_negative)
{
    ///arrange
    unsigned char source[4] = { 0x80 , 0x1, 0x2, 0x3};
    int32_t destination;

    ///act
    read_int32_t(source, &destination);

    ///assert
    ASSERT_ARE_EQUAL(int32_t, INT32_MIN + (1<<16) +(2<<8)+3, destination);

    ///clean
}

/* read_int64_t */

/*Tests_SRS_MEMORY_DATA_02_048: [ read_int64_t shall write in destination the bytes at source MSB first. ]*/
TEST_FUNCTION(read_int64_t_positive)
{
    ///arrange
    unsigned char source[8] = { 0x1 , 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8 };
    int64_t destination;

    ///act
    read_int64_t(source, &destination);

    ///assert
    ASSERT_ARE_EQUAL(int64_t, ((int64_t)1 << 56) + ((int64_t)2 << 48) + ((int64_t)3 << 40) + ((int64_t)4<<32) + ((int64_t)5<<24) + ((int64_t)6<<16) +((int64_t)7<<8)+ (int64_t)8, destination);

    ///clean
}

/*Tests_SRS_MEMORY_DATA_02_048: [ read_int64_t shall write in destination the bytes at source MSB first. ]*/
TEST_FUNCTION(read_int64_t_negative)
{
    ///arrange
    unsigned char source[8] = { 0x80 , 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7};
    int64_t destination;

    ///act
    read_int64_t(source, &destination);

    ///assert
    ASSERT_ARE_EQUAL(int64_t, INT64_MIN + ((int64_t)1 << 48) + ((int64_t)2 << 40) + ((int64_t)3 << 32) + ((int64_t)4 << 24) + ((int64_t)5 << 16) + ((int64_t)6 << 8) + (int64_t)7, destination);

    ///clean
}

/* read_uuid_t */

/*Tests_SRS_MEMORY_DATA_02_049: [ read_uuid_t shall write in destination the bytes at source. ]*/
TEST_FUNCTION(read_uuid_t_succeeds)
{
    ///arrange
    unsigned char source[sizeof(UUID_T)] = { 0x0, 0x1 , 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9 , 0xA, 0xB, 0xC, 0xD, 0xE, 0xF };
    UUID_T destination = { 0 };

    ///act
    read_uuid_t(source, &destination);

    ///assert
    ASSERT_ARE_EQUAL(int, 0, memcmp(source, destination, sizeof(UUID_T)));
}

/*Tests_SRS_MEMORY_DATA_02_049: [ read_uuid_t shall write in destination the bytes at source. ]*/
TEST_FUNCTION(read_uuid_t_succeeds_2)
{
    ///arrange
    unsigned char source[sizeof(UUID_T)] = { 0x42, 0x43 , 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0xAA, 0xA1 , 0xA2, 0xA3, 0xBB, 0xCC, 0xDD, 0xEE };
    UUID_T destination = { 0 };

    ///act
    read_uuid_t(source, &destination);

    ///assert
    ASSERT_ARE_EQUAL(int, 0, memcmp(source, destination, sizeof(UUID_T)));
}

END_TEST_SUITE(memory_data_ut)
