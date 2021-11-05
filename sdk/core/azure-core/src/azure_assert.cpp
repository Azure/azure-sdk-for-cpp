// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/azure_assert.hpp"

[[noreturn]] void Azure::Core::_internal::AzureNoReturnPath(std::string const& msg)
{
  // void msg for Release build where Assert is ignored
  (void)msg;
  AZURE_ASSERT_MSG(false, msg);
  std::abort();
}
