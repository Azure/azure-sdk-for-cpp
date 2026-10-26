// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#if ENABLE_UAMQP
#include "../../../../core/azure-core-amqp/src/impl/uamqp/amqp/private/session_impl.hpp"
#include "../../../../core/azure-core-amqp/test/ut/cleanup_test_listener.hpp"
#include "../../../../core/azure-core-amqp/test/ut/mock_amqp_server.hpp"
#include "private/eventhubs_utilities.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/platform.hpp>
#include <azure/messaging/eventhubs/producer_client.hpp>

#include <atomic>
#include <cstdlib>

#include <gtest/gtest.h>

namespace {
std::uint16_t AvailablePort()
{
  Azure::Core::Amqp::Tests::CleanupTestListener listener;
  return listener.GetPort();
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
      PropertiesEndpoint(bool failDiscovery = false)
          : MockServiceEndpoint("$management", {}), m_failDiscovery{failDiscovery}
      {
      }

    private:
      bool m_failDiscovery;
      void MessageReceived(
          std::string const&,
          std::shared_ptr<Amqp::Models::AmqpMessage> const& request) override
      {
        Amqp::Models::AmqpMessage response;
        response.ApplicationProperties["status-code"] = m_failDiscovery ? 500 : 200;
        response.Properties.CorrelationId = request->Properties.MessageId;
        Amqp::Models::AmqpMap body;
        body["name"] = "hub";
        body["created_at"]
            = Amqp::Models::AmqpTimestamp(std::chrono::milliseconds(0)).AsAmqpValue();
        body["partition_ids"] = Amqp::Models::AmqpArray{"0", "1"}.AsAmqpValue();
        response.SetBody(body.AsAmqpValue());
        auto result = GetMessageSender().Send(response, GetListenerContext());
        EXPECT_EQ(std::get<0>(result), Amqp::_internal::MessageSendStatus::Ok);
      }
    };

    void ProducerDiscoveryFailure()
    {
      {
        Amqp::Tests::MessageTests::AmqpServerMock server(AvailablePort());
        server.AddServiceEndpoint(std::make_shared<PropertiesEndpoint>(true));
        server.StartListening();
        {
          ProducerClientOptions options;
          options.RetryOptions.MaxRetries = 0;
          ProducerClient producer(
              "Endpoint=sb://localhost:" + std::to_string(server.GetPort())
                  + "/;SharedAccessKeyName=test;SharedAccessKey=YWJj;UseDevelopmentEmulator=true",
              "hub",
              options);
          auto context = Azure::Core::Context().WithDeadline(
              std::chrono::system_clock::now() + std::chrono::seconds(10));
          EXPECT_THROW(producer.GetEventHubProperties(context), EventHubsException);
          producer.Close(context);
          producer.Close(context);
        }
        server.StopListening();
      }
      Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
    }

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
                  Amqp::_internal::SessionOptions{},
                  nullptr,
                  CountEnd));
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
              // Stop client connection polling after draining its current poll. The peer cannot
              // complete DETACH before the cancelled close waits, so the close reliably throws.
              Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->RemovePollable(
                  Amqp::_detail::ConnectionFactory::GetImpl(connection));
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
        Amqp::Tests::CleanupTestListener server;
        Amqp::_internal::ConnectionOptions options;
        options.Port = server.GetPort();
        Amqp::_internal::Connection connection("localhost", nullptr, options);
        connectionLifetime = Amqp::_detail::ConnectionFactory::GetImpl(connection);
        auto session = Amqp::_detail::SessionFactory::CreateFromInternal(
            std::make_shared<Amqp::_detail::SessionImpl>(
                Amqp::_detail::ConnectionFactory::GetImpl(connection),
                Amqp::_internal::SessionOptions{},
                nullptr,
                failEnd ? FailEnd : session_end));
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
        Amqp::Tests::CleanupTestListener server;
        Amqp::_internal::ConnectionOptions options;
        options.Port = server.GetPort();
        Amqp::_internal::Connection connection("localhost", nullptr, options);
        auto session = Amqp::_detail::SessionFactory::CreateFromInternal(
            std::make_shared<Amqp::_detail::SessionImpl>(
                Amqp::_detail::ConnectionFactory::GetImpl(connection),
                Amqp::_internal::SessionOptions{},
                nullptr,
                failEnd ? FailEnd : session_end));
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

  class PropertiesCleanupDeathTest : public ::testing::Test {
  protected:
    void SetUp() override
    {
      // Re-exec the child instead of inheriting AMQP polling-thread mutexes across fork.
      ::testing::FLAGS_gtest_death_test_style = "threadsafe";
    }
  };

  TEST_F(PropertiesCleanupDeathTest, SuccessfulDiscoveryAndRepeatedClose)
  {
    EXPECT_EXIT(
        {
          SuccessfulDiscovery(false);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(PropertiesCleanupDeathTest, ManagementCloseFailureStillEndsSession)
  {
    EXPECT_EXIT(
        {
          SuccessfulDiscovery(true);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(PropertiesCleanupDeathTest, CreationFailureThenRepeatedClose)
  {
    EXPECT_EXIT(
        {
          PartialInitialization(true, false);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(PropertiesCleanupDeathTest, CreationFailureThenDestruction)
  {
    EXPECT_EXIT(
        {
          PartialInitialization(false, false);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(PropertiesCleanupDeathTest, CreationFailureAndSessionEndFailure)
  {
    EXPECT_EXIT(
        {
          PartialInitialization(true, true);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(PropertiesCleanupDeathTest, OpenFailureThenRepeatedClose)
  {
    EXPECT_EXIT(
        {
          FailedOpen(false);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(PropertiesCleanupDeathTest, OpenFailureAndSessionEndFailure)
  {
    EXPECT_EXIT(
        {
          FailedOpen(true);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(PropertiesCleanupDeathTest, ProducerDiscoveryFailureThenRepeatedClose)
  {
    EXPECT_EXIT(
        {
          ProducerDiscoveryFailure();
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }
}}}} // namespace Azure::Messaging::EventHubs::Test
#endif
