// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <map>
#include <stdexcept>
#include <string>

namespace Azure { namespace Core { namespace Http {
  class Response;
}}} // namespace Azure::Core::Http

namespace Azure { namespace Storage {

  struct StorageError : public std::runtime_error
  {
    explicit StorageError(const std::string& message)
        : std::runtime_error(message), StatusCode(), RequestId()
    {
    }
    explicit StorageError(const std::string& message, std::string errorCode)
        : std::runtime_error(message), StatusCode(std::move(errorCode)), RequestId()
    {
    }
    explicit StorageError(const std::string& message, std::string errorCode, std::string requestId)
        : std::runtime_error(message), StatusCode(std::move(errorCode)),
          RequestId(std::move(requestId))
    {
    }

    std::string StatusCode;
    std::string RequestId;
    std::map<std::string, std::string> Details;

    static StorageError CreateFromResponse(/* const */ Azure::Core::Http::Response& response);
  };
}} // namespace Azure::Storage
