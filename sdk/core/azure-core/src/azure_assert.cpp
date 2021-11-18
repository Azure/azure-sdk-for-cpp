// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/internal/azure_assert.hpp"

[[noreturn]] void Azure::Core::_internal::AzureNoReturnPath(std::string const& msg)
{
  // void msg for Release build where Assert is ignored
  (void)msg;
  _azure_ASSERT_MSG(false, msg);
  std::abort();
}
