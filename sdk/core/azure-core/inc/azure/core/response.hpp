// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Wraps raw HTTP response into a response of a specific type.
 */

#pragma once

#include "azure/core/http/http.hpp"
#include "azure/core/nullable.hpp"
#include <memory> // for unique_ptr
#include <stdexcept>
#include <utility> // for move

namespace Azure {
/**
 * @brief Wraps raw HTTP response into a response of a specific type.
 *
 * @tparam T A specific type of value to get from the raw HTTP response type.
 */
template <class T> class Response {

public:
  Azure::Core::Http::HttpStatusCode StatusCode;
  T Value;
  std::unique_ptr<Azure::Core::Http::RawResponse> RawResponse;

  /**
   * @brief Initialize a #Azure::Core::Response<T> with an initial value.
   *
   * @param initialValue Initial value.
   * @param rawResponse Raw HTTP response.
   */
  // Require a raw response to create a Response T
  explicit Response(T initialValue, std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse)
      : StatusCode(rawResponse->GetStatusCode()), Value(std::move(initialValue)),
        RawResponse(std::move(rawResponse))
  {
  }
};

} // namespace Azure
