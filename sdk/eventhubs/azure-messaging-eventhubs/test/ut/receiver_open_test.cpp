// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#if ENABLE_UAMQP
#include "../../../../core/azure-core-amqp/src/impl/uamqp/amqp/private/session_impl.hpp"
#include "../../../../core/azure-core-amqp/test/ut/cleanup_test_listener.hpp"
#include "../../../../core/azure-core-amqp/test/ut/mock_amqp_server.hpp"
#include "private/eventhubs_tracing.hpp"
#include "private/eventhubs_utilities.hpp"

#include <azure_uamqp_c/amqp_definitions.h>

#include <atomic>
#include <cstdlib>
#include <functional>
#include <future>

#include <gtest/gtest.h>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {
  namespace {
    namespace Amqp = Azure::Core::Amqp;

    class RefusingReceiverEndpoint final : public Amqp::Tests::MessageTests::MockServiceEndpoint {
    public:
      RefusingReceiverEndpoint(bool acceptFirst, std::string condition)
          : MockServiceEndpoint("receiver-open", {}), m_acceptFirst(acceptFirst),
            m_condition(std::move(condition))
      {
      }

      std::atomic<int> AttachCount{0};
      std::function<void()> Detach;
      void WaitUntilDetached()
      {
        ASSERT_EQ(
            std::future_status::ready, m_detached.get_future().wait_for(std::chrono::seconds(5)));
      }
      void WaitUntilReady()
      {
        ASSERT_EQ(
            std::future_status::ready, m_ready.get_future().wait_for(std::chrono::seconds(5)));
      }

      std::function<void(Core::Context const&)> SendEvent;

      bool OnLinkAttached(
          Amqp::_internal::Session const& session,
          std::string const& linkName,
          Amqp::_internal::LinkEndpoint& linkEndpoint,
          Amqp::_internal::SessionRole role,
          Amqp::Models::_internal::MessageSource const& source,
          Amqp::Models::_internal::MessageTarget const& target) override
      {
        (void)target;
        (void)source;
        (void)role;
        auto count = ++AttachCount;
        auto endpoint = linkEndpoint.Get();
        m_endpoint = endpoint;
        EXPECT_EQ(
            0,
            session_start_link_endpoint(
                endpoint,
                [](void* context, AMQP_VALUE frame, std::uint32_t, unsigned char const*) {
                  auto self = static_cast<RefusingReceiverEndpoint*>(context);
                  if (is_detach_type_by_descriptor(amqpvalue_get_inplace_descriptor(frame)))
                  {
                    session_destroy_link_endpoint(self->m_endpoint);
                    self->m_endpoint = nullptr;
                    if (!self->m_detachReported)
                    {
                      self->m_detachReported = true;
                      self->m_detached.set_value();
                    }
                    return;
                  }
                  if (is_flow_type_by_descriptor(amqpvalue_get_inplace_descriptor(frame))
                      && !self->m_flowReported)
                  {
                    self->m_flowReported = true;
                    self->m_credit.set_value();
                  }
                },
                nullptr,
                nullptr,
                this));
        auto attach = attach_create(linkName.c_str(), 0, role_sender);
        auto peerSource = source_create();
        auto address = amqpvalue_create_string("receiver-open");
        EXPECT_EQ(0, source_set_address(peerSource, address));
        auto sourceValue = m_acceptFirst && count == 1 ? amqpvalue_create_source(peerSource)
                                                       : amqpvalue_create_null();
        EXPECT_EQ(0, attach_set_source(attach, sourceValue));
        EXPECT_EQ(0, attach_set_initial_delivery_count(attach, 0));
        EXPECT_EQ(0, session_send_attach(endpoint, attach));
        attach_destroy(attach);
        amqpvalue_destroy(sourceValue);
        amqpvalue_destroy(address);
        source_destroy(peerSource);
        if (m_acceptFirst && count == 1)
        {
          Detach = [this, session, endpoint]() {
            auto lock = Amqp::_detail::SessionFactory::GetImpl(session)->GetConnection()->Lock();
            auto link = Amqp::_detail::LinkEndpointFactory::CreateLinkEndpoint(endpoint);
            RejectLinkAttachment(
                session,
                link,
                {Amqp::Models::_internal::AmqpErrorCondition::LinkDetachForced,
                 "reattach requested",
                 {}});
          };
          SendEvent = [this, session, endpoint](Core::Context const& context) {
            ASSERT_EQ(
                std::future_status::ready, m_credit.get_future().wait_until(context.GetDeadline()));
            Amqp::Models::AmqpMessage message;
            message.SetBody(Amqp::Models::AmqpBinaryData{1, 2, 3});
            auto bytes = Amqp::Models::AmqpMessage::Serialize(message);
            auto lock = Amqp::_detail::SessionFactory::GetImpl(session)->GetConnection()->Lock();
            auto transfer = transfer_create(0);
            unsigned char tag = 0;
            EXPECT_EQ(0, transfer_set_delivery_tag(transfer, {&tag, 1}));
            EXPECT_EQ(0, transfer_set_settled(transfer, true));
            PAYLOAD payload{bytes.data(), bytes.size()};
            delivery_number deliveryId;
            EXPECT_EQ(
                SESSION_SEND_TRANSFER_OK,
                session_send_transfer(
                    endpoint, transfer, &payload, 1, &deliveryId, nullptr, nullptr));
            transfer_destroy(transfer);
          };
          m_ready.set_value();
          return true;
        }
        RejectLinkAttachment(
            session,
            linkEndpoint,
            {Amqp::Models::_internal::AmqpErrorCondition{m_condition},
             "receiver attachment refused",
             {}});
        return true;
      }

    private:
      void MessageReceived(std::string const&, std::shared_ptr<Amqp::Models::AmqpMessage> const&)
          override
      {
      }
      LINK_ENDPOINT_HANDLE m_endpoint{};
      std::promise<void> m_detached;
      bool m_detachReported{false};
      std::promise<void> m_ready;
      std::promise<void> m_credit;
      bool m_flowReported{false};
      bool m_acceptFirst;
      std::string m_condition;
    };

    void CheckRefusal(std::function<void()> const& operation, std::string const& condition)
    {
      try
      {
        operation();
        ADD_FAILURE() << "The refused attachment succeeded.";
      }
      catch (EventHubsException const& ex)
      {
        GTEST_LOG_(INFO) << "Refusal condition: " << ex.ErrorCondition
                         << ", transient: " << ex.IsTransient;
        EXPECT_EQ(condition, ex.ErrorCondition);
        EXPECT_EQ("receiver attachment refused", ex.ErrorDescription);
        EXPECT_FALSE(ex.IsTransient);
      }
      catch (std::exception const& ex)
      {
        GTEST_LOG_(ERROR) << "Attachment did not translate to EventHubsException: " << ex.what();
        ADD_FAILURE() << "Attachment did not translate to EventHubsException: " << ex.what();
      }
    }

    void ReceiverRefusal(bool recovery, bool partial, std::string const& condition)
    {
      {
        auto port = []() {
          Amqp::Tests::CleanupTestListener listener;
          return listener.GetPort();
        }();
        Amqp::Tests::MessageTests::AmqpServerMock server(port);
        auto endpoint = std::make_shared<RefusingReceiverEndpoint>(recovery, condition);
        server.AddServiceEndpoint(endpoint);
        server.StartListening();
        {
          Amqp::_internal::ConnectionOptions options;
          options.Port = server.GetPort();
          Amqp::_internal::Connection connection("localhost", nullptr, options);
          auto session = connection.CreateSession();
          Core::Http::Policies::RetryOptions retryOptions;
          retryOptions.MaxRetries = 3;
          retryOptions.RetryDelay = std::chrono::milliseconds(0);
          retryOptions.MaxRetryDelay = std::chrono::milliseconds(0);
          auto context = Core::Context{}.WithDeadline(
              std::chrono::system_clock::now() + std::chrono::seconds(10));
          auto create = [&]() {
            return _detail::PartitionClientFactory::CreatePartitionClient(
                session,
                "receiver-open",
                "receiver",
                "client",
                "0",
                {},
                retryOptions,
                _detail::CreateTracingContextFactory(nullptr),
                "hub",
                "localhost",
                context);
          };
          if (!recovery)
          {
            CheckRefusal([&]() { auto client = create(); }, condition);
            EXPECT_EQ(1, endpoint->AttachCount);
          }
          else
          {
            auto client = create();
            endpoint->WaitUntilReady();
            if (partial)
            {
              endpoint->SendEvent(context);
            }
            endpoint->Detach();
            endpoint->WaitUntilDetached();
            {
              // The acknowledgment is sent before the client queues the error. Wait for that
              // polling callback to release the connection lock before consuming buffered events.
              auto lock = Amqp::_detail::SessionFactory::GetImpl(session)->GetConnection()->Lock();
            }
            if (partial)
            {
              EXPECT_EQ(1u, client.ReceiveEvents(2, context).size());
              EXPECT_EQ(2, endpoint->AttachCount);
            }
            CheckRefusal([&]() { client.ReceiveEvents(1, context); }, condition);
            EXPECT_EQ(2, endpoint->AttachCount);
            client.Close(context);
          }
        }
        server.StopListening();
      }
      Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
    }
  } // namespace

  class ReceiverRefusalDeathTest : public ::testing::Test {
  protected:
    void SetUp() override { ::testing::FLAGS_gtest_death_test_style = "threadsafe"; }
  };

  TEST_F(ReceiverRefusalDeathTest, InitialAttachmentPreservesCondition)
  {
    EXPECT_EXIT(
        {
          ReceiverRefusal(false, false, "amqp:not-allowed");
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(ReceiverRefusalDeathTest, ReplacementRefusalStopsRecovery)
  {
    EXPECT_EXIT(
        {
          ReceiverRefusal(true, false, "amqp:link:stolen");
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(ReceiverRefusalDeathTest, PartialReceivePreservesPermanentRefusal)
  {
    EXPECT_EXIT(
        {
          ReceiverRefusal(true, true, "amqp:not-allowed");
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }
}}}} // namespace Azure::Messaging::EventHubs::Test
#endif
