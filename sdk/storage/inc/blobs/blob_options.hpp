// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "internal/protocol/blob_rest_client.hpp"

#include <limits>
#include <string>
#include <utility>

namespace Azure { namespace Storage { namespace Blobs {

  struct BlobServiceClientOptions
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
  };

  struct ListBlobContainersOptions
  {
    Azure::Core::Context Context;
    std::string Prefix;
    std::string Marker;
    int MaxResults = 0;
    std::vector<ListBlobContainersIncludeOption> Include;
  };

  struct GetUserDelegationKeyOptions
  {
    Azure::Core::Context Context;
    std::string StartsOn;
  };

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
    std::string IfModifiedSince;
    std::string IfUnmodifiedSince;
  };

  struct GetBlobContainerPropertiesOptions
  {
    Azure::Core::Context Context;
  };

  struct SetBlobContainerMetadataOptions
  {
    Azure::Core::Context Context;
    std::string IfModifiedSince;
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

  struct BlobClientOptions
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
  };

  struct BlockBlobClientOptions : public BlobClientOptions
  {
  };

  struct AppendBlobClientOptions : public BlobClientOptions
  {
  };

  struct PageBlobClientOptions : public BlobClientOptions
  {
  };

  struct GetBlobPropertiesOptions
  {
    Azure::Core::Context Context;
    std::string IfModifiedSince;
    std::string IfUnmodifiedSince;
    std::string IfMatch;
    std::string IfNoneMatch;
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
    std::string IfModifiedSince;
    std::string IfUnmodifiedSince;
    std::string IfMatch;
    std::string IfNoneMatch;
  };

  struct SetBlobMetadataOptions
  {
    Azure::Core::Context Context;
    std::string IfModifiedSince;
    std::string IfUnmodifiedSince;
    std::string IfMatch;
    std::string IfNoneMatch;
  };

  struct SetAccessTierOptions
  {
    Azure::Core::Context Context;
    Blobs::RehydratePriority RehydratePriority = Blobs::RehydratePriority::Unknown;
  };

  struct StartCopyFromUriOptions
  {
    Azure::Core::Context Context;
    std::map<std::string, std::string> Metadata;
    std::string LeaseId;
    std::string SourceLeaseId;
    AccessTier Tier = AccessTier::Unknown;
    Blobs::RehydratePriority RehydratePriority = Blobs::RehydratePriority::Unknown;
    std::string IfModifiedSince;
    std::string IfUnmodifiedSince;
    std::string IfMatch;
    std::string IfNoneMatch;
    std::string SourceIfModifiedSince;
    std::string SourceIfUnmodifiedSince;
    std::string SourceIfMatch;
    std::string SourceIfNoneMatch;
  };

  struct AbortCopyFromUriOptions
  {
    Azure::Core::Context Context;
    std::string LeaseId;
  };

  struct DownloadBlobOptions
  {
    Azure::Core::Context Context;
    uint64_t Offset = std::numeric_limits<uint64_t>::max(); // max means the whole blob
    uint64_t Length = 0; // 0 means till the end
    std::string IfModifiedSince;
    std::string IfUnmodifiedSince;
    std::string IfMatch;
    std::string IfNoneMatch;
  };

  struct CreateSnapshotOptions
  {
    Azure::Core::Context Context;
    std::map<std::string, std::string> Metadata;
    std::string LeaseId;
    std::string IfModifiedSince;
    std::string IfUnmodifiedSince;
    std::string IfMatch;
    std::string IfNoneMatch;
  };

  struct DeleteBlobOptions
  {
    Azure::Core::Context Context;
    DeleteSnapshotsOption DeleteSnapshots = DeleteSnapshotsOption::None;
    std::string IfModifiedSince;
    std::string IfUnmodifiedSince;
    std::string IfMatch;
    std::string IfNoneMatch;
  };

  struct UndeleteBlobOptions
  {
    Azure::Core::Context Context;
  };

  struct UploadBlobOptions
  {
    Azure::Core::Context Context;
    std::string ContentMD5;
    std::string ContentCRC64;
    BlobHttpHeaders Properties;
    std::map<std::string, std::string> Metadata;
    AccessTier Tier = AccessTier::Unknown;
    std::string IfModifiedSince;
    std::string IfUnmodifiedSince;
    std::string IfMatch;
    std::string IfNoneMatch;
  };

  struct StageBlockOptions
  {
    Azure::Core::Context Context;
    std::string ContentMD5;
    std::string ContentCRC64;
  };

  struct StageBlockFromUriOptions
  {
    Azure::Core::Context Context;
    uint64_t SourceOffset = std::numeric_limits<uint64_t>::max();
    uint64_t SourceLength = 0;
    std::string ContentMD5;
    std::string ContentCRC64;
    std::string LeaseId;
    std::string SourceIfModifiedSince;
    std::string SourceIfUnmodifiedSince;
    std::string SourceIfMatch;
    std::string SourceIfNoneMatch;
  };

  struct CommitBlockListOptions
  {
    Azure::Core::Context Context;
    BlobHttpHeaders Properties;
    std::map<std::string, std::string> Metadata;
    AccessTier Tier = AccessTier::Unknown;
    std::string IfModifiedSince;
    std::string IfUnmodifiedSince;
    std::string IfMatch;
    std::string IfNoneMatch;
  };

  struct GetBlockListOptions
  {
    Azure::Core::Context Context;
    BlockListTypeOption ListType = BlockListTypeOption::All;
    std::string IfModifiedSince;
    std::string IfUnmodifiedSince;
    std::string IfMatch;
    std::string IfNoneMatch;
  };

  struct CreateAppendBlobOptions
  {
    Azure::Core::Context Context;
    BlobHttpHeaders Properties;
    std::map<std::string, std::string> Metadata;
    std::string IfModifiedSince;
    std::string IfUnmodifiedSince;
    std::string IfMatch;
    std::string IfNoneMatch;
  };

  struct AppendBlockOptions
  {
    Azure::Core::Context Context;
    std::string ContentMD5;
    std::string ContentCRC64;
    std::string LeaseId;
    uint64_t MaxSize = std::numeric_limits<uint64_t>::max();
    uint64_t AppendPosition = std::numeric_limits<uint64_t>::max();
    std::string IfModifiedSince;
    std::string IfUnmodifiedSince;
    std::string IfMatch;
    std::string IfNoneMatch;
  };

  struct CreatePageBlobOptions
  {
    Azure::Core::Context Context;
    uint64_t SequenceNumber = 0;
    BlobHttpHeaders Properties;
    std::map<std::string, std::string> Metadata;
    AccessTier Tier = AccessTier::Unknown;
    std::string IfModifiedSince;
    std::string IfUnmodifiedSince;
    std::string IfMatch;
    std::string IfNoneMatch;
  };

  struct UploadPagesOptions
  {
    Azure::Core::Context Context;
    std::string ContentMD5;
    std::string ContentCRC64;
    std::string LeaseId;
    std::string IfModifiedSince;
    std::string IfUnmodifiedSince;
    std::string IfMatch;
    std::string IfNoneMatch;
  };

  struct UploadPagesFromUriOptions
  {
    Azure::Core::Context Context;
    std::string ContentMD5;
    std::string ContentCRC64;
    std::string LeaseId;
    std::string IfModifiedSince;
    std::string IfUnmodifiedSince;
    std::string IfMatch;
    std::string IfNoneMatch;
  };

  struct ClearPagesOptions
  {
    Azure::Core::Context Context;
    std::string LeaseId;
    std::string IfModifiedSince;
    std::string IfUnmodifiedSince;
    std::string IfMatch;
    std::string IfNoneMatch;
  };

  struct ResizePageBlobOptions
  {
    Azure::Core::Context Context;
    std::string IfModifiedSince;
    std::string IfUnmodifiedSince;
    std::string IfMatch;
    std::string IfNoneMatch;
  };

  struct GetPageRangesOptions
  {
    Azure::Core::Context Context;
    std::string PreviousSnapshot;
    std::string PreviousSnapshotUrl;
    uint64_t Offset = 0;
    uint64_t Length = 0;
    std::string LeaseId;
    std::string IfModifiedSince;
    std::string IfUnmodifiedSince;
    std::string IfMatch;
    std::string IfNoneMatch;
  };

  struct IncrementalCopyPageBlobOptions
  {
    Azure::Core::Context Context;
    std::string IfModifiedSince;
    std::string IfUnmodifiedSince;
    std::string IfMatch;
    std::string IfNoneMatch;
  };

}}} // namespace Azure::Storage::Blobs
