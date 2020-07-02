// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "internal/protocol/blob_rest_client.hpp"

#include <limits>
#include <string>
#include <utility>

namespace Azure { namespace Storage { namespace Blobs {

  /**
   * @brief Service client options used to initalize BlobServiceClient.
   */
  struct BlobServiceClientOptions
  {
    /**
     * @brief Transport pipeline policies for authentication, retries, etc., that are
     * applied to every request.
     */
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
  };

  /**
   * @brief Optional parameters for BlobServiceClient::ListBlobContainers.
   */
  struct ListBlobContainersOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specifies a string that filters the results to return only containers whose
     * name begins with the specified prefix.
     */
    Azure::Core::Nullable<std::string> Prefix;

    /**
     * @brief A string value that identifies the portion of the list of containers to be
     * returned with the next listing operation. The operation returns a non-empty
     * ListContainersSegment.NextMarker value if the listing operation did not return all containers
     * remaining to be listed with the current segment. The NextMarker value can be used as the
     * value for the Marker parameter in a subsequent call to request the next segment of list
     * items.
     */
    Azure::Core::Nullable<std::string> Marker;

    /**
     * @brief Specifies the maximum number of containers to return.
     */
    Azure::Core::Nullable<int32_t> MaxResults;

    /**
     * @brief Specifies that the container's metadata be returned.
     */
    std::vector<ListBlobContainersIncludeOption> Include;
  };

  /**
   * @brief Optional parameters for BlobServiceClient::GetUserDelegationKey.
   */
  struct GetUserDelegationKeyOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  /**
   * @brief Container client options used to initalize BlobContainerClient.
   */
  struct BlobContainerClientOptions
  {
    /**
     * @brief Transport pipeline policies for authentication, retries, etc., that are
     * applied to every request.
     */
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
  };

  /**
   * @brief Optional parameters for BlobContainerClient::Create.
   */
  struct CreateBlobContainerOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specifies whether data in the container may be accessed publicly and the level
     * of access.
     */
    Azure::Core::Nullable<PublicAccessType> AccessType;

    /**
     * @brief Name-value pairs to associate with the container as metadata.
     */
    std::map<std::string, std::string> Metadata;
  };

  /**
   * @brief Optional parameters for BlobContainerClient::Delete.
   */
  struct DeleteBlobContainerOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
  };

  /**
   * @brief Optional parameters for BlobContainerClient::GetProperties.
   */
  struct GetBlobContainerPropertiesOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  /**
   * @brief Optional parameters for BlobContainerClient::SetMetadata.
   */
  struct SetBlobContainerMetadataOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;
  };

  /**
   * @brief Optional parameters for BlobContainerClient::ListBlobs.
   */
  struct ListBlobsOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specifies a string that filters the results to return only blobs whose
     * name begins with the specified prefix.
     */
    Azure::Core::Nullable<std::string> Prefix;

    /**
     * @brief Used to traverse a virtual hierarchy of blobs as though it were a file
     * system.
     */
    Azure::Core::Nullable<std::string> Delimiter;

    /**
     * @brief A string value that identifies the portion of the list of blobs to be
     * returned with the next listing operation. The operation returns a non-empty
     * BlobsFlatSegment.NextMarker value if the listing operation did not return all blobs
     * remaining to be listed with the current segment. The NextMarker value can be used as the
     * value for the Marker parameter in a subsequent call to request the next segment of list
     * items.
     */
    Azure::Core::Nullable<std::string> Marker;

    /**
     * @brief Specifies the maximum number of blobs to return.
     */
    Azure::Core::Nullable<int32_t> MaxResults;

    /**
     * @brief Specifies one or more datasets to include in the response.
     */
    std::vector<ListBlobsIncludeItem> Include;
  };

  /**
   * @brief Blob client options used to initalize BlobClient.
   */
  struct BlobClientOptions
  {
    /**
     * @brief Transport pipeline policies for authentication, retries, etc., that are
     * applied to every request.
     */
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
  };

  /**
   * @brief Block blob client options used to initalize BlockBlobClient.
   */
  struct BlockBlobClientOptions : public BlobClientOptions
  {
  };

  /**
   * @brief Append blob client options used to initalize AppendBlobClient.
   */
  struct AppendBlobClientOptions : public BlobClientOptions
  {
  };

  /**
   * @brief Page blob client options used to initalize PageBlobClient.
   */
  struct PageBlobClientOptions : public BlobClientOptions
  {
  };

  /**
   * @brief Optional parameters for BlobClient::GetProperties.
   */
  struct GetBlobPropertiesOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> IfMatch;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag does
     * not match the value specified. Specify the wildcard character (*) to perform the operation
     * only if the resource does not exist, and fail the operation if it does exist.
     */
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  /**
   * @brief Optional parameters for BlobClient::SetHttpHeaders.
   */
  struct SetBlobHttpHeadersOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief The MIME content type of the blob.
     */
    std::string ContentType;

    /**
     * @brief Specifies which content encodings have been applied to the blob.
     */
    std::string ContentEncoding;

    /**
     * @brief Specifies the natural languages used by this resource.
     */
    std::string ContentLanguage;

    /**
     * @brief Sets the blob’s MD5 hash.
     */
    std::string ContentMD5;

    /**
     * @brief Sets the blob's cache control.
     */
    std::string CacheControl;

    /**
     * @brief Sets the blob’s Content-Disposition header.
     */
    std::string ContentDisposition;

    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> IfMatch;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag does
     * not match the value specified. Specify the wildcard character (*) to perform the operation
     * only if the resource does not exist, and fail the operation if it does exist.
     */
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  /**
   * @brief Optional parameters for BlobClient::SetMetadata.
   */
  struct SetBlobMetadataOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> IfMatch;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag does
     * not match the value specified. Specify the wildcard character (*) to perform the operation
     * only if the resource does not exist, and fail the operation if it does exist.
     */
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  /**
   * @brief Optional parameters for BlobClient::SetAccessTier.
   */
  struct SetAccessTierOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @beirf Indicates the priority with which to rehydrate an archived blob. The priority
     * can be set on a blob only once. This header will be ignored on subsequent requests to the
     * same blob.
     */
    Azure::Core::Nullable<Blobs::RehydratePriority> RehydratePriority;
  };

  /**
   * @brief Optional parameters for BlobClient::StartCopyFromUri.
   */
  struct StartCopyFromUriOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specifies user-defined name-value pairs associated with the blob. If no
     * name-value pairs are specified, the operation will copy the metadata from the source blob or
     * file to the destination blob. If one or more name-value pairs are specified, the destination
     * blob is created with the specified metadata, and metadata is not copied from the source blob
     * or file.
     */
    std::map<std::string, std::string> Metadata;

    /**
     * @brief Specify this header to perform the operation only if the resource has an
     * active lease mathing this id.
     */
    Azure::Core::Nullable<std::string> LeaseId;

    /**
     * @brief Specify this header to perform the operation only if the lease id given
     * matches the active lease id of the source blob.
     */
    Azure::Core::Nullable<std::string> SourceLeaseId;

    /**
     * @brief Specifies the tier to be set on the target blob.
     */
    Azure::Core::Nullable<AccessTier> Tier = AccessTier::Unknown;

    /**
     * @beirf Indicates the priority with which to rehydrate an archived blob. The priority
     * can be set on a blob only once. This header will be ignored on subsequent requests to the
     * same blob.
     */
    Azure::Core::Nullable<Blobs::RehydratePriority> RehydratePriority;

    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> IfMatch;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag does
     * not match the value specified. Specify the wildcard character (*) to perform the operation
     * only if the resource does not exist, and fail the operation if it does exist.
     */
    Azure::Core::Nullable<std::string> IfNoneMatch;

    /**
     * @brief Specify this conditional header to copy the blob only if the source blob has
     * been modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> SourceIfModifiedSince;

    /**
     * @brief Specify this conditional header to copy the blob only if the source blob has
     * not been modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> SourceIfUnmodifiedSince;

    /**
     * @brief Specify this conditional header to copy the source blob only if its ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> SourceIfMatch;

    /**
     * @brief Specify this conditional header to copy the blob only if its ETag does not
     * match the value specified.
     */
    Azure::Core::Nullable<std::string> SourceIfNoneMatch;
  };

  /**
   * @brief Optional parameters for BlobClient::AbortCopyFromUri.
   */
  struct AbortCopyFromUriOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specify this header to perform the operation only if the resource has an
     * active lease mathing this id.
     */
    Azure::Core::Nullable<std::string> LeaseId;
  };

  /**
   * @brief Optional parameters for BlobClient::Download.
   */
  struct DownloadBlobOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Downloads only the bytes of the blob from this offset.
     */
    Azure::Core::Nullable<int64_t> Offset;

    /**
     * @brief Returns at most this number of bytes of the blob from the offset. Null means
     * download until the end.
     */
    Azure::Core::Nullable<int64_t> Length;

    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> IfMatch;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag does
     * not match the value specified. Specify the wildcard character (*) to perform the operation
     * only if the resource does not exist, and fail the operation if it does exist.
     */
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  /**
   * @brief Optional parameters for BlobClient::CreateSnapshot.
   */
  struct CreateSnapshotOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specifies user-defined name-value pairs associated with the blob. If no
     * name-value pairs are specified, the operation will copy the base blob metadata to the
     * snapshot. If one or more name-value pairs are specified, the snapshot is created with the
     * specified metadata, and metadata is not copied from the base blob.
     */
    std::map<std::string, std::string> Metadata;

    /**
     * @brief Specify this header to perform the operation only if the resource has an
     * active lease mathing this id.
     */
    Azure::Core::Nullable<std::string> LeaseId;

    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> IfMatch;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag does
     * not match the value specified. Specify the wildcard character (*) to perform the operation
     * only if the resource does not exist, and fail the operation if it does exist.
     */
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  /**
   * @brief Optional parameters for BlobClient::Delete.
   */
  struct DeleteBlobOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specifies to delete either the base blob
     * and all of its snapshots, or only the blob's snapshots and not the blob itself. Required if
     * the blob has associated snapshots.
     */
    Azure::Core::Nullable<DeleteSnapshotsOption> DeleteSnapshots;

    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> IfMatch;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag does
     * not match the value specified. Specify the wildcard character (*) to perform the operation
     * only if the resource does not exist, and fail the operation if it does exist.
     */
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  /**
   * @brief Optional parameters for BlobClient::Undelete.
   */
  struct UndeleteBlobOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  /**
   * @brief Optional parameters for BlockBlobClient::Upload.
   */
  struct UploadBlockBlobOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief An MD5 hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Core::Nullable<std::string> ContentMD5;

    /**
     * @brief A CRC64 hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Core::Nullable<std::string> ContentCRC64;

    /**
     * @brief The standard HTTP header system properties to set.
     */
    BlobHttpHeaders Properties;

    /**
     * @brief Name-value pairs associated with the blob as metadata.
     */
    std::map<std::string, std::string> Metadata;

    /**
     * @brief Indicates the tier to be set on blob.
     */
    Azure::Core::Nullable<AccessTier> Tier;

    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> IfMatch;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag does
     * not match the value specified. Specify the wildcard character (*) to perform the operation
     * only if the resource does not exist, and fail the operation if it does exist.
     */
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  /**
   * @brief Optional parameters for BlockBlobClient::StageBlock.
   */
  struct StageBlockOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief An MD5 hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Core::Nullable<std::string> ContentMD5;

    /**
     * @brief A CRC64 hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Core::Nullable<std::string> ContentCRC64;
  };

  /**
   * @brief Optional parameters for BlockBlobClient::StageBlockFromUri.
   */
  struct StageBlockFromUriOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Uploads only the bytes of the source blob from this offset.
     */
    Azure::Core::Nullable<int64_t> SourceOffset;

    /**
     * @brief Uploads this number of bytes of the source blob from the offset. Null means
     * upload until the end.
     */
    Azure::Core::Nullable<int64_t> SourceLength;

    /**
     * @brief An MD5 hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Core::Nullable<std::string> ContentMD5;

    /**
     * @brief A CRC64 hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Core::Nullable<std::string> ContentCRC64;

    /**
     * @brief Specify this header to perform the operation only if the resource has an
     * active lease mathing this id.
     */
    Azure::Core::Nullable<std::string> LeaseId;

    /**
     * @brief Specify this conditional header to copy the blob only if the source blob has
     * been modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> SourceIfModifiedSince;

    /**
     * @brief Specify this conditional header to copy the blob only if the source blob has
     * not been modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> SourceIfUnmodifiedSince;

    /**
     * @brief Specify this conditional header to copy the source blob only if its ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> SourceIfMatch;

    /**
     * @brief Specify this conditional header to copy the blob only if its ETag does not
     * match the value specified.
     */
    Azure::Core::Nullable<std::string> SourceIfNoneMatch;
  };

  /**
   * @brief Optional parameters for BlockBlobClient::CommitBlockList.
   */
  struct CommitBlockListOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief The standard HTTP header system properties to set.
     */
    BlobHttpHeaders Properties;

    /**
     * @brief Name-value pairs associated with the blob as metadata.
     */
    std::map<std::string, std::string> Metadata;

    /**
     * @brief Indicates the tier to be set on blob.
     */
    Azure::Core::Nullable<AccessTier> Tier;

    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> IfMatch;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag does
     * not match the value specified. Specify the wildcard character (*) to perform the operation
     * only if the resource does not exist, and fail the operation if it does exist.
     */
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  /**
   * @brief Optional parameters for BlockBlobClient::GetBlockList.
   */
  struct GetBlockListOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
    Azure::Core::Nullable<BlockListTypeOption> ListType;

    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> IfMatch;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag does
     * not match the value specified. Specify the wildcard character (*) to perform the operation
     * only if the resource does not exist, and fail the operation if it does exist.
     */
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  /**
   * @brief Optional parameters for AppendBlobClient::Create.
   */
  struct CreateAppendBlobOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief The standard HTTP header system properties to set.
     */
    BlobHttpHeaders Properties;

    /**
     * @brief Name-value pairs associated with the blob as metadata.
     */
    std::map<std::string, std::string> Metadata;

    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> IfMatch;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag does
     * not match the value specified. Specify the wildcard character (*) to perform the operation
     * only if the resource does not exist, and fail the operation if it does exist.
     */
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  /**
   * @brief Optional parameters for AppendBlobClient::AppendBlock.
   */
  struct AppendBlockOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief An MD5 hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Core::Nullable<std::string> ContentMD5;

    /**
     * @brief A CRC64 hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Core::Nullable<std::string> ContentCRC64;

    /**
     * @brief Specify this header to perform the operation only if the resource has an
     * active lease mathing this id.
     */
    Azure::Core::Nullable<std::string> LeaseId;

    /**
     * @brief Ensures that the AppendBlock operation succeeds only if the append blob's size
     * is less than or equal to this value.
     */
    Azure::Core::Nullable<int64_t> MaxSize;

    /**
     * @brief Ensures that the AppendBlock operation succeeds only if the append position is equal
     * to this value.
     */
    Azure::Core::Nullable<int64_t> AppendPosition;

    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> IfMatch;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag does
     * not match the value specified. Specify the wildcard character (*) to perform the operation
     * only if the resource does not exist, and fail the operation if it does exist.
     */
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  /**
   * @brief Optional parameters for AppendBlobClient::AppendBlockFromUri.
   */
  struct AppendBlockFromUriOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Uploads only the bytes of the source blob from this offset.
     */
    Azure::Core::Nullable<int64_t> SourceOffset;

    /**
     * @brief Uploads this number of bytes of the source blob from the offset. Null means
     * upload until the end.
     */
    Azure::Core::Nullable<int64_t> SourceLength;

    /**
     * @brief An MD5 hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Core::Nullable<std::string> ContentMD5;

    /**
     * @brief A CRC64 hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Core::Nullable<std::string> ContentCRC64;

    /**
     * @brief Specify this header to perform the operation only if the resource has an
     * active lease mathing this id.
     */
    Azure::Core::Nullable<std::string> LeaseId;

    /**
     * @brief Ensures that the AppendBlock operation succeeds only if the append blob's size
     * is less than or equal to this value.
     */
    Azure::Core::Nullable<int64_t> MaxSize;

    /**
     * @brief Ensures that the AppendBlock operation succeeds only if the append position is
     * equal to this value.
     */
    Azure::Core::Nullable<int64_t> AppendPosition;

    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> IfMatch;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag does
     * not match the value specified. Specify the wildcard character (*) to perform the operation
     * only if the resource does not exist, and fail the operation if it does exist.
     */
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  /**
   * @brief Optional parameters for PageBlobClient::Create.
   */
  struct CreatePageBlobOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
    Azure::Core::Nullable<int64_t> SequenceNumber;

    /**
     * @brief The standard HTTP header system properties to set.
     */
    BlobHttpHeaders Properties;

    /**
     * @brief Name-value pairs associated with the blob as metadata.
     */
    std::map<std::string, std::string> Metadata;

    /**
     * @brief Indicates the tier to be set on blob.
     */
    Azure::Core::Nullable<AccessTier> Tier;

    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> IfMatch;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag does
     * not match the value specified. Specify the wildcard character (*) to perform the operation
     * only if the resource does not exist, and fail the operation if it does exist.
     */
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  /**
   * @brief Optional parameters for PageBlobClient::UploadPages.
   */
  struct UploadPagesOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief An MD5 hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Core::Nullable<std::string> ContentMD5;

    /**
     * @brief A CRC64 hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Core::Nullable<std::string> ContentCRC64;

    /**
     * @brief Specify this header to perform the operation only if the resource has an
     * active lease mathing this id.
     */
    Azure::Core::Nullable<std::string> LeaseId;

    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> IfMatch;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag does
     * not match the value specified. Specify the wildcard character (*) to perform the operation
     * only if the resource does not exist, and fail the operation if it does exist.
     */
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  /**
   * @brief Optional parameters for PageBlobClient::UploadPagesFromUri.
   */
  struct UploadPagesFromUriOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief An MD5 hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Core::Nullable<std::string> ContentMD5;

    /**
     * @brief A CRC64 hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Core::Nullable<std::string> ContentCRC64;

    /**
     * @brief Specify this header to perform the operation only if the resource has an
     * active lease mathing this id.
     */
    Azure::Core::Nullable<std::string> LeaseId;

    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> IfMatch;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag does
     * not match the value specified. Specify the wildcard character (*) to perform the operation
     * only if the resource does not exist, and fail the operation if it does exist.
     */
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  /**
   * @brief Optional parameters for PageBlobClient::ClearPages.
   */
  struct ClearPagesOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specify this header to perform the operation only if the resource has an
     * active lease mathing this id.
     */
    Azure::Core::Nullable<std::string> LeaseId;

    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> IfMatch;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag does
     * not match the value specified. Specify the wildcard character (*) to perform the operation
     * only if the resource does not exist, and fail the operation if it does exist.
     */
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  /**
   * @brief Optional parameters for PageBlobClient::Resize.
   */
  struct ResizePageBlobOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> IfMatch;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag does
     * not match the value specified. Specify the wildcard character (*) to perform the operation
     * only if the resource does not exist, and fail the operation if it does exist.
     */
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  /**
   * @brief Optional parameters for PageBlobClient::GetPageRanges.
   */
  struct GetPageRangesOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specifies that the response will contain only pages that were changed between
     * target blob and previous snapshot. Changed pages include both updated and cleared pages.
     * The target blob may be a snapshot, as long as the snapshot specified by PreviousSnapshot is
     * the older of the two.
     */
    Azure::Core::Nullable<std::string> PreviousSnapshot;

    /**
     * @brief This parameter only works with managed disk storage accounts. Specifies that
     * the response will contain only pages that were changed between target blob and previous
     * snapshot. Changed pages include both updated and cleared pages. The target blob may be a
     * snapshot, as long as the snapshot specified by PreviousSnapshotUrl is the older of the two.
     */
    Azure::Core::Nullable<std::string> PreviousSnapshotUrl;

    /**
     * @brief Optionally specifies the offset of range over which to list ranges.
     */
    Azure::Core::Nullable<int64_t> Offset;

    /**
     * @brief Optionally specifies the length of range over which to list ranges.
     */
    Azure::Core::Nullable<int64_t> Length;

    /**
     * @brief Specify this header to perform the operation only if the resource has an
     * active lease mathing this id.
     */
    Azure::Core::Nullable<std::string> LeaseId;

    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> IfMatch;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag does
     * not match the value specified. Specify the wildcard character (*) to perform the operation
     * only if the resource does not exist, and fail the operation if it does exist.
     */
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  /**
   * @brief Optional parameters for PageBlobClient::StartCopyIncremental.
   */
  struct IncrementalCopyPageBlobOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> IfMatch;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag does
     * not match the value specified. Specify the wildcard character (*) to perform the operation
     * only if the resource does not exist, and fail the operation if it does exist.
     */
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

}}} // namespace Azure::Storage::Blobs
