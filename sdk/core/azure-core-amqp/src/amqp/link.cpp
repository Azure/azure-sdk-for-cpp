// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/link.hpp"

#include "../models/private/error_impl.hpp"
#include "../models/private/value_impl.hpp"
#include "azure/core/amqp/internal/message_receiver.hpp"
#include "azure/core/amqp/internal/message_sender.hpp"
#include "azure/core/amqp/internal/models/message_source.hpp"
#include "azure/core/amqp/internal/models/message_target.hpp"
#include "azure/core/amqp/internal/models/messaging_values.hpp"
#include "private/link_impl.hpp"
#include "private/session_impl.hpp"

#include <azure_uamqp_c/amqp_definitions_sequence_no.h>

#include <azure_uamqp_c/link.h>
#include <azure_uamqp_c/session.h>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if defined(TESTING_BUILD)
  Link::Link(
      _internal::Session const& session,
      std::string const& name,
      Azure::Core::Amqp::_internal::SessionRole role,
      Models::_internal::MessageSource const& source,
      Models::_internal::MessageTarget const& target)
      : m_impl{
          std::make_shared<LinkImpl>(SessionFactory::GetImpl(session), name, role, source, target)}
  {
  }

  Link::Link(
      _internal::Session const& session,
      _internal::LinkEndpoint& linkEndpoint,
      std::string const& name,
      _internal::SessionRole role,
      Models::_internal::MessageSource const& source,
      Models::_internal::MessageTarget const& target)
      : m_impl{std::make_shared<
          LinkImpl>(SessionFactory::GetImpl(session), linkEndpoint, name, role, source, target)}
  {
  }

  Link::~Link() noexcept {}

  Models::_internal::MessageSource const& Link::GetSource() const { return m_impl->GetSource(); }
  Models::_internal::MessageTarget const& Link::GetTarget() const { return m_impl->GetTarget(); }
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
  void Link::SetAttachProperties(Models::AmqpValue attachProperties)
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
      Models::AmqpValue& info)
  {
    return m_impl->Detach(close, errorCondition, errorDescription, info);
  }
#endif

  /****/
  /* LINK Implementation */

  LinkImpl::LinkImpl(
      std::shared_ptr<_detail::SessionImpl> session,
      std::string const& name,
      _internal::SessionRole role,
      Models::_internal::MessageSource const& source,
      Models::_internal::MessageTarget const& target)
      : m_session{session}, m_source(source), m_target(target)
  {
    Models::AmqpValue sourceValue{source.AsAmqpValue()};
    Models::AmqpValue targetValue(target.AsAmqpValue());
    m_link = link_create(
        *session,
        name.c_str(),
        role == _internal::SessionRole::Sender ? role_sender : role_receiver,
        Models::_detail::AmqpValueFactory::ToUamqp(sourceValue),
        Models::_detail::AmqpValueFactory::ToUamqp(targetValue));
  }

  LinkImpl::LinkImpl(
      std::shared_ptr<_detail::SessionImpl> session,
      _internal::LinkEndpoint& linkEndpoint,
      std::string const& name,
      _internal::SessionRole role,
      Models::_internal::MessageSource const& source,
      Models::_internal::MessageTarget const& target)
      : m_session{session}, m_source(source), m_target(target)
  {
    Models::AmqpValue sourceValue(source.AsAmqpValue());
    Models::AmqpValue targetValue(target.AsAmqpValue());
    m_link = link_create_from_endpoint(
        *session,
        LinkEndpointFactory::Release(linkEndpoint),
        name.c_str(),
        role == _internal::SessionRole::Sender ? role_sender : role_receiver,
        Models::_detail::AmqpValueFactory::ToUamqp(sourceValue),
        Models::_detail::AmqpValueFactory::ToUamqp(targetValue));
  }

  LinkImpl::~LinkImpl() noexcept
  {
    if (m_linkSubscriptionHandle != nullptr)
    {
      AZURE_ASSERT_MSG(
          m_linkSubscriptionHandle == nullptr,
          "Destroying link while link detach subscription is still active.");
      Azure::Core::_internal::AzureNoReturnPath(
          "Destroying link while link detach subscription is still active.");
    }

    if (m_link)
    {
      link_destroy(m_link);
      m_link = nullptr;
    }
  }

  Models::_internal::MessageSource const& LinkImpl::GetSource() const { return m_source; }
  Models::_internal::MessageTarget const& LinkImpl::GetTarget() const { return m_target; }

  void LinkImpl::SetMaxMessageSize(uint64_t size)
  {
    if (link_set_max_message_size(m_link, size))
    {
      throw std::runtime_error("Could not set max message size");
    }
  }
  uint64_t LinkImpl::GetMaxMessageSize() const
  {
    uint64_t maxMessageSize;
    if (link_get_max_message_size(m_link, &maxMessageSize))
    {
      throw std::runtime_error("Could not set max message size");
    }
    return maxMessageSize;
  }

  std::string LinkImpl::GetName() const
  {
    const char* name;
    if (link_get_name(m_link, &name))
    {
      throw std::runtime_error("Could not get link name");
    }
    return name;
  }
  _internal::SenderSettleMode LinkImpl::GetSenderSettleMode() const
  {
    sender_settle_mode settleMode;
    if (link_get_snd_settle_mode(m_link, &settleMode))
    {
      throw std::runtime_error("Could not get link sender settle mode.");
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
        throw std::logic_error("Unknown settle mode.");
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
      default:
        throw std::logic_error("Unknown settle mode.");
    }
    if (link_set_snd_settle_mode(m_link, settleMode))
    {
      throw std::runtime_error("Could not get link sender settle mode.");
    }
  }

  _internal::ReceiverSettleMode LinkImpl::GetReceiverSettleMode() const
  {
    receiver_settle_mode settleMode;
    if (link_get_rcv_settle_mode(m_link, &settleMode))
    {
      throw std::runtime_error("Could not get link sender settle mode.");
    }
    switch (settleMode)
    {
      case receiver_settle_mode_first:
        return _internal::ReceiverSettleMode::First;
      case receiver_settle_mode_second:
        return _internal::ReceiverSettleMode::Second;
      default:
        throw std::logic_error("Unknown settle mode.");
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
      default:
        throw std::logic_error("Unknown settle mode.");
    }
    if (link_set_rcv_settle_mode(m_link, settleMode))
    {
      throw std::runtime_error("Could not get link sender settle mode.");
    }
  }

  uint32_t LinkImpl::GetInitialDeliveryCount() const
  {
    uint32_t deliveryCount;
    if (link_get_initial_delivery_count(m_link, &deliveryCount))
    {
      throw std::runtime_error("Could not get link initial delivery count.");
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
    return peerMax;
  }

  uint32_t LinkImpl::GetReceivedMessageId() const
  {
    uint32_t messageId;
    if (link_get_received_message_id(m_link, &messageId))
    {
      throw std::runtime_error("Could not get link received message ID.");
    }
    return messageId;
  }

  void LinkImpl::SetInitialDeliveryCount(uint32_t count)
  {
    if (link_set_initial_delivery_count(m_link, count))
    {
      throw std::runtime_error("Could not set initial delivery count.");
    }
  }
  void LinkImpl::SetAttachProperties(Models::AmqpValue properties)
  {
    if (link_set_attach_properties(m_link, Models::_detail::AmqpValueFactory::ToUamqp(properties)))
    {
      throw std::runtime_error("Could not set attach properties.");
    }
  }
  void LinkImpl::SetMaxLinkCredit(uint32_t credit)
  {
    if (link_set_max_link_credit(m_link, credit))
    {
      throw std::runtime_error("Could not set attach properties.");
    }
  }

  void LinkImpl::SetDesiredCapabilities(Models::AmqpValue desiredCapabilities)
  {
    if (link_set_desired_capabilities(
            m_link, Models::_detail::AmqpValueFactory::ToUamqp(desiredCapabilities)))
    {
      throw std::runtime_error("Could not set desired capabilities.");
    }
  }

  Models::AmqpValue LinkImpl::GetDesiredCapabilities() const
  {
    AMQP_VALUE desiredCapabilitiesVal;
    if (link_get_desired_capabilities(m_link, &desiredCapabilitiesVal))
    {
      throw std::runtime_error("Could not convert field to header.");
    }
    return Models::_detail::AmqpValueFactory::FromUamqp(
        Models::_detail::UniqueAmqpValueHandle{desiredCapabilitiesVal});
  }

  void LinkImpl::SubscribeToDetachEvent(OnLinkDetachEvent onLinkDetach)
  {
    m_onLinkDetachEvent = std::move(onLinkDetach);
    m_linkSubscriptionHandle
        = link_subscribe_on_link_detach_received(m_link, OnLinkDetachEventFn, this);
  }

  void LinkImpl::UnsubscribeFromDetachEvent()
  {
    if (m_linkSubscriptionHandle != nullptr)
    {
      link_unsubscribe_on_link_detach_received(m_linkSubscriptionHandle);
      m_linkSubscriptionHandle = nullptr;
    }
  }
  void LinkImpl::OnLinkDetachEventFn(void* context, ERROR_HANDLE error)
  {
    LinkImpl* link = static_cast<LinkImpl*>(context);
    if (link->m_onLinkDetachEvent)
    {
      link->m_onLinkDetachEvent(Models::_detail::AmqpErrorFactory::FromUamqp(error));
    }
  }

  void LinkImpl::Poll()
  {
    // Ensure that the connection hierarchy's state is not modified while polling on the link.
    auto lock{m_session->GetConnection()->Lock()};
    link_dowork(m_link);
  }

  void LinkImpl::ResetLinkCredit(std::uint32_t linkCredit, bool drain)
  {
    if (link_reset_link_credit(m_link, linkCredit, drain))
    {
      throw std::runtime_error("Could not reset link credit.");
    }
  }

  void LinkImpl::Attach()
  {
    if (link_attach(m_link, nullptr, nullptr, nullptr, this))
    {
      throw std::runtime_error("Could not set attach properties.");
    }
  }
  void LinkImpl::Detach(
      bool close,
      std::string const& condition,
      std::string const& description,
      Models::AmqpValue& info)
  {
    if (link_detach(
            m_link,
            close,
            (condition.empty() ? nullptr : condition.c_str()),
            (description.empty() ? nullptr : description.c_str()),
            Models::_detail::AmqpValueFactory::ToUamqp(info)))
    {
      throw std::runtime_error("Could not set attach properties.");
    }
  }
}}}} // namespace Azure::Core::Amqp::_detail
