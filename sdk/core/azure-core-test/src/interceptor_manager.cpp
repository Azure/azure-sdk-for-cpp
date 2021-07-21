// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS // for std::getenv()
#endif

#include <azure/core/strings.hpp>

#include "azure/core/test/interceptor_manager.hpp"

#include <stdexcept>

Azure::Core::Test::TestMode Azure::Core::Test::InterceptorManager::GetTestMode()
{
  char* fromEnv = std::getenv("AZURE_TEST_MODE");
  if (!fromEnv)
  {
    return Azure::Core::Test::TestMode::LIVE;
  }

  std::string value(fromEnv);
  if (Azure::Core::Strings::LocaleInvariantCaseInsensitiveEqual(value, "RECORD"))
  {
    return Azure::Core::Test::TestMode::RECORD;
  }
  else if (Azure::Core::Strings::LocaleInvariantCaseInsensitiveEqual(value, "PLAYBACK"))
  {
    return Azure::Core::Test::TestMode::PLAYBACK;
  }
  else if (Azure::Core::Strings::LocaleInvariantCaseInsensitiveEqual(value, "LIVE"))
  {
    return Azure::Core::Test::TestMode::LIVE;
  }

  // unexpected variable value
  throw std::runtime_error("Invalid environment variable: " + value);
}
