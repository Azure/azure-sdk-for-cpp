// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TEST_UNIT_NO_REALS_H
#define TEST_UNIT_NO_REALS_H

#include "umock_c/umock_c_prod.h"

#ifdef __cplusplus
extern "C" {
#endif

MOCKABLE_INTERFACE(test_unit_no_reals,
    FUNCTION(, int, test_unit_1_arg_no_real, int, a),
    FUNCTION(, int, test_unit_2_args_no_real, int, a, int, b)
)

#ifdef __cplusplus
}
#endif

#endif /* TEST_UNIT_NO_REALS_H */
