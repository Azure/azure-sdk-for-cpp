// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/session.hpp"

#include "../models/private/performatives/detach_impl.hpp"
#include "../models/private/value_impl.hpp"
#include "azure/core/amqp/internal/connection.hpp"
#include "azure/core/amqp/internal/link.hpp"
#include "azure/core/amqp/internal/models/performatives/amqp_detach.hpp"
#include "claims_based_security_impl.hpp"
#include "connection_impl.hpp"
#include "management_impl.hpp"
#include "message_receiver_impl.hpp"
#include "message_sender_impl.hpp"
#include "session_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

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
