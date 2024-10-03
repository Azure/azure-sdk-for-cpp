// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "claims_based_security_impl.hpp"
#include "session_impl.hpp"

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using namespace Azure::Core::Amqp::_internal;

  // The non-Impl types for CBS exist only for testing purposes.
#if defined(_azure_TESTING_BUILD)
  ClaimsBasedSecurity::ClaimsBasedSecurity(Session const& session)
      : m_impl{std::make_shared<_detail::ClaimsBasedSecurityImpl>(SessionFactory::GetImpl(session))}
  {
  }

  ClaimsBasedSecurity::~ClaimsBasedSecurity() noexcept {}

  CbsOpenResult ClaimsBasedSecurity::Open(Context const& context) { return m_impl->Open(context); }
  void ClaimsBasedSecurity::Close(Context const& context) { m_impl->Close(context); }

  std::tuple<CbsOperationResult, uint32_t, std::string> ClaimsBasedSecurity::PutToken(
      CbsTokenType tokenType,
      std::string const& audience,
      std::string const& token,
      Azure::DateTime const& tokenExpirationTime,
      Context const& context)

  {
    return m_impl->PutToken(tokenType, audience, token, tokenExpirationTime, context);
  }

#endif // _azure_TESTING_BUILD

}}}} // namespace Azure::Core::Amqp::_detail
