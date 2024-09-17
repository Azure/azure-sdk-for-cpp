// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/session.hpp"

#include "../models/private/performatives/detach_impl.hpp"
#include "../models/private/value_impl.hpp"
#include "azure/core/amqp/internal/connection.hpp"
#include "azure/core/amqp/internal/link.hpp"
#include "azure/core/amqp/internal/models/performatives/amqp_detach.hpp"
#include "private/claims_based_security_impl.hpp"
#include "private/connection_impl.hpp"
#include "private/management_impl.hpp"
#include "private/message_receiver_impl.hpp"
#include "private/message_sender_impl.hpp"
#include "private/session_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#if ENABLE_UAMQP
#include <azure_uamqp_c/session.h>
#endif

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  void UniqueHandleHelper<SESSION_INSTANCE_TAG>::FreeAmqpSession(SESSION_HANDLE value)
  {
    session_destroy(value);
  }
#endif
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _internal {
  Endpoint::~Endpoint()
  {
#if ENABLE_UAMQP
    if (m_endpoint)
    {
      connection_destroy_endpoint(m_endpoint);
    }
#endif
  }

  Session::~Session() noexcept {}

  uint32_t Session::GetIncomingWindow() const { return m_impl->GetIncomingWindow(); }
  uint32_t Session::GetOutgoingWindow() const { return m_impl->GetOutgoingWindow(); }
  uint32_t Session::GetHandleMax() const { return m_impl->GetHandleMax(); }

  void Session::Begin() { m_impl->Begin(); }
  void Session::End(std::string const& condition_value, std::string const& description)
  {
    m_impl->End(condition_value, description);
  }
  void Session::SendDetach(
      _internal::LinkEndpoint const& linkEndpoint,
      bool closeLink,
      Models::_internal::AmqpError const& error) const
  {
    m_impl->SendDetach(linkEndpoint, closeLink, error);
  }

#if ENABLE_UAMQP
  MessageSender Session::CreateMessageSender(
      Models::_internal::MessageTarget const& target,
      MessageSenderOptions const& options,
      MessageSenderEvents* events) const
#elif ENABLE_RUST_AMQP
  MessageSender Session::CreateMessageSender(
      Models::_internal::MessageTarget const& target,
      MessageSenderOptions const& options) const
#endif
  {
#if ENABLE_UAMQP
    return _detail::MessageSenderFactory::CreateFromInternal(
        std::make_shared<_detail::MessageSenderImpl>(m_impl, target, options, events));
#elif ENABLE_RUST_AMQP
    return _detail::MessageSenderFactory::CreateFromInternal(
        std::make_shared<_detail::MessageSenderImpl>(m_impl, target, options));
#endif
  }
#if ENABLE_UAMQP
  MessageSender Session::CreateMessageSender(
      LinkEndpoint& endpoint,
      Models::_internal::MessageTarget const& target,
      MessageSenderOptions const& options,
      MessageSenderEvents* events) const
  {
    return _detail::MessageSenderFactory::CreateFromInternal(
        std::make_shared<_detail::MessageSenderImpl>(m_impl, endpoint, target, options, events));
  }
#endif

  MessageReceiver Session::CreateMessageReceiver(
      Models::_internal::MessageSource const& receiverSource,
      MessageReceiverOptions const& options,
      MessageReceiverEvents* events) const
  {
    return _detail::MessageReceiverFactory::CreateFromInternal(
        std::make_shared<_detail::MessageReceiverImpl>(m_impl, receiverSource, options, events));
  }

  MessageReceiver Session::CreateMessageReceiver(
      LinkEndpoint& endpoint,
      Models::_internal::MessageSource const& receiverSource,
      MessageReceiverOptions const& options,
      MessageReceiverEvents* events) const
  {
    return _detail::MessageReceiverFactory::CreateFromInternal(
        std::make_shared<_detail::MessageReceiverImpl>(
            m_impl, endpoint, receiverSource, options, events));
  }

  ManagementClient Session::CreateManagementClient(
      std::string const& entityPath,
      ManagementClientOptions const& options,
      ManagementClientEvents* events) const
  {
    return _detail::ManagementClientFactory::CreateFromInternal(
        std::make_shared<_detail::ManagementClientImpl>(m_impl, entityPath, options, events));
  }

}}}} // namespace Azure::Core::Amqp::_internal

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  SessionImpl::~SessionImpl() noexcept
  {
    if (m_isBegun)
    {
      AZURE_ASSERT_MSG(false, "Session was not ended before destruction.");
    }

    // If we have a mismatched begin/end pair, we need to stop polling on the connection so it
    // gets cleaned up properly.
#if ENABLE_UAMQP
    if (m_connectionAsyncStarted)
    {
      m_connectionToPoll->EnableAsyncOperation(false);
    }
    auto lock{m_connectionToPoll->Lock()};
    m_session.reset();
#endif
  }

#if ENABLE_UAMQP
  SessionImpl::SessionImpl(
      std::shared_ptr<_detail::ConnectionImpl> connection,
      _internal::Endpoint& endpoint,
      _internal::SessionOptions const& options,
      _internal::SessionEvents* eventHandler)
      : m_connectionToPoll(connection), m_session{session_create_from_endpoint(
                                            *connection,
                                            EndpointFactory::Release(endpoint),
                                            SessionImpl::OnLinkAttachedFn,
                                            this)},
        m_options{options}, m_eventHandler{eventHandler}
  {
    if (options.MaximumLinkCount.HasValue())
    {
      if (session_set_handle_max(m_session.get(), options.MaximumLinkCount.Value()))
      {
        throw std::runtime_error("Could not set handle max.");
      }
    }
    if (options.InitialIncomingWindowSize.HasValue())
    {
      if (session_set_incoming_window(m_session.get(), options.InitialIncomingWindowSize.Value()))
      {
        throw std::runtime_error("Could not set incoming window");
      }
    }
    if (options.InitialOutgoingWindowSize.HasValue())
    {
      if (session_set_outgoing_window(m_session.get(), options.InitialOutgoingWindowSize.Value()))
      {
        throw std::runtime_error("Could not set outgoing window");
      }
    }
  }
#endif

  SessionImpl::SessionImpl(
      std::shared_ptr<_detail::ConnectionImpl> connection,
      _internal::SessionOptions const& options
#if ENABLE_UAMQP
      ,
      _internal::SessionEvents* eventHandler
#endif
      )
      : m_connectionToPoll(connection),
#if ENABLE_UAMQP
        m_session{session_create(*connection, SessionImpl::OnLinkAttachedFn, this)},
#endif
        m_options
  {
    options
  }
#if ENABLE_UAMQP
  , m_eventHandler { eventHandler }
#endif

  {
#if ENABLE_UAMQP
    if (!m_session)
    {
      throw std::runtime_error("Could not create session.");
    }
#endif

    if (options.MaximumLinkCount.HasValue())
    {
#if ENABLE_UAMQP
      if (session_set_handle_max(m_session.get(), options.MaximumLinkCount.Value()))
      {
        throw std::runtime_error("Could not set handle max.");
      }
#endif
    }
    if (options.InitialIncomingWindowSize.HasValue())
    {
#if ENABLE_UAMQP
      if (session_set_incoming_window(m_session.get(), options.InitialIncomingWindowSize.Value()))
      {
        throw std::runtime_error("Could not set incoming window");
      }
#endif
    }
    if (options.InitialOutgoingWindowSize.HasValue())
    {
#if ENABLE_UAMQP
      if (session_set_outgoing_window(m_session.get(), options.InitialOutgoingWindowSize.Value()))
      {
        throw std::runtime_error("Could not set outgoing window");
      }
#endif
    }
  }

  uint32_t SessionImpl::GetIncomingWindow()
  {
    uint32_t window{};
#if ENABLE_UAMQP
    if (session_get_incoming_window(m_session.get(), &window))
    {
      throw std::runtime_error("Could not get incoming window");
    }
#endif
    return window;
  }

  uint32_t SessionImpl::GetOutgoingWindow()
  {
    uint32_t window{};
#if ENABLE_UAMQP
    if (session_get_outgoing_window(m_session.get(), &window))
    {
      throw std::runtime_error("Could not get outgoing window");
    }
#endif
    return window;
  }

  uint32_t SessionImpl::GetHandleMax()
  {
    uint32_t max{};
#if ENABLE_UAMQP
    if (session_get_handle_max(m_session.get(), &max))
    {
      throw std::runtime_error("Could not get handle max.");
    }
#endif
    return max;
  }

  void SessionImpl::Begin()
  {
#if ENABLE_UAMQP
    if (session_begin(m_session.get()))
    {
      throw std::runtime_error("Could not begin session");
    }
#endif

    m_isBegun = true;

// Mark the connection as async so that we can use the async APIs.
#if ENABLE_UAMQP
        GetConnection()->EnableAsyncOperation(true);
        m_connectionAsyncStarted = true;
#endif
      }
      void SessionImpl::End(const std::string& condition, const std::string& description)
      {
        if (!m_isBegun)
        {
          throw std::runtime_error("Session End without corresponding Begin.");
        }
#if ENABLE_UAMQP
        // When we end the session, it clears all the links, so we need to ensure that the
        //    m_newLinkAttachedQueue.Clear();
        if (session_end(
                m_session.get(),
                condition.empty() ? nullptr : condition.c_str(),
                description.empty() ? nullptr : description.c_str()))
        {
          throw std::runtime_error("Could not begin session");
        }
        // Mark the connection as async so that we can use the async APIs.
        GetConnection()->EnableAsyncOperation(false);
        m_connectionAsyncStarted = false;
        m_isBegun = false;
#else
        (void)condition;
        (void)description;
#endif
      }

      void SessionImpl::SendDetach(
          _internal::LinkEndpoint const& linkEndpoint,
          bool closeLink,
          Models::_internal::AmqpError const& error) const
      {
        Models::_internal::Performatives::AmqpDetach detach;

        detach.Closed = closeLink;
        detach.Error = error;
#if ENABLE_UAMQP
        if (session_send_detach(
                linkEndpoint.Get(), Models::_detail::AmqpDetachFactory::ToAmqpDetach(detach).get()))
        {
          throw std::runtime_error("Failed to send detach performative.");
        }
#else
        (void)linkEndpoint;
#endif
      }

#if ENABLE_UAMQP
      bool SessionImpl::OnLinkAttachedFn(
          void* context,
          LINK_ENDPOINT_INSTANCE_TAG* newLinkEndpoint,
          const char* name,
          bool role,
          AMQP_VALUE_DATA_TAG* source,
          AMQP_VALUE_DATA_TAG* target,
          AMQP_VALUE_DATA_TAG* properties)
      {
        SessionImpl* session = static_cast<SessionImpl*>(context);
        _internal::LinkEndpoint linkEndpoint(
            LinkEndpointFactory::CreateLinkEndpoint(newLinkEndpoint));
        if (session->m_eventHandler)
        {
          // The input source, target, and properties are owned by the caller, so we need to clone
          // them before putting them in a UniqueAmqpValueHandle so we can construct an AmqpValue.
          return session->m_eventHandler->OnLinkAttached(
              _detail::SessionFactory::CreateFromInternal(session->shared_from_this()),
              linkEndpoint,
              name,
              role == role_receiver ? Azure::Core::Amqp::_internal::SessionRole::Receiver
                                    : Azure::Core::Amqp::_internal::SessionRole::Sender,
              Models::_detail::AmqpValueFactory::FromImplementation(
                  Models::_detail::UniqueAmqpValueHandle{amqpvalue_clone(source)}),
              Models::_detail::AmqpValueFactory::FromImplementation(
                  Models::_detail::UniqueAmqpValueHandle{amqpvalue_clone(target)}),
              Models::_detail::AmqpValueFactory::FromImplementation(
                  Models::_detail::UniqueAmqpValueHandle{amqpvalue_clone(properties)}));
        }
        else
        {
          // Even if we don't have any actions to take, if we return false, the connection will be
          // aborted.
          return true;
        }
        static_cast<void>(role);
      }

      _internal::Endpoint EndpointFactory::CreateEndpoint(ENDPOINT_HANDLE endpoint)
      {
        return _internal::Endpoint(endpoint);
      }
      _internal::LinkEndpoint LinkEndpointFactory::CreateLinkEndpoint(LINK_ENDPOINT_HANDLE endpoint)
      {
        return _internal::LinkEndpoint(endpoint);
      }
#endif

}}}} // namespace Azure::Core::Amqp::_detail
