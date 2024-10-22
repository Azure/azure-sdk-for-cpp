// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/claims_based_security.hpp"
#include "azure/core/amqp/internal/management.hpp"
#include "rust_amqp_wrapper.h"
#include "unique_handle.hpp"

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  template <>
  struct UniqueHandleHelper<Azure::Core::Amqp::RustInterop::_detail::RustAmqpClaimsBasedSecurity>
  {
    static void FreeAmqpCbs(
        Azure::Core::Amqp::RustInterop::_detail::RustAmqpClaimsBasedSecurity* obj);

    using type = Core::_internal::BasicUniqueHandle<
        Azure::Core::Amqp::RustInterop::_detail::RustAmqpClaimsBasedSecurity,
        FreeAmqpCbs>;
  };
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using UniqueAmqpCbsHandle
      = UniqueHandle<Azure::Core::Amqp::RustInterop::_detail::RustAmqpClaimsBasedSecurity>;

  class ClaimsBasedSecurityImpl final {

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
        Azure::DateTime const& tokenExpirationTime,
        Context const& context);

  private:
    std::shared_ptr<_detail::SessionImpl> m_session;
    UniqueAmqpCbsHandle m_claimsBasedSecurity;
  };
}}}} // namespace Azure::Core::Amqp::_detail
