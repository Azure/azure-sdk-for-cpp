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
      constexpr const char* packageName = "cpp-azure-messaging-eventhubs-cpp";

      options.Properties.emplace("Product", + packageName);
      options.Properties.emplace("Version", PackageVersion::ToString());
#if defined(AZ_PLATFORM_WINDOWS)
      options.Properties.emplace("Platform", "Windows");
#elif defined(AZ_PLATFORM_LINUX)
      options.Properties.emplace("Platform", "Linux");
#elif defined(AZ_PLATFORM_MAC)
      options.Properties.emplace("Platform", "Mac");
#endif
      options.Properties.emplace(
          "User-Agent",
          Azure::Core::Http::_detail::UserAgentGenerator::GenerateUserAgent(
              packageName, PackageVersion::ToString(), applicationId));
    }
  };
}}}} // namespace Azure::Messaging::EventHubs::_detail
