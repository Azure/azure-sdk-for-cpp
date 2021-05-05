// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Provide assert macros to use with pre-conditions.
 *
 * @remark Asserts are turned ON even if `NDEBUG` is defined.
 *
 */

#pragma once

#if defined(NDEBUG)

/*
 * NDEBUG = defined = Build is on Release
 * Define AZURE_ASSERT to call abort directly on exp == false
 */

#include <cstdlib>

#define AZURE_ASSERT(exp) \
  if (!(exp)) \
  { \
    std::abort(); \
  }

#define AZURE_ASSERT_MSG(exp, msg) AZURE_ASSERT(exp)

#else

/*
 * NDEBUG = NOT defined = Build is on Debug
 * Define AZURE_ASSERT to call assert to provide better debug experience.
 */

#include <cassert>

#define AZURE_ASSERT(exp) assert((exp))
#define AZURE_ASSERT_MSG(exp, msg) assert(((void)msg, (exp)))

#endif

#define AZURE_ASSERT_FALSE(exp) AZURE_ASSERT(!(exp))
#define AZURE_UNREACHABLE_CODE AZURE_ASSERT_MSG(false, "Unreachable code was access")
#define AZURE_NOT_IMPLEMENTED AZURE_ASSERT_MSG(false, "Not implemented")