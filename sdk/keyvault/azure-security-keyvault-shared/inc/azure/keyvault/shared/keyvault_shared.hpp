// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Shared code between Key Vault services.
 *
 */

#pragma once

#include <azure/core/url.hpp>

#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace _internal {

  /**
   * @brief Provides functionality to get scope information from a URL.
   *
   */
  class UrlScope {
    UrlScope() = delete;

  public:
    // This is a Key-Vault only patch to calculate token scope/audience
    static std::string GetScopeFromUrl(Azure::Core::Url const& url)
    {
      std::string calculatedScope(url.GetScheme() + "://");
      auto const& hostWithAccount = url.GetHost();
      auto hostNoAccountStart = std::find(hostWithAccount.begin(), hostWithAccount.end(), '.');

      // Insert the calculated scope only when the host in the url contains at least a `.`
      // Otherwise, only the default scope will be there.
      // We don't want to throw/validate input but just leave the values go to azure to decide what
      // to do.
      if (hostNoAccountStart != hostWithAccount.end())
      {
        calculatedScope.append(hostNoAccountStart + 1, hostWithAccount.end());
        calculatedScope.append("/.default");
      }

      return calculatedScope;
    }
  };

}}}} // namespace Azure::Security::KeyVault::_internal
