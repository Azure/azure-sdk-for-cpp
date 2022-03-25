// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include <azure/core/internal/io/null_body_stream.hpp>
#include <azure/core/websockets/websockets.hpp>

#include <string>
#include <utility>
#include <vector>

using namespace Azure::Core;
using namespace Azure::Core::WebSockets;
using namespace Azure::Core::IO;

namespace Azure { namespace Core { namespace Test {

  TEST(WebSocket, basicTest)
  {
    // websocket client creation
    WebSocketClient wsClient(Url("ws://someUrl"));
    wsClient.Connect();

    // Callback for on received message
    auto const onMessageReceived = [](WebSocketInMessage const& message) -> void { (void)message; };
    wsClient.OnMessage(onMessageReceived);

    // Sending a message
    Azure::Core::IO::_internal::NullBodyStream noMessage;
    WebSocketOutMessage outMessage(WebSocketMessageType::Ping, noMessage);
    wsClient.Send(outMessage, Context{});

    // Note:  Program needs to keep waiting here while the OnMessage callback is dispatched on
    // message received....
  }
}}} // namespace Azure::Core::Test
