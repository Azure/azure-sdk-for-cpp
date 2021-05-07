// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define RequestFailedException. It is used by HTTP exceptions.
 */

#pragma once

#include "azure/core/http/http_status_code.hpp"
#include "azure/core/http/raw_response.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace Azure { namespace Core {
  /**
   * @brief An error while trying to send a request to Azure service.
   *
   */
  class RequestFailedException : public std::runtime_error {
  public:
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
     * @brief The error code from service returned in the Http response.
     *
     */
    std::string ErrorCode;

    /**
     * @brief The error message from the service returned in the Http response.
     *
     */
    std::string Message;

    /**
     * @brief The entire Http raw response.
     *
     */
    std::unique_ptr<Azure::Core::Http::RawResponse> RawResponse;

    /**
     * @brief Construct a new Request Failed Exception object.
     *
     * @remark An Exception without an Http raw response represent an exception happend
     * before sending the request to the server. There is no response yet.
     *
     * @param message The error description.
     */
    explicit RequestFailedException(std::string const& message)
        : std::runtime_error(message), Message(message)
    {
    }

    /**
     * @brief Construct a new Request Failed Exception object with an Http raw response.
     *
     * @remark The Http raw response is parsed to get the always expected information for all Azure
     * Services like the status code, reason phrase and some headers like the request id. A concrete
     * Service exception which derives from this exception uses its constructor to parse the Http
     * raw response and assing the service specific values to the exception.
     *
     * @param message The error description.
     * @param rawResponse The Http raw response from the service.
     */
    explicit RequestFailedException(
        const std::string& message,
        std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse);
  };
}} // namespace Azure::Core
