// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/connection.hpp"

#include <azure/core/amqp/internal/common/async_operation_queue.hpp>
#include <azure/core/amqp/internal/network/socket_listener.hpp>
#include <azure/core/amqp/internal/network/socket_transport.hpp>
#include <azure/core/amqp/internal/network/tls_transport.hpp>
#include <azure/core/platform.hpp>

#include <random>
#include <utility>

#include <gtest/gtest.h>

namespace Azure { namespace Core { namespace Amqp { namespace Tests {

  extern uint16_t FindAvailableSocket();

  using namespace Azure::Core::Amqp::Network::_internal;
  using namespace Azure::Core::Amqp::Network::_detail;
  using namespace Azure::Core::Amqp::Common::_internal;

  class TestTlsTransport : public testing::Test {
  protected:
    void SetUp() override {}
    void TearDown() override {}
  };

  std::string StringFromSendResult(TransportSendStatus ts)
  {
    switch (ts)
    {
      case TransportSendStatus::Unknown:
        return "Unknown";
      case TransportSendStatus::Ok:
        return "Ok";
      case TransportSendStatus::Error:
        return "Error";
      case TransportSendStatus::Cancelled:
        return "Cancelled";
      case TransportSendStatus::Invalid:
        return "**INVALID**";
    }
    throw std::logic_error("??? Unknown Transport Send Result...");
  }

  std::string StringFromOpenResult(TransportOpenStatus to)
  {
    switch (to)
    {
      case TransportOpenStatus::Ok:
        return "Ok";
      case TransportOpenStatus::Error:
        return "Error";
      case TransportOpenStatus::Cancelled:
        return "Cancelled";
      case TransportOpenStatus::Invalid:
        return "**INVALID**";
    }
    throw std::logic_error("??? Unknown Transport Open Result...");
  }

  TEST_F(TestTlsTransport, SimpleSend)
  {
    {
      class TestTransportEvents : public TransportEvents {
        AsyncOperationQueue<size_t, std::unique_ptr<uint8_t[]>> receiveBytesQueue;
        AsyncOperationQueue<bool> errorQueue;
        void OnBytesReceived(Transport const&, uint8_t const* bytes, size_t size) override
        {
          GTEST_LOG_(INFO) << "On bytes received: " << size;
          std::unique_ptr<uint8_t[]> val(new uint8_t[size]);
          memcpy(val.get(), bytes, size);
          receiveBytesQueue.CompleteOperation(size, std::move(val));
        }
        void OnIOError() override
        {
          GTEST_LOG_(INFO) << "On I/O Error";
          errorQueue.CompleteOperation(true);
        }

      public:
        std::tuple<size_t, std::unique_ptr<uint8_t[]>> WaitForReceive(
            Transport const& transport,
            Azure::Core::Context const& context)
        {
          auto result = receiveBytesQueue.WaitForPolledResult(context, transport);
          return std::make_tuple(std::get<0>(*result), std::move(std::get<1>(*result)));
        }
      };
      TestTransportEvents events;
      auto transport = TlsTransportFactory::Create("www.microsoft.com", 443, &events);

      ASSERT_EQ(TransportOpenStatus::Ok, transport.Open());

      unsigned char val[] = R"(GET / HTTP/1.1
Host: www.microsoft.com
User-Agent: AMQP Tests 0.0.1
Accept: */*

)";

      GTEST_LOG_(INFO) << "Before send" << std::endl;

      AsyncOperationQueue<TransportSendStatus> sendOperation;
      EXPECT_TRUE(transport.Send(val, sizeof(val), [&sendOperation](TransportSendStatus result) {
        std::cout << "Send complete" << StringFromSendResult(result);
        sendOperation.CompleteOperation(result);
      }));
      GTEST_LOG_(INFO) << "Wait for send" << std::endl;

      auto sendResult{sendOperation.WaitForPolledResult({}, transport)};
      EXPECT_EQ(std::get<0>(*sendResult), TransportSendStatus::Ok);

      // Wait until we receive data from the www.microsoft.com server.
      GTEST_LOG_(INFO) << "Wait for data from server." << std::endl;
      auto receiveResult = events.WaitForReceive(transport, {});

      GTEST_LOG_(INFO) << "Received data from microsoft.com server: " << std::get<0>(receiveResult)
                       << std::endl;

      EXPECT_NO_THROW(transport.Close());
    }
  }

  class TestSocketTransport : public testing::Test {
  protected:
    void SetUp() override {}
    void TearDown() override {}
  };

#if !defined(AZURE_PLATFORM_MAC)
  TEST_F(TestSocketTransport, SimpleCreate)
  {
    {
      Transport transport{
          SocketTransportFactory::Create("localhost", Azure::Core::Amqp::_internal::AmqpPort)};
    }
    {
      Transport transport1{
          SocketTransportFactory::Create("localhost", Azure::Core::Amqp::_internal::AmqpPort)};
      Transport transport2{SocketTransportFactory::Create("localhost", 5673)};
    }
  }

  TEST_F(TestSocketTransport, SimpleOpen)
  {
    // Wait until we receive data from the www.microsoft.com server, with a 10 second timeout.
    Azure::Core::Context completionContext
        = Azure::Core::Context{std::chrono::system_clock::now() + std::chrono::seconds(10)};
    {
      Transport transport{SocketTransportFactory::Create("www.microsoft.com", 80)};

      ASSERT_EQ(TransportOpenStatus::Ok, transport.Open(completionContext));
      EXPECT_NO_THROW(transport.Close());
    }

    {
      Transport transport{SocketTransportFactory::Create("www.microsoft.com", 80)};
      ASSERT_EQ(TransportOpenStatus::Ok, transport.Open(completionContext));
      transport.Close();
    }
    {
      Transport transport{SocketTransportFactory::Create("www.microsoft.com", 80)};
      EXPECT_ANY_THROW(transport.Close());
    }
    {
      Transport transport{SocketTransportFactory::Create("www.microsoft.com", 80)};
      ASSERT_EQ(TransportOpenStatus::Ok, transport.Open(completionContext));
      EXPECT_ANY_THROW(transport.Open());
    }
  }

  TEST_F(TestSocketTransport, SimpleSend)
  {
    {
      class TestTransportEvents : public TransportEvents {
        AsyncOperationQueue<size_t, std::unique_ptr<uint8_t[]>> receiveBytesQueue;
        AsyncOperationQueue<bool> errorQueue;
        void OnBytesReceived(Transport const&, uint8_t const* bytes, size_t size) override
        {
          GTEST_LOG_(INFO) << "On bytes received: " << size;
          std::unique_ptr<uint8_t[]> val(new uint8_t[size]);
          memcpy(val.get(), bytes, size);
          receiveBytesQueue.CompleteOperation(size, std::move(val));
        }
        void OnIOError() override
        {
          GTEST_LOG_(INFO) << "On I/O Error";
          errorQueue.CompleteOperation(true);
        }

      public:
        std::tuple<size_t, std::unique_ptr<uint8_t[]>> WaitForReceive(
            Transport const& transport,
            Azure::Core::Context const& context)
        {
          auto result = receiveBytesQueue.WaitForPolledResult(context, transport);
          return std::make_tuple(std::get<0>(*result), std::move(std::get<1>(*result)));
        }
      };
      TestTransportEvents events;
      Transport transport{SocketTransportFactory::Create("www.microsoft.com", 80, &events)};

      // Wait until we receive data from the www.microsoft.com server, with a 10 second timeout.
      Azure::Core::Context completionContext
          = Azure::Core::Context{std::chrono::system_clock::now() + std::chrono::seconds(10)};
      ASSERT_EQ(TransportOpenStatus::Ok, transport.Open(completionContext));

      unsigned char val[] = R"(GET / HTTP/1.1
Host: www.microsoft.com
User-Agent: AMQP Tests 0.0.1
Accept: */*

)";

      GTEST_LOG_(INFO) << "Before send" << std::endl;

      AsyncOperationQueue<TransportSendStatus> sendOperation;
      EXPECT_TRUE(transport.Send(val, sizeof(val), [&sendOperation](TransportSendStatus result) {
        std::cout << "Send complete" << StringFromSendResult(result);
        sendOperation.CompleteOperation(result);
      }));
      GTEST_LOG_(INFO) << "Wait for send" << std::endl;

      auto sendResult{sendOperation.WaitForPolledResult({}, transport)};
      EXPECT_EQ(std::get<0>(*sendResult), TransportSendStatus::Ok);

      // Wait until we receive data from the www.microsoft.com server.
      GTEST_LOG_(INFO) << "Wait for data from server." << std::endl;
      auto receiveResult = events.WaitForReceive(transport, {});

      GTEST_LOG_(INFO) << "Received data from microsoft.com server: " << std::get<0>(receiveResult)
                       << std::endl;

      AsyncOperationQueue<bool> closeResult;
      EXPECT_NO_THROW(transport.Close());
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
#if 0
    {
      SocketListener listener(8008, nullptr);

      EXPECT_ANY_THROW(listener.Stop());
    }
#endif
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

      std::shared_ptr<Transport> GetListenerTransport(
          SocketListener const& listener,
          Azure::Core::Context const& context)
      {
        auto result = m_listenerTransportQueue.WaitForPolledResult(context, listener);
        return std::move(std::get<0>(*result));
      }
      std::vector<uint8_t> WaitForReceive(
          Transport const& transport,
          Azure::Core::Context const& context)
      {
        auto result = receiveBytesQueue.WaitForPolledResult(context, transport);
        if (result)
        {
          return std::get<0>(*result);
        }
        throw Azure::Core::OperationCancelledException("Wait for receive cancelled");
      }

    private:
      AsyncOperationQueue<std::shared_ptr<Transport>> m_listenerTransportQueue;
      AsyncOperationQueue<std::vector<uint8_t>> receiveBytesQueue;
      AsyncOperationQueue<bool> errorQueue;
      virtual void OnSocketAccepted(std::shared_ptr<Transport> newTransport) override
      {
        GTEST_LOG_(INFO) << "Listener started, new connection.";
        newTransport->SetEventHandler(this);
        newTransport->Open();
        m_listenerTransportQueue.CompleteOperation(newTransport);
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
        transport.Send(const_cast<uint8_t*>(bytes), size, [](TransportSendStatus sendResult) {
          GTEST_LOG_(INFO) << "OnListener Send Bytes Complete..."
                           << StringFromSendResult(sendResult);
        });
      }
      void OnIOError() override
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
      AsyncOperationQueue<std::vector<uint8_t>> receiveBytesQueue;
      AsyncOperationQueue<bool> errorQueue;
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
      void OnIOError() override
      {
        GTEST_LOG_(INFO) << "On I/O Error";
        errorQueue.CompleteOperation(true);
      }

    public:
      std::vector<uint8_t> WaitForReceive(
          Transport const& transport,
          Azure::Core::Context const& context)
      {
        auto result = receiveBytesQueue.WaitForPolledResult(context, transport);
        return std::get<0>(*result);
      }
    };
    SendingEvents sendingEvents;
    Transport sender{SocketTransportFactory::Create("localhost", testPort, &sendingEvents)};

    ASSERT_EQ(TransportOpenStatus::Ok, sender.Open());

    // Note: Keep this string under 64 bytes in length because the default socket I/O buffer size
    // is 64 bytes and that helps ensure that this will be handled in a single OnReceiveBytes
    // call.
    unsigned char val[] = R"(GET / HTTP/1.1
Host: www.microsoft.com)";

    // Synchronously send the data to the listener.
    {
      AsyncOperationQueue<TransportSendStatus> sendOperation;

      sender.Send(val, sizeof(val), [&sendOperation](TransportSendStatus result) {
        GTEST_LOG_(INFO) << "Sender send complete " << StringFromSendResult(result);
        sendOperation.CompleteOperation(result);
      });
      auto sendResult{sendOperation.WaitForPolledResult({}, sender)};
      EXPECT_EQ(std::get<0>(*sendResult), TransportSendStatus::Ok);
    }

    GTEST_LOG_(INFO) << "Wait for listener to receive the bytes we just sent.";
    auto listenerTransport = events.GetListenerTransport(listener, {});

    GTEST_LOG_(INFO) << "Wait for received event.";
    events.WaitForReceive(
        *listenerTransport,
        Azure::Core::Context{std::chrono::system_clock::now() + std::chrono::seconds(10)});

    GTEST_LOG_(INFO) << "Listener received the bytes we just sent, now wait until the sender "
                        "received those bytes back.";

    auto receivedData = sendingEvents.WaitForReceive(sender, {});

    EXPECT_EQ(sizeof(val), receivedData.size());
    EXPECT_EQ(0, memcmp(val, receivedData.data(), receivedData.size()));
    listenerTransport->Close();
    listener.Stop();
  }
#endif // !defined(AZURE_PLATFORM_MAC)
}}}} // namespace Azure::Core::Amqp::Tests
