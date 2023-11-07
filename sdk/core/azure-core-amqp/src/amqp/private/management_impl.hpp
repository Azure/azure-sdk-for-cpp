// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/connection.hpp"
#include "azure/core/amqp/internal/management.hpp"
#include "azure/core/amqp/internal/session.hpp"
#include "connection_impl.hpp"
#include "message_receiver_impl.hpp"
#include "message_sender_impl.hpp"
#include "session_impl.hpp"

#include <azure/core/credentials/credentials.hpp>

#include <azure_uamqp_c/amqp_management.h>

#include <memory>
#include <queue>
#include <vector>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  class ManagementClientFactory final {
  public:
    static Azure::Core::Amqp::_internal::ManagementClient CreateFromInternal(
        std::shared_ptr<ManagementClientImpl> clientImpl)
    {
      return Azure::Core::Amqp::_internal::ManagementClient(clientImpl);
    }

    static std::shared_ptr<ManagementClientImpl> GetImpl(
        Azure::Core::Amqp::_internal::ManagementClient const& client)
    {
      return client.m_impl;
    }
  };

  class ManagementClientImpl final : public ::std::enable_shared_from_this<ManagementClientImpl>,
                                     public _internal::MessageSenderEvents,
                                     public _internal::MessageReceiverEvents {
  public:
    ManagementClientImpl(
        std::shared_ptr<SessionImpl> session,
        std::string const& managementEntityName,
        _internal::ManagementClientOptions const& options,
        _internal::ManagementClientEvents* managementEvents);

    ~ManagementClientImpl() noexcept;

    /**
     * @brief Open the management instance.
     *
     * @returns A tuple consisting of the status code for the open and the description of the
     * status.
     */
    _internal::ManagementOpenStatus Open(Context const& context = {});

    /**
     * @brief Close the management instance.
     */
    void Close();

    _internal::ManagementOperationResult ExecuteOperation(
        std::string const& operationToPerform,
        std::string const& typeOfOperation,
        std::string const& locales,
        Models::AmqpMessage messageToSend,
        Context const& context);

  private:
    enum class ManagementState
    {
      Idle,
      Opening,
      Closing,
      Open,
      Error
    };

    std::shared_ptr<MessageSenderImpl> m_messageSender;
    std::shared_ptr<MessageReceiverImpl> m_messageReceiver;
    ManagementState m_state = ManagementState::Idle;
    bool m_isOpen{false};
    bool m_messageSenderOpen{false};
    bool m_messageReceiverOpen{false};
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<_internal::ManagementOpenStatus>
        m_openCompleteQueue;

    uint64_t m_nextMessageId{0};

    // What is the message ID expected for the current outstanding operation?
    uint64_t m_expectedMessageId{};
    bool m_sendCompleted{false};

    void SetState(ManagementState newState);
    // Reflect the error state to the OnError callback and return a delivery rejected status.
    Models::AmqpValue IndicateError(
        std::string const& errorCondition,
        std::string const& errorDescription);

    _internal::ManagementClientOptions m_options;
    std::string m_source;
    std::shared_ptr<SessionImpl> m_session;
    _internal::ManagementClientEvents* m_eventHandler{};
    std::string m_managementEntityPath;
    Azure::Core::Credentials::AccessToken m_accessToken;

    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
        _internal::ManagementOperationStatus,
        std::uint32_t,
        Models::_internal::AmqpError,
        Models::AmqpMessage>
        m_messageQueue;

    // Inherited via MessageSenderEvents
    virtual void OnMessageSenderStateChanged(
        _internal::MessageSender const& sender,
        _internal::MessageSenderState newState,
        _internal::MessageSenderState oldState) override;
    virtual void OnMessageSenderDisconnected(Models::_internal::AmqpError const& error) override;

    // Inherited via MessageReceiverEvents
    virtual void OnMessageReceiverStateChanged(
        _internal::MessageReceiver const& receiver,
        _internal::MessageReceiverState newState,
        _internal::MessageReceiverState oldState) override;
    virtual Models::AmqpValue OnMessageReceived(
        _internal::MessageReceiver const& receiver,
        Models::AmqpMessage const& message) override;
    virtual void OnMessageReceiverDisconnected(Models::_internal::AmqpError const& error) override;
  };
}}}} // namespace Azure::Core::Amqp::_detail
