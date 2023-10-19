// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umocktypes.h"
#include "umock_c/umocktypes_stdint.h"
#include "umock_c/umockalloc.h"
#include "umock_c/umock_log.h"

#define IMPLEMENT_STRINGIFY(type, function_postfix, printf_specifier) \
    char* MU_C2(umocktypes_stringify_,function_postfix)(type* value) \
    { \
        char* result; \
        if (value == NULL) \
        { \
            UMOCK_LOG(MU_TOSTRING(MU_C2(umocktypes_stringify_,function_postfix)) ": NULL value."); \
            result = NULL; \
        } \
        else \
        { \
            char temp_buffer[32]; \
            size_t length = sprintf(temp_buffer, printf_specifier, *value); \
            result = (char*)umockalloc_malloc(length + 1); \
            if (result == NULL) \
            { \
                UMOCK_LOG(MU_TOSTRING(MU_C2(umocktypes_stringify_,function_postfix)) ": Cannot allocate memory for result string."); \
            } \
            else \
            { \
                (void)memcpy(result, temp_buffer, length + 1); \
            } \
        } \
        return result; \
    }

#define IMPLEMENT_ARE_EQUAL(type, function_postfix) \
    int MU_C2(umocktypes_are_equal_,function_postfix)(type* left, type* right) \
    { \
        int result; \
        if ((left == NULL) || (right == NULL)) \
        { \
            UMOCK_LOG(MU_TOSTRING(MU_C2(umocktypes_are_equal_,function_postfix)) ": Bad arguments: left = %p, right = %p", left, right); \
            result = -1; \
        } \
        else \
        { \
            result = ((*left) == (*right)) ? 1 : 0; \
        } \
        return result; \
    }

#define IMPLEMENT_COPY(type, function_postfix) \
    int MU_C2(umocktypes_copy_,function_postfix)(type* destination, type* source) \
    { \
        int result; \
        if ((destination == NULL) || \
            (source == NULL)) \
        { \
            UMOCK_LOG(MU_TOSTRING(MU_C2(umocktypes_are_equal_,function_postfix)) ": Bad arguments: destination = %p, source = %p", destination, source); \
            result = __LINE__; \
        } \
        else \
        { \
            *destination = *source; \
            result = 0; \
        } \
        return result; \
    }

#define IMPLEMENT_FREE(type, function_postfix) \
    void MU_C2(umocktypes_free_,function_postfix)(type* value) \
    { \
        (void)value; \
    }

#define IMPLEMENT_TYPE_HANDLERS(type, function_postfix, printf_specifier) \
    IMPLEMENT_STRINGIFY(type, function_postfix, printf_specifier) \
    IMPLEMENT_ARE_EQUAL(type, function_postfix) \
    IMPLEMENT_COPY(type, function_postfix) \
    IMPLEMENT_FREE(type, function_postfix)

/* Codes_SRS_UMOCKTYPES_STDINT_01_004: [ umocktypes_stringify_uint8_t shall return the string representation of value. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_005: [ If value is NULL, umocktypes_stringify_uint8_t shall return NULL. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_006: [ If allocating a new string to hold the string representation fails, umocktypes_stringify_uint8_t shall return NULL. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_008: [ umocktypes_are_equal_uint8_t shall compare the 2 uint8_t values pointed to by left and right. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_009: [ If any of the arguments is NULL, umocktypes_are_equal_uint8_t shall return -1. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_010: [ If the values pointed to by left and right are equal, umocktypes_are_equal_uint8_t shall return 1. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_011: [ If the values pointed to by left and right are different, umocktypes_are_equal_uint8_t shall return 0. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_012: [ umocktypes_copy_uint8_t shall copy the uint8_t value from source to destination. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_013: [ On success umocktypes_copy_uint8_t shall return 0. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_014: [ If source or destination are NULL, umocktypes_copy_uint8_t shall return a non-zero value. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_015: [ umocktypes_free_uint8_t shall do nothing. ]*/
IMPLEMENT_TYPE_HANDLERS(uint8_t, uint8_t, "%"PRIu8)

/* Codes_SRS_UMOCKTYPES_STDINT_01_016: [ umocktypes_stringify_int8_t shall return the string representation of value. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_017: [ If value is NULL, umocktypes_stringify_int8_t shall return NULL. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_018: [ If allocating a new string to hold the string representation fails, umocktypes_stringify_int8_t shall return NULL. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_020: [ umocktypes_are_equal_int8_t shall compare the 2 int8_t values pointed to by left and right. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_021: [ If any of the arguments is NULL, umocktypes_are_equal_int8_t shall return -1. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_022: [ If the values pointed to by left and right are equal, umocktypes_are_equal_int8_t shall return 1. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_023: [ If the values pointed to by left and right are different, umocktypes_are_equal_int8_t shall return 0. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_024: [ umocktypes_copy_int8_t shall copy the int8_t value from source to destination. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_025: [ On success umocktypes_copy_int8_t shall return 0. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_026: [ If source or destination are NULL, umocktypes_copy_int8_t shall return a non-zero value. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_027: [ umocktypes_free_int8_t shall do nothing. ]*/
IMPLEMENT_TYPE_HANDLERS(int8_t, int8_t, "%"PRId8)

/* Codes_SRS_UMOCKTYPES_STDINT_01_028: [ umocktypes_stringify_uint16_t shall return the string representation of value. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_029: [ If value is NULL, umocktypes_stringify_uint16_t shall return NULL. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_030: [ If allocating a new string to hold the string representation fails, umocktypes_stringify_uint16_t shall return NULL. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_032: [ umocktypes_are_equal_uint16_t shall compare the 2 uint16_t values pointed to by left and right. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_033: [ If any of the arguments is NULL, umocktypes_are_equal_uint16_t shall return -1. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_034: [ If the values pointed to by left and right are equal, umocktypes_are_equal_uint16_t shall return 1. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_035: [ If the values pointed to by left and right are different, umocktypes_are_equal_uint16_t shall return 0. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_036: [ umocktypes_copy_uint16_t shall copy the uint16_t value from source to destination. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_037: [ On success umocktypes_copy_uint16_t shall return 0. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_038: [ If source or destination are NULL, umocktypes_copy_uint16_t shall return a non-zero value. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_039: [ umocktypes_free_uint16_t shall do nothing. ]*/
IMPLEMENT_TYPE_HANDLERS(uint16_t, uint16_t, "%"PRIu16)

/* Codes_SRS_UMOCKTYPES_STDINT_01_040: [ umocktypes_stringify_int16_t shall return the string representation of value. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_041: [ If value is NULL, umocktypes_stringify_int16_t shall return NULL. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_042: [ If allocating a new string to hold the string representation fails, umocktypes_stringify_int16_t shall return NULL. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_044: [ umocktypes_are_equal_int16_t shall compare the 2 int16_t values pointed to by left and right. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_045: [ If any of the arguments is NULL, umocktypes_are_equal_int16_t shall return -1. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_046: [ If the values pointed to by left and right are equal, umocktypes_are_equal_int16_t shall return 1. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_047: [ If the values pointed to by left and right are different, umocktypes_are_equal_int16_t shall return 0. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_048: [ umocktypes_copy_int16_t shall copy the int16_t value from source to destination. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_049: [ On success umocktypes_copy_int16_t shall return 0. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_050: [ If source or destination are NULL, umocktypes_copy_int16_t shall return a non-zero value. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_051: [ umocktypes_free_int16_t shall do nothing. ]*/
IMPLEMENT_TYPE_HANDLERS(int16_t, int16_t, "%"PRId16)

/* Codes_SRS_UMOCKTYPES_STDINT_01_052: [ umocktypes_stringify_uint32_t shall return the string representation of value. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_053: [ If value is NULL, umocktypes_stringify_uint32_t shall return NULL. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_054: [ If allocating a new string to hold the string representation fails, umocktypes_stringify_uint32_t shall return NULL. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_056: [ umocktypes_are_equal_uint32_t shall compare the 2 uint32_t values pointed to by left and right. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_057: [ If any of the arguments is NULL, umocktypes_are_equal_uint32_t shall return -1. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_058: [ If the values pointed to by left and right are equal, umocktypes_are_equal_uint32_t shall return 1. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_059: [ If the values pointed to by left and right are different, umocktypes_are_equal_uint32_t shall return 0. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_060: [ umocktypes_copy_uint32_t shall copy the uint32_t value from source to destination. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_061: [ On success umocktypes_copy_uint32_t shall return 0. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_062: [ If source or destination are NULL, umocktypes_copy_uint32_t shall return a non-zero value. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_063: [ umocktypes_free_uint32_t shall do nothing. ]*/
IMPLEMENT_TYPE_HANDLERS(uint32_t, uint32_t, "%"PRIu32)

/* Codes_SRS_UMOCKTYPES_STDINT_01_064: [ umocktypes_stringify_int32_t shall return the string representation of value. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_065: [ If value is NULL, umocktypes_stringify_int32_t shall return NULL. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_066: [ If allocating a new string to hold the string representation fails, umocktypes_stringify_int32_t shall return NULL. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_068: [ umocktypes_are_equal_int32_t shall compare the 2 int32_t values pointed to by left and right. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_069: [ If any of the arguments is NULL, umocktypes_are_equal_int32_t shall return -1. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_070: [ If the values pointed to by left and right are equal, umocktypes_are_equal_int32_t shall return 1. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_071: [ If the values pointed to by left and right are different, umocktypes_are_equal_int32_t shall return 0. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_072: [ umocktypes_copy_int32_t shall copy the int32_t value from source to destination. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_073: [ On success umocktypes_copy_int32_t shall return 0. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_074: [ If source or destination are NULL, umocktypes_copy_int32_t shall return a non-zero value. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_075: [ umocktypes_free_int32_t shall do nothing. ]*/
IMPLEMENT_TYPE_HANDLERS(int32_t, int32_t, "%"PRId32)

/* Codes_SRS_UMOCKTYPES_STDINT_01_076: [ umocktypes_stringify_uint64_t shall return the string representation of value. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_077: [ If value is NULL, umocktypes_stringify_uint64_t shall return NULL. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_078: [ If allocating a new string to hold the string representation fails, umocktypes_stringify_uint64_t shall return NULL. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_080: [ umocktypes_are_equal_uint64_t shall compare the 2 uint64_t values pointed to by left and right. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_081: [ If any of the arguments is NULL, umocktypes_are_equal_uint64_t shall return -1. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_082: [ If the values pointed to by left and right are equal, umocktypes_are_equal_uint64_t shall return 1. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_083: [ If the values pointed to by left and right are different, umocktypes_are_equal_uint64_t shall return 0. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_084: [ umocktypes_copy_uint64_t shall copy the uint64_t value from source to destination. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_085: [ On success umocktypes_copy_uint64_t shall return 0. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_086: [ If source or destination are NULL, umocktypes_copy_uint64_t shall return a non-zero value. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_087: [ umocktypes_free_uint64_t shall do nothing. ]*/
IMPLEMENT_TYPE_HANDLERS(uint64_t, uint64_t, "%"PRIu64)

/* Codes_SRS_UMOCKTYPES_STDINT_01_088: [ umocktypes_stringify_int64_t shall return the string representation of value. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_089: [ If value is NULL, umocktypes_stringify_int64_t shall return NULL. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_090: [ If allocating a new string to hold the string representation fails, umocktypes_stringify_int64_t shall return NULL. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_092: [ umocktypes_are_equal_int64_t shall compare the 2 int64_t values pointed to by left and right. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_093: [ If any of the arguments is NULL, umocktypes_are_equal_int64_t shall return -1. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_094: [ If the values pointed to by left and right are equal, umocktypes_are_equal_int64_t shall return 1. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_095: [ If the values pointed to by left and right are different, umocktypes_are_equal_int64_t shall return 0. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_096: [ umocktypes_copy_int64_t shall copy the int64_t value from source to destination. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_097: [ On success umocktypes_copy_int64_t shall return 0. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_098: [ If source or destination are NULL, umocktypes_copy_int64_t shall return a non-zero value. ]*/
/* Codes_SRS_UMOCKTYPES_STDINT_01_099: [ umocktypes_free_int64_t shall do nothing. ]*/
IMPLEMENT_TYPE_HANDLERS(int64_t, int64_t, "%"PRId64)

int umocktypes_stdint_register_types(void)
{
    int result;

    /* Codes_SRS_UMOCKTYPES_STDINT_01_001: [ umocktypes_stdint_register_types shall register support for all the types in the module. ]*/
    if ((REGISTER_TYPE(uint8_t, uint8_t) != 0) ||
        (REGISTER_TYPE(int8_t, int8_t) != 0) ||
        (REGISTER_TYPE(uint16_t, uint16_t) != 0) ||
        (REGISTER_TYPE(int16_t, int16_t) != 0) ||
        (REGISTER_TYPE(uint32_t, uint32_t) != 0) ||
        (REGISTER_TYPE(int32_t, int32_t) != 0) ||
        (REGISTER_TYPE(uint64_t, uint64_t) != 0) ||
        (REGISTER_TYPE(int64_t, int64_t) != 0))
    {
        /* Codes_SRS_UMOCKTYPES_STDINT_01_003: [ If registering any of the types fails, umocktypes_stdint_register_types shall fail and return a non-zero value. ]*/
        UMOCK_LOG("umocktypes_stdint_register_types: Failed registering types."); \
        result = __LINE__;
    }
    else
    {
        /* Codes_SRS_UMOCKTYPES_STDINT_01_002: [ On success, umocktypes_stdint_register_types shall return 0. ]*/
        result = 0;
    }

    return result;
}
