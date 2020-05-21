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
  };

  struct CreateBlobContainerOptions
  {
    PublicAccessType AccessType = PublicAccessType::Private;
    std::map<std::string, std::string> Metadata;
  };

  struct DeleteBlobContainerOptions
  {
  };

  struct GetBlobContainerPropertiesOptions
  {
  };

  struct SetBlobContainerMetadataOptions
  {
  };

  struct ListBlobsOptions
  {
    std::string Prefix;
    std::string Delimiter;
    std::string Marker;
    int MaxResults = 0;
    std::vector<ListBlobsIncludeItem> Include;
  };

}}} // namespace Azure::Storage::Blobs
