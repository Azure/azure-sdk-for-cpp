// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/internal/assert.hpp"

using namespace Azure::Core::_internal;

[[noreturn]] void AzureNoReturnPath(std::string const& msg)
{
  // void msg for Release build where Assert is ignored
  (void)msg;
  _azure_ASSERT_MSG(false, msg);
  std::abort();
}
