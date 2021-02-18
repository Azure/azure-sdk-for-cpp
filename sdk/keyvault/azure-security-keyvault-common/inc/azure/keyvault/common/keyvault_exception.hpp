// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Defines a general exception for Key Vault service clients.
 *
 */

#pragma once

#include <azure/core/exception.hpp>
#include <azure/core/http/http.hpp>

#include <map>
#include <stdexcept>
#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Common {

  /**
   * @brief The general exception thrown by the Key Vault SDK clients.
   *
   */
  struct KeyVaultException : public Azure::Core::RequestFailedException
  {
    /**
     * @brief Construct a new Key Vault Exception object.
     *
     * @param message
     */
    explicit KeyVaultException(const std::string& message) : RequestFailedException(message) {}

    /**
     * @brief The Http response code.
     *
     */
    Azure::Core::Http::HttpStatusCode StatusCode = Azure::Core::Http::HttpStatusCode::None;

    /**
     * @brief The Http reason phrase from the response.
     *
     */
    std::string ReasonPhrase;

    /**
     * @brief The client request header from the Http response.
     *
     */
    std::string ClientRequestId;

    /**
     * @brief The request id header from the Http response.
     *
     */
    std::string RequestId;

    /**
     * @brief The error code from the Key Vault service returned in the Http response.
     *
     */
    std::string ErrorCode;

    /**
     * @brief The error message from the Key Vault service returned in the Http response.
     *
     */
    std::string Message;

    /**
     * @brief The entire Http raw response.
     *
     */
    std::unique_ptr<Azure::Core::Http::RawResponse> RawResponse;

    /**
     * @brief Create a #Azure::Security::KeyVault::Common::KeyVaultException by parsing the \p
     * response.
     *
     * @param response The Http raw response from the network.
     * @return KeyVaultException.
     */
    static KeyVaultException CreateFromResponse(
        std::unique_ptr<Azure::Core::Http::RawResponse> response);

    /**
     * @brief Create #Azure::Security::KeyVault::Common::KeyVaultException by parsing the \p
     * response reference.
     *
     * @param response The Http raw response from the network.
     * @return KeyVaultException.
     */
    static KeyVaultException CreateFromResponse(Azure::Core::Http::RawResponse const& response);
  };
}}}} // namespace Azure::Security::KeyVault::Common
