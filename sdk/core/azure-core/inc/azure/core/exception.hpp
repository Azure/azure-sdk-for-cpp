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
   */
  class RequestFailedException : public std::runtime_error {
  public:
    /**
     * @brief The HTTP response code.
     *
     */
    Azure::Core::Http::HttpStatusCode StatusCode = Azure::Core::Http::HttpStatusCode::None;

    /**
     * @brief The HTTP reason phrase from the response.
     *
     */
    std::string ReasonPhrase;

    /**
     * @brief The client request header from the HTTP response.
     *
     */
    std::string ClientRequestId;

    /**
     * @brief The request ID header from the HTTP response.
     *
     */
    std::string RequestId;

    /**
     * @brief The error code from service returned in the HTTP response.
     *
     */
    std::string ErrorCode;

    /**
     * @brief The error message from the service returned in the HTTP response.
     *
     */
    std::string Message;

    /**
     * @brief The entire HTTP raw response.
     *
     */
    std::unique_ptr<Azure::Core::Http::RawResponse> RawResponse;

    /**
     * @brief Constructs a new `RequestFailedException` object.
     *
     * @note An Exception without an HTTP raw response represents an exception that happened
     * before sending the request to the server.
     *
     * @param message The error description.
     */
    explicit RequestFailedException(std::string const& message)
        : std::runtime_error(message), Message(message)
    {
    }

    /**
     * @brief Constructs a new `RequestFailedException` object with an HTTP raw response.
     *
     * @note The HTTP raw response is parsed to populate information expected from all Azure
     * Services like the status code, reason phrase and some headers like the request ID. A concrete
     * Service exception which derives from this exception uses its constructor to parse the HTTP
     * raw response adding the service specific values to the exception.
     *
     * @param message The error description.
     * @param rawResponse The HTTP raw response from the service.
     */
    explicit RequestFailedException(
        const std::string& message,
        std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse);

    /**
     * @brief Constructs a new `RequestFailedException` by copying from an existing one.
     * @note Copies the #Azure::Core::Http::RawResponse into the new `RequestFailedException`.
     *
     * @param other The `RequestFailedException` to be copied.
     */
    RequestFailedException(const RequestFailedException& other)
        : std::runtime_error(other.Message), StatusCode(other.StatusCode),
          ReasonPhrase(other.ReasonPhrase), ClientRequestId(other.ClientRequestId),
          RequestId(other.RequestId), ErrorCode(other.ErrorCode), Message(other.Message),
          RawResponse(
              other.RawResponse
                  ? std::make_unique<Azure::Core::Http::RawResponse>(*other.RawResponse)
                  : nullptr)
    {
    }

    RequestFailedException(RequestFailedException&& other) = default;
    RequestFailedException& operator=(const RequestFailedException&) = delete;
    RequestFailedException& operator=(RequestFailedException&&) = delete;
    ~RequestFailedException() = default;
  };
}} // namespace Azure::Core
