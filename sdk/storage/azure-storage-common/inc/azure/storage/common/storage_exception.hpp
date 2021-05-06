// Copyright (c) Microsoft Corporation. All rights reserved.
// An SPDX-License-Identifier: MIT

#pragma once

#include <map>
#include <memory>
#include <stdexcept>
#include <string>

#include <azure/core/exception.hpp>
#include <azure/core/http/http.hpp>

namespace Azure { namespace Storage {

  struct StorageException : public Azure::Core::RequestFailedException
  {
    explicit StorageException(const std::string& message) : RequestFailedException(message) {}

    std::map<std::string, std::string> AdditionalInformation;

    static StorageException CreateFromResponse(
        std::unique_ptr<Azure::Core::Http::RawResponse> response);
  };
}} // namespace Azure::Storage
