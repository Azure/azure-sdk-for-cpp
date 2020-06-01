// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>
#include <map>

#include "protocol/blob_rest_client.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  struct BlobContainerClientOptions
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
  };

  struct CreateBlobContainerOptions
  {
    Azure::Core::Context context;
    PublicAccessType AccessType = PublicAccessType::Private;
    std::map<std::string, std::string> Metadata;
  };

  struct DeleteBlobContainerOptions
  {
    Azure::Core::Context context;
  };

  struct GetBlobContainerPropertiesOptions
  {
    Azure::Core::Context context;
  };

  struct SetBlobContainerMetadataOptions
  {
    Azure::Core::Context context;
  };

  struct ListBlobsOptions
  {
    Azure::Core::Context context;
    std::string Prefix;
    std::string Delimiter;
    std::string Marker;
    int MaxResults = 0;
    std::vector<ListBlobsIncludeItem> Include;
  };

}}} // namespace Azure::Storage::Blobs
