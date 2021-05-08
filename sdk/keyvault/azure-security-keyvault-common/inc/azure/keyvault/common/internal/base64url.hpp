// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Provides helper method for base64url.
 *
 */

#pragma once

#include <algorithm>
#include <azure/core/base64.hpp>
#include <stdexcept>
#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace _internal {

  /**
   * @brief Provides conversion methods for base64url.
   */
  struct Base64Url final
  {
    static inline std::string Base64UrlEncode(const std::vector<uint8_t>& data)
    {
      auto base64 = Azure::Core::Convert::Base64Encode(data);
      // update to base64url
      auto trail = base64.find('=');
      if (trail != std::string::npos)
      {
        base64 = base64.substr(0, trail);
      }
      std::replace(base64.begin(), base64.end(), '+', '-');
      std::replace(base64.begin(), base64.end(), '/', '_');
      return base64;
    }

    static inline std::vector<uint8_t> Base64UrlDecode(const std::string& text)
    {
      std::string base64url(text);
      // base64url to base64
      std::replace(base64url.begin(), base64url.end(), '-', '+');
      std::replace(base64url.begin(), base64url.end(), '_', '/');
      switch (base64url.size() % 4)
      {
        case 0:
          break;
        case 2:
          base64url.append("==");
          break;
        case 3:
          base64url.append("=");
          break;
        default:
          throw new std::invalid_argument("Unexpected base64 encoding in the http response.");
      }
      return Azure::Core::Convert::Base64Decode(base64url);
    }
  };
}}}} // namespace Azure::Security::KeyVault::_internal
