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
    Azure::Core::Nullable<std::string> Prefix;
    Azure::Core::Nullable<std::string> Marker;
    Azure::Core::Nullable<int32_t> MaxResults;
    std::vector<ListBlobContainersIncludeOption> Include;
  };

  struct GetUserDelegationKeyOptions
  {
    Azure::Core::Context Context;
  };

  struct BlobContainerClientOptions
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
  };

  struct CreateBlobContainerOptions
  {
    Azure::Core::Context Context;
    Azure::Core::Nullable<PublicAccessType> AccessType;
    std::map<std::string, std::string> Metadata;
  };

  struct DeleteBlobContainerOptions
  {
    Azure::Core::Context Context;
    Azure::Core::Nullable<std::string> IfModifiedSince;
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
  };

  struct GetBlobContainerPropertiesOptions
  {
    Azure::Core::Context Context;
  };

  struct SetBlobContainerMetadataOptions
  {
    Azure::Core::Context Context;
    Azure::Core::Nullable<std::string> IfModifiedSince;
  };

  struct ListBlobsOptions
  {
    Azure::Core::Context Context;
    Azure::Core::Nullable<std::string> Prefix;
    Azure::Core::Nullable<std::string> Delimiter;
    Azure::Core::Nullable<std::string> Marker;
    Azure::Core::Nullable<int32_t> MaxResults;
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
    Azure::Core::Nullable<std::string> IfModifiedSince;
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
    Azure::Core::Nullable<std::string> IfMatch;
    Azure::Core::Nullable<std::string> IfNoneMatch;
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
    Azure::Core::Nullable<std::string> IfModifiedSince;
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
    Azure::Core::Nullable<std::string> IfMatch;
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  struct SetBlobMetadataOptions
  {
    Azure::Core::Context Context;
    Azure::Core::Nullable<std::string> IfModifiedSince;
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
    Azure::Core::Nullable<std::string> IfMatch;
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  struct SetAccessTierOptions
  {
    Azure::Core::Context Context;
    Azure::Core::Nullable<Blobs::RehydratePriority> RehydratePriority;
  };

  struct StartCopyFromUriOptions
  {
    Azure::Core::Context Context;
    std::map<std::string, std::string> Metadata;
    Azure::Core::Nullable<std::string> LeaseId;
    Azure::Core::Nullable<std::string> SourceLeaseId;
    Azure::Core::Nullable<AccessTier> Tier = AccessTier::Unknown;
    Azure::Core::Nullable<Blobs::RehydratePriority> RehydratePriority;
    Azure::Core::Nullable<std::string> IfModifiedSince;
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
    Azure::Core::Nullable<std::string> IfMatch;
    Azure::Core::Nullable<std::string> IfNoneMatch;
    Azure::Core::Nullable<std::string> SourceIfModifiedSince;
    Azure::Core::Nullable<std::string> SourceIfUnmodifiedSince;
    Azure::Core::Nullable<std::string> SourceIfMatch;
    Azure::Core::Nullable<std::string> SourceIfNoneMatch;
  };

  struct AbortCopyFromUriOptions
  {
    Azure::Core::Context Context;
    Azure::Core::Nullable<std::string> LeaseId;
  };

  struct DownloadBlobOptions
  {
    Azure::Core::Context Context;
    Azure::Core::Nullable<int64_t> Offset;
    Azure::Core::Nullable<int64_t> Length;
    Azure::Core::Nullable<std::string> IfModifiedSince;
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
    Azure::Core::Nullable<std::string> IfMatch;
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  struct CreateSnapshotOptions
  {
    Azure::Core::Context Context;
    std::map<std::string, std::string> Metadata;
    Azure::Core::Nullable<std::string> LeaseId;
    Azure::Core::Nullable<std::string> IfModifiedSince;
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
    Azure::Core::Nullable<std::string> IfMatch;
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  struct DeleteBlobOptions
  {
    Azure::Core::Context Context;
    Azure::Core::Nullable<DeleteSnapshotsOption> DeleteSnapshots;
    Azure::Core::Nullable<std::string> IfModifiedSince;
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
    Azure::Core::Nullable<std::string> IfMatch;
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  struct UndeleteBlobOptions
  {
    Azure::Core::Context Context;
  };

  struct UploadBlobOptions
  {
    Azure::Core::Context Context;
    Azure::Core::Nullable<std::string> ContentMD5;
    Azure::Core::Nullable<std::string> ContentCRC64;
    BlobHttpHeaders Properties;
    std::map<std::string, std::string> Metadata;
    Azure::Core::Nullable<AccessTier> Tier;
    Azure::Core::Nullable<std::string> IfModifiedSince;
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
    Azure::Core::Nullable<std::string> IfMatch;
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  struct StageBlockOptions
  {
    Azure::Core::Context Context;
    Azure::Core::Nullable<std::string> ContentMD5;
    Azure::Core::Nullable<std::string> ContentCRC64;
  };

  struct StageBlockFromUriOptions
  {
    Azure::Core::Context Context;
    Azure::Core::Nullable<int64_t> SourceOffset;
    Azure::Core::Nullable<int64_t> SourceLength;
    Azure::Core::Nullable<std::string> ContentMD5;
    Azure::Core::Nullable<std::string> ContentCRC64;
    Azure::Core::Nullable<std::string> LeaseId;
    Azure::Core::Nullable<std::string> SourceIfModifiedSince;
    Azure::Core::Nullable<std::string> SourceIfUnmodifiedSince;
    Azure::Core::Nullable<std::string> SourceIfMatch;
    Azure::Core::Nullable<std::string> SourceIfNoneMatch;
  };

  struct CommitBlockListOptions
  {
    Azure::Core::Context Context;
    BlobHttpHeaders Properties;
    std::map<std::string, std::string> Metadata;
    Azure::Core::Nullable<AccessTier> Tier;
    Azure::Core::Nullable<std::string> IfModifiedSince;
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
    Azure::Core::Nullable<std::string> IfMatch;
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  struct GetBlockListOptions
  {
    Azure::Core::Context Context;
    Azure::Core::Nullable<BlockListTypeOption> ListType;
    Azure::Core::Nullable<std::string> IfModifiedSince;
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
    Azure::Core::Nullable<std::string> IfMatch;
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  struct CreateAppendBlobOptions
  {
    Azure::Core::Context Context;
    BlobHttpHeaders Properties;
    std::map<std::string, std::string> Metadata;
    Azure::Core::Nullable<std::string> IfModifiedSince;
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
    Azure::Core::Nullable<std::string> IfMatch;
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  struct AppendBlockOptions
  {
    Azure::Core::Context Context;
    Azure::Core::Nullable<std::string> ContentMD5;
    Azure::Core::Nullable<std::string> ContentCRC64;
    Azure::Core::Nullable<std::string> LeaseId;
    Azure::Core::Nullable<int64_t> MaxSize;
    Azure::Core::Nullable<int64_t> AppendPosition;
    Azure::Core::Nullable<std::string> IfModifiedSince;
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
    Azure::Core::Nullable<std::string> IfMatch;
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  struct AppendBlockFromUriOptions
  {
    Azure::Core::Context Context;
    Azure::Core::Nullable<int64_t> SourceOffset;
    Azure::Core::Nullable<int64_t> SourceLength;
    Azure::Core::Nullable<std::string> ContentMD5;
    Azure::Core::Nullable<std::string> ContentCRC64;
    Azure::Core::Nullable<std::string> LeaseId;
    Azure::Core::Nullable<int64_t> MaxSize;
    Azure::Core::Nullable<int64_t> AppendPosition;
    Azure::Core::Nullable<std::string> IfModifiedSince;
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
    Azure::Core::Nullable<std::string> IfMatch;
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  struct CreatePageBlobOptions
  {
    Azure::Core::Context Context;
    Azure::Core::Nullable<int64_t> SequenceNumber = 0;
    BlobHttpHeaders Properties;
    std::map<std::string, std::string> Metadata;
    Azure::Core::Nullable<AccessTier> Tier;
    Azure::Core::Nullable<std::string> IfModifiedSince;
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
    Azure::Core::Nullable<std::string> IfMatch;
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  struct UploadPagesOptions
  {
    Azure::Core::Context Context;
    Azure::Core::Nullable<std::string> ContentMD5;
    Azure::Core::Nullable<std::string> ContentCRC64;
    Azure::Core::Nullable<std::string> LeaseId;
    Azure::Core::Nullable<std::string> IfModifiedSince;
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
    Azure::Core::Nullable<std::string> IfMatch;
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  struct UploadPagesFromUriOptions
  {
    Azure::Core::Context Context;
    Azure::Core::Nullable<std::string> ContentMD5;
    Azure::Core::Nullable<std::string> ContentCRC64;
    Azure::Core::Nullable<std::string> LeaseId;
    Azure::Core::Nullable<std::string> IfModifiedSince;
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
    Azure::Core::Nullable<std::string> IfMatch;
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  struct ClearPagesOptions
  {
    Azure::Core::Context Context;
    Azure::Core::Nullable<std::string> LeaseId;
    Azure::Core::Nullable<std::string> IfModifiedSince;
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
    Azure::Core::Nullable<std::string> IfMatch;
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  struct ResizePageBlobOptions
  {
    Azure::Core::Context Context;
    Azure::Core::Nullable<std::string> IfModifiedSince;
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
    Azure::Core::Nullable<std::string> IfMatch;
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  struct GetPageRangesOptions
  {
    Azure::Core::Context Context;
    Azure::Core::Nullable<std::string> PreviousSnapshot;
    Azure::Core::Nullable<std::string> PreviousSnapshotUrl;
    Azure::Core::Nullable<int64_t> Offset;
    Azure::Core::Nullable<int64_t> Length;
    Azure::Core::Nullable<std::string> LeaseId;
    Azure::Core::Nullable<std::string> IfModifiedSince;
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
    Azure::Core::Nullable<std::string> IfMatch;
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  struct IncrementalCopyPageBlobOptions
  {
    Azure::Core::Context Context;
    Azure::Core::Nullable<std::string> IfModifiedSince;
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
    Azure::Core::Nullable<std::string> IfMatch;
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

}}} // namespace Azure::Storage::Blobs
