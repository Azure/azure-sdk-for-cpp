// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../../../../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/internal/claims_based_security.hpp"
#include "azure/core/amqp/internal/management.hpp"

#include <azure_uamqp_c/cbs.h>
struct CBS_INSTANCE_TAG;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  template <> struct UniqueHandleHelper<CBS_INSTANCE_TAG>
  {
    static void FreeAmqpCbs(CBS_HANDLE obj);

    using type = Core::_internal::BasicUniqueHandle<CBS_INSTANCE_TAG, FreeAmqpCbs>;
  };
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using UniqueAmqpCbsHandle = UniqueHandle<CBS_INSTANCE_TAG>;
  class ClaimsBasedSecurityImpl final : public _internal::ManagementClientEvents {

  public:
    ClaimsBasedSecurityImpl(std::shared_ptr<_detail::SessionImpl> session);
    virtual ~ClaimsBasedSecurityImpl() noexcept;

    // Disable copy and move because the underlying m_cbs takes a reference to this object.
    ClaimsBasedSecurityImpl(ClaimsBasedSecurityImpl const&) = delete;
    ClaimsBasedSecurityImpl& operator=(ClaimsBasedSecurityImpl const&) = delete;
    ClaimsBasedSecurityImpl(ClaimsBasedSecurityImpl&&) noexcept = delete;
    ClaimsBasedSecurityImpl& operator=(ClaimsBasedSecurityImpl&&) noexcept = delete;

    _azure_NODISCARD CbsOpenResult Open(Context const& context);
    void Close(Context const& context);
    _azure_NODISCARD std::tuple<CbsOperationResult, uint32_t, std::string> PutToken(
        CbsTokenType type,
        std::string const& audience,
        std::string const& token,
        Azure::DateTime const& expirationTime,
        Context const& context);

  private:
    std::shared_ptr<_detail::SessionImpl> m_session;
    std::shared_ptr<_detail::ManagementClientImpl> m_management;
    // Inherited via ManagementClientEvents
    void OnError(Models::_internal::AmqpError const& error) override;
  };
}}}} // namespace Azure::Core::Amqp::_detail
