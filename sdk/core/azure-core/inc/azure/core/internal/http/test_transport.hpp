// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>

#include "azure/core/http/transport.hpp"

namespace Azure { namespace Core { namespace Http { namespace _internal {
  class TestTransport : public HttpTransport {
  public:
    typedef std::function<std::unique_ptr<RawResponse>(Request& request, Context const& context)>
        SendCallback;

  private:
    SendCallback m_sendCallback;

  public:
    TestTransport(SendCallback send) : m_sendCallback(send) {}

    std::unique_ptr<RawResponse> Send(Request& request, Context const& context) override
    {
      return m_sendCallback(request, context);
    }
  };
}}}} // namespace Azure::Core::Http::_internal
