// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/websockets/websockets.hpp"
#include "azure/core/url.hpp"

using namespace Azure::Core;
using namespace Azure::Core::Http;
using namespace Azure::Core::WebSockets;
using namespace Azure::Core::WebSockets::_detail;

// Client implementation can depend on OS or CMake-options and it becomes an impl detail
namespace {
class TestImpl : public WebSocketClientImplementation {

public:
  TestImpl(Azure::Core::Url url, WebSocketClientOptions clientOptions)
      : WebSocketClientImplementation(std::move(url), std::move(clientOptions))
  {
  }

  void Connect() override {}
  void Close() override {}
  void Send(WebSocketOutMessage& message, Azure::Core::Context const& context) override
  {
    (void)message;
    (void)context;
  }
  void OnMessage(std::function<void(WebSocketInMessage const&)> const& handler) override
  {
    (void)handler;
  }
};
} // namespace

namespace Azure { namespace Core { namespace WebSockets {

  WebSocketClient::WebSocketClient(Azure::Core::Url url, WebSocketClientOptions clientOptions)
      : m_client(std::make_unique<TestImpl>(url, std::move(clientOptions)))
  {
  }

}}} // namespace Azure::Core::WebSockets
