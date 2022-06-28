// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "../../src/http/websockets/websocketsimpl.hpp"
#include "azure/core/http/websockets/websockets.hpp"
#include "azure/core/internal/json/json.hpp"
#include <chrono>
#include <gtest/gtest.h>
#include <list>

using namespace Azure::Core;
using namespace Azure::Core::Http::WebSockets;
using namespace std::chrono_literals;

TEST(WebSocketTests, CreateSimpleSocket)
{
  {
    WebSocket defaultSocket(Azure::Core::Url("http://localhost:8000"));
    defaultSocket.AddHeader("newHeader", "headerValue");
  }
}

TEST(WebSocketTests, OpenSimpleSocket)
{
  {
    WebSocketOptions options;
    WebSocket defaultSocket(Azure::Core::Url("http://localhost:8000/openclosetest"), options);
    defaultSocket.AddHeader("newHeader", "headerValue");

    defaultSocket.Open();

    // Close the socket without notifying the peer.
    defaultSocket.Close();
  }
}

TEST(WebSocketTests, OpenAndCloseSocket)
{
  {
    WebSocket defaultSocket(Azure::Core::Url("http://localhost:8000/openclosetest"));
    defaultSocket.AddHeader("newHeader", "headerValue");

    defaultSocket.Open();

    // Close the socket without notifying the peer.
    defaultSocket.Close(4500);
  }

  {
    WebSocket defaultSocket(Azure::Core::Url("http://localhost:8000/openclosetest"));
    defaultSocket.AddHeader("newHeader", "headerValue");

    defaultSocket.Open();

    // Close the socket without notifying the peer.
    defaultSocket.Close(4500, "This is a good reason.");

    //
    // Now re-open the socket - this should work to reset everything.
    defaultSocket.Open();
    defaultSocket.Close();
  }
}

TEST(WebSocketTests, SimpleEcho)
{
  {
    WebSocket testSocket(Azure::Core::Url("http://localhost:8000/echotest"));

    testSocket.Open();

    testSocket.SendFrame("Test message", true);

    auto response = testSocket.ReceiveFrame();
    EXPECT_EQ(WebSocketResultType::TextFrameReceived, response->ResultType);
    auto textResult = response->AsTextFrame();
    EXPECT_EQ("Test message", textResult->Text);

    // Close the socket gracefully.
    testSocket.Close();
  }
  {
    WebSocket testSocket(Azure::Core::Url("http://localhost:8000/echotest?delay=20"));

    testSocket.Open();

    std::vector<uint8_t> binaryData{1, 2, 3, 4, 5, 6};

    testSocket.SendFrame(binaryData, true);

    auto response = testSocket.ReceiveFrame();
    EXPECT_EQ(WebSocketResultType::BinaryFrameReceived, response->ResultType);
    auto textResult = response->AsBinaryFrame();
    EXPECT_EQ(binaryData, textResult->Data);

    // Close the socket gracefully.
    testSocket.Close();
  }
}

template <size_t N> void EchoRandomData(WebSocket& socket)
{
  std::array<uint8_t, N> data = Azure::Core::Http::WebSockets::_detail::GenerateRandomBytes<N>();
  std::vector<uint8_t> sendData{data.begin(), data.end()};

  socket.SendFrame(sendData, true);

  auto response = socket.ReceiveFrame();
  EXPECT_EQ(WebSocketResultType::BinaryFrameReceived, response->ResultType);
  auto binaryResult = response->AsBinaryFrame();
  // Make sure we get back the data we sent in the echo request.
  EXPECT_EQ(sendData.size(), binaryResult->Data.size());
  EXPECT_EQ(sendData, binaryResult->Data);
}

TEST(WebSocketTests, VariableSizeEcho)
{
  {
    WebSocket testSocket(Azure::Core::Url("http://localhost:8000/echotest"));

    testSocket.Open();
    {
      EchoRandomData<100>(testSocket);
      EchoRandomData<124>(testSocket);
      EchoRandomData<125>(testSocket);
      // The websocket protocol treats lengths of 125, 126 and > 127 specially.
      EchoRandomData<126>(testSocket);
      EchoRandomData<127>(testSocket);
      EchoRandomData<128>(testSocket);
      EchoRandomData<1020>(testSocket);
      EchoRandomData<1021>(testSocket);
      EchoRandomData<1022>(testSocket);
      EchoRandomData<1023>(testSocket);
      EchoRandomData<1024>(testSocket);
      EchoRandomData<2048>(testSocket);
      EchoRandomData<4096>(testSocket);
      EchoRandomData<8192>(testSocket);
      // The websocket protocol treats lengths of >65536 specially.
      EchoRandomData<65535>(testSocket);
      EchoRandomData<65536>(testSocket);
      EchoRandomData<131072>(testSocket);
    }
    // Close the socket gracefully.
    testSocket.Close();
  }
}

TEST(WebSocketTests, CloseDuringEcho)
{
  {
    WebSocket testSocket(Azure::Core::Url("http://localhost:8000/closeduringecho"));

    testSocket.Open();

    testSocket.SendFrame("Test message", true);

    auto response = testSocket.ReceiveFrame();
    EXPECT_EQ(WebSocketResultType::PeerClosed, response->ResultType);
    auto peerClosed = response->AsPeerCloseFrame();
    EXPECT_EQ(1001, peerClosed->RemoteStatusCode);

    // Close the socket gracefully.
    testSocket.Close();
  }
}

// Does not work because curl rejects the wss: scheme.
class LibWebSocketIncrementProtocol {
  WebSocketOptions m_options{true, {"dumb-increment-protocol"}};
  WebSocket m_socket;

public:
  LibWebSocketIncrementProtocol() : m_socket{Azure::Core::Url("wss://libwebsockets.org"), m_options}
  {
  }

  void Open() { m_socket.Open(); }
  int GetNextNumber()
  {
    // Time out in 5 seconds if no activity.
    Azure::Core::Context contextWithTimeout
        = Azure::Core::Context().WithDeadline(std::chrono::system_clock::now() + 10s);
    auto work = m_socket.ReceiveFrame(contextWithTimeout);
    if (work->ResultType == WebSocketResultType::TextFrameReceived)
    {
      auto frame = work->AsTextFrame();
      return std::atoi(frame->Text.c_str());
    }
    if (work->ResultType == WebSocketResultType::BinaryFrameReceived)
    {
      auto frame = work->AsBinaryFrame();
      throw std::runtime_error("Not implemented");
    }
    else if (work->ResultType == WebSocketResultType::PeerClosed)
    {
      GTEST_LOG_(INFO) << "Remote server closed connection." << std::endl;
      throw std::runtime_error("Remote server closed connection.");
    }
    else
    {
      throw std::runtime_error("Unknown result type");
    }
  }

  void Reset() { m_socket.SendFrame("reset\n", true); }
  void RequestClose() { m_socket.SendFrame("closeme\n", true); }
  void Close() { m_socket.Close(); }
  void Close(uint16_t closeCode, std::string const& reasonText = {})
  {
    m_socket.Close(closeCode, reasonText);
  }
  void ConsumeUntilClosed()
  {
    while (m_socket.IsOpen())
    {
      auto work = m_socket.ReceiveFrame();
      if (work->ResultType == WebSocketResultType::PeerClosed)
      {
        auto peerClose = work->AsPeerCloseFrame();
        GTEST_LOG_(INFO) << "Peer closed. Remote Code: " << peerClose->RemoteStatusCode;
        if (peerClose->BodyStream.Length() != 0)
        {
          auto closeBody = peerClose->BodyStream.ReadToEnd();
          std::string closeText(closeBody.begin(), closeBody.end());
          GTEST_LOG_(INFO) << " Peer Closed Data: " << closeText;
        }
        GTEST_LOG_(INFO) << std::endl;
        return;
      }
      else if (work->ResultType == WebSocketResultType::TextFrameReceived)
      {
        auto frame = work->AsTextFrame();
        GTEST_LOG_(INFO) << "Ignoring " << frame->Text << std::endl;
      }
    }
  }
};

class LibWebSocketStatus {

public:
  std::string GetLWSStatus()
  {
    WebSocketOptions options;

    options.ServiceName = "websockettest";
    // Send 3 protocols to LWS.
    options.Protocols.push_back("brownCow");
    options.Protocols.push_back("lws-status");
    options.Protocols.push_back("flibbityflobbidy");
    WebSocket serverSocket(Azure::Core::Url("wss://libwebsockets.org"), options);
    serverSocket.Open();

    // The server should have chosen the lws-status protocol since it doesn't understand the other
    // protocols.
    EXPECT_EQ("lws-status", serverSocket.GetChosenProtocol());
    auto lwsStatus = serverSocket.ReceiveFrame();
    if (lwsStatus->ResultType != WebSocketResultType::TextFrameReceived)
    {
      __debugbreak();
    }
    EXPECT_EQ(WebSocketResultType::TextFrameReceived, lwsStatus->ResultType);
    auto textFrame = lwsStatus->AsTextFrame();
    std::string returnValue = textFrame->Text;
    bool isFinalFrame = textFrame->IsFinalFrame;
    while (!isFinalFrame)
    {
      lwsStatus = serverSocket.ReceiveFrame();
      EXPECT_EQ(WebSocketResultType::ContinuationReceived, lwsStatus->ResultType);
      auto continuation = lwsStatus->AsContinuationFrame();
      returnValue.insert(
          returnValue.end(),
          continuation->ContinuationData.begin(),
          continuation->ContinuationData.end());
      isFinalFrame = continuation->IsFinalFrame;
    }
    serverSocket.Close();
    return returnValue;
  }
};

TEST(WebSocketTests, LibWebSocketOrg)
{
  {
    LibWebSocketStatus lwsStatus;
    auto serverStatus = lwsStatus.GetLWSStatus();
    GTEST_LOG_(INFO) << serverStatus << std::endl;

    Azure::Core::Json::_internal::json status(
        Azure::Core::Json::_internal::json::parse(serverStatus));
    EXPECT_TRUE(status["conns"].is_array());
    auto connections = status["conns"].get_ref<std::vector<Azure::Core::Json::_internal::json>&>();
    bool foundOurConnection = false;

    // Scan through the list of connections to find a connection from the websockettest.
    for (auto connection : connections)
    {
      EXPECT_TRUE(connection["ua"].is_string());
      auto userAgent = connection["ua"].get<std::string>();
      if (userAgent.find("websockettest") != std::string::npos)
      {
        foundOurConnection = true;
        break;
      }
    }
    EXPECT_TRUE(foundOurConnection);
  }
  {
    LibWebSocketIncrementProtocol incrementProtocol;
    incrementProtocol.Open();

    // Note that we cannot practically validate the numbers received from the service because
    // they may be in flight at the time the "Reset" call is made.
    for (auto i = 0; i < 100; i += 1)
    {
      if (i % 5 == 0)
      {
        GTEST_LOG_(INFO) << "Reset" << std::endl;
        incrementProtocol.Reset();
      }
      int number = incrementProtocol.GetNextNumber();
      GTEST_LOG_(INFO) << "Got next number " << number << std::endl;
    }
    incrementProtocol.RequestClose();
    incrementProtocol.ConsumeUntilClosed();
  }
}
