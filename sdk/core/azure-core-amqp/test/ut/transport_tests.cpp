// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/common/async_operation_queue.hpp"
#include "azure/core/amqp/network/socket_listener.hpp"
#include "azure/core/amqp/network/socket_transport.hpp"
#include "azure/core/amqp/network/tls_transport.hpp"
#include <gtest/gtest.h>
#include <random>
#include <utility>

extern uint16_t FindAvailableSocket();

using namespace Azure::Core::_internal::Amqp::Network;
using namespace Azure::Core::Amqp::Common::_internal;

class TestTlsTransport : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

std::string StringFromSendResult(TransportSendResult ts)
{
  switch (ts)
  {
    case TransportSendResult::Unknown:
      return "Unknown";
    case TransportSendResult::Ok:
      return "Ok";
    case TransportSendResult::Error:
      return "Error";
    case TransportSendResult::Cancelled:
      return "Cancelled";
    case TransportSendResult::Invalid:
      return "**INVALID**";
  }
  throw std::logic_error("??? Unknown Transport Send Result...");
}

std::string StringFromOpenResult(TransportOpenResult to)
{
  switch (to)
  {
    case TransportOpenResult::Ok:
      return "Ok";
    case TransportOpenResult::Error:
      return "Error";
    case TransportOpenResult::Cancelled:
      return "Cancelled";
    case TransportOpenResult::Invalid:
      return "**INVALID**";
  }
  throw std::logic_error("??? Unknown Transport Open Result...");
}

TEST_F(TestTlsTransport, SimpleCreate)
{
  {
    TlsTransport transport;
  }
}

TEST_F(TestTlsTransport, SimpleSend)
{
  {
    class TestTransportEvents : public TransportEvents {
      AsyncOperationQueue<TransportOpenResult> openResultQueue;
      AsyncOperationQueue<size_t, std::unique_ptr<uint8_t>> receiveBytesQueue;
      AsyncOperationQueue<bool> errorQueue;
      void OnOpenComplete(TransportOpenResult result) override
      {
        GTEST_LOG_(INFO) << "On open:" << static_cast<int>(result);

        openResultQueue.CompleteOperation(result);
      }
      void OnBytesReceived(Transport const&, uint8_t const* bytes, size_t size) override
      {
        GTEST_LOG_(INFO) << "On bytes received: " << size;
        std::unique_ptr<uint8_t> val(new uint8_t[size]);
        memcpy(val.get(), bytes, size);
        receiveBytesQueue.CompleteOperation(size, std::move(val));
      }
      void OnIoError() override
      {
        GTEST_LOG_(INFO) << "On I/O Error";
        errorQueue.CompleteOperation(true);
      }

    public:
      TransportOpenResult WaitForOpen(Transport const& transport, Azure::Core::Context context)
      {
        auto result = openResultQueue.WaitForPolledResult(context, transport);
        return std::get<0>(*result);
      }
      std::tuple<size_t, std::unique_ptr<uint8_t>> WaitForReceive(
          Transport const& transport,
          Azure::Core::Context context)
      {
        auto result = receiveBytesQueue.WaitForPolledResult(context, transport);
        return std::make_tuple(std::get<0>(*result), std::move(std::get<1>(*result)));
      }
    };
    TestTransportEvents events;
    TlsTransport transport("www.microsoft.com", 443, &events);

    EXPECT_TRUE(transport.Open());

    auto openResult = events.WaitForOpen(transport, {});
    EXPECT_EQ(openResult, TransportOpenResult::Ok);

    unsigned char val[] = R"(GET / HTTP/1.1
Host: www.microsoft.com
User-Agent: AMQP Tests 0.0.1
Accept: */*

)";

    GTEST_LOG_(INFO) << "Before send" << std::endl;

    AsyncOperationQueue<TransportSendResult> sendOperation;
    EXPECT_TRUE(transport.Send(val, sizeof(val), [&sendOperation](TransportSendResult result) {
      std::cout << "Send complete" << StringFromSendResult(result);
      sendOperation.CompleteOperation(result);
    }));
    GTEST_LOG_(INFO) << "Wait for send" << std::endl;

    auto sendResult{sendOperation.WaitForPolledResult({}, transport)};
    EXPECT_EQ(std::get<0>(*sendResult), TransportSendResult::Ok);

    // Wait until we receive data from the www.microsoft.com server.
    GTEST_LOG_(INFO) << "Wait for data from server." << std::endl;
    auto receiveResult = events.WaitForReceive(transport, {});

    GTEST_LOG_(INFO) << "Received data from microsoft.com server: " << std::get<0>(receiveResult)
                     << std::endl;

    AsyncOperationQueue<bool> closeResult;
    transport.Close([&closeResult] { closeResult.CompleteOperation(true); });
    auto closeComplete = closeResult.WaitForPolledResult({}, transport);
    EXPECT_EQ(true, std::get<0>(*closeComplete));
  }
}

class TestSocketTransport : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestSocketTransport, SimpleCreate)
{
  {
    SocketTransport transport("localhost", 5672);
  }
  {
    SocketTransport transport1("localhost", 5672);
    SocketTransport transport2("localhost", 5673);
  }
}

TEST_F(TestSocketTransport, SimpleOpen)
{
  {

    SocketTransport transport("www.microsoft.com", 80);

    EXPECT_TRUE(transport.Open());
    EXPECT_TRUE(transport.Close([]() {}));
  }

  {
    SocketTransport transport("www.microsoft.com", 80);
    EXPECT_TRUE(transport.Open());
    transport.Close(nullptr);
  }
  {
    SocketTransport transport("www.microsoft.com", 80);
    EXPECT_ANY_THROW(transport.Close(nullptr));
  }
  {
    SocketTransport transport("www.microsoft.com", 80);
    transport.Open();
    EXPECT_ANY_THROW(transport.Open());
  }
}

TEST_F(TestSocketTransport, SimpleSend)
{
  {
    class TestTransportEvents : public TransportEvents {
      AsyncOperationQueue<TransportOpenResult> openResultQueue;
      AsyncOperationQueue<size_t, std::unique_ptr<uint8_t>> receiveBytesQueue;
      AsyncOperationQueue<bool> errorQueue;
      void OnOpenComplete(TransportOpenResult result) override
      {
        GTEST_LOG_(INFO) << "On open:" << static_cast<int>(result);

        openResultQueue.CompleteOperation(result);
      }
      void OnBytesReceived(Transport const&, uint8_t const* bytes, size_t size) override
      {
        GTEST_LOG_(INFO) << "On bytes received: " << size;
        std::unique_ptr<uint8_t> val(new uint8_t[size]);
        memcpy(val.get(), bytes, size);
        receiveBytesQueue.CompleteOperation(size, std::move(val));
      }
      void OnIoError() override
      {
        GTEST_LOG_(INFO) << "On I/O Error";
        errorQueue.CompleteOperation(true);
      }

    public:
      TransportOpenResult WaitForOpen(Transport const& transport, Azure::Core::Context context)
      {
        auto result = openResultQueue.WaitForPolledResult(context, transport);
        return std::get<0>(*result);
      }
      std::tuple<size_t, std::unique_ptr<uint8_t>> WaitForReceive(
          Transport const& transport,
          Azure::Core::Context context)
      {
        auto result = receiveBytesQueue.WaitForPolledResult(context, transport);
        return std::make_tuple(std::get<0>(*result), std::move(std::get<1>(*result)));
      }
    };
    TestTransportEvents events;
    SocketTransport transport("www.microsoft.com", 80, &events);

    EXPECT_TRUE(transport.Open());

    auto openResult = events.WaitForOpen(transport, {});
    EXPECT_EQ(openResult, TransportOpenResult::Ok);

    unsigned char val[] = R"(GET / HTTP/1.1
Host: www.microsoft.com
User-Agent: AMQP Tests 0.0.1
Accept: */*

)";

    GTEST_LOG_(INFO) << "Before send" << std::endl;

    AsyncOperationQueue<TransportSendResult> sendOperation;
    EXPECT_TRUE(transport.Send(val, sizeof(val), [&sendOperation](TransportSendResult result) {
      std::cout << "Send complete" << StringFromSendResult(result);
      sendOperation.CompleteOperation(result);
    }));
    GTEST_LOG_(INFO) << "Wait for send" << std::endl;

    auto sendResult{sendOperation.WaitForPolledResult({}, transport)};
    EXPECT_EQ(std::get<0>(*sendResult), TransportSendResult::Ok);

    // Wait until we receive data from the www.microsoft.com server.
    GTEST_LOG_(INFO) << "Wait for data from server." << std::endl;
    auto receiveResult = events.WaitForReceive(transport, {});

    GTEST_LOG_(INFO) << "Received data from microsoft.com server: " << std::get<0>(receiveResult)
                     << std::endl;

    AsyncOperationQueue<bool> closeResult;
    transport.Close([&closeResult] { closeResult.CompleteOperation(true); });
    auto closeComplete = closeResult.WaitForPolledResult({}, transport);
    EXPECT_EQ(true, std::get<0>(*closeComplete));
  }
}

TEST_F(TestSocketTransport, SimpleListener)
{
  {
    SocketListener listener(8008, nullptr);

    listener.Start();

    listener.Stop();
  }

  {
    SocketListener listener(8008, nullptr);

    listener.Start();
    EXPECT_ANY_THROW(listener.Start());
  }

  {
    SocketListener listener1(8008, nullptr);

    listener1.Start();

    SocketListener listener2(8008, nullptr);
    EXPECT_ANY_THROW(listener2.Start());
  }
}

TEST_F(TestSocketTransport, SimpleListenerEcho)
{

  class TestListenerEvents : public SocketListenerEvents, public TransportEvents {
  public:
    TestListenerEvents() {}

    std::unique_ptr<Transport> GetListenerTransport(
        SocketListener const& listener,
        Azure::Core::Context context)
    {
      auto result = m_listenerTransportQueue.WaitForPolledResult(context, listener);
      return std::move(std::get<0>(*result));
    }
    TransportOpenResult WaitForOpen(SocketListener const& listener, Azure::Core::Context context)
    {
      auto result = openResultQueue.WaitForPolledResult(context, listener);
      return std::get<0>(*result);
    }
    std::vector<uint8_t> WaitForReceive(Transport const& transport, Azure::Core::Context context)
    {
      auto result = receiveBytesQueue.WaitForPolledResult(context, transport);
      return std::get<0>(*result);
    }

  private:
    AsyncOperationQueue<std::unique_ptr<Transport>> m_listenerTransportQueue;
    AsyncOperationQueue<TransportOpenResult> openResultQueue;
    AsyncOperationQueue<std::vector<uint8_t>> receiveBytesQueue;
    AsyncOperationQueue<bool> errorQueue;
    virtual void OnSocketAccepted(XIO_INSTANCE_TAG* newTransport) override
    {
      GTEST_LOG_(INFO) << "Listener started, new connection.";
      auto listenerTransport = std::make_unique<Transport>(newTransport, this);
      listenerTransport->Open();
      m_listenerTransportQueue.CompleteOperation(std::move(listenerTransport));
    }
    void OnOpenComplete(TransportOpenResult result) override
    {
      GTEST_LOG_(INFO) << "On open:" << static_cast<int>(result);
      openResultQueue.CompleteOperation(result);
    }
    void OnBytesReceived(Transport const& transport, uint8_t const* bytes, size_t size) override
    {
      GTEST_LOG_(INFO) << "On Listener bytes received: " << size;
      std::vector<uint8_t> echoedBytes;
      for (size_t i = 0; i < size; i += 1)
      {
        echoedBytes.push_back(bytes[i]);
      }

      receiveBytesQueue.CompleteOperation(echoedBytes);

      // Echo back the data received.
      transport.Send(const_cast<uint8_t*>(bytes), size, [](TransportSendResult sendResult) {
        GTEST_LOG_(INFO) << "OnListener Send Bytes Complete..." << StringFromSendResult(sendResult);
      });
    }
    void OnIoError() override
    {
      GTEST_LOG_(INFO) << "On I/O Error";
      errorQueue.CompleteOperation(true);
    }
  };

  TestListenerEvents events;
  uint16_t testPort = FindAvailableSocket();

  GTEST_LOG_(INFO) << "Test listener using port: " << testPort;
  SocketListener listener(testPort, &events);
  EXPECT_NO_THROW(listener.Start());

  class SendingEvents : public TransportEvents {
    AsyncOperationQueue<TransportOpenResult> openResultQueue;
    AsyncOperationQueue<std::vector<uint8_t>> receiveBytesQueue;
    AsyncOperationQueue<bool> errorQueue;
    void OnOpenComplete(TransportOpenResult result) override
    {
      GTEST_LOG_(INFO) << "On open:" << static_cast<int>(result);

      openResultQueue.CompleteOperation(result);
    }
    void OnBytesReceived(Transport const&, uint8_t const* bytes, size_t size) override
    {
      GTEST_LOG_(INFO) << "On bytes received: " << size;
      std::vector<uint8_t> echoedBytes;
      for (size_t i = 0; i < size; i += 1)
      {
        echoedBytes.push_back(bytes[i]);
      }

      receiveBytesQueue.CompleteOperation(echoedBytes);
    }
    void OnIoError() override
    {
      GTEST_LOG_(INFO) << "On I/O Error";
      errorQueue.CompleteOperation(true);
    }

  public:
    TransportOpenResult WaitForOpen(Transport const& transport, Azure::Core::Context context)
    {
      auto result = openResultQueue.WaitForPolledResult(context, transport);
      return std::get<0>(*result);
    }
    std::vector<uint8_t> WaitForReceive(Transport const& transport, Azure::Core::Context context)
    {
      auto result = receiveBytesQueue.WaitForPolledResult(context, transport);
      return std::get<0>(*result);
    }
  };
  SendingEvents sendingEvents;
  SocketTransport sender("localhost", testPort, &sendingEvents);

  EXPECT_TRUE(sender.Open());

  // Note: Keep this string under 64 bytes in length because the default socket I/O buffer size
  // is 64 bytes and that helps ensure that this will be handled in a single OnReceiveBytes
  // call.
  unsigned char val[] = R"(GET / HTTP/1.1
Host: www.microsoft.com)";

  // Synchronously send the data to the listener.
  {
    AsyncOperationQueue<TransportSendResult> sendOperation;

    sender.Send(val, sizeof(val), [&sendOperation](TransportSendResult result) {
      GTEST_LOG_(INFO) << "Sender send complete " << StringFromSendResult(result);
      sendOperation.CompleteOperation(result);
    });
    auto sendResult{sendOperation.WaitForPolledResult({}, sender)};
    EXPECT_EQ(std::get<0>(*sendResult), TransportSendResult::Ok);
  }

  GTEST_LOG_(INFO) << "Wait for listener to receive the bytes we just sent.";
  auto listenerTransport = events.GetListenerTransport(listener, {});

  GTEST_LOG_(INFO) << "Wait for received event.";
  events.WaitForReceive(*listenerTransport, {});

  GTEST_LOG_(INFO) << "Listener received the bytes we just sent, now wait until the sender "
                      "received those bytes back.";

  auto receivedData = sendingEvents.WaitForReceive(sender, {});

  EXPECT_EQ(sizeof(val), receivedData.size());
  EXPECT_EQ(0, memcmp(val, receivedData.data(), receivedData.size()));
  listenerTransport->Close(nullptr);
  listener.Stop();
}
