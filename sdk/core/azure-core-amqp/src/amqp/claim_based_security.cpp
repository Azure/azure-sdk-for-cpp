// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/claims_based_security.hpp"
#include "azure/core/amqp/connection.hpp"
#include "azure/core/amqp/private/claims_based_security_impl.hpp"
#include "azure/core/amqp/private/connection_impl.hpp"
#include "azure/core/amqp/private/session_impl.hpp"
#include <iostream>

void Azure::Core::_internal::UniqueHandleHelper<CBS_INSTANCE_TAG>::FreeAmqpCbs(CBS_HANDLE value)
{
  cbs_destroy(value);
}

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using namespace Azure::Core::Amqp::_internal;

  ClaimsBasedSecurity::ClaimsBasedSecurity(Session const& session)
      : m_impl{std::make_shared<_detail::ClaimsBasedSecurityImpl>(session.GetImpl())}
  {
  }

  ClaimsBasedSecurity::~ClaimsBasedSecurity() noexcept {}

  CbsOpenResult ClaimsBasedSecurity::Open(Azure::Core::Context context)
  {
    return m_impl->Open(context);
  }
  void ClaimsBasedSecurity::Close() { m_impl->Close(); }

  std::tuple<CbsOperationResult, uint32_t, std::string> ClaimsBasedSecurity::PutToken(
      CbsTokenType tokenType,
      std::string const& audience,
      std::string const& token,
      Azure::Core::Context context)

  {
    return m_impl->PutToken(tokenType, audience, token, context);
  }

  void ClaimsBasedSecurity::SetTrace(bool traceOn) { m_impl->SetTrace(traceOn); }

  ClaimsBasedSecurityImpl::ClaimsBasedSecurityImpl(std::shared_ptr<_detail::SessionImpl> session)
      : m_session{session}, m_cbs(cbs_create(*session))
  {
  }

  ClaimsBasedSecurityImpl::~ClaimsBasedSecurityImpl() noexcept {}

  CbsOpenResult CbsOpenResultStateFromLowLevel(CBS_OPEN_COMPLETE_RESULT lowLevel)
  {
    switch (lowLevel)
    {
      case CBS_OPEN_OK:
        return CbsOpenResult::Ok;
      case CBS_OPEN_CANCELLED: // LCOV_EXCL_LINE
        return CbsOpenResult::Cancelled; // LCOV_EXCL_LINE
      case CBS_OPEN_ERROR:
        return CbsOpenResult::Error;
      default: // LCOV_EXCL_LINE
        throw std::logic_error("Unknown CBS Open result."); // LCOV_EXCL_LINE
    }
  }

  CbsOperationResult CbsOperationResultStateFromLowLevel(CBS_OPERATION_RESULT lowLevel)
  {
    switch (lowLevel)
    {
      case CBS_OPERATION_RESULT_CBS_ERROR: // LCOV_EXCL_LINE
        return CbsOperationResult::Error; // LCOV_EXCL_LINE
      case CBS_OPERATION_RESULT_INSTANCE_CLOSED: // LCOV_EXCL_LINE
        return CbsOperationResult::InstanceClosed; // LCOV_EXCL_LINE
      case CBS_OPERATION_RESULT_INVALID: // LCOV_EXCL_LINE
        return CbsOperationResult::Invalid; // LCOV_EXCL_LINE
      case CBS_OPERATION_RESULT_OK:
        return CbsOperationResult::Ok;
      case CBS_OPERATION_RESULT_OPERATION_FAILED: // LCOV_EXCL_LINE
        return CbsOperationResult::Failed; // LCOV_EXCL_LINE
      default: // LCOV_EXCL_LINE
        throw std::logic_error("Unknown CBS Operation result."); // LCOV_EXCL_LINE
    }
  }

  void ClaimsBasedSecurityImpl::OnCbsErrorFn(void* context)
  {
    auto cbs = static_cast<ClaimsBasedSecurityImpl*>(const_cast<void*>(context));
    (void)cbs;
  }

  void ClaimsBasedSecurityImpl::OnCbsOpenCompleteFn(
      void* context,
      CBS_OPEN_COMPLETE_RESULT openCompleteResult)
  {
    auto cbs = static_cast<ClaimsBasedSecurityImpl*>(const_cast<void*>(context));
    cbs->m_openResultQueue.CompleteOperation(CbsOpenResultStateFromLowLevel(openCompleteResult));
  }

  const char* OperationResultStringFromLowLevel(CBS_OPERATION_RESULT result)
  {
    switch (result)
    {
      case CBS_OPERATION_RESULT_CBS_ERROR: // LCOV_EXCL_LINE
        return "CbsOperationResult::Error"; // LCOV_EXCL_LINE
      case CBS_OPERATION_RESULT_INSTANCE_CLOSED: // LCOV_EXCL_LINE
        return "CbsOperationResult::InstanceClosed"; // LCOV_EXCL_LINE
      case CBS_OPERATION_RESULT_INVALID: // LCOV_EXCL_LINE
        return "CbsOperationResult::Invalid"; // LCOV_EXCL_LINE
      case CBS_OPERATION_RESULT_OK:
        return "CbsOperationResult::Ok";
      case CBS_OPERATION_RESULT_OPERATION_FAILED: // LCOV_EXCL_LINE
        return "CbsOperationResult::Failed"; // LCOV_EXCL_LINE
      default: // LCOV_EXCL_LINE
        throw std::logic_error("Unknown CBS Operation result."); // LCOV_EXCL_LINE
    }
  }

  void ClaimsBasedSecurityImpl::OnCbsOperationCompleteFn(
      void* context,
      CBS_OPERATION_RESULT operationCompleteResult,
      uint32_t statusCode,
      const char* statusDescription)
  {
    auto cbs = static_cast<ClaimsBasedSecurityImpl*>(const_cast<void*>(context));
    std::cout << "OnCbsOperationComplete: "
              << OperationResultStringFromLowLevel(operationCompleteResult)
              << " StatusCode: " << statusCode << " StatusDescription: "
              << (statusDescription ? std::string(statusDescription) : "(NULL)") << std::endl;
    cbs->m_operationResultQueue.CompleteOperation(
        CbsOperationResultStateFromLowLevel(operationCompleteResult),
        statusCode,
        statusDescription ? statusDescription : std::string());
  }

  CbsOpenResult ClaimsBasedSecurityImpl::Open(Azure::Core::Context context)
  {
    if (cbs_open_async(
            m_cbs.get(), ClaimsBasedSecurityImpl::OnCbsOpenCompleteFn, this, OnCbsErrorFn, this))
    {
      return CbsOpenResult::Error;
    }
    auto result = m_openResultQueue.WaitForPolledResult(context, *m_session->GetConnectionToPoll());
    return std::get<0>(*result);
  }

  void ClaimsBasedSecurityImpl::Close()
  {
    if (cbs_close(m_cbs.get()))
    {
      throw std::runtime_error("Could not close cbs"); // LCOV_EXCL_LINE
    }
  }

  void ClaimsBasedSecurityImpl::SetTrace(bool traceOn) { cbs_set_trace(m_cbs.get(), traceOn); }

  std::tuple<CbsOperationResult, uint32_t, std::string> ClaimsBasedSecurityImpl::PutToken(
      CbsTokenType tokenType,
      std::string const& audience,
      std::string const& token,
      Azure::Core::Context context)
  {
    if (cbs_put_token_async(
            m_cbs.get(),
            (tokenType == CbsTokenType::Jwt ? "jwt" : "servicebus.windows.net:sastoken"),
            audience.c_str(),
            token.c_str(),
            OnCbsOperationCompleteFn,
            this)
        == nullptr)
    {
      throw std::runtime_error("Could not put CBS token."); // LCOV_EXCL_LINE
    }
    auto result
        = m_operationResultQueue.WaitForPolledResult(context, *m_session->GetConnectionToPoll());

    // Throw an error if we failed to authenticate with the server.
    if (std::get<0>(*result) != CbsOperationResult::Ok)
    {
      throw std::runtime_error(
          "Could not authenticate to client. Error Status: " + std::to_string(std::get<1>(*result))
          + " reason: " + std::get<2>(*result));
    }
    return std::move(*result);
  }

}}}} // namespace Azure::Core::Amqp::_detail
