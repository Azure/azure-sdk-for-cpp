// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#define AZURE_CORE_VERSION_MAJOR 1
#define AZURE_CORE_VERSION_MINOR 0
#define AZURE_CORE_VERSION_PATCH 0
#define AZURE_CORE_VERSION_PRERELEASE "beta.1"

namespace Azure { namespace Core {

  class Version {
  public:
    static constexpr int Major = AZURE_CORE_VERSION_MAJOR;
    static constexpr int Minor = AZURE_CORE_VERSION_MINOR;
    static constexpr int Patch = AZURE_CORE_VERSION_PATCH;
    static std::string const PreRelease;
    static std::string const VersionString();

  private:
    // To avoid leaking out the #define values we smuggle out the value
    // which will later be used to initialize the PreRelease std::string
    static constexpr const char* secret = AZURE_CORE_VERSION_PRERELEASE;
  };

}} // namespace Azure::Core

#undef AZURE_CORE_VERSION_MAJOR
#undef AZURE_CORE_VERSION_MINOR
#undef AZURE_CORE_VERSION_PATCH
#undef AZURE_CORE_VERSION_PRERELEASE
