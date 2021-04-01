// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/common/keyvault_exception.hpp"
#include "azure/keyvault/common/keyvault_constants.hpp"

#include <azure/core/http/policies/policy.hpp>

#include <azure/core/internal/json/json.hpp>
#include <type_traits>

using namespace Azure::Security::KeyVault::Common;

namespace {

inline std::string GetHeaderOrEmptyString(
    Azure::Core::CaseInsensitiveMap const& headers,
    std::string const& headerName)
{
  auto header = headers.find(headerName);
  if (header != headers.end())
  {
    return header->second; // second is the header value.
  }
  return {}; // empty string
}

} // namespace

KeyVaultException::KeyVaultException(
    const std::string& message,
    std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse)
    : RequestFailedException(message)
{
  std::vector<uint8_t> bodyBuffer = std::move(rawResponse->GetBody());

  auto httpStatusCode = rawResponse->GetStatusCode();
  std::string reasonPhrase = rawResponse->GetReasonPhrase();
  auto& headers = rawResponse->GetHeaders();
  std::string requestId = GetHeaderOrEmptyString(headers, _detail::MsRequestId);
  std::string clientRequestId = GetHeaderOrEmptyString(headers, _detail::MsClientRequestId);
  std::string contentType = GetHeaderOrEmptyString(headers, _detail::ContentType);
  std::string errorCode;
  std::string generatedMessage = message;

  if (contentType.find("json") != std::string::npos)
  {
    auto jsonParser = Azure::Core::Json::_internal::json::parse(bodyBuffer);
    auto& error = jsonParser["error"];
    errorCode = error["code"].get<std::string>();
    generatedMessage = error["message"].get<std::string>();
  }
  else
  {
    generatedMessage = std::string(bodyBuffer.begin(), bodyBuffer.end());
  }

  KeyVaultException result = KeyVaultException(
      std::to_string(static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
          httpStatusCode))
      + " " + reasonPhrase + "\n" + message + "\nRequest ID: " + requestId);
  StatusCode = httpStatusCode;
  ReasonPhrase = std::move(reasonPhrase);
  RequestId = std::move(requestId);
  ErrorCode = std::move(errorCode);
  Message = std::move(generatedMessage);
  RawResponse = std::move(rawResponse);
}
