// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/claims_based_security.hpp"
#include "azure/core/amqp/connection.hpp"
#include "azure/core/amqp/private/claims_based_security_impl.hpp"
#include "azure/core/amqp/private/session_impl.hpp"
#include <iostream>

namespace Azure { namespace Core { namespace _internal { namespace Amqp {
  ClaimBasedSecurity::ClaimBasedSecurity(Session const& session, Connection const& connection)
      : m_impl{std::make_shared<_detail::CbsImpl>(session, connection)}
  {
  }

  ClaimBasedSecurity::~ClaimBasedSecurity() noexcept {}

  CbsOpenResult ClaimBasedSecurity::Open(Azure::Core::Context context)
  {
    return m_impl->Open(context);
  }
  void ClaimBasedSecurity::Close() { m_impl->Close(); }

  std::tuple<CbsOperationResult, uint32_t, std::string> ClaimBasedSecurity::PutToken(
      CbsTokenType tokenType,
      std::string const& audience,
      std::string const& token,
      Azure::Core::Context context)

  {
    return m_impl->PutToken(tokenType, audience, token, context);
  }

  void ClaimBasedSecurity::SetTrace(bool traceOn) { m_impl->SetTrace(traceOn); }

  namespace _detail {

    CbsImpl::CbsImpl(Session const& session, Connection const& connection)
        : m_connectionToPoll(connection)
    {
      m_cbs = cbs_create(*(session.GetImpl()));
      cbs_set_trace(m_cbs, true);
    }

    CbsImpl::~CbsImpl() noexcept
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
        default:
          throw std::logic_error("Unknown CBS Open result.");
      }
    }

    CbsOperationResult CbsOperationResultStateFromLowLevel(CBS_OPERATION_RESULT lowLevel)
    {
      switch (lowLevel)
      {
        case CBS_OPERATION_RESULT_CBS_ERROR:
          return CbsOperationResult::Error;
        case CBS_OPERATION_RESULT_INSTANCE_CLOSED:
          return CbsOperationResult::InstanceClosed;
        case CBS_OPERATION_RESULT_INVALID:
          return CbsOperationResult::Invalid;
        case CBS_OPERATION_RESULT_OK:
          return CbsOperationResult::Ok;
        case CBS_OPERATION_RESULT_OPERATION_FAILED:
          return CbsOperationResult::Failed;
        default: // LCOV_EXCL_LINE
          throw std::logic_error("Unknown CBS Operation result."); // LCOV_EXCL_LINE
      }
    }

    void CbsImpl::OnCbsErrorFn(void* context)
    {
      auto cbs = static_cast<CbsImpl*>(const_cast<void*>(context));
      (void)cbs;
    }

    void CbsImpl::OnCbsOpenCompleteFn(void* context, CBS_OPEN_COMPLETE_RESULT openCompleteResult)
    {
      auto cbs = static_cast<CbsImpl*>(const_cast<void*>(context));
      cbs->m_openResultQueue.CompleteOperation(CbsOpenResultStateFromLowLevel(openCompleteResult));
    }

    const char* OperationResultStringFromLowLevel(CBS_OPERATION_RESULT result)
    {
      switch (result)
      {
        case CBS_OPERATION_RESULT_CBS_ERROR:
          return "CbsOperationResult::Error";
        case CBS_OPERATION_RESULT_INSTANCE_CLOSED:
          return "CbsOperationResult::InstanceClosed";
        case CBS_OPERATION_RESULT_INVALID:
          return "CbsOperationResult::Invalid";
        case CBS_OPERATION_RESULT_OK:
          return "CbsOperationResult::Ok";
        case CBS_OPERATION_RESULT_OPERATION_FAILED:
          return "CbsOperationResult::Failed";
        default: // LCOV_EXCL_LINE
          throw std::logic_error("Unknown CBS Operation result."); // LCOV_EXCL_LINE
      }
    }

    void CbsImpl::OnCbsOperationCompleteFn(
        void* context,
        CBS_OPERATION_RESULT operationCompleteResult,
        uint32_t statusCode,
        const char* statusDescription)
    {
      auto cbs = static_cast<CbsImpl*>(const_cast<void*>(context));
      std::cout << "OnCbsOperationComplete: "
                << OperationResultStringFromLowLevel(operationCompleteResult)
                << " StatusCode: " << statusCode << " StatusDescription: "
                << (statusDescription ? std::string(statusDescription) : "(NULL)") << std::endl;
      cbs->m_operationResultQueue.CompleteOperation(
          CbsOperationResultStateFromLowLevel(operationCompleteResult),
          statusCode,
          statusDescription ? statusDescription : std::string());
    }

    CbsOpenResult CbsImpl::Open(Azure::Core::Context context)
    {
      if (cbs_open_async(m_cbs, CbsImpl::OnCbsOpenCompleteFn, this, OnCbsErrorFn, this))
      {
        return CbsOpenResult::Error;
      }
      auto result = m_openResultQueue.WaitForPolledResult(context, m_connectionToPoll);
      return std::get<0>(*result);
    }

    void CbsImpl::Close()
    {
      if (cbs_close(m_cbs))
      {
        throw std::runtime_error("Could not close cbs");
      }
    }

    void CbsImpl::SetTrace(bool traceOn) { cbs_set_trace(m_cbs, traceOn); }

    std::tuple<CbsOperationResult, uint32_t, std::string> CbsImpl::PutToken(
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
            "Could not authenticate to client. Error Status: "
            + std::to_string(std::get<1>(*result)) + " reason: " + std::get<2>(*result));
      }
      return std::move(*result);
    }

  } // namespace _detail
}}}} // namespace Azure::Core::_internal::Amqp