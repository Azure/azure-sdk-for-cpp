// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/common/keyvault_exception.hpp"

#include <azure/core/http/policy.hpp>

#include <nlohmann/json.hpp>
#include <type_traits>

using namespace Azure::Security::KeyVault::Common;

KeyVaultException KeyVaultException::CreateFromResponse(
    std::unique_ptr<Azure::Core::Http::RawResponse> response)
{
  std::vector<uint8_t> bodyBuffer = std::move(response->GetBody());

  auto httpStatusCode = response->GetStatusCode();
  std::string reasonPhrase = response->GetReasonPhrase();
  std::string requestId;
  if (response->GetHeaders().find("x-ms-request-id") != response->GetHeaders().end())
  {
    requestId = response->GetHeaders().at("x-ms-request-id");
  }

  std::string clientRequestId;
  if (response->GetHeaders().find("x-ms-client-request-id") != response->GetHeaders().end())
  {
    clientRequestId = response->GetHeaders().at("x-ms-client-request-id");
  }

  std::string errorCode;
  std::string message;

  if (response->GetHeaders().find("content-type") != response->GetHeaders().end())
  {
    if (response->GetHeaders().at("content-type").find("json") != std::string::npos)
    {
      auto jsonParser = nlohmann::json::parse(bodyBuffer);
      errorCode = jsonParser["error"]["code"].get<std::string>();
      message = jsonParser["error"]["message"].get<std::string>();
    }
    else
    {
      message = std::string(bodyBuffer.begin(), bodyBuffer.end());
    }
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
  result.RawResponse = std::move(response);
  return result;
}
