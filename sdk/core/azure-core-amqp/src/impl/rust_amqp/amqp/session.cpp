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

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
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
      : m_connectionToPoll(connection), m_options{options}

  {
    if (options.MaximumLinkCount.HasValue())
    {
    }
    if (options.InitialIncomingWindowSize.HasValue())
    {
    }
    if (options.InitialOutgoingWindowSize.HasValue())
    {
    }
  }

  uint32_t SessionImpl::GetIncomingWindow()
  {
    uint32_t window{};
    return window;
  }

  uint32_t SessionImpl::GetOutgoingWindow()
  {
    uint32_t window{};
    return window;
  }

  uint32_t SessionImpl::GetHandleMax()
  {
    uint32_t max{};
    return max;
  }

  void SessionImpl::Begin() { m_isBegun = true; }
  void SessionImpl::End(const std::string& condition, const std::string& description)
  {
    if (!m_isBegun)
    {
      throw std::runtime_error("Session End without corresponding Begin.");
    }
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
