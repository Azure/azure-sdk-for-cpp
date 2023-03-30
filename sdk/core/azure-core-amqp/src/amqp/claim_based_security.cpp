// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/claims_based_security.hpp"
#include "azure/core/amqp/connection.hpp"
#include "azure/core/amqp/private/claims_based_security_impl.hpp"
#include "azure/core/amqp/private/session_impl.hpp"
#include <iostream>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using namespace Azure::Core::Amqp::_internal;

  ClaimsBasedSecurity::ClaimsBasedSecurity(Session const& session, Connection const& connection)
      : m_impl{std::make_shared<_detail::ClaimsBasedSecurityImpl>(session, connection)}
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

  ClaimsBasedSecurityImpl::ClaimsBasedSecurityImpl(
      Session const& session,
      Connection const& connection)
      : m_connectionToPoll(connection)
  {
    m_cbs = cbs_create(*(session.GetImpl()));
    cbs_set_trace(m_cbs, true);
  }

  ClaimsBasedSecurityImpl::~ClaimsBasedSecurityImpl() noexcept
  {
    if (m_cbs)
    {
      cbs_destroy(m_cbs);
      m_cbs = nullptr;
    }
  }

  CbsOpenResult CbsOpenResultStateFromLowLevel(CBS_OPEN_COMPLETE_RESULT lowLevel)
  {
    switch (lowLevel)
    {
      case CBS_OPEN_OK:
        return CbsOpenResult::Ok;
      case CBS_OPEN_CANCELLED:
        return CbsOpenResult::Cancelled;
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
            m_cbs, ClaimsBasedSecurityImpl::OnCbsOpenCompleteFn, this, OnCbsErrorFn, this))
    {
      return CbsOpenResult::Error;
    }
    auto result = m_openResultQueue.WaitForPolledResult(context, m_connectionToPoll);
    return std::get<0>(*result);
  }

  void ClaimsBasedSecurityImpl::Close()
  {
    if (cbs_close(m_cbs))
    {
      throw std::runtime_error("Could not close cbs");
    }
  }

  void ClaimsBasedSecurityImpl::SetTrace(bool traceOn) { cbs_set_trace(m_cbs, traceOn); }

  std::tuple<CbsOperationResult, uint32_t, std::string> ClaimsBasedSecurityImpl::PutToken(
      CbsTokenType tokenType,
      std::string const& audience,
      std::string const& token,
      Azure::Core::Context context)
  {
    if (cbs_put_token_async(
            m_cbs,
            (tokenType == CbsTokenType::Jwt ? "jwt" : "servicebus.windows.net:sastoken"),
            audience.c_str(),
            token.c_str(),
            OnCbsOperationCompleteFn,
            this)
        == nullptr)
    {
      throw std::runtime_error("Could not put CBS token.");
    }
    auto result = m_operationResultQueue.WaitForPolledResult(context, m_connectionToPoll);

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
