// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/claims_based_security.hpp"
#include "azure/core/amqp/internal/management.hpp"
#include "unique_handle.hpp"

struct CBS_INSTANCE_TAG;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  template <> struct UniqueHandleHelper<CBS_INSTANCE_TAG>
  {
    static void FreeAmqpCbs(CBS_HANDLE obj);

    using type = Core::_internal::BasicUniqueHandle<CBS_INSTANCE_TAG, FreeAmqpCbs>;
  };
#endif
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  using UniqueAmqpCbsHandle = UniqueHandle<CBS_INSTANCE_TAG>;
#endif // ENABLE_UAMQP
  class ClaimsBasedSecurityImpl final
#if ENABLE_UAMQP
      : public _internal::ManagementClientEvents
#endif
  {

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
        Context const& context);

  private:
    std::shared_ptr<_detail::SessionImpl> m_session;
    std::shared_ptr<_detail::ManagementClientImpl> m_management;
#if ENABLE_UAMQP
    // Inherited via ManagementClientEvents
    void OnError(Models::_internal::AmqpError const& error) override;
#endif
  };
}}}} // namespace Azure::Core::Amqp::_detail
