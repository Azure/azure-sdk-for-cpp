// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/azure_assert.hpp"

// Calling this function would terminate program, therefore this function can't be covered in tests.
// LCOV_EXCL_START
[[noreturn]] void Azure::Core::_internal::AzureNoReturnPath(std::string const& msg)
{
  // void msg for Release build where Assert is ignored
  (void)msg;
  AZURE_ASSERT_MSG(false, msg);
  std::abort();
}
// LCOV_EXCL_STOP
