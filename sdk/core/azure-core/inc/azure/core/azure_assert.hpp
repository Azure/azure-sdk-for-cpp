// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Provide assert macros to use as pre-conditions.
 *
 * @remark Asserts are turned off is `NDEBUG` is defined
 *
 */

#pragma once

// uncomment next line to disable asserts
// #define NDEBUG
#include <cassert>
#include <stdexcept>

#define AZURE_ASSERT(exp) assert(exp)
#define AZURE_ASSERT_FALSE(exp) assert(!(exp))
#define AZURE_ASSERT_MSG(exp, msg) assert(((void)msg, exp))
#define AZURE_UNREACHABLE_CODE AZURE_ASSERT_MSG(false, "Unreachable code was access")
#define AZURE_NOT_IMPLEMENTED throw std::runtime_error("Not implemented");
