// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "private/environment.hpp"
#include "azure/core/platform.hpp"

#include <azure/core/internal/strings.hpp>

#include <cstdlib>
#include <stdexcept>

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#include <windows.h>
#endif

using namespace Azure::Core::Test::_detail;

std::string Environment::GetVariable(const char* name)
{
#if !defined(WINAPI_PARTITION_DESKTOP) \
    || WINAPI_PARTITION_DESKTOP // See azure/core/platform.hpp for explanation.
#if defined(_MSC_VER)
#pragma warning(push)
// warning C4996: 'getenv': This function or variable may be unsafe. Consider using _dupenv_s
// instead.
#pragma warning(disable : 4996)
#endif

  if (auto envVar = std::getenv(name))
  {
    return std::string(envVar);
  }

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#endif

  return std::string();
}

Azure::Core::Test::TestMode Environment::GetTestMode()
{
  auto value = Environment::GetVariable("AZURE_TEST_MODE");
  if (value.empty())
  {
    return Azure::Core::Test::TestMode::LIVE;
  }

  if (Azure::Core::_internal::StringExtensions::LocaleInvariantCaseInsensitiveEqual(
          value, "RECORD"))
  {
    return Azure::Core::Test::TestMode::RECORD;
  }
  else if (Azure::Core::_internal::StringExtensions::LocaleInvariantCaseInsensitiveEqual(
               value, "PLAYBACK"))
  {
    return Azure::Core::Test::TestMode::PLAYBACK;
  }
  else if (Azure::Core::_internal::StringExtensions::LocaleInvariantCaseInsensitiveEqual(
               value, "LIVE"))
  {
    return Azure::Core::Test::TestMode::LIVE;
  }

  // unexpected variable value
  throw std::runtime_error("Invalid environment variable: " + value);
}
