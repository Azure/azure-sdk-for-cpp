// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/random.h>

#include "azure_macro_utils/macro_utils.h"
#include "azure_c_shared_utility/uniqueid.h"
#include "azure_c_shared_utility/xlogging.h"

#define UUID_LENGTH 36

MU_DEFINE_ENUM_STRINGS(UNIQUEID_RESULT, UNIQUEID_RESULT_VALUES);

// UUID fields as specified in RFC 4122
typedef struct UUID_TAG 
{
    uint32_t    time_low;
    uint16_t    time_mid;
    uint16_t    time_hi_and_version;
    uint16_t    clock_seq_and_variant;
    uint8_t     node[6];
} UUID;

// lowercase uuid format expected by azure iot sdk
static const char* uuid_format =
    "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x";

UNIQUEID_RESULT UniqueId_Generate(char* uid, size_t len)
{
    UNIQUEID_RESULT result;

    /* Codes_SRS_UNIQUEID_07_002: [If uid is NULL then UniqueId_Generate shall return UNIQUEID_INVALID_ARG] */
    /* Codes_SRS_UNIQUEID_07_003: [If len is less then 37 then UniqueId_Generate shall return UNIQUEID_INVALID_ARG] */
    if (uid == NULL || len < (UUID_LENGTH + 1)) // UUID_LENGTH + 1 to compensate "/0"
    {
        result = UNIQUEID_INVALID_ARG;
        LogError("Buffer Size is Null or Shorter than 37 Characters. (result = %" PRI_MU_ENUM ")", MU_ENUM_VALUE(UNIQUEID_RESULT, result));
    }
    else
    {
        // generate version 4 uuid
        UUID unique_id;

        // get random bytes
        ssize_t bytes_got = getrandom(&unique_id, sizeof(unique_id), GRND_NONBLOCK);
        if (bytes_got != sizeof(unique_id)) 
        {
            LogError("Failed to obtain random numbers from getrandom.");
            result = UNIQUEID_ERROR;
        }
        else
        {
            // format as version 4 (random) uuid variant 2
            unique_id.clock_seq_and_variant = (unique_id.clock_seq_and_variant & 0x3FFF) | 0x8000;
            unique_id.time_hi_and_version = (unique_id.time_hi_and_version & 0x0FFF) | 0x4000;

            /* Codes_SRS_UNIQUEID_07_001: [UniqueId_Generate shall create a unique Id 36 character long string.] */
            // write out unique_id as string
            memset(uid, 0, len);
            int chars_written = snprintf(uid, len, uuid_format,
                unique_id.time_low, unique_id.time_mid, unique_id.time_hi_and_version,
                unique_id.clock_seq_and_variant >> 8, unique_id.clock_seq_and_variant & 0xFF,
                unique_id.node[0], unique_id.node[1], unique_id.node[2],
                unique_id.node[3], unique_id.node[4], unique_id.node[5]);

            if (chars_written != UUID_LENGTH) 
            {
                LogError("Failed to convert binary uuid to string format.");
                result = UNIQUEID_ERROR;
            } 
            else 
            {
                result = UNIQUEID_OK;
            }
        }
    }
    return result;
}
