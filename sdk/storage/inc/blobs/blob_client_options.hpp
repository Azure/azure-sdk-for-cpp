// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "internal/protocol/blob_rest_client.hpp"

#include <limits>
#include <string>
#include <utility>

namespace Azure { namespace Storage { namespace Blobs {

  struct BlobClientOptions
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
  };

  struct BlockBlobClientOptions : public BlobClientOptions
  {
  };

  struct GetBlobPropertiesOptions
  {
    Azure::Core::Context Context;
  };

  struct SetBlobHttpHeadersOptions
  {
    Azure::Core::Context Context;
    std::string ContentType;
    std::string ContentEncoding;
    std::string ContentLanguage;
    std::string ContentMD5;
    std::string CacheControl;
    std::string ContentDisposition;
  };

  struct SetBlobMetadataOptions
  {
    Azure::Core::Context Context;
  };

  struct DownloadBlobOptions
  {
    Azure::Core::Context Context;
    uint64_t Offset = std::numeric_limits<uint64_t>::max();
    uint64_t Length = 0;
  };

  struct DeleteBlobOptions
  {
    Azure::Core::Context Context;
    DeleteSnapshotsOption DeleteSnapshots = DeleteSnapshotsOption::None;
  };

  struct UploadBlobOptions
  {
    Azure::Core::Context Context;
    std::string ContentMD5;
    std::string ContentCRC64;
    BlobHttpHeaders Properties;
    std::map<std::string, std::string> Metadata;
    AccessTier Tier = AccessTier::Unknown;
  };

  struct StageBlockOptions
  {
    Azure::Core::Context Context;
    std::string ContentMD5;
    std::string ContentCRC64;
  };

  struct CommitBlockListOptions
  {
    Azure::Core::Context Context;
    BlobHttpHeaders Properties;
    std::map<std::string, std::string> Metadata;
    AccessTier Tier = AccessTier::Unknown;
  };

  struct GetBlockListOptions
  {
    Azure::Core::Context Context;
    BlockListTypeOption ListType = BlockListTypeOption::All;
  };

}}} // namespace Azure::Storage::Blobs
