// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <stdexcept>
#include <string>

#include <azure/core/http/http.hpp>

namespace Azure { namespace Storage {

  struct StorageException : public std::runtime_error
  {
    explicit StorageException(const std::string& message) : std::runtime_error(message) {}

    Azure::Core::Http::HttpStatusCode StatusCode = Azure::Core::Http::HttpStatusCode::None;
    std::string ReasonPhrase;
    std::string ClientRequestId;
    std::string RequestId;
    std::string ErrorCode;
    std::string Message;
    std::unique_ptr<Azure::Core::Http::RawResponse> RawResponse;

    static StorageException CreateFromResponse(
        std::unique_ptr<Azure::Core::Http::RawResponse> response);
  };
}} // namespace Azure::Storage
