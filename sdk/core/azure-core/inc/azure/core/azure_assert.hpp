// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Provide assert macros to use with pre-conditions.
 *
 * @remark Asserts are turned ON when `NDEBUG` is NOT defined (for Debug build). For Release build,
 * `std::abort()` is directly called if the condition is false, whitout calling assert().
 *
 */

#pragma once

#include "platform.hpp"

#include <cstdlib>
#include <string>

#if defined(NDEBUG)

/*
 * NDEBUG = defined = Build is on Release
 * Define AZURE_ASSERT to call abort directly on exp == false
 */

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

#if defined(AZ_PLATFORM_WINDOWS)
__declspec(noreturn) void AzureNoReturnPath(std::string msg);
#else
__attribute__((__noreturn__)) void AzureNoReturnPath(std::string msg);
#endif

#define AZURE_ASSERT_FALSE(exp) AZURE_ASSERT(!(exp))
#define AZURE_UNREACHABLE_CODE AzureNoReturnPath("unreachable code!")
#define AZURE_NOT_IMPLEMENTED AzureNoReturnPath("not implemented code!")
