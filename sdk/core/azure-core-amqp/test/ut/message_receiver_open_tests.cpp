// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#if ENABLE_UAMQP
#include "../../src/impl/uamqp/amqp/private/session_impl.hpp"
#include "cleanup_test_listener.hpp"
#include "mock_amqp_server.hpp"

#include <azure/core/amqp/internal/message_receiver.hpp>

#include <azure_uamqp_c/amqp_definitions.h>

#include <chrono>
#include <cstdlib>
#include <functional>
#include <future>
#include <thread>

#include <gtest/gtest.h>

namespace Azure { namespace Core { namespace Amqp { namespace Tests {
  namespace {
    using namespace Azure::Core::Amqp::_internal;

    class ControlledReceiverEndpoint final : public MessageTests::MockServiceEndpoint {
    public:
      ControlledReceiverEndpoint(
          MessageTests::MockServiceEndpointOptions const& options,
          std::shared_future<void> release,
          bool reject)
          : MockServiceEndpoint("receiver-open", options), m_release(std::move(release)),
            m_reject(reject)
      {
      }

      std::future<void> AttachReceived() { return m_attachReceived.get_future(); }
      std::future<std::function<void()>> RejectionReady() { return m_rejectionReady.get_future(); }

      bool OnLinkAttached(
          Session const& session,
          std::string const& linkName,
          LinkEndpoint& linkEndpoint,
          SessionRole role,
          Models::_internal::MessageSource const& source,
          Models::_internal::MessageTarget const& target) override
      {
        if (m_reject && m_attachReported)
        {
          return true;
        }
        if (!m_attachReported)
        {
          m_attachReceived.set_value();
          m_attachReported = true;
        }
        if (m_release.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
        {
          return false;
        }
        if (m_reject)
        {
          // Refuse the source according to AMQP 1.0 section 2.6.3: first ATTACH with a null
          // source, then DETACH with the error. Keep the target present to catch false OPENs.
          EXPECT_EQ(
              0,
              session_start_link_endpoint(
                  linkEndpoint.Get(),
                  [](void*, AMQP_VALUE, std::uint32_t, unsigned char const*) {},
                  nullptr,
                  nullptr,
                  nullptr));
          auto attach = attach_create(linkName.c_str(), 0, role_sender);
          auto nullSource = amqpvalue_create_null();
          auto peerTarget = target_create();
          auto targetValue = amqpvalue_create_target(peerTarget);
          EXPECT_EQ(0, attach_set_source(attach, nullSource));
          EXPECT_EQ(0, attach_set_target(attach, targetValue));
          EXPECT_EQ(0, attach_set_initial_delivery_count(attach, 0));
          EXPECT_EQ(0, session_send_attach(linkEndpoint.Get(), attach));
          attach_destroy(attach);
          amqpvalue_destroy(nullSource);
          amqpvalue_destroy(targetValue);
          target_destroy(peerTarget);
          auto endpoint = linkEndpoint.Get();
          m_rejectionReady.set_value([this, session, endpoint]() {
            auto lock = _detail::SessionFactory::GetImpl(session)->GetConnection()->Lock();
            auto rejectedEndpoint = _detail::LinkEndpointFactory::CreateLinkEndpoint(endpoint);
            RejectLinkAttachment(
                session,
                rejectedEndpoint,
                {Models::_internal::AmqpErrorCondition::NotAllowed,
                 "receiver attachment rejected by test peer",
                 {}});
          });
          return true;
        }
        return MockServiceEndpoint::OnLinkAttached(
            session, linkName, linkEndpoint, role, source, target);
      }

    private:
      void MessageReceived(std::string const&, std::shared_ptr<Models::AmqpMessage> const&) override
      {
      }

      std::promise<void> m_attachReceived;
      std::promise<std::function<void()>> m_rejectionReady;
      bool m_attachReported{false};
      std::shared_future<void> m_release;
      bool m_reject;
    };

    void OpenWithPeerResponse(bool reject)
    {
      {
        MessageTests::AmqpServerMock server;
        MessageTests::MockServiceEndpointOptions endpointOptions;
        endpointOptions.EnableTrace = false;
        endpointOptions.ListenerContext = server.GetListenerContext();
        std::promise<void> release;
        auto endpoint = std::make_shared<ControlledReceiverEndpoint>(
            endpointOptions, release.get_future().share(), reject);
        auto attached = endpoint->AttachReceived();
        auto rejection = endpoint->RejectionReady();
        server.AddServiceEndpoint(endpoint);
        server.StartListening();

        ConnectionOptions connectionOptions;
        connectionOptions.Port = server.GetPort();
        Connection connection("localhost", nullptr, connectionOptions);
        auto session = connection.CreateSession();
        MessageReceiverOptions receiverOptions;
        receiverOptions.Name = "controlled-receiver";
        receiverOptions.MessageTarget = Models::_internal::MessageTarget("receiver-target");
        receiverOptions.AuthenticationRequired = false;
        receiverOptions.WaitForOpen = true;
        auto receiver = session.CreateMessageReceiver("receiver-open", receiverOptions);
        auto context
            = Context{}.WithDeadline(std::chrono::system_clock::now() + std::chrono::seconds(10));
        auto opening = std::async(std::launch::async, [&]() { receiver.Open(context); });
        auto const attachStatus = attached.wait_for(std::chrono::seconds(5));
        auto const openStatus = opening.wait_for(std::chrono::milliseconds(100));
        release.set_value();
        EXPECT_EQ(std::future_status::ready, attachStatus);
        EXPECT_EQ(std::future_status::timeout, openStatus);
        if (reject)
        {
          if (rejection.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
          {
            context.Cancel();
            ADD_FAILURE() << "The peer did not send the refusal ATTACH.";
          }
          else
          {
            auto refusalStatus = opening.wait_for(std::chrono::milliseconds(100));
            if (refusalStatus != std::future_status::timeout)
            {
              GTEST_LOG_(ERROR)
                  << "Open completed after a null-source refusal ATTACH, before DETACH.";
            }
            EXPECT_EQ(std::future_status::timeout, refusalStatus);
            rejection.get()();
          }
          try
          {
            opening.get();
            GTEST_LOG_(ERROR) << "The rejected receiver attachment succeeded.";
            ADD_FAILURE() << "The rejected receiver attachment succeeded.";
            receiver.Close(context);
          }
          catch (std::runtime_error const& ex)
          {
            EXPECT_NE(std::string::npos, std::string(ex.what()).find("rejected by test peer"));
          }
        }
        else
        {
          EXPECT_NO_THROW(opening.get());
          EXPECT_NO_THROW(receiver.Close());
        }
        // A failed Open must be safe to destroy without an explicit Close.
        server.StopListening();
      }
      Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
    }

    void OpenWithoutPeerResponse(bool cancel, bool waitForOpen)
    {
      {
        CleanupTestListener server;
        ConnectionOptions connectionOptions;
        connectionOptions.Port = server.GetPort();
        Connection connection("localhost", nullptr, connectionOptions);
        auto session = connection.CreateSession();
        MessageReceiverOptions receiverOptions;
        receiverOptions.AuthenticationRequired = false;
        receiverOptions.WaitForOpen = waitForOpen;
        auto receiver = session.CreateMessageReceiver("receiver-open", receiverOptions);
        auto context = Context{}.WithDeadline(
            std::chrono::system_clock::now() + std::chrono::milliseconds(cancel ? 5000 : 300));
        auto opening = std::async(std::launch::async, [&]() { receiver.Open(context); });
        if (waitForOpen)
        {
          EXPECT_EQ(std::future_status::timeout, opening.wait_for(std::chrono::milliseconds(100)));
          if (cancel)
          {
            context.Cancel();
          }
          EXPECT_THROW(opening.get(), Azure::Core::OperationCancelledException);
        }
        else
        {
          EXPECT_EQ(std::future_status::ready, opening.wait_for(std::chrono::seconds(2)));
          EXPECT_NO_THROW(opening.get());
          EXPECT_NO_THROW(receiver.Close());
        }
      }
      Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
    }
  } // namespace

  class ReceiverOpenDeathTest : public ::testing::Test {
  protected:
    void SetUp() override { ::testing::FLAGS_gtest_death_test_style = "threadsafe"; }
  };

  TEST_F(ReceiverOpenDeathTest, WaitsForConfirmedAttachment)
  {
    EXPECT_EXIT(
        {
          OpenWithPeerResponse(false);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(ReceiverOpenDeathTest, ReportsRejectedAttachment)
  {
    EXPECT_EXIT(
        {
          OpenWithPeerResponse(true);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(ReceiverOpenDeathTest, CancellationCleansUpPendingAttachment)
  {
    EXPECT_EXIT(
        {
          OpenWithoutPeerResponse(true, true);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(ReceiverOpenDeathTest, DeadlineCleansUpPendingAttachment)
  {
    EXPECT_EXIT(
        {
          OpenWithoutPeerResponse(false, true);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(ReceiverOpenDeathTest, DefaultOpenRemainsAsynchronous)
  {
    EXPECT_EXIT(
        {
          OpenWithoutPeerResponse(false, false);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }
}}}} // namespace Azure::Core::Amqp::Tests
#endif
