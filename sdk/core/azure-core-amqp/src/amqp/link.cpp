// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#include "azure/core/amqp/link.hpp"
#include "azure/core/amqp/message_sender.hpp"
#include "azure/core/amqp/session.hpp"
#include "azure/core/amqp/message_receiver.hpp"
#include "azure/core/amqp/models/messaging_values.hpp"
#include <azure_uamqp_c/amqp_definitions_sequence_no.h>

#include <azure_uamqp_c/link.h>
#include <azure_uamqp_c/session.h>

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace _detail {

  Link::Link(
      Session const& session,
      std::string const& name,
      SessionRole role,
      std::string const& source,
      std::string const& target)
      : m_session{session}, m_source(source), m_target(target)
  {
    m_link = link_create(
        session,
        name.c_str(),
        role == SessionRole::Sender ? role_sender : role_receiver,
        Azure::Core::_internal::Amqp::Models::Messaging::CreateSource(source),
        Azure::Core::_internal::Amqp::Models::Messaging::CreateTarget(target));
  }

  Link::Link(
      Session const& session,
      LinkEndpoint& linkEndpoint,
      std::string const& name,
      SessionRole role,
      std::string const& source,
      std::string const& target)
      : m_session{session}, m_source(source), m_target(target)
  {
    m_link = link_create_from_endpoint(
        session,
        linkEndpoint.Release(),
        name.c_str(),
        role == SessionRole::Sender ? role_sender : role_receiver,
        Azure::Core::_internal::Amqp::Models::Messaging::CreateSource(source),
        Azure::Core::_internal::Amqp::Models::Messaging::CreateTarget(target));
  }

  Link::~Link()
  {
    if (m_link)
    {
      link_destroy(m_link);
      m_link = nullptr;
    }
  }

  std::string const& Link::GetSource() const { return m_source; }
  std::string const& Link::GetTarget() const { return m_target; }

  void Link::SetMaxMessageSize(uint64_t size)
  {
    if (link_set_max_message_size(m_link, size))
    {
      throw std::runtime_error("Could not set max message size");
    }
  }
  uint64_t Link::GetMaxMessageSize() const
  {
    uint64_t maxMessageSize;
    if (link_get_max_message_size(m_link, &maxMessageSize))
    {
      throw std::runtime_error("Could not set max message size");
    }
    return maxMessageSize;
  }

  std::string Link::GetName() const
  {
    const char* name;
    if (link_get_name(m_link, &name))
    {
      throw std::runtime_error("Could not get link name");
    }
    return name;
  }
  SenderSettleMode Link::GetSenderSettleMode() const
  {
    sender_settle_mode settleMode;
    if (link_get_snd_settle_mode(m_link, &settleMode))
    {
      throw std::runtime_error("Could not get link sender settle mode.");
    }
    switch (settleMode)
    {
      case sender_settle_mode_mixed:
        return SenderSettleMode::Mixed;
      case sender_settle_mode_settled:
        return SenderSettleMode::Settled;
      case sender_settle_mode_unsettled:
        return SenderSettleMode::Unsettled;
      default:
        throw std::logic_error("Unknown settle mode.");
    }
  }
  void Link::SetSenderSettleMode(SenderSettleMode mode)
  {
    sender_settle_mode settleMode;
    switch (mode)
    {
      case SenderSettleMode::Unsettled:
        settleMode = sender_settle_mode_unsettled;
        break;
      case SenderSettleMode::Settled:
        settleMode = sender_settle_mode_settled;
        break;
      case SenderSettleMode::Mixed:
        settleMode = sender_settle_mode_mixed;
        break;
      default:
        throw std::logic_error("Unknown settle mode.");
    }
    if (link_set_snd_settle_mode(m_link, settleMode))
    {
      throw std::runtime_error("Could not get link sender settle mode.");
    }
  }

  ReceiverSettleMode Link::GetReceiverSettleMode() const
  {
    receiver_settle_mode settleMode;
    if (link_get_rcv_settle_mode(m_link, &settleMode))
    {
      throw std::runtime_error("Could not get link sender settle mode.");
    }
    switch (settleMode)
    {
      case receiver_settle_mode_first:
        return ReceiverSettleMode::First;
      case receiver_settle_mode_second:
        return ReceiverSettleMode::Second;
      default:
        throw std::logic_error("Unknown settle mode.");
    }
  }
  void Link::SetReceiverSettleMode(ReceiverSettleMode mode)
  {
    receiver_settle_mode settleMode;
    switch (mode)
    {
      case ReceiverSettleMode::First:
        settleMode = receiver_settle_mode_first;
        break;
      case ReceiverSettleMode::Second:
        settleMode = receiver_settle_mode_second;
        break;
      default:
        throw std::logic_error("Unknown settle mode.");
    }
    if (link_set_rcv_settle_mode(m_link, settleMode))
    {
      throw std::runtime_error("Could not get link sender settle mode.");
    }
  }

  uint32_t Link::GetInitialDeliveryCount() const
  {
    uint32_t deliveryCount;
    if (link_get_initial_delivery_count(m_link, &deliveryCount))
    {
      throw std::runtime_error("Could not get link initial delivery count.");
    }
    return deliveryCount;
  }

  uint64_t Link::GetPeerMaxMessageSize() const
  {
    uint64_t peerMax;
    if (link_get_peer_max_message_size(m_link, &peerMax))
    {
      throw std::runtime_error("Could not get link initial delivery count.");
    }
    return peerMax;
  }

  uint32_t Link::GetReceivedMessageId() const
  {
    uint32_t messageId;
    if (link_get_received_message_id(m_link, &messageId))
    {
      throw std::runtime_error("Could not get link received message ID.");
    }
    return messageId;
  }

  void Link::SetInitialDeliveryCount(uint32_t count)
  {
    if (link_set_initial_delivery_count(m_link, count))
    {
      throw std::runtime_error("Could not set initial delivery count.");
    }
  }
  void Link::SetAttachProperties(Azure::Core::Amqp::Models::Value properties)
  {
    if (link_set_attach_properties(m_link, properties))
    {
      throw std::runtime_error("Could not set attach properties.");
    }
  }
  void Link::SetMaxLinkCredit(uint32_t credit)
  {
    if (link_set_max_link_credit(m_link, credit))
    {
      throw std::runtime_error("Could not set attach properties.");
    }
  }

  void Link::Attach(LinkEvents* eventHandler)
  {
    if (link_attach(
            m_link,
            (eventHandler ? OnTransferReceivedFn : nullptr),
            (eventHandler ? OnLinkStateChangedFn : nullptr),
            (eventHandler ? OnLinkFlowOnFn : nullptr),
            this))
    {
      throw std::runtime_error("Could not set attach properties.");
    }
  }
  void Link::Detach(
      bool close,
      std::string const& condition,
      std::string const& description,
      Azure::Core::Amqp::Models::Value& info)
  {
    if (link_detach(
            m_link,
            close,
            (condition.empty() ? nullptr : condition.c_str()),
            (description.empty() ? nullptr : description.c_str()),
            info))
    {
      throw std::runtime_error("Could not set attach properties.");
    }
  }

  AMQP_VALUE Link::OnTransferReceivedFn(
      void* context,
      TRANSFER_HANDLE transfer,
      uint32_t payloadSize,
      const uint8_t* payloadBytes)
  {
    Link* link = static_cast<Link*>(context);
    if (link->m_eventHandler)
    {
      Models::TransferInstance transferInstance(transfer);
      return link->m_eventHandler->OnTransferReceived(
          *link, transferInstance, payloadSize, payloadBytes);
    }
    return {};
  }

  inline LinkState LinkStateFromLINK_STATE(LINK_STATE state)
  {
    switch (state)
    {
      case LINK_STATE_ATTACHED:
        return LinkState::Attached;
      case LINK_STATE_DETACHED:
        return LinkState::Detached;
      case LINK_STATE_ERROR:
        return LinkState::Error;
      case LINK_STATE_HALF_ATTACHED_ATTACH_RECEIVED:
        return LinkState::HalfAttachAttachReceived;
      case LINK_STATE_HALF_ATTACHED_ATTACH_SENT:
        return LinkState::HalfAttachedAttachSent;
      case LINK_STATE_INVALID:
        return LinkState::Invalid;
      default:
        throw std::logic_error("Unknown link state");
    }
  }
  void Link::OnLinkStateChangedFn(void* context, LINK_STATE newLinkState, LINK_STATE oldLinkState)
  {
    Link* link = static_cast<Link*>(context);
    if (link->m_eventHandler)
    {

      link->m_eventHandler->OnLinkStateChanged(
          *link, LinkStateFromLINK_STATE(newLinkState), LinkStateFromLINK_STATE(oldLinkState));
    }
  }

  void Link::OnLinkFlowOnFn(void* context)
  {
    Link* link = static_cast<Link*>(context);
    if (link->m_eventHandler)
    {
      link->m_eventHandler->OnLinkFlowOn(*link);
    }
  }

}}}}} // namespace Azure::Core::_internal::Amqp::_detail