// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/http/http.hpp>

#include <map>
#include <stdexcept>
#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Common {

  struct KeyVaultException : public std::runtime_error
  {
    explicit KeyVaultException(const std::string& message) : std::runtime_error(message) {}

    Azure::Core::Http::HttpStatusCode StatusCode = Azure::Core::Http::HttpStatusCode::None;
    std::string ReasonPhrase;
    std::string ClientRequestId;
    std::string RequestId;
    std::string ErrorCode;
    std::string Message;
    std::unique_ptr<Azure::Core::Http::RawResponse> RawResponse;

    static KeyVaultException CreateFromResponse(
        std::unique_ptr<Azure::Core::Http::RawResponse> response);
  };
}}}} // namespace Azure::Security::KeyVault::Common
