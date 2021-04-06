// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/template/version.hpp>
#include <sstream>
#include <string>

using namespace Azure::Template::_detail;

const std::string PackageVersion::PreRelease = secret;

std::string PackageVersion::ToString()
{
  static const std::string versionString = [] {
    std::string version;
    std::stringstream ss;
    std::string dot = ".";

    ss << PackageVersion::Major << dot << PackageVersion::Minor << dot << PackageVersion::Patch;

    if (!PackageVersion::PreRelease.empty())
      ss << "-" << PackageVersion::PreRelease;

    return ss.str();
  }();

  return versionString;
}
