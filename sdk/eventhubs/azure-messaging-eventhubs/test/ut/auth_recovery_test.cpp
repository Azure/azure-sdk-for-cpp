// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../../../core/azure-core-amqp/test/ut/mock_amqp_server.hpp"
#include "eventhubs_test_base.hpp"

#include <azure/core/amqp/internal/common/global_state.hpp>
#include <azure/core/amqp/internal/connection.hpp>
#include <azure/core/amqp/internal/models/messaging_values.hpp>
#include <azure/core/amqp/internal/session.hpp>
#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

namespace Azure { namespace Messaging { namespace EventHubs { namespace _detail {
  void SetProducerSessionSnapshotHook(std::function<void()> hook);
  void SetPartitionClientStateCloseHook(std::function<void()> hook);
}}}} // namespace Azure::Messaging::EventHubs::_detail

#if defined(AZ_PLATFORM_POSIX)
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>
#elif defined(AZ_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#endif

namespace Azure { namespace Core { namespace Amqp { namespace Tests {

  uint16_t FindAvailableSocket()
  {
    auto state = Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance();
    (void)state;

    for (uint32_t port = 45000; port != 46000; ++port)
    {
#if defined(AZ_PLATFORM_WINDOWS)
      auto socketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      if (socketHandle == INVALID_SOCKET)
      {
        continue;
      }
#else
      auto socketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      if (socketHandle < 0)
      {
        continue;
      }
#endif

      sockaddr_in address{};
      address.sin_family = AF_INET;
      address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
      address.sin_port = htons(static_cast<uint16_t>(port));
      auto const result
          = bind(socketHandle, reinterpret_cast<sockaddr*>(&address), sizeof(address));
#if defined(AZ_PLATFORM_WINDOWS)
      closesocket(socketHandle);
#else
      close(socketHandle);
#endif
      if (result == 0)
      {
        return static_cast<uint16_t>(port);
      }
    }

    throw std::runtime_error("Could not find a free test socket.");
  }

}}}} // namespace Azure::Core::Amqp::Tests

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {
  namespace {
    using Azure::Core::Amqp::_internal::Connection;
    using Azure::Core::Amqp::_internal::ConnectionOptions;
    using Azure::Core::Amqp::_internal::MessageReceiver;
    using Azure::Core::Amqp::_internal::MessageSender;
    using Azure::Core::Amqp::_internal::Session;
    using Azure::Core::Amqp::_internal::SessionRole;
    using Azure::Core::Amqp::Models::AmqpMessage;
    using Azure::Core::Amqp::Models::AmqpSymbol;
    using Azure::Core::Amqp::Models::AmqpValue;
    using Azure::Core::Amqp::Models::_internal::AmqpError;
    using Azure::Core::Amqp::Models::_internal::AmqpErrorCondition;
    using Azure::Core::Amqp::Tests::MessageTests::AmqpServerMock;
    using Azure::Core::Amqp::Tests::MessageTests::MockServiceEndpoint;
    using Azure::Core::Amqp::Tests::MessageTests::MockServiceEndpointOptions;

    Azure::Core::Http::Policies::RetryOptions FastRetryOptions(int32_t maxRetries = 1)
    {
      Azure::Core::Http::Policies::RetryOptions options;
      options.MaxRetries = maxRetries;
      options.RetryDelay = std::chrono::milliseconds(1);
      options.MaxRetryDelay = std::chrono::milliseconds(2);
      return options;
    }

    Azure::Messaging::EventHubs::EventDataBatchOptions BatchOptions()
    {
      Azure::Messaging::EventHubs::EventDataBatchOptions options;
      options.MaxBytes = 1024;
      options.PartitionId = "0";
      return options;
    }

    class CbsScript final {
    public:
      std::atomic<int> OpenFailures{0};
      std::atomic<int> PutTokenFailures{0};
      std::atomic<int> OpenAttempts{0};
      std::atomic<int> PutTokenAttempts{0};
    };

    class EventScript final {
    public:
      std::atomic<int> TransferFailures{0};
      std::atomic<int> TransferAttempts{0};
      std::atomic<int> AcceptedTransfers{0};
      std::atomic<int> DeliveryLinks{0};
      std::atomic<int> DeliveryNumber{0};
      bool DeliverEvents{false};
    };

    bool Consume(std::atomic<int>& count)
    {
      auto current = count.load();
      while (current > 0 && !count.compare_exchange_weak(current, current - 1))
      {
      }
      return current > 0;
    }

    class ScriptedCbsEndpoint final : public MockServiceEndpoint {
    public:
      ScriptedCbsEndpoint(
          MockServiceEndpointOptions const& options,
          std::shared_ptr<CbsScript> script)
          : MockServiceEndpoint("$cbs", options), m_script{std::move(script)}
      {
      }

      bool OnLinkAttached(
          Session const& session,
          std::string const& linkName,
          Azure::Core::Amqp::_internal::LinkEndpoint& linkEndpoint,
          SessionRole role,
          Azure::Core::Amqp::Models::_internal::MessageSource const& source,
          Azure::Core::Amqp::Models::_internal::MessageTarget const& target) override
      {
        if (role == SessionRole::Receiver)
        {
          ++m_script->OpenAttempts;
          if (Consume(m_script->OpenFailures))
          {
            AmqpError error;
            error.Condition = AmqpErrorCondition::InternalError;
            error.Description = "CBS open failed";
            DetachLink(session, linkEndpoint, true, error);
            return false;
          }
        }
        return MockServiceEndpoint::OnLinkAttached(
            session, linkName, linkEndpoint, role, source, target);
      }

    private:
      void MessageReceived(std::string const&, std::shared_ptr<AmqpMessage> const& message) override
      {
        auto const operation
            = static_cast<std::string>(message->ApplicationProperties.at("operation"));
        if (operation != "put-token")
        {
          return;
        }

        ++m_script->PutTokenAttempts;
        bool const failed = Consume(m_script->PutTokenFailures);
        AmqpMessage response;
        auto correlationId = message->Properties.CorrelationId;
        if (correlationId.IsNull())
        {
          correlationId = message->Properties.MessageId;
        }
        response.Properties.CorrelationId = correlationId;
        response.ApplicationProperties["status-code"] = failed ? 401 : 200;
        response.ApplicationProperties["status-description"]
            = failed ? "CBS PutToken failed" : "OK-put";
        response.SetBody(AmqpValue{});

        auto const result = GetMessageSender().Send(response, GetListenerContext());
        if (std::get<0>(result) != Azure::Core::Amqp::_internal::MessageSendStatus::Ok)
        {
          GTEST_LOG_(INFO) << "Failed to send scripted CBS response: " << std::get<1>(result);
        }
      }

      std::shared_ptr<CbsScript> m_script;
    };

    class EventHubEndpoint final : public MockServiceEndpoint {
    public:
      EventHubEndpoint(
          std::string name,
          MockServiceEndpointOptions const& options,
          std::shared_ptr<EventScript> script)
          : MockServiceEndpoint(std::move(name), options), m_script{std::move(script)}
      {
      }

      ~EventHubEndpoint() override
      {
        for (auto& worker : m_deliveryWorkers)
        {
          if (worker.joinable())
          {
            worker.join();
          }
        }
      }

      bool OnLinkAttached(
          Session const& session,
          std::string const& linkName,
          Azure::Core::Amqp::_internal::LinkEndpoint& linkEndpoint,
          SessionRole role,
          Azure::Core::Amqp::Models::_internal::MessageSource const& source,
          Azure::Core::Amqp::Models::_internal::MessageTarget const& target) override
      {
        auto const attached = MockServiceEndpoint::OnLinkAttached(
            session, linkName, linkEndpoint, role, source, target);
        if (attached && role == SessionRole::Receiver && m_script->DeliverEvents)
        {
          ++m_script->DeliveryLinks;
          m_deliveryWorkers.emplace_back([this, session, &linkEndpoint, linkName]() {
            Deliver(session, linkEndpoint, linkName);
          });
        }
        return attached;
      }

    protected:
      AmqpValue OnMessageReceived(MessageReceiver const&, std::shared_ptr<AmqpMessage> const&)
          override
      {
        ++m_script->TransferAttempts;
        if (Consume(m_script->TransferFailures))
        {
          return Azure::Core::Amqp::Models::_internal::Messaging::DeliveryRejected(
              "amqp:unauthorized-access", "stale transfer", {});
        }
        ++m_script->AcceptedTransfers;
        return Azure::Core::Amqp::Models::_internal::Messaging::DeliveryAccepted();
      }

    private:
      void MessageReceived(std::string const&, std::shared_ptr<AmqpMessage> const&) override {}

      void Deliver(
          Session const& session,
          Azure::Core::Amqp::_internal::LinkEndpoint& linkEndpoint,
          std::string const& linkName)
      {
        while (!GetListenerContext().IsCancelled() && !HasMessageSender(linkName))
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        if (GetListenerContext().IsCancelled())
        {
          return;
        }

        auto sender = GetMessageSender(linkName);
        auto const delivery = ++m_script->DeliveryNumber;
        auto const offset = delivery == 1 ? "10" : "11";
        AmqpMessage message;
        message.MessageAnnotations[AmqpSymbol{"x-opt-offset"}] = AmqpValue{offset};
        message.SetBody(AmqpValue{"event"});
        auto const sendResult = sender.Send(message, GetListenerContext());
        if (std::get<0>(sendResult) != Azure::Core::Amqp::_internal::MessageSendStatus::Ok)
        {
          GTEST_LOG_(INFO) << "Failed to send scripted event: " << std::get<1>(sendResult);
          return;
        }

        if (delivery == 1)
        {
          AmqpError error;
          error.Condition = AmqpErrorCondition::UnauthorizedAccess;
          error.Description = "stale receive";
          DetachLink(session, linkEndpoint, true, error);
        }
      }

      std::shared_ptr<EventScript> m_script;
      std::vector<std::thread> m_deliveryWorkers;
    };

    class AuthRecoveryServer final {
    public:
      AuthRecoveryServer(
          int openFailures = 0,
          int putTokenFailures = 0,
          int transferFailures = 0,
          bool deliverEvents = false)
          : m_port{Azure::Core::Amqp::Tests::FindAvailableSocket()},
            m_server{m_port, testing::UnitTest::GetInstance()->current_test_info()->name(), false},
            m_cbsScript{std::make_shared<CbsScript>()}, m_eventScript{
                                                            std::make_shared<EventScript>()}
      {
        m_cbsScript->OpenFailures = openFailures;
        m_cbsScript->PutTokenFailures = putTokenFailures;
        m_eventScript->TransferFailures = transferFailures;
        m_eventScript->DeliverEvents = deliverEvents;

        MockServiceEndpointOptions endpointOptions;
        endpointOptions.ListenerContext = m_server.GetListenerContext();
        m_server.AddServiceEndpoint(
            std::make_shared<ScriptedCbsEndpoint>(endpointOptions, m_cbsScript));
        m_server.AddServiceEndpoint(std::make_shared<EventHubEndpoint>(
            ProducerPartitionEndpoint(), endpointOptions, m_eventScript));
        m_server.AddServiceEndpoint(std::make_shared<EventHubEndpoint>(
            ProducerGatewayEndpoint(), endpointOptions, m_eventScript));
        m_server.AddServiceEndpoint(std::make_shared<EventHubEndpoint>(
            ConsumerPartitionEndpoint(), endpointOptions, m_eventScript));
      }

      ~AuthRecoveryServer() { Stop(); }

      void Start()
      {
        if (!m_started)
        {
          m_server.StartListening();
          m_started = true;
        }
      }

      void Stop()
      {
        if (m_started)
        {
          m_server.StopListening();
          m_started = false;
        }
      }

      uint16_t Port() const { return m_port; }
      std::size_t ConnectionCount() const { return m_server.GetConnectionCount(); }
      int PutTokenAttempts() const { return m_cbsScript->PutTokenAttempts.load(); }
      int CbsOpenAttempts() const { return m_cbsScript->OpenAttempts.load(); }
      int TransferAttempts() const { return m_eventScript->TransferAttempts.load(); }
      int AcceptedTransfers() const { return m_eventScript->AcceptedTransfers.load(); }
      int DeliveryLinks() const { return m_eventScript->DeliveryLinks.load(); }

      void SetPutTokenFailures(int failures) { m_cbsScript->PutTokenFailures = failures; }

      std::string ConnectionString() const
      {
        return "Endpoint=sb://127.0.0.1:" + std::to_string(m_port)
            + "/;SharedAccessKeyName=TestKey;SharedAccessKey=abcdabcd;EntityPath=eh;"
              "UseDevelopmentEmulator=true";
      }

      std::string ProducerPartitionEndpoint() const
      {
        return "amqp://localhost:" + std::to_string(m_port) + "/eh/Partitions/0";
      }

      std::string ProducerGatewayEndpoint() const
      {
        return "amqp://localhost:" + std::to_string(m_port) + "/eh";
      }

      std::string ConsumerPartitionEndpoint() const
      {
        return "amqp://localhost:" + std::to_string(m_port)
            + "/eh/ConsumerGroups/$Default/Partitions/0";
      }

    private:
      uint16_t m_port;
      AmqpServerMock m_server;
      std::shared_ptr<CbsScript> m_cbsScript;
      std::shared_ptr<EventScript> m_eventScript;
      bool m_started{false};
    };

    class FailingCredential final : public Azure::Core::Credentials::TokenCredential {
    public:
      FailingCredential() : TokenCredential("FailingCredential") {}

      Azure::Core::Credentials::AccessToken GetToken(
          Azure::Core::Credentials::TokenRequestContext const&,
          Azure::Core::Context const&) const override
      {
        ++m_attempts;
        throw Azure::Core::Credentials::AuthenticationException("credential failure");
      }

      int Attempts() const { return m_attempts.load(); }

    private:
      mutable std::atomic<int> m_attempts{0};
    };

  } // anonymous namespace

  class AuthRecoveryTest : public EventHubsTestBase {
  protected:
    void SetUp() override
    {
      EventHubsTestBase::SetUp();
#if defined(AZ_PLATFORM_MAC)
      GTEST_SKIP() << "The uAMQP socket client tests are not supported on Apple platforms.";
#endif
    }
  };

  TEST_F(AuthRecoveryTest, ProducerCreateBatchRecoversPutTokenAuthenticationWithFreshStack)
  {
    AuthRecoveryServer server(0, 1);
    server.Start();

    ProducerClientOptions options;
    options.RetryOptions = FastRetryOptions();
    ProducerClient producer(server.ConnectionString(), "", options);

    EXPECT_NO_THROW(producer.CreateBatch(BatchOptions()));
    EXPECT_EQ(2U, server.ConnectionCount());
    EXPECT_EQ(2, server.PutTokenAttempts());
  }

  TEST_F(AuthRecoveryTest, ProducerSendRecoversStaleUnauthorizedWithOneFreshStack)
  {
    AuthRecoveryServer server(0, 0, 1);
    server.Start();

    ProducerClientOptions options;
    options.RetryOptions = FastRetryOptions();
    ProducerClient producer(server.ConnectionString(), "", options);
    auto batch = producer.CreateBatch(BatchOptions());
    ASSERT_TRUE(batch.TryAdd(Models::EventData{"payload"}));

    EXPECT_NO_THROW(producer.Send(batch));
    EXPECT_EQ(2U, server.ConnectionCount());
    EXPECT_EQ(2, server.TransferAttempts());
    EXPECT_EQ(1, server.AcceptedTransfers());
  }

  TEST_F(AuthRecoveryTest, ProducerCloseAtSessionSnapshotPreservesRetryContract)
  {
    AuthRecoveryServer server;
    server.Start();

    ProducerClientOptions options;
    options.RetryOptions = FastRetryOptions();
    ProducerClient producer(server.ConnectionString(), "", options);

    std::atomic<bool> hookCalled{false};
    std::exception_ptr closeFailure;
    _detail::SetProducerSessionSnapshotHook([&]() {
      hookCalled = true;
      std::thread closeThread([&]() {
        try
        {
          producer.Close();
        }
        catch (...)
        {
          closeFailure = std::current_exception();
        }
      });
      closeThread.join();
    });

    std::exception_ptr operationFailure;
    try
    {
      producer.CreateBatch(BatchOptions());
    }
    catch (...)
    {
      operationFailure = std::current_exception();
    }
    _detail::SetProducerSessionSnapshotHook({});

    ASSERT_TRUE(hookCalled.load());
    if (closeFailure)
    {
      std::rethrow_exception(closeFailure);
    }
    if (operationFailure)
    {
      try
      {
        std::rethrow_exception(operationFailure);
      }
      catch (std::out_of_range const&)
      {
        ADD_FAILURE() << "Producer session invalidation escaped as std::out_of_range.";
      }
      catch (EventHubsException const& exception)
      {
        ADD_FAILURE() << "Producer retry escaped as EventHubsException: " << exception.what();
      }
      catch (std::exception const& exception)
      {
        ADD_FAILURE() << "Producer retry escaped as an unexpected exception: " << exception.what();
      }
    }

    EXPECT_FALSE(operationFailure);
    EXPECT_EQ(2U, server.ConnectionCount());
  }

  TEST_F(AuthRecoveryTest, ProducerConvenienceSendSharesOneRecoveryBudgetAcrossBatchAndTransfer)
  {
    AuthRecoveryServer server(0, 1, 1);
    server.Start();

    ProducerClientOptions options;
    options.RetryOptions = FastRetryOptions();
    ProducerClient producer(server.ConnectionString(), "", options);
    EventHubsException failure{"no failure"};
    try
    {
      producer.Send(Models::EventData{"payload"});
      ADD_FAILURE() << "Expected unauthorized transfer after the authentication budget was used.";
    }
    catch (EventHubsException const& exception)
    {
      failure = exception;
    }

    EXPECT_EQ(2U, server.ConnectionCount());
    EXPECT_EQ(2, server.PutTokenAttempts());
    EXPECT_EQ(1, server.TransferAttempts());
    EXPECT_EQ("amqp:unauthorized-access", failure.ErrorCondition);
    EXPECT_EQ("stale transfer", failure.ErrorDescription);
    EXPECT_FALSE(failure.IsTransient);
  }

  TEST_F(AuthRecoveryTest, ConsumerCreatePartitionClientRecoversPutToken)
  {
    AuthRecoveryServer server(0, 1);
    server.Start();

    ConsumerClientOptions options;
    options.RetryOptions = FastRetryOptions();
    ConsumerClient consumer(server.ConnectionString(), "", DefaultConsumerGroup, options);

    auto partition = consumer.CreatePartitionClient("0");
    EXPECT_TRUE(partition.ReceiveEvents(0).empty());
    EXPECT_EQ(2U, server.ConnectionCount());
    EXPECT_EQ(2, server.PutTokenAttempts());
  }

  TEST_F(AuthRecoveryTest, PartitionMoveAssignmentClosesActiveReceive)
  {
    AuthRecoveryServer server;
    server.Start();

    ConsumerClientOptions destinationOptions;
    destinationOptions.Name = "destination";
    destinationOptions.RetryOptions = FastRetryOptions();
    ConsumerClient destinationConsumer(
        server.ConnectionString(), "", DefaultConsumerGroup, destinationOptions);

    ConsumerClientOptions sourceOptions;
    sourceOptions.Name = "source";
    sourceOptions.RetryOptions = FastRetryOptions();
    ConsumerClient sourceConsumer(
        server.ConnectionString(), "", DefaultConsumerGroup, sourceOptions);

    auto destination = destinationConsumer.CreatePartitionClient("0");
    auto source = sourceConsumer.CreatePartitionClient("0");

    Azure::Core::Context receiveContext{Azure::DateTime::clock::now() + std::chrono::seconds(5)};
    std::atomic<bool> receiveStarted{false};
    std::atomic<bool> receiveComplete{false};
    std::exception_ptr receiveFailure;
    std::thread receiveThread([&]() {
      receiveStarted = true;
      try
      {
        destination.ReceiveEvents(1, receiveContext);
      }
      catch (...)
      {
        receiveFailure = std::current_exception();
      }
      receiveComplete = true;
    });

    auto cleanup = [&]() {
      _detail::SetPartitionClientStateCloseHook({});
      receiveContext.Cancel();
      if (receiveThread.joinable())
      {
        receiveThread.join();
      }
    };

    auto const startDeadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
    while (!receiveStarted.load() && std::chrono::steady_clock::now() < startDeadline)
    {
      std::this_thread::yield();
    }
    if (!receiveStarted.load())
    {
      cleanup();
      ADD_FAILURE() << "The destination receive did not start.";
      return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (receiveComplete.load())
    {
      cleanup();
      ADD_FAILURE() << "The destination receive did not block.";
      return;
    }

    std::atomic<int> closeHookCalls{0};
    _detail::SetPartitionClientStateCloseHook([&]() { ++closeHookCalls; });
    auto const moveStart = std::chrono::steady_clock::now();
    std::exception_ptr moveFailure;
    try
    {
      destination = std::move(source);
    }
    catch (...)
    {
      moveFailure = std::current_exception();
    }
    auto const moveElapsed = std::chrono::steady_clock::now() - moveStart;
    auto const receiveDeadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
    while (!receiveComplete.load() && std::chrono::steady_clock::now() < receiveDeadline)
    {
      std::this_thread::yield();
    }
    auto const receiveExited = receiveComplete.load();
    cleanup();

    ASSERT_FALSE(moveFailure);
    EXPECT_EQ(1, closeHookCalls.load());
    EXPECT_TRUE(receiveExited);
    EXPECT_LT(moveElapsed, std::chrono::seconds(1));
    if (receiveFailure)
    {
      try
      {
        std::rethrow_exception(receiveFailure);
      }
      catch (Azure::Core::OperationCancelledException const&)
      {
      }
      catch (std::exception const& exception)
      {
        ADD_FAILURE() << "The destination receive failed unexpectedly: " << exception.what();
      }
    }
  }

  TEST_F(AuthRecoveryTest, ReceiverReceiveRecoversUnauthorizedAndResumesWithoutDuplicate)
  {
    AuthRecoveryServer server(0, 0, 0, true);
    server.Start();

    ConsumerClientOptions options;
    options.RetryOptions = FastRetryOptions();
    ConsumerClient consumer(server.ConnectionString(), "", DefaultConsumerGroup, options);
    PartitionClientOptions partitionOptions;
    partitionOptions.StartPosition.Earliest = true;
    auto partition = consumer.CreatePartitionClient("0", partitionOptions);

    auto events = partition.ReceiveEvents(2);
    ASSERT_EQ(2U, events.size());
    ASSERT_TRUE(events[0]->Offset.HasValue());
    ASSERT_TRUE(events[1]->Offset.HasValue());
    EXPECT_EQ("10", events[0]->Offset.Value());
    EXPECT_EQ("11", events[1]->Offset.Value());
    EXPECT_EQ(2U, server.ConnectionCount());
    EXPECT_EQ(2, server.DeliveryLinks());
  }

  TEST_F(AuthRecoveryTest, ReceiverPartialDeliveryPreservesPendingAuthenticationFailure)
  {
    AuthRecoveryServer server(0, 0, 0, true);
    server.Start();

    ConsumerClientOptions options;
    options.RetryOptions = FastRetryOptions();
    ConsumerClient consumer(server.ConnectionString(), "", DefaultConsumerGroup, options);
    PartitionClientOptions partitionOptions;
    partitionOptions.StartPosition.Earliest = true;
    auto partition = consumer.CreatePartitionClient("0", partitionOptions);

    server.SetPutTokenFailures(2);
    auto events = partition.ReceiveEvents(2);

    ASSERT_EQ(1U, events.size());
    ASSERT_TRUE(events[0]->Offset.HasValue());
    EXPECT_EQ("10", events[0]->Offset.Value());
    EXPECT_EQ(2U, server.ConnectionCount());
    EXPECT_EQ(2, server.PutTokenAttempts());

    try
    {
      partition.ReceiveEvents(1);
      ADD_FAILURE() << "Expected the pending authentication failure.";
    }
    catch (Azure::Core::Credentials::AuthenticationException const& exception)
    {
      EXPECT_EQ(
          "Could not authenticate client. Error Status: 401 reason: CBS PutToken failed",
          exception.what());
    }
    EXPECT_EQ(3U, server.ConnectionCount());
    EXPECT_EQ(3, server.PutTokenAttempts());
  }

  TEST_F(AuthRecoveryTest, CbsOpenErrorUsesOrdinaryBudgetAndLeavesAuthBudgetAvailable)
  {
    AuthRecoveryServer server(1, 1);
    server.Start();

    ProducerClientOptions options;
    options.RetryOptions = FastRetryOptions();
    ProducerClient producer(server.ConnectionString(), "", options);

    EXPECT_NO_THROW(producer.CreateBatch(BatchOptions()));
    EXPECT_EQ(3U, server.ConnectionCount());
    EXPECT_EQ(3, server.CbsOpenAttempts());
    EXPECT_EQ(2, server.PutTokenAttempts());
  }

  TEST_F(AuthRecoveryTest, PositiveMaxRetriesEnablesRecoveryAndZeroDisablesIt)
  {
    {
      AuthRecoveryServer server(0, 1);
      server.Start();
      ProducerClientOptions options;
      options.RetryOptions = FastRetryOptions(1);
      ProducerClient producer(server.ConnectionString(), "", options);
      EXPECT_NO_THROW(producer.CreateBatch(BatchOptions()));
      EXPECT_EQ(2U, server.ConnectionCount());
    }

    {
      AuthRecoveryServer server(0, 1);
      server.Start();
      ProducerClientOptions options;
      options.RetryOptions = FastRetryOptions(0);
      ProducerClient producer(server.ConnectionString(), "", options);
      EXPECT_THROW(
          producer.CreateBatch(BatchOptions()), Azure::Core::Credentials::AuthenticationException);
      EXPECT_EQ(1U, server.ConnectionCount());
    }
  }

  TEST_F(AuthRecoveryTest, SecondAuthenticationFailureStopsWithoutThirdAttemptAndPreservesFailure)
  {
    {
      AuthRecoveryServer server(0, 2);
      server.Start();

      ProducerClientOptions options;
      options.RetryOptions = FastRetryOptions();
      ProducerClient producer(server.ConnectionString(), "", options);
      try
      {
        producer.CreateBatch(BatchOptions());
        ADD_FAILURE() << "Expected the second CBS PutToken failure.";
      }
      catch (Azure::Core::Credentials::AuthenticationException const& exception)
      {
        EXPECT_EQ(
            "Could not authenticate client. Error Status: 401 reason: CBS PutToken failed",
            exception.what());
      }
      EXPECT_EQ(2U, server.ConnectionCount());
      EXPECT_EQ(2, server.PutTokenAttempts());
    }
    {
      AuthRecoveryServer server(0, 0, 2);
      server.Start();

      ProducerClientOptions options;
      options.RetryOptions = FastRetryOptions();
      ProducerClient producer(server.ConnectionString(), "", options);
      auto batch = producer.CreateBatch(BatchOptions());
      ASSERT_TRUE(batch.TryAdd(Models::EventData{"payload"}));

      EventHubsException failure{"no failure"};
      try
      {
        producer.Send(batch);
        ADD_FAILURE() << "Expected the second unauthorized transfer failure.";
      }
      catch (EventHubsException const& exception)
      {
        failure = exception;
      }
      EXPECT_EQ(2U, server.ConnectionCount());
      EXPECT_EQ(2, server.TransferAttempts());
      EXPECT_EQ("amqp:unauthorized-access", failure.ErrorCondition);
      EXPECT_EQ("stale transfer", failure.ErrorDescription);
      EXPECT_FALSE(failure.IsTransient);
    }
  }

  TEST_F(AuthRecoveryTest, CredentialAuthenticationExceptionIsPermanent)
  {
    AuthRecoveryServer server;
    server.Start();

    auto credential = std::make_shared<FailingCredential>();
    ConnectionOptions options;
    options.Port = server.Port();
    options.AuthenticationScopes = {"https://eventhubs.azure.net/.default"};
    Connection connection("localhost", credential, options);
    Session session{connection.CreateSession({})};
    MessageSender sender{session.CreateMessageSender(server.ProducerGatewayEndpoint(), {})};

    bool threw = false;
    try
    {
      auto const result = sender.Open();
      (void)result;
    }
    catch (Azure::Core::Credentials::AuthenticationException const&)
    {
      threw = true;
    }
    EXPECT_TRUE(threw);
    EXPECT_EQ(1, credential->Attempts());
  }

}}}} // namespace Azure::Messaging::EventHubs::Test
