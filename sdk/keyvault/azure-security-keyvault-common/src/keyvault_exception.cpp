// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/common/keyvault_exception.hpp"
#include "azure/keyvault/common/keyvault_constants.hpp"

#include <azure/core/http/policies/policy.hpp>

#include <azure/core/internal/json/json.hpp>
#include <type_traits>

using namespace Azure::Security::KeyVault;

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

KeyVaultException KeyVaultException::CreateFromResponse(
    std::unique_ptr<Azure::Core::Http::RawResponse> response)
{
  return CreateFromResponse(*response);
}

KeyVaultException KeyVaultException::CreateFromResponse(
    Azure::Core::Http::RawResponse const& response)
{
  std::vector<uint8_t> bodyBuffer = std::move(response.GetBody());

  auto httpStatusCode = response.GetStatusCode();
  std::string reasonPhrase = response.GetReasonPhrase();
  auto& headers = response.GetHeaders();
  std::string requestId = GetHeaderOrEmptyString(headers, _detail::MsRequestId);
  std::string clientRequestId = GetHeaderOrEmptyString(headers, _detail::MsClientRequestId);
  std::string contentType = GetHeaderOrEmptyString(headers, _detail::ContentType);
  std::string errorCode;
  std::string message;

  if (contentType.find("json") != std::string::npos)
  {
    auto jsonParser = Azure::Core::Json::_internal::json::parse(bodyBuffer);
    auto& error = jsonParser["error"];
    errorCode = error["code"].get<std::string>();
    message = error["message"].get<std::string>();
  }
  else
  {
    message = std::string(bodyBuffer.begin(), bodyBuffer.end());
  }

  KeyVaultException result = KeyVaultException(
      std::to_string(static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
          httpStatusCode))
      + " " + reasonPhrase + "\n" + message + "\nRequest ID: " + requestId);
  result.StatusCode = httpStatusCode;
  result.ReasonPhrase = std::move(reasonPhrase);
  result.RequestId = std::move(requestId);
  result.ErrorCode = std::move(errorCode);
  result.Message = std::move(message);
  result.RawResponse = std::make_unique<Azure::Core::Http::RawResponse>(response);
  return result;
}
