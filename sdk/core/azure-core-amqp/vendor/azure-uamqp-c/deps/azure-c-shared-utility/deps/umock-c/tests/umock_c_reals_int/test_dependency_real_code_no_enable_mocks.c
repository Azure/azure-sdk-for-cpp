// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "test_dependency_no_enable_mocks.h"
#include "umock_c/umock_c_prod.h"

IMPLEMENT_MOCKABLE_FUNCTION(, int, test_dependency_no_args_no_ENABLE_MOCKS)
{
    return 42;
}
