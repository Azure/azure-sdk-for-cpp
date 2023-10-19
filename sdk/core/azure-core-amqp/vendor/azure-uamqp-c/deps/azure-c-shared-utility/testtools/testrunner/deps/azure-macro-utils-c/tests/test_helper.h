// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TEST_HELPER_H
#define TEST_HELPER_H

#include <stdio.h>

/*macro_utils cannot have any dependency on a more evolved testing framework....*/

#define TEST_HELPER_TO_STRING_(x) #x
#define TEST_HELPER_TO_STRING(x) TEST_HELPER_TO_STRING_(x)

/*a macro that returns a non-zero value from a function when a condition is false*/
#define POOR_MANS_ASSERT(condition)                                                                                                     \
do                                                                                                                                      \
{                                                                                                                                       \
    if (!(condition))                                                                                                                   \
    {                                                                                                                                   \
        printf("condition \"" #condition "\" in " __FILE__ ":" TEST_HELPER_TO_STRING(__LINE__) " was UNEXPECTEDLY FALSE\n");            \
        return __LINE__;                                                                                                                \
    }                                                                                                                                   \
} while (0)

#endif /*TEST_HELPER_H*/
