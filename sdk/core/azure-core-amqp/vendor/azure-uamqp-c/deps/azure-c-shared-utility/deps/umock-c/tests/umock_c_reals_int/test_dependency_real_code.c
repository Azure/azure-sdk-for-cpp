// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "test_dependency.h"
#include "umock_c/umock_c_prod.h"

IMPLEMENT_MOCKABLE_FUNCTION(, int, test_dependency_no_args)
{
    return 42;
}

IMPLEMENT_MOCKABLE_FUNCTION(, int, test_dependency_1_arg, int, a)
{
    (void)a;
    return 42;
}

IMPLEMENT_MOCKABLE_FUNCTION(, int, test_dependency_2_args, int, a, int, b)
{
    (void)a;
    (void)b;
    return 42;
}
