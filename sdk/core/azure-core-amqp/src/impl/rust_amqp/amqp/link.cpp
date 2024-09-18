// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/link.hpp"

#include "../../../models/private/error_impl.hpp"
#include "../../../models/private/performatives/transfer_impl.hpp"
#include "../../../models/private/value_impl.hpp"
#include "azure/core/amqp/internal/common/completion_operation.hpp"
#include "azure/core/amqp/internal/common/global_state.hpp"
#include "azure/core/amqp/internal/models/message_source.hpp"
#include "azure/core/amqp/internal/models/message_target.hpp"
#include "private/link_impl.hpp"
#include "private/session_impl.hpp"

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

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
    auto connectionLock{m_session->GetConnection()->Lock()};
    (void)name;
    (void)role;
    (void)sourceValue;
    (void)targetValue;
  }

  LinkImpl::~LinkImpl() noexcept {}

  Models::_internal::MessageSource const& LinkImpl::GetSource() const { return m_source; }
  Models::_internal::MessageTarget const& LinkImpl::GetTarget() const { return m_target; }

  void LinkImpl::SetMaxMessageSize(uint64_t size) { (void)size; }
  uint64_t LinkImpl::GetMaxMessageSize() const
  {
    uint64_t maxMessageSize = {};
    return maxMessageSize;
  }

  std::string LinkImpl::GetName() const
  {
    const char* name = {};
    return name;
  }
  _internal::SenderSettleMode LinkImpl::GetSenderSettleMode() const { return {}; }
  void LinkImpl::SetSenderSettleMode(_internal::SenderSettleMode mode) { (void)mode; }

  _internal::ReceiverSettleMode LinkImpl::GetReceiverSettleMode() const { return {}; }
  void LinkImpl::SetReceiverSettleMode(_internal::ReceiverSettleMode mode) { (void)mode; }

  uint32_t LinkImpl::GetInitialDeliveryCount() const
  {
    uint32_t deliveryCount = {};
    return deliveryCount;
  }

  uint64_t LinkImpl::GetPeerMaxMessageSize() const
  {
    uint64_t peerMax = {};
    return peerMax;
  }

  uint32_t LinkImpl::GetReceivedMessageId() const
  {
    uint32_t messageId = {};
    return messageId;
  }

  void LinkImpl::SetInitialDeliveryCount(uint32_t count) { (void)count; }
  void LinkImpl::SetAttachProperties(Models::AmqpValue properties) { (void)properties; }
  void LinkImpl::SetMaxLinkCredit(uint32_t credit) { (void)credit; }

  void LinkImpl::SetDesiredCapabilities(Models::AmqpValue desiredCapabilities)
  {
    (void)desiredCapabilities;
  }

  Models::AmqpValue LinkImpl::GetDesiredCapabilities() const { return {}; }

  void LinkImpl::ResetLinkCredit(std::uint32_t linkCredit, bool drain)
  {
    (void)linkCredit;
    (void)drain;
  }

  void LinkImpl::Attach() {}
  void LinkImpl::Detach(
      bool close,
      std::string const& condition,
      std::string const& description,
      Models::AmqpValue const& info)
  {
    (void)close;
    (void)condition;
    (void)description;
    (void)info;
  }

  std::tuple<uint32_t, LinkDeliverySettleReason, Models::AmqpValue> LinkImpl::Transfer(
      std::vector<uint8_t> const& payload,
      Azure::Core::Context const& context)

  {
    (void)payload;
    (void)context;
    throw std::runtime_error("Not implemented.");
  }
}}}} // namespace Azure::Core::Amqp::_detail
