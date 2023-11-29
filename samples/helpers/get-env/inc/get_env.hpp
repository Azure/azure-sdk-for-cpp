// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Since `getenv()` may generate warnings on MSVC, and is not available on UWP, sample code
// gets cluttered with insignificant nuances. This file makes it so that `std::getenv()` compiles
// and works the same on Linux, macOS, Win32, and UWP.

#pragma once

// The AppStore partition for the Win32 API surface does not include a definition for std::getenv,
// so we provide our own definition here.
#if !defined(WINAPI_PARTITION_DESKTOP) || WINAPI_PARTITION_DESKTOP
// Win32
#include <cstdlib>
#else
// UWP
namespace std {
char* getenv(const char* name);
}
#endif

#include <stdexcept>
#include <string>

struct GetEnvHelper
{
  static std::string GetEnv(char const* env)
  {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
    auto const val = std::getenv(env);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
    if (val == nullptr)
    {
      throw std::runtime_error("Could not find required environment variable: " + std::string(env));
    }
    return std::string(val);
  }
};


