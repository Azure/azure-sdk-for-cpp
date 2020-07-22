// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <http/http.hpp>

namespace Azure { namespace Core {

  template <class T> class Response {
    T m_value;
    std::unique_ptr<Http::RawResponse> m_rawResponse;

  public:
    // Require a raw response to create a Response T
    explicit Response(T initialValue, std::unique_ptr<Http::RawResponse>&& rawResponse)
        : m_value(std::move(initialValue)), m_rawResponse(std::move(rawResponse))
    {
    }

    // Do not give up raw response ownership.
    Http::RawResponse& GetRawResponse() { return *this->m_rawResponse; }

    const T* operator->() const { return &this->m_value; };
    T* operator->() { return &this->m_value; };
    T& operator*() { return this->m_value; };
    const T& operator*() const { return this->m_value; };

    T&& ExtractValue() { return std::move(this->m_value); }

    std::unique_ptr<Http::RawResponse>&& ExtractRawResponse()
    {
      return std::move(this->m_rawResponse);
    }
  };
}} // namespace Azure::Core
