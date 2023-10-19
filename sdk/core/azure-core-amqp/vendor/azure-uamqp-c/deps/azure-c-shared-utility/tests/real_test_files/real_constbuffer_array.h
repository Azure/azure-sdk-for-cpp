// Copyright (c) Microsoft. All rights reserved.

#ifndef REAL_CONSTBUFFER_ARRAY_H
#define REAL_CONSTBUFFER_ARRAY_H

#include "azure_macro_utils/macro_utils.h"

#define R2(X) REGISTER_GLOBAL_MOCK_HOOK(X, real_##X);

#define REGISTER_CONSTBUFFER_ARRAY_GLOBAL_MOCK_HOOK() \
    MU_FOR_EACH_1(R2, \
        constbuffer_array_create, \
        constbuffer_array_create_with_move_buffers, \
        constbuffer_array_create_from_array_array, \
        constbuffer_array_create_empty, \
        constbuffer_array_inc_ref, \
        constbuffer_array_dec_ref, \
        constbuffer_array_add_front, \
        constbuffer_array_remove_front, \
        constbuffer_array_get_buffer_count, \
        constbuffer_array_get_buffer, \
        constbuffer_array_get_buffer_content, \
        constbuffer_array_get_all_buffers_size, \
        constbuffer_array_get_const_buffer_handle_array, \
        CONSTBUFFER_ARRAY_HANDLE_contain_same \
)

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

CONSTBUFFER_ARRAY_HANDLE real_constbuffer_array_create(const CONSTBUFFER_HANDLE* buffers, uint32_t buffer_count);
CONSTBUFFER_ARRAY_HANDLE real_constbuffer_array_create_with_move_buffers(CONSTBUFFER_HANDLE* buffers, uint32_t buffer_count);
CONSTBUFFER_ARRAY_HANDLE real_constbuffer_array_create_empty(void);
CONSTBUFFER_ARRAY_HANDLE real_constbuffer_array_create_from_array_array(const CONSTBUFFER_ARRAY_HANDLE* buffer_arrays, uint32_t buffer_array_count);

void real_constbuffer_array_inc_ref(CONSTBUFFER_ARRAY_HANDLE constbuffer_array_handle);
void real_constbuffer_array_dec_ref(CONSTBUFFER_ARRAY_HANDLE constbuffer_array_handle);

/*add in front*/
CONSTBUFFER_ARRAY_HANDLE real_constbuffer_array_add_front(CONSTBUFFER_ARRAY_HANDLE constbuffer_array_handle, CONSTBUFFER_HANDLE constbuffer_handle);

/*remove front*/
CONSTBUFFER_ARRAY_HANDLE real_constbuffer_array_remove_front(CONSTBUFFER_ARRAY_HANDLE constbuffer_array_handle, CONSTBUFFER_HANDLE* constbuffer_handle);

/* getters */
int real_constbuffer_array_get_buffer_count(CONSTBUFFER_ARRAY_HANDLE constbuffer_array_handle, uint32_t* buffer_count);
CONSTBUFFER_HANDLE real_constbuffer_array_get_buffer(CONSTBUFFER_ARRAY_HANDLE constbuffer_array_handle, uint32_t buffer_index);
const CONSTBUFFER* real_constbuffer_array_get_buffer_content(CONSTBUFFER_ARRAY_HANDLE constbuffer_array_handle, uint32_t buffer_index);
int real_constbuffer_array_get_all_buffers_size(CONSTBUFFER_ARRAY_HANDLE constbuffer_array_handle, uint32_t* all_buffers_size);
const CONSTBUFFER_HANDLE* real_constbuffer_array_get_const_buffer_handle_array(CONSTBUFFER_ARRAY_HANDLE constbuffer_array_handle);

/* compare */
bool real_CONSTBUFFER_ARRAY_HANDLE_contain_same(CONSTBUFFER_ARRAY_HANDLE left, CONSTBUFFER_ARRAY_HANDLE right);

#ifdef __cplusplus
}
#endif

#endif // REAL_CONSTBUFFER_ARRAY_H
