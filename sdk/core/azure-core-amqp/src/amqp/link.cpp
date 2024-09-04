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
#include "private/link_impl.hpp"
#include "private/session_impl.hpp"

#if ENABLE_UAMQP
#include <azure_uamqp_c/link.h>
#endif

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if defined(_azure_TESTING_BUILD)

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

  Link::Link(
      _internal::Session const& session,
      std::string const& name,
      Azure::Core::Amqp::_internal::SessionRole role,
      Models::_internal::MessageSource const& source,
      Models::_internal::MessageTarget const& target,
      LinkEvents* linkEvents)
      : m_implEvents{std::make_shared<LinkImplEventsImpl>(linkEvents)},
        m_impl{std::make_shared<LinkImpl>(
            SessionFactory::GetImpl(session),
            name,
            role,
            source,
            target,
            m_implEvents.get())}
  {
  }

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
    Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->AddPollable(
        m_impl);
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
    Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->RemovePollable(
        m_impl);
  }
#endif // _azure_TESTING_BUILD

  std::ostream& operator<<(std::ostream& os, LinkState linkState)
  {
    switch (linkState)
    {
      case LinkState::Attached:
        os << "Attached";
        break;
      case LinkState::Detached:
        os << "Detached";
        break;
      case LinkState::Error:
        os << "Error";
        break;
      case LinkState::HalfAttachedAttachReceived:
        os << "HalfAttachedAttachReceived";
        break;
      case LinkState::HalfAttachedAttachSent:
        os << "HalfAttachedAttachSent";
        break;
      case LinkState::Invalid:
        os << "Invalid";
        break;
      default:
        os << "Unknown";
        break;
    }

    return os;
  }

#if ENABLE_UAMQP
  std::ostream& operator<<(std::ostream& os, LINK_TRANSFER_RESULT transfer)
  {
    switch (transfer)
    {
      case LINK_TRANSFER_BUSY:
        os << "LINK_TRANSFER_BUSY";
        break;
      case LINK_TRANSFER_ERROR:
        os << "LINK_TRANSFER_ERROR";
        break;
      default:
        os << "Unknown (" << transfer << ")";
        break;
    }
    return os;
  }
#endif

  /****/
  /* LINK Implementation */

  LinkImpl::LinkImpl(
      std::shared_ptr<_detail::SessionImpl> session,
      std::string const& name,
      _internal::SessionRole role,
      Models::_internal::MessageSource const& source,
      Models::_internal::MessageTarget const& target,
      LinkImplEvents* events)
      : m_session{session}, m_source(source), m_target(target), m_eventHandler{events}
  {
    Models::AmqpValue sourceValue{source.AsAmqpValue()};
    Models::AmqpValue targetValue(target.AsAmqpValue());
    auto connectionLock{m_session->GetConnection()->Lock()};
#if ENABLE_UAMQP
    m_link = link_create(
        *session,
        name.c_str(),
        role == _internal::SessionRole::Sender ? role_sender : role_receiver,
        Models::_detail::AmqpValueFactory::ToImplementation(sourceValue),
        Models::_detail::AmqpValueFactory::ToImplementation(targetValue));
#else
    (void)name;
    (void)role;
    (void)sourceValue;
    (void)targetValue;

#endif
  }

  LinkImpl::LinkImpl(
      std::shared_ptr<_detail::SessionImpl> session,
      _internal::LinkEndpoint& linkEndpoint,
      std::string const& name,
      _internal::SessionRole role,
      Models::_internal::MessageSource const& source,
      Models::_internal::MessageTarget const& target,
      LinkImplEvents* events)
      : m_session{session}, m_source(source), m_target(target), m_eventHandler{events}
  {
    Models::AmqpValue sourceValue(source.AsAmqpValue());
    Models::AmqpValue targetValue(target.AsAmqpValue());
    auto connectionLock{m_session->GetConnection()->Lock()};
#if ENABLE_UAMQP
    m_link = link_create_from_endpoint(
        *session,
        LinkEndpointFactory::Release(linkEndpoint),
        name.c_str(),
        role == _internal::SessionRole::Sender ? role_sender : role_receiver,
        Models::_detail::AmqpValueFactory::ToImplementation(sourceValue),
        Models::_detail::AmqpValueFactory::ToImplementation(targetValue));
#else
    (void)linkEndpoint;
    (void)name;
    (void)role;

#endif
  }

  LinkImpl::~LinkImpl() noexcept
  {
#if ENABLE_UAMQP
    if (m_linkSubscriptionHandle != nullptr)
    {
      AZURE_ASSERT_MSG(
          m_linkSubscriptionHandle == nullptr,
          "Destroying link while link detach subscription is still active.");
      Azure::Core::_internal::AzureNoReturnPath(
          "Destroying link while link detach subscription is still active.");
    }
    auto lock{m_session->GetConnection()->Lock()};
    if (m_link)
    {
      link_destroy(m_link);
      m_link = nullptr;
    }
#endif
  }

  Models::_internal::MessageSource const& LinkImpl::GetSource() const { return m_source; }
  Models::_internal::MessageTarget const& LinkImpl::GetTarget() const { return m_target; }

  void LinkImpl::SetMaxMessageSize(uint64_t size)
  {
#if ENABLE_UAMQP
    if (link_set_max_message_size(m_link, size))
    {
      throw std::runtime_error("Could not set max message size");
    }
#else
    (void)size;
#endif
  }
  uint64_t LinkImpl::GetMaxMessageSize() const
  {
    uint64_t maxMessageSize = {};
#if ENABLE_UAMQP
    if (link_get_max_message_size(m_link, &maxMessageSize))
    {
      throw std::runtime_error("Could not set max message size");
    }
#endif
    return maxMessageSize;
  }

  std::string LinkImpl::GetName() const
  {
    const char* name = {};
#if ENABLE_UAMQP
    if (link_get_name(m_link, &name))
    {
      throw std::runtime_error("Could not get link name");
    }
#endif
    return name;
  }
  _internal::SenderSettleMode LinkImpl::GetSenderSettleMode() const
  {
#if ENABLE_UAMQP
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
#else
    return {};
#endif
  }
  void LinkImpl::SetSenderSettleMode(_internal::SenderSettleMode mode)
  {
#if ENABLE_UAMQP
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
#else
    (void)mode;
#endif
  }

  _internal::ReceiverSettleMode LinkImpl::GetReceiverSettleMode() const
  {
#if ENABLE_UAMQP
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
#else
    return {};
#endif
  }
  void LinkImpl::SetReceiverSettleMode(_internal::ReceiverSettleMode mode)
  {
#if ENABLE_UAMQP
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
#else
    (void)mode;
#endif
  }

  uint32_t LinkImpl::GetInitialDeliveryCount() const
  {
    uint32_t deliveryCount = {};
#if ENABLE_UAMQP
    if (link_get_initial_delivery_count(m_link, &deliveryCount))
    {
      throw std::runtime_error("Could not get link initial delivery count.");
    }
#endif
    return deliveryCount;
  }

  uint64_t LinkImpl::GetPeerMaxMessageSize() const
  {
    uint64_t peerMax = {};
#if ENABLE_UAMQP
    if (link_get_peer_max_message_size(m_link, &peerMax))
    {
      throw std::runtime_error("Could not get peer max message size.");
    }
#endif
    return peerMax;
  }

  uint32_t LinkImpl::GetReceivedMessageId() const
  {
    uint32_t messageId = {};
#if ENABLE_UAMQP
    if (link_get_received_message_id(m_link, &messageId))
    {
      throw std::runtime_error("Could not get link received message ID.");
    }
#endif
    return messageId;
  }

  void LinkImpl::SetInitialDeliveryCount(uint32_t count)
  {
#if ENABLE_UAMQP
    if (link_set_initial_delivery_count(m_link, count))
    {
      throw std::runtime_error("Could not set initial delivery count.");
    }
#else
    (void)count;
#endif
  }
  void LinkImpl::SetAttachProperties(Models::AmqpValue properties)
  {
#if ENABLE_UAMQP
    if (link_set_attach_properties(
            m_link, Models::_detail::AmqpValueFactory::ToImplementation(properties)))
    {
      throw std::runtime_error("Could not set attach properties.");
    }
#else
    (void)properties;
#endif
  }
  void LinkImpl::SetMaxLinkCredit(uint32_t credit)
  {
#if ENABLE_UAMQP
    if (link_set_max_link_credit(m_link, credit))
    {
      throw std::runtime_error("Could not set attach properties.");
    }
#else
    (void)credit;
#endif
  }

  void LinkImpl::SetDesiredCapabilities(Models::AmqpValue desiredCapabilities)
  {
#if ENABLE_UAMQP
    if (link_set_desired_capabilities(
            m_link, Models::_detail::AmqpValueFactory::ToImplementation(desiredCapabilities)))
    {
      throw std::runtime_error("Could not set desired capabilities.");
    }
#else
    (void)desiredCapabilities;
#endif
  }

  Models::AmqpValue LinkImpl::GetDesiredCapabilities() const
  {
#if ENABLE_UAMQP
    AMQP_VALUE desiredCapabilitiesVal;
    if (link_get_desired_capabilities(m_link, &desiredCapabilitiesVal))
    {
      throw std::runtime_error("Could not convert field to header.");
    }
    return Models::_detail::AmqpValueFactory::FromImplementation(
        Models::_detail::UniqueAmqpValueHandle{desiredCapabilitiesVal});
#else
    return {};
#endif
  }

  void LinkImpl::SubscribeToDetachEvent(OnLinkDetachEvent onLinkDetach)
  {
#if ENABLE_UAMQP
    m_onLinkDetachEvent = std::move(onLinkDetach);
    m_linkSubscriptionHandle
        = link_subscribe_on_link_detach_received(m_link, OnLinkDetachEventFn, this);
#else
    (void)onLinkDetach;
#endif
  }

  void LinkImpl::UnsubscribeFromDetachEvent()
  {
#if ENABLE_UAMQP
    if (m_linkSubscriptionHandle != nullptr)
    {
      link_unsubscribe_on_link_detach_received(m_linkSubscriptionHandle);
      m_linkSubscriptionHandle = nullptr;
    }
#endif
  }
#if ENABLE_UAMQP
  void LinkImpl::OnLinkDetachEventFn(void* context, ERROR_HANDLE error)
  {
    LinkImpl* link = static_cast<LinkImpl*>(context);
    if (link->m_onLinkDetachEvent)
    {
      link->m_onLinkDetachEvent(Models::_detail::AmqpErrorFactory::FromImplementation(error));
    }
  }

  void LinkImpl::OnLinkFlowOnFn(void* context)
  {
    LinkImpl* link = static_cast<LinkImpl*>(context);
    if (link->m_eventHandler)
    {
      link->m_eventHandler->OnLinkFlowOn(link->shared_from_this());
    }
  }
  namespace {
    LinkState LinkStateFromLINK_STATE(LINK_STATE state)
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
          return LinkState::HalfAttachedAttachReceived;
        case LINK_STATE_HALF_ATTACHED_ATTACH_SENT:
          return LinkState::HalfAttachedAttachSent;
        case LINK_STATE_INVALID:
        default:
          return LinkState::Invalid;
      }
    }
  } // namespace
  void LinkImpl::OnLinkStateChangedFn(void* context, LINK_STATE newState, LINK_STATE oldState)
  {
    LinkImpl* link = static_cast<LinkImpl*>(context);
    if (link->m_eventHandler)
    {
      link->m_eventHandler->OnLinkStateChanged(
          link->shared_from_this(),
          LinkStateFromLINK_STATE(newState),
          LinkStateFromLINK_STATE(oldState));
    }
  }

  AMQP_VALUE LinkImpl::OnTransferReceivedFn(
      void* context,
      TRANSFER_HANDLE transfer,
      uint32_t payload_size,
      const unsigned char* payload_bytes)
  {
    LinkImpl* link = static_cast<LinkImpl*>(context);
    if (link->m_eventHandler)
    {

      return Models::_detail::AmqpValueFactory::ToImplementation(
          link->m_eventHandler->OnTransferReceived(
              link->shared_from_this(),
              Models::_detail::AmqpTransferFactory::FromImplementation(transfer),
              payload_size,
              payload_bytes));
    }
    return nullptr;
  }
#endif

  void LinkImpl::Poll()
  {
#if ENABLE_UAMQP
    // Ensure that the connection hierarchy's state is not modified while polling on the link.
    auto lock{m_session->GetConnection()->Lock()};
    link_dowork(m_link);
#endif
  }

  void LinkImpl::ResetLinkCredit(std::uint32_t linkCredit, bool drain)
  {
#if ENABLE_UAMQP
    if (link_reset_link_credit(m_link, linkCredit, drain))
    {
      throw std::runtime_error("Could not reset link credit.");
    }
#else
    (void)linkCredit;
    (void)drain;
#endif
  }

  void LinkImpl::Attach()
  {
#if ENABLE_UAMQP
    {
      auto lock{m_session->GetConnection()->Lock()};
      if (link_attach(m_link, OnTransferReceivedFn, OnLinkStateChangedFn, OnLinkFlowOnFn, this))
      {
        throw std::runtime_error("Could not set attach properties.");
      }
    }
#endif
    // Mark the connection as async so that we can use the async APIs.
    m_session->GetConnection()->EnableAsyncOperation(true);
  }
  void LinkImpl::Detach(
      bool close,
      std::string const& condition,
      std::string const& description,
      Models::AmqpValue const& info)
  {
#if ENABLE_UAMQP
    {
      auto lock{m_session->GetConnection()->Lock()};
      if (link_detach(
              m_link,
              close,
              (condition.empty() ? nullptr : condition.c_str()),
              (description.empty() ? nullptr : description.c_str()),
              Models::_detail::AmqpValueFactory::ToImplementation(info)))
      {
        throw std::runtime_error("Could not set attach properties.");
      }
    }
#else
    (void)close;
    (void)condition;
    (void)description;
    (void)info;
#endif
    m_session->GetConnection()->EnableAsyncOperation(false);
  }

#if ENABLE_UAMQP
  template <typename CompleteFn> struct RewriteTransferComplete
  {
    static void OnOperation(
        CompleteFn onComplete,
        delivery_number deliveryId,
        LINK_DELIVERY_SETTLE_REASON reason,
        AMQP_VALUE disposition)
    {
      LinkDeliverySettleReason result{};
      switch (reason)
      {
        case LINK_DELIVERY_SETTLE_REASON_CANCELLED:
          result = LinkDeliverySettleReason::Cancelled;
          break;
        case LINK_DELIVERY_SETTLE_REASON_INVALID:
          result = LinkDeliverySettleReason::Invalid;
          break;
        case LINK_DELIVERY_SETTLE_REASON_NOT_DELIVERED:
          result = LinkDeliverySettleReason::NotDelivered;
          break;
        case LINK_DELIVERY_SETTLE_REASON_DISPOSITION_RECEIVED:
          result = LinkDeliverySettleReason::DispositionReceived;
          break;
        case LINK_DELIVERY_SETTLE_REASON_SETTLED:
          result = LinkDeliverySettleReason::Settled;
          break;
        case LINK_DELIVERY_SETTLE_REASON_TIMEOUT:
          result = LinkDeliverySettleReason::Timeout;
          break;
      }

      // Reference disposition so that we don't over-release when the AmqpValue passed to
      // OnComplete is destroyed.
      onComplete(
          static_cast<uint32_t>(deliveryId),
          result,
          Models::_detail::AmqpValueFactory::FromImplementation(
              Models::_detail::UniqueAmqpValueHandle{amqpvalue_clone(disposition)}));
    }
  };
#endif

  std::tuple<uint32_t, LinkDeliverySettleReason, Models::AmqpValue> LinkImpl::Transfer(
      std::vector<uint8_t> const& payload,
      Azure::Core::Context const& context)

  {
#if ENABLE_UAMQP
    {

      auto lock{m_session->GetConnection()->Lock()};

      auto onTransferComplete =
          [this](
              uint32_t deliveryId, LinkDeliverySettleReason settleReason, Models::AmqpValue value) {
            m_transferCompleteQueue.CompleteOperation(deliveryId, settleReason, value);
          };
      using MessageSendCompleteCallback = std::function<void(
          uint32_t deliveryId,
          LinkDeliverySettleReason reason,
          Models::AmqpValue const& deliveryState)>;

      auto operation(
          std::make_unique<Azure::Core::Amqp::Common::_internal::CompletionOperation<
              MessageSendCompleteCallback,
              RewriteTransferComplete<MessageSendCompleteCallback>>>(onTransferComplete));
      PAYLOAD payloadToSend{payload.data(), payload.size()};
      LINK_TRANSFER_RESULT transferResult;
      auto asyncResult = link_transfer_async(
          m_link,
          0,
          &payloadToSend,
          1,
          std::remove_pointer<decltype(operation)::element_type>::type::OnOperationFn,
          operation.release(),
          &transferResult,
          0 /*timeout*/);
      if (asyncResult == nullptr)
      {
        std::stringstream ss;
        ss << "Could not send message: " << transferResult;
        throw std::runtime_error(ss.str());
      }
    }

    auto result = m_transferCompleteQueue.WaitForResult(context);
    if (result)
    {
      return std::move(*result);
    }
    throw std::runtime_error("Error transferring data");
#else
    (void)payload;
    (void)context;
    throw std::runtime_error("Not implemented.");
#endif
  }

}}}} // namespace Azure::Core::Amqp::_detail
