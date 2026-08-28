// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "private/claims_based_security_impl.hpp"
#include "private/connection_impl.hpp"
#include "private/management_impl.hpp"
#include "private/session_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include <type_traits>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;
using namespace Azure::Core::Amqp::_internal;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
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
      if (rv != ManagementOpenStatus::Ok)
      {
        auto const detail = m_management->GetOpenFailureDetail();
        Log::Stream(Logger::Level::Warning)
            << "ClaimsBasedSecurityImpl::Open: the $cbs management client did not open. Status: "
            << ManagementOpenStatusName(rv) << "."
            << (detail.empty() ? std::string{} : " Reason: " + detail + ".");
      }
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
      Log::Stream(Logger::Level::Warning)
          << "ClaimsBasedSecurityImpl::Open: Open was called a second time on the same object. "
             "This object cannot be opened more than once.";
      return CbsOpenResult::Error;
    }
  }

  void ClaimsBasedSecurityImpl::Close(Context const& context) { m_management->Close(context); }

  // The management client holds the reason that its own layer produced. The AMQP error that the
  // service sent is richer, because it names the condition, the description, and the info map, so
  // it is added when one arrived. Either part may be absent.
  std::string ClaimsBasedSecurityImpl::GetOpenFailureDetail() const
  {
    std::string detail{m_management ? m_management->GetOpenFailureDetail() : std::string{}};

    Models::_internal::AmqpError lastError;
    {
      std::lock_guard<std::mutex> lock(m_errorLock);
      lastError = m_lastError;
    }
    if (lastError)
    {
      std::stringstream ss;
      if (!detail.empty())
      {
        ss << detail << "; ";
      }
      ss << "the service reported condition: " << lastError.Condition.ToString()
         << ", description: " << lastError.Description;
      // The info map carries the fields that make a condition actionable, such as the
      // network-host and port of a redirect. It is usually empty, so it is only added when the
      // service sent one.
      if (!lastError.Info.empty())
      {
        ss << ", info: " << lastError.Info;
      }
      return ss.str();
    }
    return detail;
  }

  std::tuple<CbsOperationResult, uint32_t, std::string> ClaimsBasedSecurityImpl::PutToken(
      CbsTokenType tokenType,
      std::string const& audience,
      std::string const& token,
      Azure::DateTime const& tokenExpirationTime,
      Context const& context)
  {
    Models::AmqpMessage message;
    message.SetBody(static_cast<Models::AmqpValue>(token));

    message.ApplicationProperties["name"] = static_cast<Models::AmqpValue>(audience);

    // It appears that on Android, if you get a "long long" (from std::chrono::seconds::count()) and
    // try to pass it to the set of overloads of AmqpValue() which do exist for all the range of
    // fixed bit integers, including int64_t and uint64_t, it is still ambiguous for the compiler
    // which overload to choose, as if "std::u?int*_t" were not typedefs, but some custom types.
    static_assert(
        std::is_signed<std::chrono::seconds::rep>::value
            && std::is_integral<std::chrono::seconds::rep>::value
            && sizeof(std::chrono::seconds::rep) <= sizeof(std::int64_t),
        "std::chrono::seconds::rep is expected to fit into std::int64_t");
    message.ApplicationProperties["expiration"] = static_cast<std::int64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(tokenExpirationTime.time_since_epoch())
            .count());

    auto result = m_management->ExecuteOperation(
        "put-token",
        (tokenType == CbsTokenType::Jwt ? "jwt" : "servicebus.windows.net:sastoken"),
        {},
        message,
        context);
    if (result.Status != ManagementOperationStatus::Ok)
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
        case ManagementOperationStatus::Cancelled:
          cbsResult = CbsOperationResult::Cancelled;
          break;
        default:
          throw std::runtime_error("Unknown management operation status.");
      }
      Log::Stream(Logger::Level::Informational)
          << "CBS PutToken result: " << cbsResult << " status code: " << result.StatusCode
          << " Error: " << result.Error << ".";
      return std::make_tuple(cbsResult, result.StatusCode, result.Error.Description);
    }
    else
    {
      return std::make_tuple(CbsOperationResult::Ok, result.StatusCode, result.Error.Description);
    }
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

  void ClaimsBasedSecurityImpl::OnError(Models::_internal::AmqpError const& error)
  {
    Log::Stream(Logger::Level::Warning) << "AMQP Error processing ClaimsBasedSecurity: " << error;
    // This is the only place the service's own condition and description reach this object. A
    // caller that reads the exception and nothing else would otherwise never see them.
    std::lock_guard<std::mutex> lock(m_errorLock);
    m_lastError = error;
  }

}}}} // namespace Azure::Core::Amqp::_detail
