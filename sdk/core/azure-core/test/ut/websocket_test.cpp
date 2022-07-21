// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "../../src/http/websockets/websocketsimpl.hpp"
#include "azure/core/http/websockets/websockets.hpp"
#include "azure/core/internal/json/json.hpp"
#include <chrono>
#include <gtest/gtest.h>
#include <list>
#include <set>
#include <thread>
#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include "azure/core/http/websockets/curl_websockets_transport.hpp"
#endif
// cspell::words closeme flibbityflobbidy

using namespace Azure::Core;
using namespace Azure::Core::Http::WebSockets;
using namespace std::chrono_literals;

class WebSocketTests : public testing::Test {
private:
protected:
  // Create
  static void SetUpTestSuite() {}
  static void TearDownTestSuite()
  {
    //    GTEST_LOG_(INFO) << "Shut down test server" << std::endl;
    //    WebSocket controlSocket(Azure::Core::Url("http://localhost:8000/control"));
    //    controlSocket.Open();
    //    controlSocket.SendFrame("close", true);
    //    auto controlResponse = controlSocket.ReceiveFrame();
    //    EXPECT_EQ(controlResponse->FrameType, WebSocketFrameType::TextFrameReceived);
  }
};

TEST_F(WebSocketTests, CreateSimpleSocket)
{
  {
    WebSocket defaultSocket(Azure::Core::Url("http://localhost:8000"));
    defaultSocket.AddHeader("newHeader", "headerValue");
    EXPECT_THROW(defaultSocket.GetChosenProtocol(), std::runtime_error);
  }
}

TEST_F(WebSocketTests, OpenSimpleSocket)
{
  {
    WebSocketOptions options;
    WebSocket defaultSocket(Azure::Core::Url("http://localhost:8000/openclosetest"), options);
    defaultSocket.AddHeader("newHeader", "headerValue");

    defaultSocket.Open();

    EXPECT_THROW(defaultSocket.AddHeader("newHeader", "headerValue"), std::runtime_error);

    // Close the socket without notifying the peer.
    defaultSocket.Close();
  }

  {
    WebSocketOptions options;
    WebSocket defaultSocket(Azure::Core::Url("http://www.microsoft.com/"), options);
    defaultSocket.AddHeader("newHeader", "headerValue");

    // When running this test locally, the call times out, so drop in a 5 second timeout on
    // the request.
    Azure::Core::Context requestContext = Azure::Core::Context::ApplicationContext.WithDeadline(
        std::chrono::system_clock::now() + 15s);
    EXPECT_THROW(defaultSocket.Open(requestContext), std::runtime_error);
  }
}

TEST_F(WebSocketTests, OpenAndCloseSocket)
{
  if (false)
  {
    WebSocket defaultSocket(Azure::Core::Url("http://localhost:8000/openclosetest"));
    defaultSocket.AddHeader("newHeader", "headerValue");

    defaultSocket.Open();

    // Close the socket without notifying the peer.
    defaultSocket.Close(4500);
  }

  {
    WebSocket defaultSocket(Azure::Core::Url("http://localhost:8000/openclosetest"));

    defaultSocket.Open();

    // Close the socket without notifying the peer.
    defaultSocket.Close(4500, "This is a good reason.");

    //
    // Now re-open the socket - this should work to reset everything.
    defaultSocket.Open();
    EXPECT_THROW(defaultSocket.Open(), std::runtime_error);
    defaultSocket.Close();
  }
}

TEST_F(WebSocketTests, SimpleEcho)
{
  {
    WebSocket testSocket(Azure::Core::Url("http://localhost:8000/echotest"));

    testSocket.Open();

    testSocket.SendFrame("Test message", true);

    auto response = testSocket.ReceiveFrame();
    EXPECT_EQ(WebSocketFrameType::TextFrameReceived, response->FrameType);
    EXPECT_THROW(response->AsBinaryFrame(), std::logic_error);
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
    EXPECT_EQ(WebSocketFrameType::BinaryFrameReceived, response->FrameType);
    EXPECT_THROW(response->AsPeerCloseFrame(), std::logic_error);
    EXPECT_THROW(response->AsTextFrame(), std::logic_error);
    auto textResult = response->AsBinaryFrame();
    EXPECT_EQ(binaryData, textResult->Data);

    // Close the socket gracefully.
    testSocket.Close();
  }

  {
    WebSocket testSocket(Azure::Core::Url("http://localhost:8000/echotest?fragment=true"));

    testSocket.Open();

    std::vector<uint8_t> binaryData{1, 2, 3, 4, 5, 6};

    testSocket.SendFrame(binaryData, true);

    std::vector<uint8_t> responseData;
    std::shared_ptr<Azure::Core::Http::WebSockets::WebSocketFrame> response;
    do
    {
      response = testSocket.ReceiveFrame();
      EXPECT_EQ(WebSocketFrameType::BinaryFrameReceived, response->FrameType);
      auto binaryResult = response->AsBinaryFrame();
      responseData.insert(responseData.end(), binaryResult->Data.begin(), binaryResult->Data.end());
    } while (!response->IsFinalFrame);

    auto textResult = response->AsBinaryFrame();
    EXPECT_EQ(binaryData, responseData);

    // Close the socket gracefully.
    testSocket.Close();
  }
}

template <size_t N> void EchoRandomData(WebSocket& socket)
{
  std::vector<uint8_t> sendData = Azure::Core::Http::WebSockets::_detail::GenerateRandomBytes(N);

  socket.SendFrame(sendData, true);

  std::vector<uint8_t> receiveData;

  std::shared_ptr<WebSocketFrame> response;
  do
  {
    response = socket.ReceiveFrame();
    EXPECT_EQ(WebSocketFrameType::BinaryFrameReceived, response->FrameType);
    auto binaryResult = response->AsBinaryFrame();
    receiveData.insert(receiveData.end(), binaryResult->Data.begin(), binaryResult->Data.end());
  } while (!response->IsFinalFrame);

  // Make sure we get back the data we sent in the echo request.
  EXPECT_EQ(sendData.size(), receiveData.size());
  EXPECT_EQ(sendData, receiveData);
}

TEST_F(WebSocketTests, VariableSizeEcho)
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

TEST_F(WebSocketTests, CloseDuringEcho)
{
  {
    WebSocket testSocket(Azure::Core::Url("ws://localhost:8000/closeduringecho"));

    testSocket.Open();

    testSocket.SendFrame("Test message", true);

    auto response = testSocket.ReceiveFrame();
    EXPECT_EQ(WebSocketFrameType::PeerClosedReceived, response->FrameType);
    auto PeerClosedReceived = response->AsPeerCloseFrame();
    EXPECT_EQ(1001, PeerClosedReceived->RemoteStatusCode);

    // Close the socket gracefully.
    testSocket.Close();
  }
}

TEST_F(WebSocketTests, ExpectThrow)
{
  {
    WebSocket testSocket(Azure::Core::Url("ws://localhost:8000/closeduringecho"));

    EXPECT_THROW(testSocket.SendFrame("Foo", true), std::runtime_error);
    std::vector<uint8_t> data{1, 2, 3, 4};
    EXPECT_THROW(testSocket.SendFrame(data, true), std::runtime_error);
    EXPECT_THROW(testSocket.ReceiveFrame(), std::runtime_error);
  }
}

std::string ToHexString(std::vector<uint8_t> const& data)
{
  std::stringstream ss;
  for (auto const& byte : data)
  {
    ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(byte);
  }
  return ss.str();
}

// Generator for random bytes. Used in WebSocketImplementation and tests.
std::vector<uint8_t> GenerateRandomBytes(size_t index, size_t vectorSize)
{
  std::random_device randomEngine;

  std::vector<uint8_t> rv(vectorSize + 4);
  rv[0] = index & 0xff;
  rv[1] = (index >> 8) & 0xff;
  rv[2] = (index >> 16) & 0xff;
  rv[3] = (index >> 24) & 0xff;
  std::generate(std::begin(rv) + 4, std::end(rv), [&randomEngine]() mutable {
    return static_cast<uint8_t>(randomEngine() % UINT8_MAX);
  });
  return rv;
}

TEST_F(WebSocketTests, PingReceiveTest)
{
  WebSocket testSocket(Azure::Core::Url("http://localhost:8000/echotest"));

  testSocket.Open();
  if (!testSocket.HasNativeWebSocketSupport())
  {

    GTEST_LOG_(INFO) << "Sleeping for 15 seconds to collect pings.";
    Azure::Core::Context receiveContext = Azure::Core::Context::ApplicationContext.WithDeadline(
        Azure::DateTime{std::chrono::system_clock::now() + 15s});
    EXPECT_THROW(testSocket.ReceiveFrame(receiveContext), Azure::Core::OperationCancelledException);
    auto statistics = testSocket.GetStatistics();
    GTEST_LOG_(INFO) << "Total bytes sent: " << std::dec << statistics.BytesSent;
    GTEST_LOG_(INFO) << "Total bytes received: " << std::dec << statistics.BytesReceived;
    GTEST_LOG_(INFO) << "Ping Frames received: " << std::dec << statistics.PingFramesReceived;
    GTEST_LOG_(INFO) << "Ping Frames sent: " << std::dec << statistics.PingFramesSent;
    GTEST_LOG_(INFO) << "Pong Frames received: " << std::dec << statistics.PongFramesReceived;
    GTEST_LOG_(INFO) << "Pong Frames sent: " << std::dec << statistics.PongFramesSent;
    GTEST_LOG_(INFO) << "Binary frames sent: " << std::dec << statistics.BinaryFramesSent;
    GTEST_LOG_(INFO) << "Binary frames received: " << std::dec << statistics.BinaryFramesReceived;
    GTEST_LOG_(INFO) << "Total frames lost: " << std::dec << statistics.FramesDropped;
    GTEST_LOG_(INFO) << "Transport Reads " << std::dec << statistics.TransportReads;
    GTEST_LOG_(INFO) << "Transport Bytes Read " << std::dec << statistics.TransportReadBytes;
    EXPECT_NE(0, statistics.PingFramesReceived);
    EXPECT_NE(0, statistics.PongFramesSent);
  }
}

TEST_F(WebSocketTests, PingSendTest)
{
  // Configure the socket to ping every second.
  WebSocketOptions socketOptions;
  socketOptions.PingInterval = std::chrono::seconds(1);
  WebSocket testSocket(Azure::Core::Url("http://localhost:8000/echotest"), socketOptions);

  testSocket.Open();
  if (!testSocket.HasNativeWebSocketSupport())
  {

    GTEST_LOG_(INFO) << "Sleeping for 10 seconds to collect pings.";
    // Note that we cannot collect incoming pings or outgoing pongs unless we are receiving
    // data from the server.
    Azure::Core::Context receiveContext = Azure::Core::Context::ApplicationContext.WithDeadline(
        Azure::DateTime{std::chrono::system_clock::now() + 10s});
    EXPECT_THROW(testSocket.ReceiveFrame(receiveContext), Azure::Core::OperationCancelledException);
    auto statistics = testSocket.GetStatistics();
    GTEST_LOG_(INFO) << "Total bytes sent: " << std::dec << statistics.BytesSent;
    GTEST_LOG_(INFO) << "Total bytes received: " << std::dec << statistics.BytesReceived;
    GTEST_LOG_(INFO) << "Ping Frames received: " << std::dec << statistics.PingFramesReceived;
    GTEST_LOG_(INFO) << "Ping Frames sent: " << std::dec << statistics.PingFramesSent;
    GTEST_LOG_(INFO) << "Pong Frames received: " << std::dec << statistics.PongFramesReceived;
    GTEST_LOG_(INFO) << "Pong Frames sent: " << std::dec << statistics.PongFramesSent;
    GTEST_LOG_(INFO) << "Binary frames sent: " << std::dec << statistics.BinaryFramesSent;
    GTEST_LOG_(INFO) << "Binary frames received: " << std::dec << statistics.BinaryFramesReceived;
    GTEST_LOG_(INFO) << "Total frames lost: " << std::dec << statistics.FramesDropped;
    GTEST_LOG_(INFO) << "Transport Reads " << std::dec << statistics.TransportReads;
    GTEST_LOG_(INFO) << "Transport Bytes Read " << std::dec << statistics.TransportReadBytes;
    EXPECT_NE(0, statistics.PingFramesSent);
    EXPECT_NE(0, statistics.PongFramesReceived);
    EXPECT_NE(0, statistics.PingFramesReceived);
    EXPECT_NE(0, statistics.PongFramesSent);
  }
}

TEST_F(WebSocketTests, MultiThreadedTestOnSingleSocket)
{
  constexpr size_t threadCount = 50;
  constexpr size_t testDataLength = 200000;
  constexpr size_t testDataSize = 100;
  constexpr auto testDuration = 10s;

  WebSocket testSocket(Azure::Core::Url("http://localhost:8000/echotest"));

  testSocket.Open();

  // seed test data for the operations.
  std::vector<std::vector<uint8_t>> testData(testDataLength);
  std::vector<std::vector<uint8_t>> receivedData(testDataLength);
  std::atomic_size_t iterationCount(0);

  // Spin up threadCount threads and hammer the echo server for 10 seconds.
  std::vector<std::thread> threads;
  std::atomic_int32_t cancellationExceptions{0};
  std::atomic_int32_t exceptions{0};
  for (size_t threadIndex = 0; threadIndex < threadCount; threadIndex += 1)
  {
    threads.push_back(std::thread([&]() {
      std::chrono::time_point<std::chrono::system_clock> startTime
          = std::chrono::system_clock::now();
      // Set the context to expire *after* the test is supposed to finish.
      Azure::Core::Context context = Azure::Core::Context::ApplicationContext.WithDeadline(
          Azure::DateTime{startTime} + testDuration + 10s);
      size_t iteration = 0;
      try
      {
        do
        {
          iteration = iterationCount++;
          std::vector<uint8_t> sendData = GenerateRandomBytes(iteration, testDataSize);
          {
            if (iteration < testData.size())
            {
              if (testData[iteration].size() != 0)
              {
                GTEST_LOG_(ERROR) << "Overwriting send frame at offset " << iteration << std::endl;
              }
              EXPECT_EQ(0, testData[iteration].size());
              testData[iteration] = sendData;
            }
          }

          testSocket.SendFrame(sendData, true /*, context*/);
          auto response = testSocket.ReceiveFrame(context);
          EXPECT_EQ(WebSocketFrameType::BinaryFrameReceived, response->FrameType);
          auto binaryResult = response->AsBinaryFrame();

          // Make sure we get back the data we sent in the echo request.
          if (binaryResult->Data.size() == 0)
          {
            GTEST_LOG_(ERROR) << "Received empty frame at offset " << iteration << std::endl;
          }
          EXPECT_EQ(sendData.size(), binaryResult->Data.size());
          {
            // There is no ordering expectation on the results, so we just remember the data
            // as it comes in. We'll make sure we received everything later on.
            if (iteration < receivedData.size())
            {
              if (receivedData[iteration].size() != 0)
              {
                GTEST_LOG_(ERROR) << "Overwriting receive frame at offset " << iteration
                                  << std::endl;
              }

              EXPECT_EQ(0, receivedData[iteration].size());
              receivedData[iteration] = binaryResult->Data;
            }
          }
        } while (std::chrono::system_clock::now() - startTime < testDuration);
      }
      catch (Azure::Core::OperationCancelledException& ex)
      {
        GTEST_LOG_(ERROR) << "Cancelled Exception: " << ex.what() << " at index " << iteration
                          << " Current Thread: " << std::this_thread::get_id() << std::endl;
        cancellationExceptions++;
      }
      catch (std::exception const& ex)
      {
        GTEST_LOG_(ERROR) << "Exception: " << ex.what() << std::endl;
        exceptions++;
      }
    }));
  }
  //  std::this_thread::sleep_for(10s);

  // Wait for all the threads to exit.
  for (auto& thread : threads)
  {
    thread.join();
  }

  // We no longer need to worry about synchronization since all the worker threads are done.
  GTEST_LOG_(INFO) << "Total server requests: " << iterationCount.load() << std::endl;
  GTEST_LOG_(INFO) << "Estimated " << std::dec << testData.size() << " iterations (0x" << std::hex
                   << testData.size() << ")" << std::endl;
  EXPECT_GE(testDataLength, iterationCount.load());

  auto statistics = testSocket.GetStatistics();
  GTEST_LOG_(INFO) << "Total bytes sent: " << std::dec << statistics.BytesSent;
  GTEST_LOG_(INFO) << "Total bytes received: " << std::dec << statistics.BytesReceived;
  GTEST_LOG_(INFO) << "Ping Frames received: " << std::dec << statistics.PingFramesReceived;
  GTEST_LOG_(INFO) << "Ping Frames sent: " << std::dec << statistics.PingFramesSent;
  GTEST_LOG_(INFO) << "Pong Frames received: " << std::dec << statistics.PongFramesReceived;
  GTEST_LOG_(INFO) << "Pong Frames sent: " << std::dec << statistics.PongFramesSent;
  GTEST_LOG_(INFO) << "Binary frames sent: " << std::dec << statistics.BinaryFramesSent;
  GTEST_LOG_(INFO) << "Binary frames received: " << std::dec << statistics.BinaryFramesReceived;
  GTEST_LOG_(INFO) << "Total frames lost: " << std::dec << statistics.FramesDropped;
  GTEST_LOG_(INFO) << "Transport Reads " << std::dec << statistics.TransportReads;
  GTEST_LOG_(INFO) << "Transport Bytes Read " << std::dec << statistics.TransportReadBytes;

  // Close the socket gracefully.
  testSocket.Close();

  EXPECT_EQ(iterationCount.load(), statistics.BinaryFramesSent);
  EXPECT_EQ(iterationCount.load(), statistics.BinaryFramesReceived);

  // Resize the test data to the number of actual iterations.
  testData.resize(iterationCount.load());
  receivedData.resize(iterationCount.load());

  // If we've processed every iteration, let's make sure that we received everything we sent.
  // If we dropped some results, then we can't check to ensure that we have received everything
  // because we can't account for everything sent.
  std::multiset<std::string> testDataStrings;
  std::multiset<std::string> receivedDataStrings;
  for (auto const& data : testData)
  {
    testDataStrings.emplace(ToHexString(data));
  }
  for (auto const& data : receivedData)
  {
    receivedDataStrings.emplace(ToHexString(data));
  }

  //  EXPECT_EQ(testDataStrings, receivedDataStrings);
  for (auto const& data : testDataStrings)
  {
    if (receivedDataStrings.count(data) != testDataStrings.count(data))
    {
      GTEST_LOG_(INFO) << "Missing data. TestDataCount: " << testDataStrings.count(data)
                       << " ReceivedDataCount: " << receivedDataStrings.count(data)
                       << " Missing Data: " << data << std::endl;
    }
    EXPECT_NE(receivedDataStrings.end(), receivedDataStrings.find(data));
  }
  for (auto const& data : receivedDataStrings)
  {
    if (testDataStrings.count(data) != receivedDataStrings.count(data))
    {
      GTEST_LOG_(INFO) << "Extra data. TestDataCount: " << testDataStrings.count(data)
                       << " ReceivedDataCount: " << receivedDataStrings.count(data)
                       << " Missing Data: " << data << std::endl;
    }

    EXPECT_NE(testDataStrings.end(), testDataStrings.find(data));
  }

  // We shouldn't have seen any exceptions during the run.
  EXPECT_EQ(0, exceptions.load());
  EXPECT_EQ(0, cancellationExceptions.load());
}

// Does not work because curl rejects the wss: scheme.
class LibWebSocketIncrementProtocol {
  WebSocketOptions m_options{{"dumb-increment-protocol"}};
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
    if (work->FrameType == WebSocketFrameType::TextFrameReceived)
    {
      auto frame = work->AsTextFrame();
      return std::atoi(frame->Text.c_str());
    }
    if (work->FrameType == WebSocketFrameType::BinaryFrameReceived)
    {
      auto frame = work->AsBinaryFrame();
      throw std::runtime_error("Not implemented");
    }
    else if (work->FrameType == WebSocketFrameType::PeerClosedReceived)
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
      if (work->FrameType == WebSocketFrameType::PeerClosedReceived)
      {
        auto peerClose = work->AsPeerCloseFrame();
        GTEST_LOG_(INFO) << "Peer closed. Remote Code: " << std::dec << peerClose->RemoteStatusCode
                         << " (0x" << std::hex << peerClose->RemoteStatusCode << ")" << std::endl;
        if (!peerClose->RemoteCloseReason.empty())
        {
          GTEST_LOG_(INFO) << " Peer Closed Data: " << peerClose->RemoteCloseReason;
        }
        GTEST_LOG_(INFO) << std::endl;
        return;
      }
      else if (work->FrameType == WebSocketFrameType::TextFrameReceived)
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
    std::string returnValue;
    std::shared_ptr<WebSocketFrame> lwsStatus;
    do
    {

      lwsStatus = serverSocket.ReceiveFrame();
      EXPECT_EQ(WebSocketFrameType::TextFrameReceived, lwsStatus->FrameType);
      if (lwsStatus->FrameType == WebSocketFrameType::TextFrameReceived)
      {
        auto textFrame = lwsStatus->AsTextFrame();
        returnValue.insert(returnValue.end(), textFrame->Text.begin(), textFrame->Text.end());
      }
    } while (!lwsStatus->IsFinalFrame);
    serverSocket.Close();
    return returnValue;
  }
};

TEST_F(WebSocketTests, LibWebSocketOrgLwsStatus)
{
  {
    LibWebSocketStatus lwsStatus;
    auto serverStatus = lwsStatus.GetLWSStatus();
    GTEST_LOG_(INFO) << "Server status: " << serverStatus << std::endl;

    Azure::Core::Json::_internal::json status;
    EXPECT_NO_THROW(status = Azure::Core::Json::_internal::json::parse(serverStatus));
    EXPECT_TRUE(status["conns"].is_array());
    auto& connections = status["conns"].get_ref<std::vector<Azure::Core::Json::_internal::json>&>();
    bool foundOurConnection = false;

    // Scan through the list of connections to find a connection from the websockettest.
    for (auto& connection : connections)
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
}
TEST_F(WebSocketTests, LibWebSocketOrgIncrement)
{
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
#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
TEST_F(WebSocketTests, CurlTransportCoverage)
{
  {

    Azure::Core::Http::CurlTransportOptions transportOptions;
    transportOptions.HttpKeepAlive = false;
    auto transport
        = std::make_shared<Azure::Core::Http::WebSockets::CurlWebSocketTransport>(transportOptions);

    EXPECT_THROW(transport->NativeCloseSocket(1001, {}, {}), std::runtime_error);
    EXPECT_THROW(transport->NativeGetCloseSocketInformation({}), std::runtime_error);
    EXPECT_THROW(
        transport->NativeSendFrame(WebSocketTransport::NativeWebSocketFrameType::FrameTypeBinary, {}, {}),
        std::runtime_error);
    EXPECT_THROW(transport->NativeReceiveFrame({}), std::runtime_error);
  }
}

#endif