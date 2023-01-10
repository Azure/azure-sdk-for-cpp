// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "get_env.hpp"

#include <locale>

#if defined(WINAPI_PARTITION_DESKTOP) && !WINAPI_PARTITION_DESKTOP

char* std::getenv(const char* name)
{
  char* buf = GetEnvironmentStringsA();

  for (auto i = 0; *buf != '\0';)
  {
    if (name[i] == '\0' && *buf == '=')
    {
      // We've found "name=", the rest will be the value with '\0' at the end.
      return buf + 1;
    }

    // We're still trying to match the name.
    if (std::toupper(*buf, std::locale::classic()) == std::toupper(name[i], std::locale::classic()))
    {
      // Matching so far, keep matching name and buffer, char by char, case insensitive.
      ++i;
      ++buf;

      continue;
    }

    // Variable name character did not match the buffer, reset.
    i = 0;

    // Skip till the end of current "name=value" pair.
    do
    {
      ++buf;
    } while (*buf != '\0');

    // Increment buf to point to the start of the next "name=value" pair.
    ++buf;
  }

  return nullptr;
}

#endif
