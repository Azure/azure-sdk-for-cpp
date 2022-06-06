// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief HTTP pipeline is a stack of HTTP policies.
 * @remark See #policy.hpp
 */

#pragma once

#include <string>

namespace Azure { namespace Core { namespace Http { namespace _internal {
  class UserAgentGenerator {
  public:
    static std::string GenerateUserAgent(
        std::string const& componentName,
        std::string const& componentVersion,
        std::string const& applicationId);
  };
}}}} // namespace Azure::Core::Http::_internal
