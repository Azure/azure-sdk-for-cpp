// Copyright (c) Microsoft. All rights reserved.

#ifndef REAL_CONSTBUFFER_H
#define REAL_CONSTBUFFER_H

#include "azure_macro_utils/macro_utils.h"

#define R2(X) REGISTER_GLOBAL_MOCK_HOOK(X, real_##X);

#define REGISTER_CONSTBUFFER_GLOBAL_MOCK_HOOK() \
    MU_FOR_EACH_1(R2, \
        CONSTBUFFER_Create, \
        CONSTBUFFER_CreateFromBuffer, \
        CONSTBUFFER_CreateWithMoveMemory, \
        CONSTBUFFER_CreateWithCustomFree, \
        CONSTBUFFER_IncRef, \
        CONSTBUFFER_GetContent, \
        CONSTBUFFER_DecRef, \
        CONSTBUFFER_HANDLE_contain_same, \
        CONSTBUFFER_CreateFromOffsetAndSize \
)

#ifdef __cplusplus
#include <cstddef>
extern "C"
{
#else
#include <stddef.h>
#include <stdbool.h>
#endif

CONSTBUFFER_HANDLE real_CONSTBUFFER_Create(const unsigned char* source, size_t size);

CONSTBUFFER_HANDLE real_CONSTBUFFER_CreateFromBuffer(BUFFER_HANDLE buffer);

CONSTBUFFER_HANDLE real_CONSTBUFFER_CreateWithMoveMemory(unsigned char* source, size_t size);

CONSTBUFFER_HANDLE real_CONSTBUFFER_CreateWithCustomFree(const unsigned char* source, size_t size, CONSTBUFFER_CUSTOM_FREE_FUNC custom_free_func, void* custom_free_func_context);

void real_CONSTBUFFER_IncRef(CONSTBUFFER_HANDLE constbufferHandle);

const CONSTBUFFER* real_CONSTBUFFER_GetContent(CONSTBUFFER_HANDLE constbufferHandle);

void real_CONSTBUFFER_DecRef(CONSTBUFFER_HANDLE constbufferHandle);

bool real_CONSTBUFFER_HANDLE_contain_same(CONSTBUFFER_HANDLE left, CONSTBUFFER_HANDLE right);

CONSTBUFFER_HANDLE real_CONSTBUFFER_CreateFromOffsetAndSize(CONSTBUFFER_HANDLE handle, size_t offset, size_t size);

#ifdef __cplusplus
}
#endif

#endif //REAL_CONSTBUFFER_H