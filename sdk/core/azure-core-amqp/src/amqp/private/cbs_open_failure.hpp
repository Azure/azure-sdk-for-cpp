// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/claims_based_security.hpp"

#include <azure/core/datetime.hpp>

#include <chrono>
#include <exception>
#include <sstream>
#include <string>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  // The function that asked for the token. A first authentication and a
  // proactive refresh fail for different reasons, so the text names the caller.
  enum class CbsOpenCaller
  {
    Authenticate,
    Refresh,
  };

  inline char const* CbsOpenCallerName(CbsOpenCaller caller)
  {
    switch (caller)
    {
      case CbsOpenCaller::Authenticate:
        return "ConnectionImpl::AuthenticateAudience";
      case CbsOpenCaller::Refresh:
        return "ConnectionImpl::RefreshTokenForAudience";
    }
    return "Unknown";
  }

  inline char const* CbsTokenTypeName(CbsTokenType tokenType)
  {
    switch (tokenType)
    {
      case CbsTokenType::Invalid:
        return "Invalid";
      case CbsTokenType::Sas:
        return "Sas";
      case CbsTokenType::Jwt:
        return "Jwt";
    }
    return "Unknown";
  }

  // Azure::DateTime::ToString throws std::invalid_argument for a date before
  // 0001-01-01 or after 9999-12-31, and the expiry comes from customer
  // credential code, so it holds any value. A throw here would replace the
  // exception that the caller must throw for the failed open.
  inline std::string FormatTokenExpiry(Azure::DateTime const& expiresOn)
  {
    try
    {
      return expiresOn.ToString();
    }
    catch (std::exception const&)
    {
      return "unknown";
    }
  }

  // The sentence that the caller reads in the exception. Neither this function
  // nor the one below takes the token, so no failure text can hold a secret.
  //
  // `detail` is the reason the layer below reported. `CbsOpenResult::Error` covers every
  // transport, TLS and link failure, so without the reason a reader cannot tell a refused socket
  // from a rejected handshake. It is empty when that layer produced no reason, and the sentence
  // then reads exactly as it did before.
  inline std::string DescribeCbsOpenFailure(
      CbsOpenResult result,
      std::string const& audienceUrl,
      CbsOpenCaller caller,
      std::string const& detail = {})
  {
    std::stringstream ss;
    ss << "Could not open Claims Based Security object. Result: " << result
       << ", audience: " << audienceUrl << ", caller: " << CbsOpenCallerName(caller);
    if (!detail.empty())
    {
      ss << ", reason: " << detail;
    }
    // The reason comes from a layer below and often ends in its own full stop. A second one
    // reads as a typo in the customer's log.
    if (detail.empty() || detail.back() != '.')
    {
      ss << ".";
    }
    return ss.str();
  }

  // The connection summary and the elapsed time are what separate a client
  // fault from a service fault. A dead connection state, or an open that ends
  // in about no time, says the failure never reached the service. A repeated
  // instance number says the client did not build a new connection.
  inline std::string FormatCbsOpenFailureLog(
      CbsOpenResult result,
      std::string const& audienceUrl,
      CbsTokenType tokenType,
      Azure::DateTime const& expiresOn,
      CbsOpenCaller caller,
      std::string const& connectionSummary,
      std::chrono::milliseconds elapsed,
      std::string const& detail = {})
  {
    std::stringstream ss;
    ss << DescribeCbsOpenFailure(result, audienceUrl, caller, detail)
       << " Token type: " << CbsTokenTypeName(tokenType)
       << ", token expires: " << FormatTokenExpiry(expiresOn)
       << ". Connection: " << (connectionSummary.empty() ? "unknown" : connectionSummary)
       << ". The open failed after " << elapsed.count() << " ms.";
    return ss.str();
  }

}}}} // namespace Azure::Core::Amqp::_detail
