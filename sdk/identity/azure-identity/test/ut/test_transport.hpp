// Copyright (c) Microsoft Corporation. All rights reserved.
// An SPDX-License-Identifier: MIT

#pragma once

#include <functional>

#include <azure/core/http/transport.hpp>

class TestTransport : public Azure::Core::Http::HttpTransport {
public:
  typedef std::function<std::unique_ptr<Azure::Core::Http::RawResponse>(
      Azure::Core::Http::Request& request,
      Azure::Core::Context const& context)>
      SendCallback;

private:
  SendCallback m_sendCallback;

public:
  TestTransport(SendCallback send) : m_sendCallback(send) {}

  std::unique_ptr<Azure::Core::Http::RawResponse> Send(
      Azure::Core::Http::Request& request,
      Azure::Core::Context const& context) override
  {
    return m_sendCallback(request, context);
  }
};
