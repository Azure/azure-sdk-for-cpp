// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TARGET_H
#define TARGET_H

#include "umock_c/umock_c_prod.h"

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#endif /* __cplusplus */

#define TARGET_RESULT_VALUES         \
    TARGET_RESULT_OK,                \
    TARGET_RESULT_FAIL,              \
    TARGET_RESULT_OUT_OF_MEMORY
MU_DEFINE_ENUM(TARGET_RESULT, TARGET_RESULT_VALUES);


/**
*  @brief    target_create is the function that your unit test intend to test. For example,
*               it will create the target instance.
*
* @param    size_t.
*
* @return    TARGET_RESULT
*/
MOCKABLE_FUNCTION(, TARGET_RESULT, target_create, size_t, size);

/**
*  @brief    target_create is the function that your unit test intend to test. For example,
*               it will destroy the target instance.
*
* @param    void
*
* @return    void
*/
MOCKABLE_FUNCTION(, void, target_destroy);


/** @brief    target_foo is the function that your unit test intend to test. For example,
*               it will do foo stuffs for the target.
*
* @param
*
* @return    TARGET_RESULT
*/
MOCKABLE_FUNCTION(, TARGET_RESULT, target_foo);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //TARGET_H
