// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Declaration of the UserAgentGenerator type.
 */

#pragma once

#include <string>

namespace Azure { namespace Core { namespace Http { namespace _detail {
  // NOTE: Treat Azure::Core::Http::_detail::UserAgentGenerator::GenerateUserAgent() as _internal -
  // it is being/has been used by eventhubs.
  class UserAgentGenerator {
  public:
    static std::string GenerateUserAgent(
        std::string const& componentName,
        std::string const& componentVersion,
        std::string const& applicationId,
        long cplusplusValue);

    [[deprecated("Use an overload with additional cplusplusValue parameter.")]] static std::string
    GenerateUserAgent(
        std::string const& componentName,
        std::string const& componentVersion,
        std::string const& applicationId)
    {
      // The value of -3L is to signify that an old version of signature has been used (older
      // version of eventhubs); we can't rely on cpp version reported by it.
      return GenerateUserAgent(componentName, componentVersion, applicationId, -3L);
    }
  };
}}}} // namespace Azure::Core::Http::_detail
