// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Declaration of the UserAgentGenerator type.
 */

#pragma once

#include <string>

namespace Azure { namespace Core { namespace Http { namespace _detail {
  class UserAgentGenerator {
  public:
    static std::string GenerateUserAgent(
        std::string const& componentName,
        std::string const& componentVersion,
        std::string const& applicationId);
  };
}}}} // namespace Azure::Core::Http::_detail
