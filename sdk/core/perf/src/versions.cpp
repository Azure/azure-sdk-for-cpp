// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/perf/versions.hpp"

#include <iostream>
#include <sstream>

namespace {

std::string CompilerInfo()
{
  std::ostringstream os;
#if defined(__clang__)
  os << "clang " << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__;
#elif defined(__GNUC__)
  os << "gcc " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__;
#elif defined(_MSC_VER)
  os << "MSVC " << _MSC_VER;
#else
  os << "unknown";
#endif
  return os.str();
}

std::string LanguageStandard()
{
  std::ostringstream os;
#if defined(__cplusplus)
  os << "C++ __cplusplus=" << __cplusplus;
#else
  os << "C++ unknown";
#endif
  return os.str();
}

} // namespace

namespace Azure { namespace Perf {

  void PrintVersionsBlock(
      std::vector<std::pair<std::string, std::string>> const& injectedVersions)
  {
    std::cout << std::endl << "=== Versions ===" << std::endl;
    std::cout << "Compiler: " << CompilerInfo() << std::endl;
    std::cout << "Language: " << LanguageStandard() << std::endl;
    for (auto const& kv : injectedVersions)
    {
      if (kv.second.empty())
      {
        continue;
      }
      std::cout << kv.first << ": " << kv.second << std::endl;
    }
  }

}} // namespace Azure::Perf
