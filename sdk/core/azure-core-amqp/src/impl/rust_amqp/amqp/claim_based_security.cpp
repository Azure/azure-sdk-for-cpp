// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "private/claims_based_security_impl.hpp"
#include "private/connection_impl.hpp"
#include "private/management_impl.hpp"
#include "private/session_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using namespace Azure::Core::Amqp::_internal;

  ClaimsBasedSecurityImpl::ClaimsBasedSecurityImpl(std::shared_ptr<_detail::SessionImpl> session)
      : m_session{session}
  {
  }

  ClaimsBasedSecurityImpl::~ClaimsBasedSecurityImpl() noexcept {}

  CbsOpenResult ClaimsBasedSecurityImpl::Open(Context const& context)
  {
    throw std::runtime_error("Not yet implemented.");
    (void)context;
  }

  void ClaimsBasedSecurityImpl::Close(Context const& context) { m_management->Close(context); }

  std::tuple<CbsOperationResult, uint32_t, std::string> ClaimsBasedSecurityImpl::PutToken(
      CbsTokenType tokenType,
      std::string const& audience,
      std::string const& token,
      Context const& context)
  {
    throw std::runtime_error("Not yet implemented.");
    (void)tokenType;
    (void)audience;
    (void)token;
    (void)context;
  }

  std::ostream& operator<<(std::ostream& os, CbsOperationResult operationResult)
  {
    switch (operationResult)
    {
      case CbsOperationResult::Invalid:
        os << "Invalid";
        break;
      case CbsOperationResult::Ok:
        os << "Ok";
        break;
      case CbsOperationResult::Error:
        os << "Error";
        break;
      case CbsOperationResult::Failed:
        os << "Failed";
        break;
      case CbsOperationResult::InstanceClosed:
        os << "InstanceClosed";
        break;
      case CbsOperationResult::Cancelled:
        os << "Cancelled";
        break;
    }
    return os;
  }

  std::ostream& operator<<(std::ostream& os, CbsOpenResult openResult)
  {
    switch (openResult)
    {
      case CbsOpenResult::Invalid:
        os << "Invalid";
        break;
      case CbsOpenResult::Ok:
        os << "Ok";
        break;
      case CbsOpenResult::Error:
        os << "Error";
        break;
      case CbsOpenResult::Cancelled:
        os << "Cancelled";
        break;
    }
    return os;
  }
#if ENABLE_UAMQP
  void ClaimsBasedSecurityImpl::OnError(Models::_internal::AmqpError const& error)
  {
    Log::Stream(Logger::Level::Warning) << "AMQP Error processing ClaimsBasedSecurity: " << error;
  }
#endif

}}}} // namespace Azure::Core::Amqp::_detail
