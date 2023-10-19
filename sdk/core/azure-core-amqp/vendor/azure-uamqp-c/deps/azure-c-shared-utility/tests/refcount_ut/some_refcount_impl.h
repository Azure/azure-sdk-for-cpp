// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef SOME_REFCOUNT_IMPL_H
#define SOME_REFCOUNT_IMPL_H

#ifdef __cplusplus
#include <cstddef>
extern "C"
{
#else
#include <stddef.h>
#endif

typedef struct pos_TAG* POS_HANDLE;

POS_HANDLE Pos_Create(int x);
POS_HANDLE Pos_Create_With_Extra_Size(int x, size_t extraSize);
POS_HANDLE Pos_Clone(POS_HANDLE posHandle);
void Pos_Destroy(POS_HANDLE posHandle);

#ifdef __cplusplus
}
#endif

#endif /*SOME_REFCOUNT_IMPL_H*/
