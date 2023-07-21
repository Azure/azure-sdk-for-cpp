// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Useful utilities for the Event Hubs Clients.
#pragma once

#include "package_version.hpp"

#include <azure/core/internal/http/user_agent.hpp>

namespace Azure { namespace Messaging { namespace EventHubs { namespace _detail {

  class EventHubUtilities {
  public:
    template <typename T> static void SetUserAgent(T& options, std::string const& applicationId)
    {
      options.Properties.emplace("product", "Messaging.EventHubs");
      options.Properties.emplace("version", PackageVersion::ToString());
#if defined(AZ_PLATFORM_WINDOWS)
      options.Properties.emplace("platform", "Windows");
#elif defined(AZ_PLATFORM_LINUX)
      options.Properties.emplace("platform", "Linux");
#elif defined(AZ_PLATFORM_MAC)
      options.Properties.emplace("platform", "Mac");
#endif
      options.Properties.emplace(
          "user-agent",
          Azure::Core::Http::_detail::UserAgentGenerator::GenerateUserAgent(
              "Messaging.EventHubs", PackageVersion::ToString(), applicationId));
    }
  };
}}}} // namespace Azure::Messaging::EventHubs::_detail
