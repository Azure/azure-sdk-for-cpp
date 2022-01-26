// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "get_env.hpp"

#if defined(_MSC_VER)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <vector>

#include <windows.h>
#else
#include <stdlib.h>
#endif

std::string GetEnv(const char* name)
{
  if (name != nullptr && name[0] != 0)
  {
#if defined(_MSC_VER)
    std::vector<char> bufferVector;
    char* buffer = nullptr;
    DWORD bufferSize = 0;
    while (const auto requiredSize = GetEnvironmentVariableA(name, buffer, bufferSize))
    {
      if (requiredSize < bufferSize)
      {
        return std::string(buffer, buffer + (bufferSize - 1));
      }

      bufferVector.resize(static_cast<decltype(bufferVector)::size_type>(requiredSize));
      bufferSize = requiredSize;
      buffer = bufferVector.data();
    }
#else
    if (const auto value = getenv(name))
    {
      return std::string(value);
    }
#endif
  }
  return std::string();
}
