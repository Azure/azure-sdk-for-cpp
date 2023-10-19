// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TEST_DEPENDENCY_H
#define TEST_DEPENDENCY_H

#include "umock_c/umock_c_prod.h"

#ifdef __cplusplus
extern "C" {
#endif

MOCKABLE_FUNCTION(, int, test_dependency_no_args);
MOCKABLE_FUNCTION(, int, test_dependency_no_args_no_real);

/* Tests_SRS_UMOCK_C_LIB_01_215: [ Each item in ... shall be an entry for one mockable function. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_216: [ Each item in ... shall be defined using a macro called FUNCTION, which shall be an alias for MOCKABLE_FUNCTION. ]*/
MOCKABLE_INTERFACE(test_interface,
    FUNCTION(, int, test_dependency_1_arg, int, a),
    FUNCTION(, int, test_dependency_2_args, int, a, int, b)
)

MOCKABLE_INTERFACE(test_interface_no_reals,
    FUNCTION(, int, test_dependency_1_arg_no_real, int, a),
    FUNCTION(, int, test_dependency_2_args_no_real, int, a, int, b)
)

#ifdef __cplusplus
}
#endif

#endif /* TEST_DEPENDENCY_H */
