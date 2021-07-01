// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/exception.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/policies/policy.hpp"
#include "azure/core/internal/json/json.hpp"
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

using namespace Azure::Core::Http::_internal;

namespace Azure { namespace Core {

  RequestFailedException::RequestFailedException(
      const std::string& message,
      std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse)
      : std::runtime_error(message), Message(message)
  {
    auto const& headers = rawResponse->GetHeaders();

    StatusCode = rawResponse->GetStatusCode();
    ReasonPhrase = rawResponse->GetReasonPhrase();
    RequestId = HttpShared::GetHeaderOrEmptyString(headers, HttpShared::MsRequestId);
    ClientRequestId = HttpShared::GetHeaderOrEmptyString(headers, HttpShared::MsClientRequestId);
    Message = message;
    RawResponse = std::move(rawResponse);
  }

  RequestFailedException::RequestFailedException(
      std::unique_ptr<Azure::Core::Http::RawResponse>& rawResponse)
      : std::runtime_error(getRawResponseField(rawResponse, "message"))
  {
    auto& headers = rawResponse->GetHeaders();

    StatusCode = rawResponse->GetStatusCode();
    ErrorCode = getRawResponseField(rawResponse, "code");
    StatusCode = rawResponse->GetStatusCode();
    ReasonPhrase = rawResponse->GetReasonPhrase();
    RequestId = HttpShared::GetHeaderOrEmptyString(headers, HttpShared::MsRequestId);
    ClientRequestId = HttpShared::GetHeaderOrEmptyString(headers, HttpShared::MsClientRequestId);
    Message = this->what();
  }

  std::string RequestFailedException::getRawResponseField(
      std::unique_ptr<Azure::Core::Http::RawResponse>& rawResponse,
      std::string fieldName)
  {
    auto& headers = rawResponse->GetHeaders();
    std::string contentType = HttpShared::GetHeaderOrEmptyString(headers, HttpShared::ContentType);
    std::vector<uint8_t> bodyBuffer = rawResponse->GetBody();
    std::string result;

    if (contentType.find("json") != std::string::npos)
    {
      auto jsonParser = Azure::Core::Json::_internal::json::parse(bodyBuffer);
      auto error = jsonParser.find("error");
      if (error != jsonParser.end() && error.value().contains(fieldName))
      {
        result = error.value()[fieldName].get<std::string>();
      }
      else if (fieldName == "message")
      {
        result = std::string(bodyBuffer.begin(), bodyBuffer.end());
      }
    }
    else if (fieldName == "message")
    {
      result = std::string(bodyBuffer.begin(), bodyBuffer.end());
    }

    return result;
  }
}} // namespace Azure::Core
