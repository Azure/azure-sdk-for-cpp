// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#if ENABLE_UAMQP
#include "private/eventhubs_utilities.hpp"
#include "../../../../core/azure-core-amqp/src/impl/uamqp/amqp/private/session_impl.hpp"

#include <azure/messaging/eventhubs/producer_client.hpp>

#include "../../../../core/azure-core-amqp/test/ut/mock_amqp_server.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/platform.hpp>
#include <atomic>
#include <cstdlib>
#if defined(AZ_PLATFORM_WINDOWS)
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
#include <gtest/gtest.h>

namespace {
std::uint16_t AvailablePort()
{
  // Initialize the platform before socket APIs (required on Windows).
  auto state = Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance();
  (void)state;
  auto socketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (bind(socketHandle, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0)
  {
    throw std::runtime_error("Could not reserve a test port");
  }
#if defined(AZ_PLATFORM_WINDOWS)
  int size = sizeof(address);
#else
  socklen_t size = sizeof(address);
#endif
  auto result = getsockname(socketHandle, reinterpret_cast<sockaddr*>(&address), &size);
#if defined(AZ_PLATFORM_WINDOWS)
  closesocket(socketHandle);
#else
  close(socketHandle);
#endif
  if (result != 0)
  {
    throw std::runtime_error("Could not read the test port");
  }
  return ntohs(address.sin_port);
}
} // namespace

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {
  namespace {
    namespace Amqp = Azure::Core::Amqp;
    using _detail::EventHubsPropertiesClient;

    Amqp::_internal::ManagementClient FailCreation(
        Amqp::_internal::Session const&,
        std::string const&,
        Amqp::_internal::ManagementClientOptions const&)
    {
      throw std::runtime_error("injected management creation failure");
    }

    int FailEnd(SESSION_HANDLE, char const*, char const*) { return 1; }

    class PropertiesEndpoint final : public Amqp::Tests::MessageTests::MockServiceEndpoint {
    public:
      PropertiesEndpoint() : MockServiceEndpoint("$management", {}) {}

    private:
      void MessageReceived(
          std::string const&, std::shared_ptr<Amqp::Models::AmqpMessage> const& request) override
      {
        Amqp::Models::AmqpMessage response;
        response.ApplicationProperties["status-code"] = 200;
        response.Properties.CorrelationId = request->Properties.MessageId;
        Amqp::Models::AmqpMap body;
        body["name"] = "hub";
        body["created_at"] = Amqp::Models::AmqpTimestamp(std::chrono::milliseconds(0)).AsAmqpValue();
        body["partition_ids"] = Amqp::Models::AmqpArray{"0", "1"}.AsAmqpValue();
        response.SetBody(body.AsAmqpValue());
        auto result = GetMessageSender().Send(response, GetListenerContext());
        EXPECT_EQ(std::get<0>(result), Amqp::_internal::MessageSendStatus::Ok);
      }
    };

    int endCalls = 0;
    int CountEnd(SESSION_HANDLE session, char const* condition, char const* description)
    {
      ++endCalls;
      return session_end(session, condition, description);
    }

    void SuccessfulDiscovery(bool cancelledClose)
    {
      std::atomic<bool> closeFailureLogged{false};
      Azure::Core::Diagnostics::Logger::SetLevel(Azure::Core::Diagnostics::Logger::Level::Warning);
      Azure::Core::Diagnostics::Logger::SetListener(
          [&](Azure::Core::Diagnostics::Logger::Level, std::string const& line) {
            if (line.find("Exception while closing the management client") != std::string::npos)
            {
              closeFailureLogged = true;
            }
          });
      {
        Amqp::Tests::MessageTests::AmqpServerMock server(AvailablePort());
        server.AddServiceEndpoint(std::make_shared<PropertiesEndpoint>());
        server.StartListening();
        {
          Amqp::_internal::ConnectionOptions options;
          options.Port = server.GetPort();
          Amqp::_internal::Connection connection("localhost", nullptr, options);
          auto session = Amqp::_detail::SessionFactory::CreateFromInternal(
              std::make_shared<Amqp::_detail::SessionImpl>(
                  Amqp::_detail::ConnectionFactory::GetImpl(connection),
                  Amqp::_internal::SessionOptions{}, nullptr, CountEnd));
          endCalls = 0;
          {
            EventHubsPropertiesClient properties(session, "hub");
            auto context = Azure::Core::Context().WithDeadline(
                std::chrono::system_clock::now() + std::chrono::seconds(10));
            auto result = properties.GetEventHubsProperties("hub", context);
            EXPECT_EQ(result.Name, "hub");
            EXPECT_EQ(result.PartitionIds, (std::vector<std::string>{"0", "1"}));
            if (cancelledClose)
            {
              context.Cancel();
            }
            properties.Close(context);
            properties.Close(context);
          }
          EXPECT_EQ(endCalls, 1);
        }
        server.StopListening();
      }
      Azure::Core::Diagnostics::Logger::SetListener(nullptr);
      EXPECT_EQ(closeFailureLogged.load(), cancelledClose);
      Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
    }

    void PartialInitialization(bool explicitClose, bool failEnd)
    {
      std::weak_ptr<Amqp::_detail::ConnectionImpl> connectionLifetime;
      {
        Amqp::Tests::MessageTests::AmqpServerMock server(AvailablePort());
        server.StartListening();
        Amqp::_internal::ConnectionOptions options;
        options.Port = server.GetPort();
        Amqp::_internal::Connection connection("localhost", nullptr, options);
        connectionLifetime = Amqp::_detail::ConnectionFactory::GetImpl(connection);
        auto session = Amqp::_detail::SessionFactory::CreateFromInternal(
            std::make_shared<Amqp::_detail::SessionImpl>(
                Amqp::_detail::ConnectionFactory::GetImpl(connection),
                Amqp::_internal::SessionOptions{}, nullptr, failEnd ? FailEnd : session_end));
        {
          EventHubsPropertiesClient properties(session, "hub", FailCreation);
          // Retrying discovery must not acquire a second polling reference.
          for (int attempt = 0; attempt != 2; ++attempt)
          {
            try
            {
              properties.GetEventHubsProperties("hub", {});
              ADD_FAILURE() << "Expected management creation to fail";
            }
            catch (std::runtime_error const& error)
            {
              EXPECT_STREQ(error.what(), "injected management creation failure");
            }
          }
          if (explicitClose)
          {
            properties.Close({});
            properties.Close({});
          }
        }
      }
      Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
      EXPECT_TRUE(connectionLifetime.expired());
    }

    void FailedOpen(bool failEnd)
    {
      {
        Amqp::Tests::MessageTests::AmqpServerMock server(AvailablePort());
        server.StartListening();
        Amqp::_internal::ConnectionOptions options;
        options.Port = server.GetPort();
        Amqp::_internal::Connection connection("localhost", nullptr, options);
        auto session = Amqp::_detail::SessionFactory::CreateFromInternal(
            std::make_shared<Amqp::_detail::SessionImpl>(
                Amqp::_detail::ConnectionFactory::GetImpl(connection),
                Amqp::_internal::SessionOptions{}, nullptr, failEnd ? FailEnd : session_end));
        EventHubsPropertiesClient properties(session, "hub");
        Azure::Core::Context cancelled;
        cancelled.Cancel();
        EXPECT_ANY_THROW(properties.GetEventHubsProperties("hub", cancelled));
        properties.Close({});
        properties.Close({});
      }
      Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
    }
  } // namespace

  TEST(PropertiesCleanupDeathTest, SuccessfulDiscoveryAndRepeatedClose)
  {
    EXPECT_EXIT({
      SuccessfulDiscovery(false);
      std::exit(::testing::Test::HasFailure() ? 1 : 0);
    }, ::testing::ExitedWithCode(0), "");
  }

  TEST(PropertiesCleanupDeathTest, ManagementCloseFailureStillEndsSession)
  {
    EXPECT_EXIT({
      SuccessfulDiscovery(true);
      std::exit(::testing::Test::HasFailure() ? 1 : 0);
    }, ::testing::ExitedWithCode(0), "");
  }

  TEST(PropertiesCleanupDeathTest, CreationFailureThenRepeatedClose)
  {
    EXPECT_EXIT({
      PartialInitialization(true, false);
      std::exit(::testing::Test::HasFailure() ? 1 : 0);
    }, ::testing::ExitedWithCode(0), "");
  }

  TEST(PropertiesCleanupDeathTest, CreationFailureThenDestruction)
  {
    EXPECT_EXIT({
      PartialInitialization(false, false);
      std::exit(::testing::Test::HasFailure() ? 1 : 0);
    }, ::testing::ExitedWithCode(0), "");
  }

  TEST(PropertiesCleanupDeathTest, CreationFailureAndSessionEndFailure)
  {
    EXPECT_EXIT({
      PartialInitialization(true, true);
      std::exit(::testing::Test::HasFailure() ? 1 : 0);
    }, ::testing::ExitedWithCode(0), "");
  }

  TEST(PropertiesCleanupDeathTest, OpenFailureThenRepeatedClose)
  {
    EXPECT_EXIT({
      FailedOpen(false);
      std::exit(::testing::Test::HasFailure() ? 1 : 0);
    }, ::testing::ExitedWithCode(0), "");
  }

  TEST(PropertiesCleanupDeathTest, OpenFailureAndSessionEndFailure)
  {
    EXPECT_EXIT({
      FailedOpen(true);
      std::exit(::testing::Test::HasFailure() ? 1 : 0);
    }, ::testing::ExitedWithCode(0), "");
  }

  TEST(PropertiesCleanupDeathTest, ProducerDiscoveryFailureThenRepeatedClose)
  {
    EXPECT_EXIT({
      {
        ProducerClientOptions options;
        options.RetryOptions.MaxRetries = 0;
        ProducerClient producer("localhost", "hub", nullptr, options);
        Azure::Core::Context cancelled;
        cancelled.Cancel();
        EXPECT_ANY_THROW(producer.GetEventHubProperties(cancelled));
        EXPECT_NO_THROW(producer.Close());
        EXPECT_NO_THROW(producer.Close());
      }
      Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
      std::exit(::testing::Test::HasFailure() ? 1 : 0);
    }, ::testing::ExitedWithCode(0), "");
  }
}}}} // namespace Azure::Messaging::EventHubs::Test
#endif
