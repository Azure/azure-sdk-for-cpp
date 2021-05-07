// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <cstdlib>

inline std::string GetEnv(std::string const& name)
{
#if defined(_MSC_VER)
#pragma warning(push)
// warning C4996: 'getenv': This function or variable may be unsafe. Consider using _dupenv_s
// instead.
#pragma warning(disable : 4996)
#endif
  auto const result = std::getenv(name.c_str());
  return result != nullptr ? result : "";
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
}
