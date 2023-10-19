// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stddef.h>
#include "azure_c_shared_utility/refcount.h"
#include "some_refcount_impl.h"

typedef struct pos_TAG
{
    int x;
#ifdef _MSC_VER
    /*warning C4200: nonstandard extension used: zero-sized array in struct/union */
#pragma warning(disable:4200)
#endif
    int flexible_array[];
} pos;

DEFINE_REFCOUNT_TYPE(pos);

POS_HANDLE Pos_Create(int x)
{
    pos* result = REFCOUNT_TYPE_CREATE(pos);
    if (result != NULL)
    {
        result->x = x;
    }
    return result;
}

POS_HANDLE Pos_Create_With_Extra_Size(int x, size_t extra_size)
{
    pos* result = REFCOUNT_TYPE_CREATE_WITH_EXTRA_SIZE(pos, extra_size);
    if (result != NULL)
    {
        result->x = x;
    }
    return result;
}

POS_HANDLE Pos_Clone(POS_HANDLE posHandle)
{

    if (posHandle != NULL)
    {
        pos* p = posHandle;
        INC_REF(pos, p);
    }
    return posHandle;
}

void Pos_Destroy(POS_HANDLE posHandle)
{
    if (posHandle != NULL)
    {
        pos* p = posHandle;
        if (DEC_REF(pos, p) == DEC_RETURN_ZERO)
        {
            REFCOUNT_TYPE_DESTROY(pos, p);
        }
    }
}
