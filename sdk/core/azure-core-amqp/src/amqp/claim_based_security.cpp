// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

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

#endif // TESTING_BUILD

  ClaimsBasedSecurityImpl::ClaimsBasedSecurityImpl(std::shared_ptr<_detail::SessionImpl> session)
      : m_session{session}
  {
  }

  ClaimsBasedSecurityImpl::~ClaimsBasedSecurityImpl() noexcept {}

  CbsOpenResult ClaimsBasedSecurityImpl::Open(Context const& context)
  {
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
        case ManagementOpenStatus::Invalid: // LCOV_EXCL_LINE
          return CbsOpenResult::Invalid; // LCOV_EXCL_LINE
        case ManagementOpenStatus::Ok:
          return CbsOpenResult::Ok;
        case ManagementOpenStatus::Error:
          return CbsOpenResult::Error;
        case ManagementOpenStatus::Cancelled: // LCOV_EXCL_LINE
          return CbsOpenResult::Cancelled; // LCOV_EXCL_LINE
        default:
          throw std::runtime_error("Unknown return value from Management::Open()");
      }
    }
    else
    {
      return CbsOpenResult::Error; // LCOV_EXCL_LINE
    }
  }

  void ClaimsBasedSecurityImpl::Close() { m_management->Close(); }

  std::tuple<CbsOperationResult, uint32_t, std::string> ClaimsBasedSecurityImpl::PutToken(
      CbsTokenType tokenType,
      std::string const& audience,
      std::string const& token,
      Context const& context)
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
        case ManagementOperationStatus::Invalid: // LCOV_EXCL_LINE
          cbsResult = CbsOperationResult::Invalid; // LCOV_EXCL_LINE
          break;
        case ManagementOperationStatus::Ok:
          cbsResult = CbsOperationResult::Ok;
          break;
        case ManagementOperationStatus::Error: // LCOV_EXCL_LINE
          cbsResult = CbsOperationResult::Error; // LCOV_EXCL_LINE
          break;
        case ManagementOperationStatus::FailedBadStatus: // LCOV_EXCL_LINE
          cbsResult = CbsOperationResult::Failed; // LCOV_EXCL_LINE
          break;
        case ManagementOperationStatus::InstanceClosed: // LCOV_EXCL_LINE
          cbsResult = CbsOperationResult::InstanceClosed; // LCOV_EXCL_LINE
          break;
        default:
          throw std::runtime_error("Unknown management operation status."); // LCOV_EXCL_LINE
      }
      Log::Stream(Logger::Level::Error)
          << "CBS PutToken result: " << cbsResult << " status code: " << result.StatusCode
          << " Error: " << result.Error.Description << ".";
      return std::make_tuple(cbsResult, result.StatusCode, result.Error.Description);
    }
  }
  std::ostream& operator<<(std::ostream& os, CbsOperationResult const& operationResult)
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
      default:
        os << "Unknown CbsOperationResult."
           << static_cast<std::underlying_type<CbsOperationResult>::type>(operationResult);
    }
    return os;
  }

  std::ostream& operator<<(std::ostream& os, CbsOpenResult const& openResult)
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
      default:
        os << "Unknown CbsOpenResult."
           << static_cast<std::underlying_type<CbsOpenResult>::type>(openResult);
    }
    return os;
  }

  void ClaimsBasedSecurityImpl::OnError(Models::_internal::AmqpError const& error)
  {
    Log::Stream(Logger::Level::Error) << "AMQP Error processing ClaimsBasedSecurity: " << error;
  }

}}}} // namespace Azure::Core::Amqp::_detail
