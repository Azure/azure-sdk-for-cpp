// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/link.hpp"

#include "../models/private/error_impl.hpp"
#include "../models/private/performatives/transfer_impl.hpp"
#include "../models/private/value_impl.hpp"
#include "azure/core/amqp/internal/common/completion_operation.hpp"
#include "azure/core/amqp/internal/common/global_state.hpp"
#include "azure/core/amqp/internal/models/message_source.hpp"
#include "azure/core/amqp/internal/models/message_target.hpp"
#include "link_impl.hpp"
#include "session_impl.hpp"

#if ENABLE_UAMQP
#include <azure_uamqp_c/link.h>
#endif

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if defined(_azure_TESTING_BUILD)

#if ENABLE_UAMQP
  class LinkImplEventsImpl : public LinkImplEvents {
  public:
    LinkImplEventsImpl(LinkEvents* linkEvents) : m_linkEvents(linkEvents) {}

  private:
    virtual Models::AmqpValue OnTransferReceived(
        std::shared_ptr<LinkImpl> const& link,
        Models::_internal::Performatives::AmqpTransfer transfer,
        uint32_t payloadSize,
        const unsigned char* payloadBytes) override
    {
      if (m_linkEvents)
      {
        return m_linkEvents->OnTransferReceived(Link{link}, transfer, payloadSize, payloadBytes);
      }
      return Models::AmqpValue{};
    }
    virtual void OnLinkStateChanged(
        std::shared_ptr<LinkImpl> const& link,
        LinkState newLinkState,
        LinkState previousLinkState) override
    {
      if (m_linkEvents)
      {
        m_linkEvents->OnLinkStateChanged(Link{link}, newLinkState, previousLinkState);
      }
    }
    virtual void OnLinkFlowOn(std::shared_ptr<LinkImpl> const& link) override
    {
      if (m_linkEvents)
      {
        m_linkEvents->OnLinkFlowOn(Link{link});
      }
    }

    LinkEvents* m_linkEvents;
  };
#endif // ENABLE_UAMQP

  Link::Link(
      _internal::Session const& session,
      std::string const& name,
      Azure::Core::Amqp::_internal::SessionRole role,
      Models::_internal::MessageSource const& source,
      Models::_internal::MessageTarget const& target
#if ENABLE_UAMQP
      ,
      LinkEvents* linkEvents
#endif
      )
      :
#if ENABLE_UAMQP
        m_implEvents{std::make_shared<LinkImplEventsImpl>(linkEvents)},
#endif
        m_impl
  {
    std::make_shared<LinkImpl>(
        SessionFactory::GetImpl(session),
        name,
        role,
        source,
        target
#if ENABLE_UAMQP
        ,
        m_implEvents.get()
#endif
    )
  }
  {
  }
#if ENABLE_UAMQP
  Link::Link(
      _internal::Session const& session,
      _internal::LinkEndpoint& linkEndpoint,
      std::string const& name,
      _internal::SessionRole role,
      Models::_internal::MessageSource const& source,
      Models::_internal::MessageTarget const& target,
      LinkEvents* linkEvents)
      : m_implEvents{std::make_shared<LinkImplEventsImpl>(linkEvents)},
        m_impl{std::make_shared<LinkImpl>(
            SessionFactory::GetImpl(session),
            linkEndpoint,
            name,
            role,
            source,
            target,
            m_implEvents.get())}
  {
  }
#endif

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
  void Link::SetAttachProperties(Models::AmqpValue const& attachProperties)
  {
    m_impl->SetAttachProperties(attachProperties);
  }

  Models::AmqpValue Link::GetDesiredCapabilities() const
  {
    return m_impl->GetDesiredCapabilities();
  }
  void Link::SetDesiredCapabilities(Models::AmqpValue const& desiredCapabilities)
  {
    m_impl->SetDesiredCapabilities(desiredCapabilities);
  }

  void Link::ResetLinkCredit(std::uint32_t linkCredit, bool drain)
  {
    m_impl->ResetLinkCredit(linkCredit, drain);
  }
  void Link::SetMaxLinkCredit(uint32_t credit) { m_impl->SetMaxLinkCredit(credit); }
  std::string Link::GetName() const { return m_impl->GetName(); }

  uint32_t Link::GetReceivedMessageId() const { return m_impl->GetReceivedMessageId(); }
  void Link::Attach()
  {
#if ENABLE_UAMQP
    Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->AddPollable(
        m_impl);
#endif
    return m_impl->Attach();
  }

  std::tuple<uint32_t, LinkDeliverySettleReason, Models::AmqpValue> Link::Transfer(
      std::vector<uint8_t> const& payload,
      Azure::Core::Context const& context)
  {
    return m_impl->Transfer(payload, context);
  }

  void Link::Detach(
      bool close,
      std::string const& errorCondition,
      std::string const& errorDescription,
      Models::AmqpValue const& info)
  {
    m_impl->Detach(close, errorCondition, errorDescription, info);
#if ENABLE_UAMQP
    Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->RemovePollable(
        m_impl);
#endif
  }
#endif // _azure_TESTING_BUILD

}}}} // namespace Azure::Core::Amqp::_detail
