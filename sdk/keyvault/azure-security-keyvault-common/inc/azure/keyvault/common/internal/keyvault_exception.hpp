// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Defines a general exception for Key Vault service clients.
 *
 */

#pragma once

#include <azure/core/exception.hpp>
#include <azure/core/http/http.hpp>

#include <memory>
#include <stdexcept>
#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace _internal {
  /**
   * @brief Container for static methods to parse Key Vault payloads to Azure Core Exception.
   *
   */
  struct KeyVaultException final
  {
    /**
     * @brief Parsed the HTTP payload into an #Azure::Core::RequestFailedException
     *
     * @param rawResponse The HTTP raw response.
     * @return Azure::Core::RequestFailedException
     */
    static Azure::Core::RequestFailedException CreateException(
        std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse);
  };
}}}} // namespace Azure::Security::KeyVault::_internal
