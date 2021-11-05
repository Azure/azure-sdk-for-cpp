// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Provide assert macros to use with pre-conditions.
 *
 * @remark Asserts are turned ON when `NDEBUG` is NOT defined (for Debug build). For Release build,
 * `std::abort()` is directly called if the condition is false, without calling assert().
 *
 */

#pragma once

#include "azure/core/platform.hpp"

#include <cstdlib>
#include <string>

#if defined(NDEBUG)

/*
 * NDEBUG = defined = Build is on Release
 * Define _azure_ASSERT to call abort directly on exp == false
 */

#define _azure_ASSERT(exp) \
  do \
  { \
    if (!(exp)) \
    { \
      std::abort(); \
    } \
  } while (0)

#define _azure_ASSERT_MSG(exp, msg) _azure_ASSERT(exp)

#else

/*
 * NDEBUG = NOT defined = Build is on Debug
 * Define _azure_ASSERT to call assert to provide better debug experience.
 */

#include <cassert>

#define _azure_ASSERT(exp) assert((exp))
#define _azure_ASSERT_MSG(exp, msg) assert(((void)msg, (exp)))

#endif

namespace Azure { namespace Core { namespace _internal {
  [[noreturn]] void AzureNoReturnPath(std::string const& msg);
}}} // namespace Azure::Core::_internal

#define _azure_ASSERT_FALSE(exp) _azure_ASSERT(!(exp))
#define _azure_UNREACHABLE_CODE() ::Azure::Core::_internal::AzureNoReturnPath("unreachable code!")
#define _azure_NOT_IMPLEMENTED() \
  ::Azure::Core::_internal::AzureNoReturnPath("not implemented code!")
