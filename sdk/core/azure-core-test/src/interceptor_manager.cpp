// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS // for std::getenv()
#endif

#include <azure/core/internal/strings.hpp>

#include "azure/core/test/interceptor_manager.hpp"
#include "private/environment.hpp"

#include <stdexcept>

Azure::Core::Test::TestMode Azure::Core::Test::InterceptorManager::GetTestMode()
{
  return Azure::Core::Test::_detail::Environment::GetTestMode();
}
