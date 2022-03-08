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
     * @note This string is purely for informational or diagnostic purposes, and should't be relied
     * on at runtime.
     *
     */
    std::string Message;

    /**
     * @brief The entire HTTP raw response.
     *
     */
    std::unique_ptr<Azure::Core::Http::RawResponse> RawResponse;

    /**
     * @brief Constructs a new `%RequestFailedException` with a \p message string.
     *
     * @note An Exception without an HTTP raw response represents an exception that happened
     * before sending the request to the server.
     *
     * @param what The explanatory string.
     */
    explicit RequestFailedException(std::string const& what);

    /**
     * @brief Constructs a new `%RequestFailedException` object with an HTTP raw response.
     *
     * @note The HTTP raw response is parsed to populate information expected from all Azure
     * Services like the status code, reason phrase and some headers like the request ID. A concrete
     * Service exception which derives from this exception uses its constructor to parse the HTTP
     * raw response adding the service specific values to the exception.
     *
     * @param rawResponse The HTTP raw response from the service.
     */
    explicit RequestFailedException(std::unique_ptr<Azure::Core::Http::RawResponse>& rawResponse);

    /**
     * @brief Constructs a new `%RequestFailedException` by copying from an existing one.
     * @note Copies the #Azure::Core::Http::RawResponse into the new `RequestFailedException`.
     *
     * @param other The `%RequestFailedException` to be copied.
     */
    RequestFailedException(const RequestFailedException& other)
        : std::runtime_error(other.Message),
          RawResponse(
              other.RawResponse
                  ? std::make_unique<Azure::Core::Http::RawResponse>(*other.RawResponse)
                  : nullptr),
          StatusCode(other.StatusCode), ReasonPhrase(other.ReasonPhrase),
          ClientRequestId(other.ClientRequestId), RequestId(other.RequestId),
          ErrorCode(other.ErrorCode), Message(other.Message)
    {
    }

    /**
     * @brief Constructs a new `%RequestFailedException` by moving in an existing one.
     * @param other The `%RequestFailedException` to move in.
     */
    RequestFailedException(RequestFailedException&& other) = default;

    /**
     * @brief An instance of `%RequestFailedException` class cannot be assigned.
     *
     */
    RequestFailedException& operator=(const RequestFailedException&) = delete;

    /**
     * @brief An instance of `%RequestFailedException` class cannot be moved into another instance
     * after creation.
     *
     */
    RequestFailedException& operator=(RequestFailedException&&) = delete;

    /**
     * @brief Destructs `%RequestFailedException`.
     *
     */
    ~RequestFailedException() = default;

  private:
    std::string GetRawResponseField(
        std::unique_ptr<Azure::Core::Http::RawResponse> const& rawResponse,
        std::string fieldName);

  };
}} // namespace Azure::Core
