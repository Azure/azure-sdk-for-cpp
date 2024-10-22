// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// cspell: words amqpclaimsbasedsecurity
#include "azure/core/amqp/internal/claims_based_security.hpp"
#include "azure/core/amqp/internal/common/global_state.hpp"
#include "azure/core/amqp/internal/common/runtime_context.hpp"
#include "claims_based_security_impl.hpp"
#include "session_impl.hpp"

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;
using namespace Azure::Core::Amqp::RustInterop::_detail;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using namespace Azure::Core::Amqp::_internal;

  void UniqueHandleHelper<RustAmqpClaimsBasedSecurity>::FreeAmqpCbs(
      RustAmqpClaimsBasedSecurity* obj)
  {
    amqpclaimsbasedsecurity_destroy(obj);
  }

  ClaimsBasedSecurityImpl::ClaimsBasedSecurityImpl(std::shared_ptr<_detail::SessionImpl> session)
      : m_session{session}
  {
    if (!session)
    {
      throw std::runtime_error("Session is required to create ClaimsBasedSecurity");
    }

    Common::_detail::CallContext callContext(
        Common::_detail::GlobalStateHolder::GlobalStateInstance()->GetRuntimeContext(), {});
    RustAmqpClaimsBasedSecurity* cbs;
    if (amqpclaimsbasedsecurity_create(
            callContext.GetCallContext(), session->GetAmqpSession().get(), &cbs))
    {
      throw std::runtime_error("Could not create Claims Based Security" + callContext.GetError());
    }
    m_claimsBasedSecurity = UniqueAmqpCbsHandle{cbs};
  }

  ClaimsBasedSecurityImpl::~ClaimsBasedSecurityImpl() noexcept {}

  CbsOpenResult ClaimsBasedSecurityImpl::Open(Context const& context)
  {
    Common::_detail::CallContext callContext(
        Common::_detail::GlobalStateHolder::GlobalStateInstance()->GetRuntimeContext(), context);
    if (amqpclaimsbasedsecurity_attach(callContext.GetCallContext(), m_claimsBasedSecurity.get()))
    {
      throw std::runtime_error("Could not open Claims Based Security" + callContext.GetError());
    }
    else
    {
      return CbsOpenResult::Ok;
    }
  }
  void ClaimsBasedSecurityImpl::Close(Context const& context)
  {
    Common::_detail::CallContext callContext(
        Common::_detail::GlobalStateHolder::GlobalStateInstance()->GetRuntimeContext(), context);

    if (amqpclaimsbasedsecurity_detach_and_release(
            callContext.GetCallContext(), m_claimsBasedSecurity.release()))
    {
      throw std::runtime_error("Could not close claims based security: " + callContext.GetError());
    }
  }

  std::tuple<CbsOperationResult, uint32_t, std::string> ClaimsBasedSecurityImpl::PutToken(
      CbsTokenType tokenType,
      std::string const& audience,
      std::string const& token,
      Azure::DateTime const& expirationTime,
      Context const& context)
  {
    Common::_detail::CallContext callContext(
        Common::_detail::GlobalStateHolder::GlobalStateInstance()->GetRuntimeContext(), context);

    // The Rust AMQP implementation only supports JWT tokens.
    if (tokenType != CbsTokenType::Jwt)
    {
      throw std::runtime_error("Unsupported Token Type");
    }

    if (amqpclaimsbasedsecurity_authorize_path(
            callContext.GetCallContext(),
            m_claimsBasedSecurity.get(),
            audience.c_str(),
            token.c_str(),
            std::chrono::duration_cast<std::chrono::seconds>(expirationTime.time_since_epoch())
                .count()))
    {
      throw std::runtime_error("Could not put token" + callContext.GetError());
    }
    return std::make_tuple<CbsOperationResult, uint32_t, std::string>(
        CbsOperationResult::Ok, 200, {});
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
}}}} // namespace Azure::Core::Amqp::_detail
