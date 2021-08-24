// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <map>
#include <memory>
#include <stdexcept>
#include <string>

#include <azure/core/exception.hpp>
#include <azure/core/http/http.hpp>

namespace Azure { namespace Storage {

  /**
   * @brief An exception thrown when storage service request fails.
   */
  struct StorageException final : public Azure::Core::RequestFailedException
  {
    /**
     * @brief Constructs a #StorageException with a message.
     *
     * @param whatArg The explanatory string.
     */
    explicit StorageException(const std::string& whatArg) : RequestFailedException(whatArg) {}

    /**
     * Some storage-specific information in response body.
     */
    std::map<std::string, std::string> AdditionalInformation;

    /**
     * @brief Constructs a #StorageException from a failed storage service response.
     *
     * @param response Raw HTTP response from storage service.
     * @return #StorageException.
     */
    static StorageException CreateFromResponse(
        std::unique_ptr<Azure::Core::Http::RawResponse> response);
  };
}} // namespace Azure::Storage
