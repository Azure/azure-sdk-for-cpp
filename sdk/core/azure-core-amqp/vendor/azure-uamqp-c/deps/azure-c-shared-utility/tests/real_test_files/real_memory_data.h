// Copyright (c) Microsoft. All rights reserved.

#ifndef REAL_MEMORY_DATA_H
#define REAL_MEMORY_DATA_H

#include "azure_macro_utils/macro_utils.h"

#include "azure_c_shared_utility/uuid.h"

#define R2(X) REGISTER_GLOBAL_MOCK_HOOK(X, real_##X);

#define REGISTER_MEMORY_DATA_GLOBAL_MOCK_HOOK() \
    MU_FOR_EACH_1(R2, \
        read_uint8_t, \
        read_uint16_t, \
        read_uint32_t, \
        read_uint64_t, \
        read_int8_t, \
        read_int16_t, \
        read_int32_t, \
        read_int64_t, \
        read_uuid_t, \
        write_uint8_t, \
        write_uint16_t, \
        write_uint32_t, \
        write_uint64_t, \
        write_int8_t, \
        write_int16_t, \
        write_int32_t, \
        write_int64_t, \
        write_uuid_t \
    )

#ifdef __cplusplus
#include <cstdint>
extern "C"
{
#else
#include <stdint.h>
#endif

    void real_read_uint8_t(const unsigned char* source, uint8_t* destination);
    void real_read_uint16_t(const unsigned char* source, uint16_t* destination);
    void real_read_uint32_t(const unsigned char* source, uint32_t* destination);
    void real_read_uint64_t(const unsigned char* source, uint64_t* destination);

    void real_read_int8_t(const unsigned char* source, int8_t* destination);
    void real_read_int16_t(const unsigned char* source, int16_t* destination);
    void real_read_int32_t(const unsigned char* source, int32_t* destination);
    void real_read_int64_t(const unsigned char* source, int64_t* destination);

    void real_read_uuid_t(const unsigned char* source, UUID_T* destination);

    void real_write_uint8_t(unsigned char* destination, uint8_t value);
    void real_write_uint16_t(unsigned char* destination, uint16_t value);
    void real_write_uint32_t(unsigned char* destination, uint32_t value);
    void real_write_uint64_t(unsigned char* destination, uint64_t value);

    void real_write_int8_t(unsigned char* destination, int8_t value);
    void real_write_int16_t(unsigned char* destination, int16_t value);
    void real_write_int32_t(unsigned char* destination, int32_t value);
    void real_write_int64_t(unsigned char* destination, int64_t value);

    void real_write_uuid_t(unsigned char* destination, const UUID_T value);

#ifdef __cplusplus
}
#endif

#endif //REAL_MEMORY_DATA_H
