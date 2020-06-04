// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>
#include <map>

#include "internal/protocol/blob_rest_client.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  struct BlobContainerClientOptions
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
  };

  struct CreateBlobContainerOptions
  {
    Azure::Core::Context Context;
    PublicAccessType AccessType = PublicAccessType::Private;
    std::map<std::string, std::string> Metadata;
  };

  struct DeleteBlobContainerOptions
  {
    Azure::Core::Context Context;
  };

  struct GetBlobContainerPropertiesOptions
  {
    Azure::Core::Context Context;
  };

  struct SetBlobContainerMetadataOptions
  {
    Azure::Core::Context Context;
  };

  struct ListBlobsOptions
  {
    Azure::Core::Context Context;
    std::string Prefix;
    std::string Delimiter;
    std::string Marker;
    int MaxResults = 0;
    std::vector<ListBlobsIncludeItem> Include;
  };

}}} // namespace Azure::Storage::Blobs
