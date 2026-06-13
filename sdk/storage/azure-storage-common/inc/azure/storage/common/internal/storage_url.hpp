// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/nullable.hpp>
#include <azure/core/url.hpp>

namespace Azure { namespace Storage { namespace _internal {
  struct StorageUrlParts
  {
    std::string Service;
    std::string AccountName;
    Azure::Nullable<std::string> ContainerName;
    Azure::Nullable<std::string> ContainerUrl;
  };
  Azure::Nullable<StorageUrlParts> ParseStorageUrl(const Azure::Core::Url& url);
}}} // namespace Azure::Storage::_internal
