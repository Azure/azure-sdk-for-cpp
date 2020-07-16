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
    Response(const T& initialValue, std::unique_ptr<Http::RawResponse>&& rawResponse)
        : m_value(initialValue), m_rawResponse(std::move(rawResponse))
    {
    }

    // Do not give up raw response ownership.
    Http::RawResponse& GetRawResponse() { return *this->m_rawResponse; }

    T& operator=(const Response& other) = delete;

    const T* operator->() const { return &this->m_value; };
    T* operator->() { return &this->m_value; };
    T& operator*() { return this->m_value; };
    const T& operator*() const { return this->m_value; };

    T GetValue() { return std::move(this->m_value); }
  };
}} // namespace Azure::Core
