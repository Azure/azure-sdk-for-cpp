// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include "azure/core/amqp/claims_based_security.hpp"
#include "azure/core/amqp/management.hpp"

#include <azure_uamqp_c/cbs.h>

struct CBS_INSTANCE_TAG;

namespace Azure { namespace Core { namespace _internal {

  template <> struct UniqueHandleHelper<CBS_INSTANCE_TAG>
  {
    static void FreeAmqpCbs(CBS_HANDLE obj);

    using type = BasicUniqueHandle<CBS_INSTANCE_TAG, FreeAmqpCbs>;
  };
}}} // namespace Azure::Core::_internal

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using UniqueAmqpCbsHandle = Azure::Core::_internal::UniqueHandle<CBS_INSTANCE_TAG>;

  class ClaimsBasedSecurityImpl final : public _internal::ManagementClientEvents {

  public:
    ClaimsBasedSecurityImpl(std::shared_ptr<_detail::SessionImpl> session);
    virtual ~ClaimsBasedSecurityImpl() noexcept;

    // Disable copy and move because the underlying m_cbs takes a reference to this object.
    ClaimsBasedSecurityImpl(ClaimsBasedSecurityImpl const&) = delete;
    ClaimsBasedSecurityImpl& operator=(ClaimsBasedSecurityImpl const&) = delete;
    ClaimsBasedSecurityImpl(ClaimsBasedSecurityImpl&&) noexcept = delete;
    ClaimsBasedSecurityImpl& operator=(ClaimsBasedSecurityImpl&&) noexcept = delete;

    CbsOpenResult Open(Context const& context);
    void Close();
    std::tuple<CbsOperationResult, uint32_t, std::string> PutToken(
        CbsTokenType type,
        std::string const& audience,
        std::string const& token,
        Context const& context);

  private:
    std::shared_ptr<_detail::SessionImpl> m_session;
    std::shared_ptr<_detail::ManagementClientImpl> m_management;
    // Inherited via ManagementClientEvents
    void OnError(Models::_internal::AmqpError const& error) override;
  };
}}}} // namespace Azure::Core::Amqp::_detail
