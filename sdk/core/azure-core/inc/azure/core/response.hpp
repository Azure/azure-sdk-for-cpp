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
#include <utility> // for move

namespace Azure { namespace Core {
  /**
   * @brief Wraps raw HTTP response into a response of a specific type.
   *
   * @tparam T A specific type of value to get from the raw HTTP response type.
   */
  template <class T> class Response {
    Nullable<T> m_value;
    std::unique_ptr<Http::RawResponse> m_rawResponse;

  public:
    /**
     * @brief Initialize a #Azure::Core::Response<T> with an initial value.
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
     * @brief Initialize a #Azure::Core::Response<T> with an absent value.
     *
     * @param rawResponse Raw HTTP response.
     */
    explicit Response(std::unique_ptr<Http::RawResponse>&& rawResponse)
        : m_rawResponse(std::move(rawResponse))
    {
    }

    /**
     * @brief Get raw HTTP response.
     */
    // Do not give up raw response ownership.
    Http::RawResponse& GetRawResponse() { return *this->m_rawResponse; }

    /**
     * @brief Check whether a value is contained.
     *
     * @return `true` If a value is contained, `false` if value is absent.
     */
    bool HasValue() const noexcept { return this->m_value.HasValue(); }

    /**
     * @brief Get a pointer to a value of a specific type.
     */
    const T* operator->() const
    {
      return &this->m_value.GetValue(); // GetValue ensures there is a contained value
    }

    /**
     * @brief Get a pointer to a value of a specific type.
     */
    T* operator->() { return &this->m_value.GetValue(); }

    /**
     * @brief Get value of a specific type.
     */
    T& operator*() { return this->m_value.GetValue(); }

    /**
     * @brief Get value of a specific type.
     */
    const T& operator*() const { return this->m_value.GetValue(); }

    /**
     * @brief Get an rvalue reference to the value of a specific type.
     */
    T&& ExtractValue() { return std::move(this->m_value).GetValue(); }

    /**
     * @brief Get a smart pointer rvalue reference to the value of a specific type.
     */
    std::unique_ptr<Http::RawResponse>&& ExtractRawResponse()
    {
      return std::move(this->m_rawResponse);
    }
  };

  /**
   * @brief Wraps raw HTTP response into a response of a void type.
   */
  template <> class Response<void> {
    std::unique_ptr<Http::RawResponse> m_rawResponse;

  public:
    /**
     * @brief Initialize a #Azure::Core::Response<void> with a raw response.
     *
     * @param rawResponse Raw HTTP response.
     */
    explicit Response(std::unique_ptr<Http::RawResponse>&& rawResponse)
        : m_rawResponse(std::move(rawResponse))
    {
    }

    /**
     * @brief Get raw HTTP response.
     */
    // Do not give up raw response ownership.
    Http::RawResponse& GetRawResponse() { return *this->m_rawResponse; }

    /**
     * @brief Get a smart pointer rvalue reference to the value of a specific type.
     */
    std::unique_ptr<Http::RawResponse>&& ExtractRawResponse()
    {
      return std::move(this->m_rawResponse);
    }
  };
}} // namespace Azure::Core
