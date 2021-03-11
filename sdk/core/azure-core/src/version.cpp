// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/version.hpp"

#include <sstream>
#include <string>

using namespace Azure::Core;

const std::string Version::PreRelease = secret;

std::string Version::VersionString()
{
  static const std::string versionString = [] {
    std::string version;
    std::stringstream ss;
    std::string dot = ".";

    ss << Version::Major << dot << Version::Minor << dot << Version::Patch;

    if (!Version::PreRelease.empty())
      ss << "-" << Version::PreRelease;

    return ss.str();
  }();

  return versionString;
}
