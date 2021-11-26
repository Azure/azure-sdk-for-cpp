// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <azure/core/internal/client_options.hpp>
#include <azure/core/match_conditions.hpp>
#include <azure/core/modified_conditions.hpp>
#include <azure/storage/common/access_conditions.hpp>

#include "azure/storage/blobs/protocol/blob_rest_client.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  /**
   * @brief Specifies access conditions for a container.
   */
  struct BlobContainerAccessConditions final : public Azure::ModifiedConditions,
                                               public LeaseAccessConditions
  {
  };

  /**
   * @brief Specifies HTTP options for conditional requests based on tags.
   */
  struct TagAccessConditions
  {
    /**
     * @brief Destructor.
     *
     */
    virtual ~TagAccessConditions() = default;

    /**
     * @brief Optional SQL statement to apply to the tags of the Blob. Refer to
     * https://docs.microsoft.com/rest/api/storageservices/specifying-conditional-headers-for-blob-service-operations#tags-predicate-syntax
     * for the format of SQL statements.
     */
    Azure::Nullable<std::string> TagConditions;
  };

  /**
   * @brief Specifies access conditions for a blob.
   */
  struct BlobAccessConditions : public Azure::ModifiedConditions,
                                public Azure::MatchConditions,
                                public LeaseAccessConditions,
                                public TagAccessConditions
  {
  };

  /**
   * @brief Specifies access conditions for blob lease operations.
   */
  struct LeaseBlobAccessConditions final : public Azure::ModifiedConditions,
                                           public Azure::MatchConditions,
                                           public TagAccessConditions
  {
  };

  /**
   * @brief Specifies access conditions for an append blob.
   */
  struct AppendBlobAccessConditions final : public BlobAccessConditions
  {
    /**
     * @brief Ensures that the AppendBlock operation succeeds only if the append blob's size
     * is less than or equal to this value.
     */
    Azure::Nullable<int64_t> IfMaxSizeLessThanOrEqual;

    /**
     * @brief Ensures that the AppendBlock operation succeeds only if the append position is equal
     * to this value.
     */
    Azure::Nullable<int64_t> IfAppendPositionEqual;
  };

  /**
   * @brief Specifies access conditions for a page blob.
   */
  struct PageBlobAccessConditions final : public BlobAccessConditions
  {
    /**
     * @brief IfSequenceNumberLessThan ensures that the page blob operation succeeds only if
     * the blob's sequence number is less than a value.
     */
    Azure::Nullable<int64_t> IfSequenceNumberLessThan;

    /**
     * @brief IfSequenceNumberLessThanOrEqual ensures that the page blob operation succeeds
     * only if the blob's sequence number is less than or equal to a value.
     */
    Azure::Nullable<int64_t> IfSequenceNumberLessThanOrEqual;

    /**
     * @brief IfSequenceNumberEqual ensures that the page blob operation succeeds only
     * if the blob's sequence number is equal to a value.
     */
    Azure::Nullable<int64_t> IfSequenceNumberEqual;
  };

  /**
   * @brief Wrapper for an encryption key to be used with client provided key server-side
   * encryption.
   */
  struct EncryptionKey final
  {
    /**
     * @brief Base64 encoded string of the AES256 encryption key.
     */
    std::string Key;

    /**
     * @brief SHA256 hash of the AES256 encryption key.
     */
    std::vector<uint8_t> KeyHash;

    /**
     * @brief The algorithm for Azure Blob Storage to encrypt with.
     */
    Models::EncryptionAlgorithmType Algorithm;
  };

  /**
   * @brief Client options used to initialize all kinds of blob clients.
   */
  struct BlobClientOptions final : Azure::Core::_internal::ClientOptions
  {
    /**
     * @brief Holds the customer provided key used when making requests.
     */
    Azure::Nullable<EncryptionKey> CustomerProvidedKey;

    /**
     * @brief Holds the encryption scope used when making requests.
     */
    Azure::Nullable<std::string> EncryptionScope;

    /**
     * SecondaryHostForRetryReads specifies whether the retry policy should retry a read
     * operation against another host. If SecondaryHostForRetryReads is "" (the default) then
     * operations are not retried against another host. NOTE: Before setting this field, make sure
     * you understand the issues around reading stale & potentially-inconsistent data at this
     * webpage: https://docs.microsoft.com/azure/storage/common/geo-redundant-design.
     */
    std::string SecondaryHostForRetryReads;

    /**
     * API version used by this client.
     */
    std::string ApiVersion = _detail::ApiVersion;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobServiceClient::ListBlobContainers.
   */
  struct ListBlobContainersOptions final
  {
    /**
     * @brief Specifies a string that filters the results to return only containers whose
     * name begins with the specified prefix.
     */
    Azure::Nullable<std::string> Prefix;

    /**
     * @brief A string value that identifies the portion of the list of containers to be
     * returned with the next listing operation. The operation returns a non-empty
     * ListBlobContainersSegment.ContinuationToken value if the listing operation did not return all
     * containers remaining to be listed with the current segment. The ContinuationToken value can
     * be used as the value for the ContinuationToken parameter in a subsequent call to request the
     * next segment of list items.
     */
    Azure::Nullable<std::string> ContinuationToken;

    /**
     * @brief Specifies the maximum number of containers to return.
     */
    Azure::Nullable<int32_t> PageSizeHint;

    /**
     * @brief Specifies that the container's metadata be returned.
     */
    Models::ListBlobContainersIncludeFlags Include = Models::ListBlobContainersIncludeFlags::None;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobServiceClient::GetUserDelegationKey.
   */
  struct GetUserDelegationKeyOptions final
  {
    /**
     * @brief Start time for the key's validity. The time should be specified in UTC, and
     * will be truncated to second.
     */
    Azure::DateTime StartsOn = std::chrono::system_clock::now();
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobServiceClient::SetProperties.
   */
  struct SetServicePropertiesOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobServiceClient::GetProperties.
   */
  struct GetServicePropertiesOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobServiceClient::GetAccountInfo.
   */
  struct GetAccountInfoOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobServiceClient::GetStatistics.
   */
  struct GetBlobServiceStatisticsOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobServiceClient::FindBlobsByTags.
   */
  struct FindBlobsByTagsOptions final
  {
    /**
     * @brief A string value that identifies the portion of the result set to be returned
     * with the next operation. The operation returns a ContinuationToken value within the response
     * body if the result set returned was not complete. The ContinuationToken value may then be
     * used in a subsequent call to request the next set of items..
     */
    Azure::Nullable<std::string> ContinuationToken;

    /**
     * @brief Specifies the maximum number of blobs to return.
     */
    Azure::Nullable<int32_t> PageSizeHint;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobContainerClient::Create.
   */
  struct CreateBlobContainerOptions final
  {
    /**
     * @brief Specifies whether data in the container may be accessed publicly and the level
     * of access.
     */
    Models::PublicAccessType AccessType = Models::PublicAccessType::None;

    /**
     * @brief Name-value pairs to associate with the container as metadata.
     */
    Storage::Metadata Metadata;

    /**
     * @brief The encryption scope to use as the default on the container.
     */
    Azure::Nullable<std::string> DefaultEncryptionScope;

    /**
     * @brief If true, prevents any blob upload from specifying a different encryption
     * scope.
     */
    Azure::Nullable<bool> PreventEncryptionScopeOverride;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobContainerClient::Delete.
   */
  struct DeleteBlobContainerOptions final
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobContainerAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobContainerClient::Undelete.
   */
  struct UndeleteBlobContainerOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobContainerClient::GetProperties.
   */
  struct GetBlobContainerPropertiesOptions final
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobContainerClient::SetMetadata.
   */
  struct SetBlobContainerMetadataOptions final
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    struct : public LeaseAccessConditions
    {
      /**
       * @brief Specify this header to perform the operation only if the resource has been
       * modified since the specified time. This timestamp will be truncated to second.
       */
      Azure::Nullable<Azure::DateTime> IfModifiedSince;
    } AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobContainerClient::ListBlobs and
   * #Azure::Storage::Blobs::BlobContainerClient::ListBlobsByHierarchy.
   */
  struct ListBlobsOptions final
  {
    /**
     * @brief Specifies a string that filters the results to return only blobs whose
     * name begins with the specified prefix.
     */
    Azure::Nullable<std::string> Prefix;

    /**
     * @brief A string value that identifies the portion of the list of blobs to be
     * returned with the next listing operation. The operation returns a non-empty
     * BlobsFlatSegment.ContinuationToken value if the listing operation did not return all blobs
     * remaining to be listed with the current segment. The ContinuationToken value can be used as
     * the value for the ContinuationToken parameter in a subsequent call to request the next
     * segment of list items.
     */
    Azure::Nullable<std::string> ContinuationToken;

    /**
     * @brief Specifies the maximum number of blobs to return.
     */
    Azure::Nullable<int32_t> PageSizeHint;

    /**
     * @brief Specifies one or more datasets to include in the response.
     */
    Models::ListBlobsIncludeFlags Include = Models::ListBlobsIncludeFlags::None;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobContainerClient::GetAccessPolicy.
   */
  struct GetBlobContainerAccessPolicyOptions final
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobContainerClient::SetAccessPolicy.
   */
  struct SetBlobContainerAccessPolicyOptions final
  {
    /**
     * @brief Specifies whether data in the container may be accessed publicly and the level
     * of access.
     */
    Models::PublicAccessType AccessType = Models::PublicAccessType::None;

    /**
     * @brief Stored access policies that you can use to provide fine grained control over
     * container permissions.
     */
    std::vector<Models::SignedIdentifier> SignedIdentifiers;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobContainerAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobClient::GetProperties.
   */
  struct GetBlobPropertiesOptions final
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobClient::SetHttpHeaders.
   */
  struct SetBlobHttpHeadersOptions final
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobClient::SetMetadata.
   */
  struct SetBlobMetadataOptions final
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobClient::SetAccessTier.
   */
  struct SetBlobAccessTierOptions final
  {
    /**
     * @brief Indicates the priority with which to rehydrate an archived blob. The priority
     * can be set on a blob only once. This header will be ignored on subsequent requests to the
     * same blob.
     */
    Azure::Nullable<Models::RehydratePriority> RehydratePriority;
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    struct : public LeaseAccessConditions, public TagAccessConditions
    {
    } AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobClient::StartCopyFromUri.
   */
  struct StartBlobCopyFromUriOptions final
  {
    /**
     * @brief Specifies user-defined name-value pairs associated with the blob. If no
     * name-value pairs are specified, the operation will copy the metadata from the source blob or
     * file to the destination blob. If one or more name-value pairs are specified, the destination
     * blob is created with the specified metadata, and metadata is not copied from the source blob
     * or file.
     */
    Storage::Metadata Metadata;

    /**
     * @brief The tags to set for this blob.
     */
    std::map<std::string, std::string> Tags;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;

    /**
     * @brief Optional conditions that the source must meet to perform this operation.
     *
     * @note Lease access condition only works for API versions before 2012-02-12.
     */
    struct : public Azure::ModifiedConditions,
             public Azure::MatchConditions,
             public LeaseAccessConditions,
             public TagAccessConditions
    {
    } SourceAccessConditions;

    /**
     * @brief Specifies the tier to be set on the target blob.
     */
    Azure::Nullable<Models::AccessTier> AccessTier;

    /**
     * @brief Indicates the priority with which to rehydrate an archived blob. The priority
     * can be set on a blob only once. This header will be ignored on subsequent requests to the
     * same blob.
     */
    Azure::Nullable<Models::RehydratePriority> RehydratePriority;

    /**
     * @brief If the destination blob should be sealed. Only applicable for Append Blobs.
     */
    Azure::Nullable<bool> ShouldSealDestination;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobClient::CopyFromUri.
   */
  struct CopyBlobFromUriOptions
  {
    /**
     * @brief Specifies user-defined name-value pairs associated with the blob. If no
     * name-value pairs are specified, the operation will copy the metadata from the source blob or
     * file to the destination blob. If one or more name-value pairs are specified, the destination
     * blob is created with the specified metadata, and metadata is not copied from the source blob
     * or file.
     */
    Storage::Metadata Metadata;

    /**
     * @brief The tags to set for this blob.
     */
    std::map<std::string, std::string> Tags;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;

    /**
     * @brief Optional conditions that the source must meet to perform this operation.
     *
     * @note Lease access condition only works for API versions before 2012-02-12.
     */
    struct : public Azure::ModifiedConditions, public Azure::MatchConditions
    {
    } SourceAccessConditions;

    /**
     * @brief Specifies the tier to be set on the target blob.
     */
    Azure::Nullable<Models::AccessTier> AccessTier;

    /**
     * @brief Hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Nullable<ContentHash> TransactionalContentHash;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobClient::AbortCopyFromUri.
   */
  struct AbortBlobCopyFromUriOptions final
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobClient::Download.
   */
  struct DownloadBlobOptions final
  {
    /**
     * @brief Downloads only the bytes of the blob in the specified range.
     */
    Azure::Nullable<Core::Http::HttpRange> Range;

    /**
     * @brief When specified together with Range, service returns hash for the range as long as the
     * range is less than or equal to 4 MiB in size.
     */
    Azure::Nullable<HashAlgorithm> RangeHashAlgorithm;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobClient::DownloadTo.
   */
  struct DownloadBlobToOptions final
  {
    /**
     * @brief Downloads only the bytes of the blob in the specified range.
     */
    Azure::Nullable<Core::Http::HttpRange> Range;

    /**
     * @brief Options for parallel transfer.
     */
    struct
    {
      /**
       * @brief The size of the first range request in bytes. Blobs smaller than this limit will be
       * downloaded in a single request. Blobs larger than this limit will continue being downloaded
       * in chunks of size ChunkSize.
       */
      int64_t InitialChunkSize = 256 * 1024 * 1024;

      /**
       * @brief The maximum number of bytes in a single request.
       */
      int64_t ChunkSize = 4 * 1024 * 1024;

      /**
       * @brief The maximum number of threads that may be used in a parallel transfer.
       */
      int32_t Concurrency = 5;
    } TransferOptions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobClient::CreateSnapshot.
   */
  struct CreateBlobSnapshotOptions final
  {
    /**
     * @brief Specifies user-defined name-value pairs associated with the blob. If no
     * name-value pairs are specified, the operation will copy the base blob metadata to the
     * snapshot. If one or more name-value pairs are specified, the snapshot is created with the
     * specified metadata, and metadata is not copied from the base blob.
     */
    Storage::Metadata Metadata;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobClient::Delete.
   */
  struct DeleteBlobOptions final
  {
    /**
     * @brief Specifies to delete either the base blob
     * and all of its snapshots, or only the blob's snapshots and not the blob itself. Required if
     * the blob has associated snapshots.
     */
    Azure::Nullable<Models::DeleteSnapshotsOption> DeleteSnapshots;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobClient::Undelete.
   */
  struct UndeleteBlobOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobLeaseClient::Acquire.
   */
  struct AcquireLeaseOptions final
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    LeaseBlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobLeaseClient::Renew.
   */
  struct RenewLeaseOptions final
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    LeaseBlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobLeaseClient::Change.
   */
  struct ChangeLeaseOptions final
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    LeaseBlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobLeaseClient::Release.
   */
  struct ReleaseLeaseOptions final
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    LeaseBlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobLeaseClient::Break.
   */
  struct BreakLeaseOptions final
  {
    /**
     * @brief Proposed duration the lease should continue before it is broken, in seconds,
     * between 0 and 60. This break period is only used if it is shorter than the time remaining on
     * the lease. If longer, the time remaining on the lease is used. A new lease will not be
     * available before the break period has expired, but the lease may be held for longer than the
     * break period.
     */
    Azure::Nullable<std::chrono::seconds> BreakPeriod;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    LeaseBlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobClient::SetTags.
   */
  struct SetBlobTagsOptions final
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    struct : public TagAccessConditions, LeaseAccessConditions
    {
    } AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlobClient::GetTags.
   */
  struct GetBlobTagsOptions final
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    struct : public TagAccessConditions, LeaseAccessConditions
    {
    } AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlockBlobClient::Upload.
   */
  struct UploadBlockBlobOptions final
  {
    /**
     * @brief Hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Nullable<ContentHash> TransactionalContentHash;

    /**
     * @brief The standard HTTP header system properties to set.
     */
    Models::BlobHttpHeaders HttpHeaders;

    /**
     * @brief Name-value pairs associated with the blob as metadata.
     */
    Storage::Metadata Metadata;

    /**
     * @brief The tags to set for this blob.
     */
    std::map<std::string, std::string> Tags;

    /**
     * @brief Indicates the tier to be set on blob.
     */
    Azure::Nullable<Models::AccessTier> AccessTier;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlockBlobClient::UploadFrom.
   */
  struct UploadBlockBlobFromOptions final
  {
    /**
     * @brief The standard HTTP header system properties to set.
     */
    Models::BlobHttpHeaders HttpHeaders;

    /**
     * @brief Name-value pairs associated with the blob as metadata.
     */
    Storage::Metadata Metadata;

    /**
     * @brief The tags to set for this blob.
     */
    std::map<std::string, std::string> Tags;

    /**
     * @brief Indicates the tier to be set on blob.
     */
    Azure::Nullable<Models::AccessTier> AccessTier;

    /**
     * @brief Options for parallel transfer.
     */
    struct
    {
      /**
       * @brief Blob smaller than this will be uploaded with a single upload operation. This value
       * cannot be larger than 5000 MiB.
       */
      int64_t SingleUploadThreshold = 256 * 1024 * 1024;

      /**
       * @brief The maximum number of bytes in a single request. This value cannot be larger than
       * 4000 MiB.
       */
      Azure::Nullable<int64_t> ChunkSize;

      /**
       * @brief The maximum number of threads that may be used in a parallel transfer.
       */
      int32_t Concurrency = 5;
    } TransferOptions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlockBlobClient::StageBlock.
   */
  struct StageBlockOptions final
  {
    /**
     * @brief Hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Nullable<ContentHash> TransactionalContentHash;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlockBlobClient::StageBlockFromUri.
   */
  struct StageBlockFromUriOptions final
  {
    /**
     * @brief Uploads only the bytes of the source blob in the specified range.
     */
    Azure::Nullable<Core::Http::HttpRange> SourceRange;

    /**
     * @brief Hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Nullable<ContentHash> TransactionalContentHash;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    LeaseAccessConditions AccessConditions;

    /**
     * @brief Optional conditions that the source must meet to perform this operation.
     */
    struct : public Azure::ModifiedConditions, public Azure::MatchConditions
    {
    } SourceAccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlockBlobClient::CommitBlockList.
   */
  struct CommitBlockListOptions final
  {
    /**
     * @brief The standard HTTP header system properties to set.
     */
    Models::BlobHttpHeaders HttpHeaders;

    /**
     * @brief Name-value pairs associated with the blob as metadata.
     */
    Storage::Metadata Metadata;

    /**
     * @brief The tags to set for this blob.
     */
    std::map<std::string, std::string> Tags;

    /**
     * @brief Indicates the tier to be set on blob.
     */
    Azure::Nullable<Models::AccessTier> AccessTier;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::BlockBlobClient::GetBlockList.
   */
  struct GetBlockListOptions final
  {
    /**
     * @brief Specifies whether to return the list of committed blocks, the list of uncommitted
     * blocks, or both lists together.
     */
    Models::BlockListType ListType = Models::BlockListType::Committed;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    struct : public LeaseAccessConditions, public TagAccessConditions
    {
    } AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::AppendBlobClient::Create.
   */
  struct CreateAppendBlobOptions final
  {
    /**
     * @brief The standard HTTP header system properties to set.
     */
    Models::BlobHttpHeaders HttpHeaders;

    /**
     * @brief Name-value pairs associated with the blob as metadata.
     */
    Storage::Metadata Metadata;

    /**
     * @brief The tags to set for this blob.
     */
    std::map<std::string, std::string> Tags;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::AppendBlobClient::AppendBlock.
   */
  struct AppendBlockOptions final
  {
    /**
     * @brief Hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Nullable<ContentHash> TransactionalContentHash;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    AppendBlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::AppendBlobClient::AppendBlockFromUri.
   */
  struct AppendBlockFromUriOptions final
  {
    /**
     * @brief Uploads only the bytes of the source blob in the specified range.
     */
    Azure::Nullable<Core::Http::HttpRange> SourceRange;

    /**
     * @brief Hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Nullable<ContentHash> TransactionalContentHash;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    AppendBlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::AppendBlobClient::Seal.
   */
  struct SealAppendBlobOptions final
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    AppendBlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::PageBlobClient::Create.
   */
  struct CreatePageBlobOptions final
  {
    /**
     * @brief The sequence number is a user-controlled value that you can use to track requests. The
     * value of the sequence number must be between 0 and 2^63 - 1.
     */
    Azure::Nullable<int64_t> SequenceNumber;

    /**
     * @brief The standard HTTP header system properties to set.
     */
    Models::BlobHttpHeaders HttpHeaders;

    /**
     * @brief Name-value pairs associated with the blob as metadata.
     */
    Storage::Metadata Metadata;

    /**
     * @brief Indicates the tier to be set on blob.
     */
    Azure::Nullable<Models::AccessTier> AccessTier;

    /**
     * @brief The tags to set for this blob.
     */
    std::map<std::string, std::string> Tags;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::PageBlobClient::UploadPages.
   */
  struct UploadPagesOptions final
  {
    /**
     * @brief Hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Nullable<ContentHash> TransactionalContentHash;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    PageBlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::PageBlobClient::UploadPagesFromUri.
   */
  struct UploadPagesFromUriOptions final
  {
    /**
     * @brief Hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Nullable<ContentHash> TransactionalContentHash;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    PageBlobAccessConditions AccessConditions;

    /**
     * @brief Optional conditions that the source must meet to perform this operation.
     */
    struct : public Azure::ModifiedConditions, public Azure::MatchConditions
    {
    } SourceAccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::PageBlobClient::ClearPages.
   */
  struct ClearPagesOptions final
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    PageBlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::PageBlobClient::Resize.
   */
  struct ResizePageBlobOptions final
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::PageBlobClient::UpdateSequenceNumber.
   */
  struct UpdatePageBlobSequenceNumberOptions final
  {
    /**
     * @brief An updated sequence number of your choosing, if Action is Max or Update.
     */
    Azure::Nullable<int64_t> SequenceNumber;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::PageBlobClient::GetPageRanges.
   */
  struct GetPageRangesOptions final
  {
    /**
     * @brief Optionally specifies the range of bytes over which to list ranges, inclusively. If
     * omitted, then all ranges for the blob are returned.
     */
    Azure::Nullable<Core::Http::HttpRange> Range;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Blobs::PageBlobClient::StartCopyIncremental.
   */
  struct StartBlobCopyIncrementalOptions final
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

}}} // namespace Azure::Storage::Blobs
