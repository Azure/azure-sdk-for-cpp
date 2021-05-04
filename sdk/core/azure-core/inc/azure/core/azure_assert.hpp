// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Provide assert macros to use as pre-conditions.
 *
 * @remark Asserts are turned ON even if `NDEBUG` is defined.
 *
 */

#pragma once

#if defined(NDEBUG)
#define AZURE_SDK_INTERNAL_IMPLEMENTATION_OVERRIDE_NDEBUG
#undef NDEBUG
#endif

#include <cassert>
#include <stdexcept>

#define AZURE_ASSERT(exp) assert(exp)
#define AZURE_ASSERT_FALSE(exp) assert(!(exp))
#define AZURE_ASSERT_MSG(exp, msg) assert(((void)msg, exp))
#define AZURE_UNREACHABLE_CODE AZURE_ASSERT_MSG(false, "Unreachable code was access")
#define AZURE_NOT_IMPLEMENTED AZURE_ASSERT_MSG(false, "Not implemented")

#if defined(AZURE_SDK_INTERNAL_IMPLEMENTATION_OVERRIDE_NDEBUG)
#define NDEBUG
#endif
