// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "http/http.hpp"

#include <map>
#include <stdexcept>
#include <string>

namespace Azure { namespace Storage {

  struct StorageError : public std::runtime_error
  {
    explicit StorageError(const std::string& message) : std::runtime_error(message) {}

    Azure::Core::Http::HttpStatusCode StatusCode;
    std::string ReasonPhrase;
    std::string ClientRequestId;
    std::string RequestId;
    std::string ErrorCode;
    std::string Message;
    std::unique_ptr<Azure::Core::Http::RawResponse> RawResponse;

    static StorageError CreateFromResponse(
        /* const */ std::unique_ptr<Azure::Core::Http::RawResponse> response);
  };
}} // namespace Azure::Storage
