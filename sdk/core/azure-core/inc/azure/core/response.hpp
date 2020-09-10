// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Wraps raw HTTP response into a response of a specific type.
 */

#pragma once

#include <azure/core/http/http.hpp>

namespace Azure { namespace Core {
  /**
   * @brief Wraps raw HTTP response into a response of a specific type.
   *
   * @tparam T A specific type of value to get from the raw HTTP response type.
   */
  template <class T> class Response {
    T m_value;
    std::unique_ptr<Http::RawResponse> m_rawResponse;

  public:
    /**
     * @brief Initialize a #Response<T> with an initial value.
     *
     * @param initialValue Initial value.
     * @param rawResponse Raw HTTP response.
     */
    // Require a raw response to create a Response T
    explicit Response(T initialValue, std::unique_ptr<Http::RawResponse>&& rawResponse)
        : m_value(std::move(initialValue)), m_rawResponse(std::move(rawResponse))
    {
    }

    /**
     * @brief Get raw HTTP response.
     */
    // Do not give up raw response ownership.
    Http::RawResponse& GetRawResponse() { return *this->m_rawResponse; }

    /**
     * @brief Get a pointer to a value of a specific type.
     */
    const T* operator->() const { return &this->m_value; };

    /**
     * @brief Get a tpointer to a value of a specific type.
     */
    T* operator->() { return &this->m_value; };

    /**
     * @brief Get value of a specific type.
     */
    T& operator*() { return this->m_value; };

    /**
     * @brief Get value of a specific type.
     */
    const T& operator*() const { return this->m_value; };

    /**
     * @brief Get an rvalue reference to the value of a specific type.
     */
    T&& ExtractValue() { return std::move(this->m_value); }

    /**
     * @brief Get a smaprt pointer rvalue reference to the value of a specific type.
     */
    std::unique_ptr<Http::RawResponse>&& ExtractRawResponse()
    {
      return std::move(this->m_rawResponse);
    }
  };
}} // namespace Azure::Core
