// Copyright (c) Microsoft Corporation. All rights reserved.
// An SPDX-License-Identifier: MIT

#include "azure/keyvault/common/keyvault_exception.hpp"
#include "azure/keyvault/common/keyvault_constants.hpp"

#include <azure/core/http/policies/policy.hpp>

#include <azure/core/internal/json/json.hpp>
#include <type_traits>

using namespace Azure::Security::KeyVault;
using namespace Azure::Core::Http::_internal;

Azure::Core::RequestFailedException _detail::KeyVaultException::CreateException(
    std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse)
{
  std::vector<uint8_t> bodyBuffer = std::move(rawResponse->GetBody());
  auto& headers = rawResponse->GetHeaders();
  std::string contentType = HttpShared::GetHeaderOrEmptyString(headers, HttpShared::ContentType);
  std::string message;
  std::string errorCode;

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
  Azure::Core::RequestFailedException exception(message, std::move(rawResponse));
  exception.ErrorCode = std::move(errorCode);
  return exception;
}
