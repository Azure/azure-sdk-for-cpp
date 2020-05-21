// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <limits>
#include <utility>

#include "protocol/blob_rest_client.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  struct BlobClientOptions
  {
  };

  struct BlockBlobClientOptions : public BlobClientOptions
  {

  };

  struct GetBlobPropertiesOptions
  {
  };

  struct SetBlobHttpHeadersOptions
  {
    std::string ContentType;
    std::string ContentEncoding;
    std::string ContentLanguage;
    std::string ContentMD5;
    std::string CacheControl;
    std::string ContentDisposition;
  };

  struct SetBlobMetadataOptions
  {
  };

  struct DownloadBlobOptions
  {
    uint64_t Offset = std::numeric_limits<uint64_t>::max();
    uint64_t Length = 0;
  };

  struct DeleteBlobOptions
  {
    DeleteSnapshotsOption DeleteSnapshots = DeleteSnapshotsOption::None;
  };

  struct UploadBlobOptions
  {
    std::string ContentMD5;
    std::string ContentCRC64;
    BlobHTTPHeaders Properties;
    std::map<std::string, std::string> Metadata;
    AccessTier Tier = AccessTier::Unknown;
  };

  struct StageBlockOptions
  {
    std::string ContentMD5;
    std::string ContentCRC64;
  };

  struct CommitBlockListOptions
  {
    BlobHTTPHeaders Properties;
    std::map<std::string, std::string> Metadata;
    AccessTier Tier = AccessTier::Unknown;
  };

  struct GetBlockListOptions
  {
    BlockListTypeOption ListType = BlockListTypeOption::All;
  };

}}} // namespace Azure::Storage::Blobs
