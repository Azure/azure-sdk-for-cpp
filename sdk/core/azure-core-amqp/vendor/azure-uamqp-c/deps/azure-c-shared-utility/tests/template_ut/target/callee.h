// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef CALLEE_H
#define CALLEE_H

#include "umock_c/umock_c_prod.h"

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#endif /* __cplusplus */

typedef void* CALLEE_HANDLE;

#define CALLEE_RESULT_VALUES         \
    CALLEE_RESULT_OK,                \
    CALLEE_RESULT_FAIL,              \
    CALLEE_RESULT_OUT_OF_MEMORY
MU_DEFINE_ENUM(CALLEE_RESULT, CALLEE_RESULT_VALUES);


/**
 * This is the set of mockable functions that template_ut.c will replace
 */
MOCKABLE_FUNCTION(, CALLEE_HANDLE, callee_open, size_t, parameter);
MOCKABLE_FUNCTION(, void, callee_close, CALLEE_HANDLE, handle);
MOCKABLE_FUNCTION(, CALLEE_RESULT, callee_bar_1);
MOCKABLE_FUNCTION(, CALLEE_RESULT, callee_bar_2, char, parameter);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //CALLEE_H
