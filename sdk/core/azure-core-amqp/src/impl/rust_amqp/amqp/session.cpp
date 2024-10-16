// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words amqpsession amqpsessionoptions amqpsessionoptionsbuilder

#include "azure/core/amqp/internal/session.hpp"

#include "private/connection_impl.hpp"
#include "private/session_impl.hpp"

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
    if (!connection->IsOpen())
    {
      throw std::runtime_error("Cannot create session on unopened connection.");
    }
  }

  uint32_t SessionImpl::GetIncomingWindow()
  {
    if (m_options.InitialIncomingWindowSize.HasValue())
    {
      return m_options.InitialIncomingWindowSize.Value();
    }
    else
    {
      return 1;
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

  void SessionImpl::Begin(Azure::Core::Context const& context)
  {
    Azure::Core::Amqp::Common::_detail::CallContext callContext{
        Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()
            ->GetRuntimeContext(),
        context};

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
            callContext.GetCallContext(),
            m_session.get(),
            m_connection->GetConnection(),
            sessionOptions.get()))
    {
      throw std::runtime_error("Failed to begin session." + callContext.GetError());
    }
    m_isBegun = true;
  }

  void SessionImpl::End(Azure::Core::Context const& context)
  {
    Azure::Core::Amqp::Common::_detail::CallContext callContext{
        Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()
            ->GetRuntimeContext(),
        context};

    if (!m_isBegun)
    {
      throw std::runtime_error("Session End without corresponding Begin.");
    }
    if (m_session)
    {
      // We always say we're no longer begun even if the end fails.
      m_isBegun = false;
      if (amqpsession_end(callContext.GetCallContext(), m_session.get()))
      {
        throw std::runtime_error("Failed to end session." + callContext.GetError());
      }
    }
    else
    {
      Log::Stream(Logger::Level::Informational) << "Ending an already ended session.";
    }
  }

  void SessionImpl::End(
      const std::string& condition,
      const std::string& description,
      Azure::Core::Context const& context)
  {
    Azure::Core::Amqp::Common::_detail::CallContext callContext{
        Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()
            ->GetRuntimeContext(),
        context};

    if (!m_isBegun)
    {
      throw std::runtime_error("Session End without corresponding Begin.");
    }

    m_isBegun = false;
    if (amqpsession_end(callContext.GetCallContext(), m_session.get()))
    {
      throw std::runtime_error("Failed to end session." + callContext.GetError());
    }
    (void)condition;
    (void)description;
  }

}}}} // namespace Azure::Core::Amqp::_detail
