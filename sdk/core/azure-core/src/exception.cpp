// Copyright (c) Microsoft Corporation. All rights reserved.
// An SPDX-License-Identifier: MIT

#include "azure/core/exception.hpp"
#include "azure/core/http/http.hpp"

#include <memory>
#include <stdexcept>
#include <string>
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

}} // namespace Azure::Core
