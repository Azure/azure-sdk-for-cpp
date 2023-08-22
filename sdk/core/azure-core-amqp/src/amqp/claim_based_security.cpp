// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#if defined(USE_UAMQP_CBS)
#include "azure/core/amqp/claims_based_security.hpp"
#include "azure/core/amqp/connection.hpp"
#endif

#include "private/claims_based_security_impl.hpp"
#include "private/connection_impl.hpp"
#include "private/management_impl.hpp"
#include "private/session_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include <iostream>
#include <sstream>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Core { namespace _internal {
  void UniqueHandleHelper<CBS_INSTANCE_TAG>::FreeAmqpCbs(CBS_HANDLE value) { cbs_destroy(value); }
}}} // namespace Azure::Core::_internal

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using namespace Azure::Core::Amqp::_internal;

  // The non-Impl types for CBS exist only for testing purposes.
#if defined(TESTING_BUILD)
  ClaimsBasedSecurity::ClaimsBasedSecurity(Session const& session)
      : m_impl{std::make_shared<_detail::ClaimsBasedSecurityImpl>(SessionFactory::GetImpl(session))}
  {
  }

  ClaimsBasedSecurity::~ClaimsBasedSecurity() noexcept {}

  CbsOpenResult ClaimsBasedSecurity::Open(Context const& context) { return m_impl->Open(context); }
  void ClaimsBasedSecurity::Close() { m_impl->Close(); }

  std::tuple<CbsOperationResult, uint32_t, std::string> ClaimsBasedSecurity::PutToken(
      CbsTokenType tokenType,
      std::string const& audience,
      std::string const& token,
      Context const& context)

  {
    return m_impl->PutToken(tokenType, audience, token, context);
  }

#if defined(USE_UAMQP_CBS)
  void ClaimsBasedSecurity::SetTrace(bool traceOn) { m_impl->SetTrace(traceOn); }
#endif

#endif // TESTING_BUILD

#if defined(USE_UAMQP_CBS)
  ClaimsBasedSecurityImpl::ClaimsBasedSecurityImpl(std::shared_ptr<_detail::SessionImpl> session)
      : m_cbs(cbs_create(*session)), m_session{session}
  {
  }
#else
  ClaimsBasedSecurityImpl::ClaimsBasedSecurityImpl(std::shared_ptr<_detail::SessionImpl> session)
      : m_session{session}
  {
  }
#endif

  ClaimsBasedSecurityImpl::~ClaimsBasedSecurityImpl() noexcept
  {
#if defined(USE_UAMQP_CBS)
    auto lock{m_session->GetConnection()->Lock()};

    m_cbs.reset();
#endif
  }

#if defined(USE_UAMQP_CBS)
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

  const char* OpenResultStringFromLowLevel(CBS_OPEN_COMPLETE_RESULT result)
  {
    switch (result)
    {
      case CBS_OPEN_OK:
        return "CbsOpenResult::Ok";
        // LCOV_EXCL_START
      case CBS_OPEN_CANCELLED:
        return "CbsOpenResult::Cancelled";
      case CBS_OPEN_ERROR:
        return "CbsOpenResult::Error";
      default:
        throw std::logic_error("Unknown CBS Open result.");
        // LCOV_EXCL_STOP
    }
  }

  void ClaimsBasedSecurityImpl::OnCbsOpenCompleteFn(
      void* context,
      CBS_OPEN_COMPLETE_RESULT openCompleteResult)
  {
    auto cbs = static_cast<ClaimsBasedSecurityImpl*>(const_cast<void*>(context));
    if (cbs->m_traceEnabled)
    {
      Log::Stream(Logger::Level::Informational)
          << "OnCbsOpenComplete: " << OpenResultStringFromLowLevel(openCompleteResult);
    }
    cbs->m_openResultQueue.CompleteOperation(CbsOpenResultStateFromLowLevel(openCompleteResult));
  }

  const char* OperationResultStringFromLowLevel(CBS_OPERATION_RESULT result)
  {
    switch (result)
    {
      case CBS_OPERATION_RESULT_OK:
        return "CbsOperationResult::Ok";

        // LCOV_EXCL_START
      case CBS_OPERATION_RESULT_CBS_ERROR:
        return "CbsOperationResult::Error";
      case CBS_OPERATION_RESULT_INSTANCE_CLOSED:
        return "CbsOperationResult::InstanceClosed";
      case CBS_OPERATION_RESULT_INVALID:
        return "CbsOperationResult::Invalid";
      case CBS_OPERATION_RESULT_OPERATION_FAILED:
        return "CbsOperationResult::Failed";
      default:
        throw std::logic_error("Unknown CBS Operation result.");
        // LCOV_EXCL_STOP
    }
  }

  void ClaimsBasedSecurityImpl::OnCbsOperationCompleteFn(
      void* context,
      CBS_OPERATION_RESULT operationCompleteResult,
      uint32_t statusCode,
      const char* statusDescription)
  {
    auto cbs = static_cast<ClaimsBasedSecurityImpl*>(const_cast<void*>(context));
    if (cbs->m_traceEnabled)
    {
      Log::Stream(Logger::Level::Informational)
          << "OnCbsOperationComplete: "
          << OperationResultStringFromLowLevel(operationCompleteResult)
          << " StatusCode: " << statusCode << " StatusDescription: "
          << (statusDescription ? std::string(statusDescription) : "(NULL)");
    }

    cbs->m_operationResultQueue.CompleteOperation(
        CbsOperationResultStateFromLowLevel(operationCompleteResult),
        statusCode,
        statusDescription ? statusDescription : std::string());
  }
#endif // defined(USE_UAMQP_CBS)

  CbsOpenResult ClaimsBasedSecurityImpl::Open(Context const& context)
  {
#if defined(USE_UAMQP_CBS)
    if (cbs_open_async(
            m_cbs.get(), ClaimsBasedSecurityImpl::OnCbsOpenCompleteFn, this, OnCbsErrorFn, this))
    {
      return CbsOpenResult::Error;
    }
    auto result = m_openResultQueue.WaitForPolledResult(context, *m_session->GetConnection());
    if (result && std::get<0>(*result) == CbsOpenResult::Ok)
    {
      m_cbsOpen = true;
    }
    return std::get<0>(*result);
#else
    if (!m_management)
    {
      ManagementClientOptions managementOptions;
      managementOptions.EnableTrace = m_session->GetConnection()->IsTraceEnabled();
      managementOptions.ExpectedStatusCodeKeyName = "status-code";
      managementOptions.ExpectedStatusDescriptionKeyName = "status-description";
      managementOptions.ManagementNodeName = "$cbs";
      m_management
          = std::make_shared<ManagementClientImpl>(m_session, "$cbs", managementOptions, this);

      auto rv{m_management->Open(context)};
      switch (rv)
      {
        case ManagementOpenStatus::Invalid:
          return CbsOpenResult::Invalid;
        case ManagementOpenStatus::Ok:
          return CbsOpenResult::Ok;
        case ManagementOpenStatus::Error:
          return CbsOpenResult::Error;
        case ManagementOpenStatus::Cancelled:
          return CbsOpenResult::Cancelled;
        default:
          throw std::runtime_error("Unknown return value from Management::Open()");
      }
    }
    else
    {
      return CbsOpenResult::Error;
    }
#endif
  }

  void ClaimsBasedSecurityImpl::Close()
  {
#if defined(USE_UAMQP_CBS)
    if (m_cbsOpen)
    {
      if (cbs_close(m_cbs.get()))
      {
        throw std::runtime_error("Could not close cbs"); // LCOV_EXCL_LINE
      }
    }
#else
    m_management->Close();
#endif
  }
#if defined(USE_UAMQP_CBS)
  void ClaimsBasedSecurityImpl::SetTrace(bool traceOn)
  {
    cbs_set_trace(m_cbs.get(), traceOn);
    m_traceEnabled = traceOn;
  }
#endif

  std::tuple<CbsOperationResult, uint32_t, std::string> ClaimsBasedSecurityImpl::PutToken(
      CbsTokenType tokenType,
      std::string const& audience,
      std::string const& token,
      Context const& context)
  {
#if defined(USE_AMQP_CBS)
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
          = m_operationResultQueue.WaitForPolledResult(context, *m_session->GetConnection());

      // Throw an error if we failed to authenticate with the server.
      if (std::get<0>(*result) != CbsOperationResult::Ok)
      {
        throw std::runtime_error(
            "Could not authenticate to client. Error Status: "
            + std::to_string(std::get<1>(*result)) + " reason: " + std::get<2>(*result));
      }
      return std::move(*result);
    }
#else
    {
      Models::AmqpMessage message;
      message.SetBody(static_cast<Models::AmqpValue>(token));

      message.ApplicationProperties["name"] = static_cast<Models::AmqpValue>(audience);
      auto result = m_management->ExecuteOperation(
          "put-token",
          (tokenType == CbsTokenType::Jwt ? "jwt" : "servicebus.windows.net:sastoken"),
          {},
          message,
          context);
      if (result.Status != ManagementOperationStatus::Ok)
      {
        throw std::runtime_error(
            "Could not authenticate to client. Error Status: " + std::to_string(result.StatusCode)
            + " condition: " + result.Error.Condition.ToString()
            + " reason: " + result.Error.Description);
      }
      else
      {
        CbsOperationResult cbsResult;
        switch (result.Status)
        {
          case ManagementOperationStatus::Invalid:
            cbsResult = CbsOperationResult::Invalid;
            break;
          case ManagementOperationStatus::Ok:
            cbsResult = CbsOperationResult::Ok;
            break;
          case ManagementOperationStatus::Error:
            cbsResult = CbsOperationResult::Error;
            break;
          case ManagementOperationStatus::FailedBadStatus:
            cbsResult = CbsOperationResult::Failed;
            break;
          case ManagementOperationStatus::InstanceClosed:
            cbsResult = CbsOperationResult::InstanceClosed;
            break;
          default:
            throw std::runtime_error("Unknown management operation status.");
        }
        return std::make_tuple(cbsResult, result.StatusCode, result.Error.Description);
      }
    }
#endif
  }

  void ClaimsBasedSecurityImpl::OnError(Models::_internal::AmqpError const& error)
  {
    Log::Stream(Logger::Level::Error) << "AMQP Error processing ClaimsBasedSecurity: " << error;
  }

}}}} // namespace Azure::Core::Amqp::_detail
