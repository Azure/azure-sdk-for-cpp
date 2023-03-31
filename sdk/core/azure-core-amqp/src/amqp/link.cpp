// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/link.hpp"
#include "azure/core/amqp/message_receiver.hpp"
#include "azure/core/amqp/message_sender.hpp"
#include "azure/core/amqp/models/messaging_values.hpp"
#include "azure/core/amqp/private/link_impl.hpp"
#include "azure/core/amqp/private/session_impl.hpp"

#include <azure_uamqp_c/amqp_definitions_sequence_no.h>

#include <azure_uamqp_c/link.h>
#include <azure_uamqp_c/session.h>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  Link::Link(
      _internal::Session const& session,
      std::string const& name,
      Azure::Core::Amqp::_internal::SessionRole role,
      std::string const& source,
      std::string const& target)
      : m_impl{std::make_shared<LinkImpl>(session, name, role, source, target)}
  {
  }

  Link::Link(
      _internal::Session const& session,
      _internal::LinkEndpoint& linkEndpoint,
      std::string const& name,
      _internal::SessionRole role,
      std::string const& source,
      std::string const& target)
      : m_impl{std::make_shared<LinkImpl>(session, linkEndpoint, name, role, source, target)}
  {
  }

  Link::~Link() noexcept {}

  Link::operator LINK_HANDLE() const { return m_impl->operator LINK_HANDLE(); }
  std::string const& Link::GetSource() const { return m_impl->GetSource(); }
  std::string const& Link::GetTarget() const { return m_impl->GetTarget(); }
  _internal::SenderSettleMode Link::GetSenderSettleMode() const
  {
    return m_impl->GetSenderSettleMode();
  }
  void Link::SetSenderSettleMode(_internal::SenderSettleMode mode)
  {
    m_impl->SetSenderSettleMode(mode);
  }
  _internal::ReceiverSettleMode Link::GetReceiverSettleMode() const
  {
    return m_impl->GetReceiverSettleMode();
  }
  void Link::SetReceiverSettleMode(_internal::ReceiverSettleMode mode)
  {
    m_impl->SetReceiverSettleMode(mode);
  }
  void Link::SetInitialDeliveryCount(uint32_t initialDeliveryCount)
  {
    m_impl->SetInitialDeliveryCount(initialDeliveryCount);
  }
  uint32_t Link::GetInitialDeliveryCount() const { return m_impl->GetInitialDeliveryCount(); }
  void Link::SetMaxMessageSize(uint64_t maxMessageSize)
  {
    m_impl->SetMaxMessageSize(maxMessageSize);
  }
  uint64_t Link::GetMaxMessageSize() const { return m_impl->GetMaxMessageSize(); }
  uint64_t Link::GetPeerMaxMessageSize() const { return m_impl->GetPeerMaxMessageSize(); }
  void Link::SetAttachProperties(Azure::Core::Amqp::Models::AmqpValue attachProperties)
  {
    m_impl->SetAttachProperties(attachProperties);
  }
  void Link::SetMaxLinkCredit(uint32_t credit) { m_impl->SetMaxLinkCredit(credit); }
  std::string Link::GetName() const { return m_impl->GetName(); }

  uint32_t Link::GetReceivedMessageId() const { return m_impl->GetReceivedMessageId(); }

  void Link::Attach() { return m_impl->Attach(); }

  void Link::Detach(
      bool close,
      std::string const& errorCondition,
      std::string const& errorDescription,
      Azure::Core::Amqp::Models::AmqpValue& info)
  {
    return m_impl->Detach(close, errorCondition, errorDescription, info);
  }

  /****/
  /* LINK Implementation */

  LinkImpl::LinkImpl(
      _internal::Session const& session,
      std::string const& name,
      _internal::SessionRole role,
      std::string const& source,
      std::string const& target)
      : m_session{session}, m_source(source), m_target(target)
  {
    m_link = link_create(
        *session.GetImpl(),
        name.c_str(),
        role == _internal::SessionRole::Sender ? role_sender : role_receiver,
        Azure::Core::Amqp::Models::_internal::Messaging::CreateSource(source),
        Azure::Core::Amqp::Models::_internal::Messaging::CreateTarget(target));
  }

  LinkImpl::LinkImpl(
      _internal::Session const& session,
      _internal::LinkEndpoint& linkEndpoint,
      std::string const& name,
      _internal::SessionRole role,
      std::string const& source,
      std::string const& target)
      : m_session{session}, m_source(source), m_target(target)
  {
    m_link = link_create_from_endpoint(
        *session.GetImpl(),
        linkEndpoint.Release(),
        name.c_str(),
        role == _internal::SessionRole::Sender ? role_sender : role_receiver,
        Azure::Core::Amqp::Models::_internal::Messaging::CreateSource(source),
        Azure::Core::Amqp::Models::_internal::Messaging::CreateTarget(target));
  }

  LinkImpl::~LinkImpl() noexcept
  {
    if (m_link)
    {
      link_destroy(m_link);
      m_link = nullptr;
    }
  }

  std::string const& LinkImpl::GetSource() const { return m_source; }
  std::string const& LinkImpl::GetTarget() const { return m_target; }

  void LinkImpl::SetMaxMessageSize(uint64_t size)
  {
    if (link_set_max_message_size(m_link, size))
    {
      throw std::runtime_error("Could not set max message size"); // LCOV_EXCL_LINE
    }
  }
  uint64_t LinkImpl::GetMaxMessageSize() const
  {
    uint64_t maxMessageSize;
    if (link_get_max_message_size(m_link, &maxMessageSize))
    {
      throw std::runtime_error("Could not set max message size"); // LCOV_EXCL_LINE
    }
    return maxMessageSize;
  }

  std::string LinkImpl::GetName() const
  {
    const char* name;
    if (link_get_name(m_link, &name))
    {
      throw std::runtime_error("Could not get link name"); // LCOV_EXCL_LINE
    }
    return name;
  }
  _internal::SenderSettleMode LinkImpl::GetSenderSettleMode() const
  {
    sender_settle_mode settleMode;
    if (link_get_snd_settle_mode(m_link, &settleMode))
    {
      throw std::runtime_error("Could not get link sender settle mode."); // LCOV_EXCL_LINE
    }
    switch (settleMode)
    {
      case sender_settle_mode_mixed:
        return Azure::Core::Amqp::_internal::SenderSettleMode::Mixed;
      case sender_settle_mode_settled:
        return Azure::Core::Amqp::_internal::SenderSettleMode::Settled;
      case sender_settle_mode_unsettled:
        return Azure::Core::Amqp::_internal::SenderSettleMode::Unsettled;
      default:
        throw std::logic_error("Unknown settle mode."); // LCOV_EXCL_LINE
    }
  }
  void LinkImpl::SetSenderSettleMode(_internal::SenderSettleMode mode)
  {
    sender_settle_mode settleMode;
    switch (mode)
    {
      case Azure::Core::Amqp::_internal::SenderSettleMode::Unsettled:
        settleMode = sender_settle_mode_unsettled;
        break;
      case Azure::Core::Amqp::_internal::SenderSettleMode::Settled:
        settleMode = sender_settle_mode_settled;
        break;
      case Azure::Core::Amqp::_internal::SenderSettleMode::Mixed:
        settleMode = sender_settle_mode_mixed;
        break;
      default: // LCOV_EXCL_LINE
        throw std::logic_error("Unknown settle mode."); // LCOV_EXCL_LINE
    }
    if (link_set_snd_settle_mode(m_link, settleMode))
    {
      throw std::runtime_error("Could not get link sender settle mode."); // LCOV_EXCL_LINE
    }
  }

  _internal::ReceiverSettleMode LinkImpl::GetReceiverSettleMode() const
  {
    receiver_settle_mode settleMode;
    if (link_get_rcv_settle_mode(m_link, &settleMode))
    {
      throw std::runtime_error("Could not get link sender settle mode."); // LCOV_EXCL_LINE
    }
    switch (settleMode)
    {
      case receiver_settle_mode_first:
        return _internal::ReceiverSettleMode::First;
      case receiver_settle_mode_second:
        return _internal::ReceiverSettleMode::Second;
      default: // LCOV_EXCL_LINE
        throw std::logic_error("Unknown settle mode."); // LCOV_EXCL_LINE
    }
  }
  void LinkImpl::SetReceiverSettleMode(_internal::ReceiverSettleMode mode)
  {
    receiver_settle_mode settleMode;
    switch (mode)
    {
      case _internal::ReceiverSettleMode::First:
        settleMode = receiver_settle_mode_first;
        break;
      case _internal::ReceiverSettleMode::Second:
        settleMode = receiver_settle_mode_second;
        break;
      default: // LCOV_EXCL_LINE
        throw std::logic_error("Unknown settle mode."); // LCOV_EXCL_LINE
    }
    if (link_set_rcv_settle_mode(m_link, settleMode))
    {
      throw std::runtime_error("Could not get link sender settle mode."); // LCOV_EXCL_LINE
    }
  }

  uint32_t LinkImpl::GetInitialDeliveryCount() const
  {
    uint32_t deliveryCount;
    if (link_get_initial_delivery_count(m_link, &deliveryCount))
    {
      throw std::runtime_error("Could not get link initial delivery count."); // LCOV_EXCL_LINE
    }
    return deliveryCount;
  }

  uint64_t LinkImpl::GetPeerMaxMessageSize() const
  {
    uint64_t peerMax;
    if (link_get_peer_max_message_size(m_link, &peerMax))
    {
      throw std::runtime_error("Could not get link initial delivery count.");
    }
    return peerMax; // LCOV_EXCL_LINE
  }

  uint32_t LinkImpl::GetReceivedMessageId() const
  {
    uint32_t messageId;
    if (link_get_received_message_id(m_link, &messageId))
    {
      throw std::runtime_error("Could not get link received message ID."); // LCOV_EXCL_LINE
    }
    return messageId;
  }

  void LinkImpl::SetInitialDeliveryCount(uint32_t count)
  {
    if (link_set_initial_delivery_count(m_link, count))
    {
      throw std::runtime_error("Could not set initial delivery count."); // LCOV_EXCL_LINE
    }
  }
  void LinkImpl::SetAttachProperties(Azure::Core::Amqp::Models::AmqpValue properties)
  {
    if (link_set_attach_properties(m_link, properties))
    {
      throw std::runtime_error("Could not set attach properties."); // LCOV_EXCL_LINE
    }
  }
  void LinkImpl::SetMaxLinkCredit(uint32_t credit)
  {
    if (link_set_max_link_credit(m_link, credit))
    {
      throw std::runtime_error("Could not set attach properties."); // LCOV_EXCL_LINE
    }
  }

  void LinkImpl::Attach()
  {
    if (link_attach(m_link, nullptr, nullptr, nullptr, this))
    {
      throw std::runtime_error("Could not set attach properties."); // LCOV_EXCL_LINE
    }
  }
  void LinkImpl::Detach(
      bool close,
      std::string const& condition,
      std::string const& description,
      Azure::Core::Amqp::Models::AmqpValue& info)
  {
    if (link_detach(
            m_link,
            close,
            (condition.empty() ? nullptr : condition.c_str()),
            (description.empty() ? nullptr : description.c_str()),
            info))
    {
      throw std::runtime_error("Could not set attach properties."); // LCOV_EXCL_LINE
    }
  }

}}}} // namespace Azure::Core::Amqp::_detail
