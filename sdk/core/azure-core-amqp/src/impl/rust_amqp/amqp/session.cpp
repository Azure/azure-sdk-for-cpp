// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/session.hpp"

#include "../../../models/private/performatives/detach_impl.hpp"
#include "../../../models/private/value_impl.hpp"
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

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;
using namespace Azure::Core::Amqp::_detail::RustInterop;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  void UniqueHandleHelper<AmqpSessionImplementation>::FreeAmqpSession(
      AmqpSessionImplementation* value)
  {
    amqpsession_destroy(value);
  }
  void UniqueHandleHelper<AmqpSessionOptions>::FreeAmqpSessionOptions(AmqpSessionOptions* value)
  {
    amqpsessionoptions_destroy(value);
  }
  void UniqueHandleHelper<AmqpSessionOptionsBuilder>::FreeAmqpSessionOptionsBuilder(
      AmqpSessionOptionsBuilder* value)
  {
    amqpsessionoptionsbuilder_destroy(value);
  }

}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

}}}} // namespace Azure::Core::Amqp::_internal

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  SessionImpl::~SessionImpl() noexcept
  {
    if (m_isBegun)
    {
      AZURE_ASSERT_MSG(false, "Session was not ended before destruction.");
    }
  }

  SessionImpl::SessionImpl(
      std::shared_ptr<_detail::ConnectionImpl> connection,
      _internal::SessionOptions const& options)
      : m_session{amqpsession_create()}, m_connection(connection), m_options{options}

  {
  }

  uint32_t SessionImpl::GetIncomingWindow()
  {
    if (m_options.InitialIncomingWindowSize.HasValue())
    {
      return m_options.InitialIncomingWindowSize.Value();
    }
    else
    {
      return std::numeric_limits<std::uint32_t>::max();
    }
  }

  uint32_t SessionImpl::GetOutgoingWindow()
  {
    if (m_options.InitialOutgoingWindowSize.HasValue())
    {
      return m_options.InitialOutgoingWindowSize.Value();
    }
    else
    {
      return 1;
    }
  }

  uint32_t SessionImpl::GetHandleMax()
  {
    if (m_options.MaximumLinkCount.HasValue())
    {
      return m_options.MaximumLinkCount.Value();
    }
    else
    {
      return std::numeric_limits<uint32_t>::max();
    }
  }

  void SessionImpl::Begin()
  {
    UniqueAmqpSessionOptionsBuilder optionsBuilder{amqpsessionoptionsbuilder_create()};

    if (m_options.MaximumLinkCount.HasValue())
    {
      amqpsessionoptionsbuilder_set_handle_max(
          optionsBuilder.get(), m_options.MaximumLinkCount.Value());
    }
    if (m_options.InitialIncomingWindowSize.HasValue())
    {
      amqpsessionoptionsbuilder_set_incoming_window(
          optionsBuilder.get(), m_options.InitialIncomingWindowSize.Value());
    }
    if (m_options.InitialOutgoingWindowSize.HasValue())
    {
      amqpsessionoptionsbuilder_set_outgoing_window(
          optionsBuilder.get(), m_options.InitialOutgoingWindowSize.Value());
    }
    if (!m_options.DesiredCapabilities.empty())
    {
    }
    UniqueAmqpSessionOptions sessionOptions{amqpsessionoptionsbuilder_build(optionsBuilder.get())};
    if (amqpsession_begin(
            m_session.get(),
            m_connection->GetConnection(),
            sessionOptions.get(),
            Common::_detail::GlobalStateHolder::GlobalStateInstance()->GetRuntimeContext()))
    {
      throw std::runtime_error("Failed to begin session.");
    }
    m_isBegun = true;
  }
  void SessionImpl::End(const std::string& condition, const std::string& description)
  {
    if (!m_isBegun)
    {
      throw std::runtime_error("Session End without corresponding Begin.");
    }
    if (amqpsession_end(
            m_session.get(),
            Common::_detail::GlobalStateHolder::GlobalStateInstance()->GetRuntimeContext()))
    {
      throw std::runtime_error("Failed to end session.");
    }
    m_isBegun = false;
    (void)condition;
    (void)description;
  }

  void SessionImpl::SendDetach(
      _internal::LinkEndpoint const& linkEndpoint,
      bool closeLink,
      Models::_internal::AmqpError const& error) const
  {
    Models::_internal::Performatives::AmqpDetach detach;

    detach.Closed = closeLink;
    detach.Error = error;
    (void)linkEndpoint;
  }

}}}} // namespace Azure::Core::Amqp::_detail
