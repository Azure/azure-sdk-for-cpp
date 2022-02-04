// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

/**
 * @file
 * @brief Deprecated assert macros. It is only temporarily made available for compatibility and
 * should NOT be used by any callers outside of the SDK.
 *
 */

#include "azure/core/internal/azure_assert.hpp"

namespace Azure { namespace Core { namespace _internal {
  // LCOV_EXCL_START
  [[deprecated("The Azure Assert macros are meant for internal use within the SDK only, use the "
               "standard library's assert() instead.")]] [[noreturn]] inline void
  DeprecatedAzureAssert(bool exp, const char* msg = nullptr)
  {
    static_cast<void>(msg);
#if defined(NDEBUG)
    if (!exp)
    {
      std::abort();
    }
#else
    assert(exp);
#endif
  }

  [[deprecated("The Azure Unreachable Code macros are meant for internal use within the SDK only, "
               "use std::abort() instead.")]] [[noreturn]] inline void
  DeprecatedAzureUnreachableCode(std::string const& msg)
  {
    AzureNoReturnPath(std::string const& msg);
  }
  // LCOV_EXCL_STOP
}}} // namespace Azure::Core::_internal

#define AZURE_ASSERT(exp) ::Azure::Core::_internal::DeprecatedAzureAssert(exp)
#define AZURE_ASSERT_MSG(exp, msg) ::Azure::Core::_internal::DeprecatedAzureAssert(exp, msg)
#define AZURE_ASSERT_FALSE(exp) AZURE_ASSERT(!(exp))

#define AZURE_UNREACHABLE_CODE() \
  ::Azure::Core::_internal::DeprecatedAzureUnreachableCode("unreachable code!")

#define AZURE_NOT_IMPLEMENTED() \
  ::Azure::Core::_internal::DeprecatedAzureUnreachableCode("not implemented code!")
