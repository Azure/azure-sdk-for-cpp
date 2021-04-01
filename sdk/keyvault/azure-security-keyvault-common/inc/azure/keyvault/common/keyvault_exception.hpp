// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Defines a general exception for Key Vault service clients.
 *
 */

#pragma once

#include <azure/core/exception.hpp>
#include <azure/core/http/http.hpp>

#include <stdexcept>
#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Common {

  /**
   * @brief The general exception thrown by the Key Vault SDK clients.
   *
   */
  class KeyVaultException : public Azure::Core::RequestFailedException {
  public:
    /**
     * @brief Construct a new Key Vault Exception object without an Http raw response.
     *
     * @remark A Key Vault Exception without an Http raw response represent an exception happend
     * before sending the request to the server. There is no response yet.
     *
     * @param message An error message for the exception.
     */
    explicit KeyVaultException(const std::string& message) : RequestFailedException(message) {}

    /**
     * @brief Construct a new Key Vault Exception object with an Http raw response.
     *
     * @param message  An error message for the exception.
     * @param rawResponse The Http raw response from the service.
     */
    explicit KeyVaultException(
        const std::string& message,
        std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse);
  };
}}}} // namespace Azure::Security::KeyVault::Common
