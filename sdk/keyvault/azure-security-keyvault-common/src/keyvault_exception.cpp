// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/common/keyvault_exception.hpp"
#include "azure/keyvault/common/keyvault_constants.hpp"

#include <azure/core/http/policies/policy.hpp>

#include <azure/core/internal/json/json.hpp>
#include <type_traits>

using namespace Azure::Security::KeyVault::Common;
using namespace Azure::Core::Http::_internal;

KeyVaultException::KeyVaultException(
    const std::string& message,
    std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse)
    : RequestFailedException(message, std::move(rawResponse))
{
  std::vector<uint8_t> bodyBuffer = std::move(RawResponse->GetBody());
  auto& headers = RawResponse->GetHeaders();
  std::string contentType = HttpShared::GetHeaderOrEmptyString(headers, HttpShared::ContentType);

  if (contentType.find("json") != std::string::npos)
  {
    auto jsonParser = Azure::Core::Json::_internal::json::parse(bodyBuffer);
    auto& error = jsonParser["error"];
    ErrorCode = error["code"].get<std::string>();
    Message = error["message"].get<std::string>();
  }
  else
  {
    Message = std::string(bodyBuffer.begin(), bodyBuffer.end());
  }
}
