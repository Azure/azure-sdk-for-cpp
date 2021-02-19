// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <azure/storage/common/access_conditions.hpp>
#include <azure/storage/common/storage_retry_policy.hpp>

#include "azure/storage/blobs/protocol/blob_rest_client.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  /**
   * @brief Specifies access conditions for a container.
   */
  struct BlobContainerAccessConditions : public ModifiedTimeConditions, public LeaseAccessConditions
  {
  };

  /**
   * @brief Specifies HTTP options for conditioanal requests based on tags.
   */
  struct TagAccessConditions
  {
    /**
     * @brief Optional SQL statement to apply to the tags of the Blob. Refer to
     * https://docs.microsoft.com/en-us/rest/api/storageservices/specifying-conditional-headers-for-blob-service-operations#tags-predicate-syntax
     * for the format of SQL statements.
     */
    Azure::Core::Nullable<std::string> TagConditions;
  };

  /**
   * @brief Specifies access conditions for a blob.
   */
  struct BlobAccessConditions : public ModifiedTimeConditions,
                                public ETagAccessConditions,
                                public LeaseAccessConditions,
                                public TagAccessConditions
  {
  };

  /**
   * @brief Specifies access conditions for blob lease operations.
   */
  struct LeaseBlobAccessConditions : public ModifiedTimeConditions,
                                     public ETagAccessConditions,
                                     public TagAccessConditions
  {
  };

  /**
   * @brief Specifies access conditions for a append blob.
   */
  struct AppendBlobAccessConditions : public BlobAccessConditions
  {
    /**
     * @brief Ensures that the AppendBlock operation succeeds only if the append blob's size
     * is less than or equal to this value.
     */
    Azure::Core::Nullable<int64_t> IfMaxSizeLessThanOrEqual;

    /**
     * @brief Ensures that the AppendBlock operation succeeds only if the append position is equal
     * to this value.
     */
    Azure::Core::Nullable<int64_t> IfAppendPositionEqual;
  };

  /**
   * @brief Specifies access conditions for a page blob.
   */
  struct PageBlobAccessConditions : public BlobAccessConditions
  {
    /**
     * @brief IfSequenceNumberLessThan ensures that the page blob operation succeeds only if
     * the blob's sequence number is less than a value.
     */
    Azure::Core::Nullable<int64_t> IfSequenceNumberLessThan;

    /**
     * @brief IfSequenceNumberLessThanOrEqual ensures that the page blob operation succeeds
     * only if the blob's sequence number is less than or equal to a value.
     */
    Azure::Core::Nullable<int64_t> IfSequenceNumberLessThanOrEqual;

    /**
     * @brief IfSequenceNumberEqual ensures that the page blob operation succeeds only
     * if the blob's sequence number is equal to a value.
     */
    Azure::Core::Nullable<int64_t> IfSequenceNumberEqual;
  };

  /**
   * @brief Wrapper for an encryption key to be used with client provided key server-side
   * encryption.
   */
  struct EncryptionKey
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
   * @brief Client options used to initalize all kinds of blob clients.
   */
  struct BlobClientOptions
  {
    /**
     * @brief Transport pipeline policies for authentication, additional HTTP headers, etc., that
     * are applied to every request.
     */
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> PerOperationPolicies;

    /**
     * @brief Transport pipeline policies for authentication, additional HTTP headers, etc., that
     * are applied to every retrial.
     */
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> PerRetryPolicies;

    /**
     * @brief Holds the customer provided key used when making requests.
     */
    Azure::Core::Nullable<EncryptionKey> CustomerProvidedKey;

    /**
     * @brief Holds the encryption scope used when making requests.
     */
    Azure::Core::Nullable<std::string> EncryptionScope;

    /**
     * @brief Specify the number of retries and other retry-related options.
     */
    StorageRetryWithSecondaryOptions RetryOptions;

    /**
     * @brief Customized HTTP client. We're going to use the default one if this is empty.
     */
    Azure::Core::Http::TransportPolicyOptions TransportPolicyOptions;

    /**
     * @brief The last part of the user agent for telemetry.
     */
    std::string ApplicationId;

    /**
     * API version used by this client.
     */
    std::string ApiVersion = Details::ApiVersion;
  };

  /**
   * @brief Optional parameters for BlobServiceClient::ListBlobContainersSinglePage.
   */
  struct ListBlobContainersSinglePageOptions
  {
    /**
     * @brief Specifies a string that filters the results to return only containers whose
     * name begins with the specified prefix.
     */
    Azure::Core::Nullable<std::string> Prefix;

    /**
     * @brief A string value that identifies the portion of the list of containers to be
     * returned with the next listing operation. The operation returns a non-empty
     * ListBlobContainersSegment.ContinuationToken value if the listing operation did not return all
     * containers remaining to be listed with the current segment. The ContinuationToken value can
     * be used as the value for the ContinuationToken parameter in a subsequent call to request the
     * next segment of list items.
     */
    Azure::Core::Nullable<std::string> ContinuationToken;

    /**
     * @brief Specifies the maximum number of containers to return.
     */
    Azure::Core::Nullable<int32_t> PageSizeHint;

    /**
     * @brief Specifies that the container's metadata be returned.
     */
    Models::ListBlobContainersIncludeFlags Include = Models::ListBlobContainersIncludeFlags::None;
  };

  /**
   * @brief Optional parameters for BlobServiceClient::GetUserDelegationKey.
   */
  struct GetUserDelegationKeyOptions
  {
    /**
     * @brief Start time for the key's validity. The time should be specified in UTC, and
     * will be truncated to second.
     */
    Azure::Core::DateTime startsOn = std::chrono::system_clock::now();
  };

  /**
   * @brief Optional parameters for BlobServiceClient::SetProperties.
   */
  struct SetServicePropertiesOptions
  {
  };

  /**
   * @brief Optional parameters for BlobServiceClient::GetProperties.
   */
  struct GetServicePropertiesOptions
  {
  };

  /**
   * @brief Optional parameters for BlobServiceClient::GetAccountInfo.
   */
  struct GetAccountInfoOptions
  {
  };

  /**
   * @brief Optional parameters for BlobServiceClient::GetStatistics.
   */
  struct GetBlobServiceStatisticsOptions
  {
  };

  /**
   * @brief Optional parameters for BlobServiceClient::FindBlobsByTagsSinglePage.
   */
  struct FindBlobsByTagsSinglePageOptions
  {
    /**
     * @brief A string value that identifies the portion of the result set to be returned
     * with the next operation. The operation returns a ContinuationToken value within the response
     * body if the result set returned was not complete. The ContinuationToken value may then be
     * used in a subsequent call to request the next set of items..
     */
    Azure::Core::Nullable<std::string> ContinuationToken;

    /**
     * @brief Specifies the maximum number of blobs to return.
     */
    Azure::Core::Nullable<int32_t> PageSizeHint;
  };

  /**
   * @brief Optional parameters for BlobContainerClient::Create.
   */
  struct CreateBlobContainerOptions
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
    Azure::Core::Nullable<std::string> DefaultEncryptionScope;

    /**
     * @brief If true, prevents any blob upload from specifying a different encryption
     * scope.
     */
    Azure::Core::Nullable<bool> PreventEncryptionScopeOverride;
  };

  /**
   * @brief Optional parameters for BlobContainerClient::Delete.
   */
  struct DeleteBlobContainerOptions
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobContainerAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for BlobContainerClient::Undelete.
   */
  struct UndeleteBlobContainerOptions
  {
    /**
     * @brief Use this parameter if you would like to restore the container under a
     * different name.
     */
    Azure::Core::Nullable<std::string> DestinationBlobContainerName;
  };

  /**
   * @brief Optional parameters for BlobContainerClient::GetProperties.
   */
  struct GetBlobContainerPropertiesOptions
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for BlobContainerClient::SetMetadata.
   */
  struct SetBlobContainerMetadataOptions
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
      Azure::Core::Nullable<Azure::Core::DateTime> IfModifiedSince;
    } AccessConditions;
  };

  /**
   * @brief Optional parameters for BlobContainerClient::ListBlobsSinglePage and
   * BlobContainerClient::ListBlobsByHierarchySinglePage.
   */
  struct ListBlobsSinglePageOptions
  {
    /**
     * @brief Specifies a string that filters the results to return only blobs whose
     * name begins with the specified prefix.
     */
    Azure::Core::Nullable<std::string> Prefix;

    /**
     * @brief A string value that identifies the portion of the list of blobs to be
     * returned with the next listing operation. The operation returns a non-empty
     * BlobsFlatSegment.ContinuationToken value if the listing operation did not return all blobs
     * remaining to be listed with the current segment. The ContinuationToken value can be used as
     * the value for the ContinuationToken parameter in a subsequent call to request the next
     * segment of list items.
     */
    Azure::Core::Nullable<std::string> ContinuationToken;

    /**
     * @brief Specifies the maximum number of blobs to return.
     */
    Azure::Core::Nullable<int32_t> PageSizeHint;

    /**
     * @brief Specifies one or more datasets to include in the response.
     */
    Models::ListBlobsIncludeFlags Include = Models::ListBlobsIncludeFlags::None;
  };

  /**
   * @brief Optional parameters for BlobContainerClient::GetAccessPolicy.
   */
  struct GetBlobContainerAccessPolicyOptions
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for BlobContainerClient::SetAccessPolicy.
   */
  struct SetBlobContainerAccessPolicyOptions
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
    std::vector<Models::BlobSignedIdentifier> SignedIdentifiers;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobContainerAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for BlobClient::GetProperties.
   */
  struct GetBlobPropertiesOptions
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for BlobClient::SetHttpHeaders.
   */
  struct SetBlobHttpHeadersOptions
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for BlobClient::SetMetadata.
   */
  struct SetBlobMetadataOptions
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for BlobClient::SetAccessTier.
   */
  struct SetBlobAccessTierOptions
  {
    /**
     * @brief Indicates the priority with which to rehydrate an archived blob. The priority
     * can be set on a blob only once. This header will be ignored on subsequent requests to the
     * same blob.
     */
    Azure::Core::Nullable<Models::RehydratePriority> RehydratePriority;
  };

  /**
   * @brief Optional parameters for BlobClient::StartCopyFromUri.
   */
  struct StartCopyBlobFromUriOptions
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
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;

    /**
     * @brief Optional conditions that the source must meet to perform this operation.
     */
    BlobAccessConditions SourceAccessConditions;

    /**
     * @brief Specifies the tier to be set on the target blob.
     */
    Azure::Core::Nullable<Models::AccessTier> Tier;

    /**
     * @brief Indicates the priority with which to rehydrate an archived blob. The priority
     * can be set on a blob only once. This header will be ignored on subsequent requests to the
     * same blob.
     */
    Azure::Core::Nullable<Models::RehydratePriority> RehydratePriority;

    /**
     * @brief If the destination blob should be sealed. Only applicable for Append Blobs.
     */
    Azure::Core::Nullable<bool> ShouldSealDestination;
  };

  /**
   * @brief Optional parameters for BlobClient::AbortCopyFromUri.
   */
  struct AbortCopyBlobFromUriOptions
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for BlobClient::Download.
   */
  struct DownloadBlobOptions
  {
    /**
     * @brief Downloads only the bytes of the blob in the specified range.
     */
    Azure::Core::Nullable<Core::Http::Range> Range;

    /**
     * @brief When specified together with Range, service returns hash for the range as long as the
     * range is less than or equal to 4 MiB in size.
     */
    Azure::Core::Nullable<HashAlgorithm> RangeHashAlgorithm;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for BlobClient::DownloadTo.
   */
  struct DownloadBlobToOptions
  {
    /**
     * @brief Downloads only the bytes of the blob in the specified range.
     */
    Azure::Core::Nullable<Core::Http::Range> Range;

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
      int Concurrency = 5;
    } TransferOptions;
  };

  /**
   * @brief Optional parameters for BlobClient::CreateSnapshot.
   */
  struct CreateBlobSnapshotOptions
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
   * @brief Optional parameters for BlobClient::Delete.
   */
  struct DeleteBlobOptions
  {
    /**
     * @brief Specifies to delete either the base blob
     * and all of its snapshots, or only the blob's snapshots and not the blob itself. Required if
     * the blob has associated snapshots.
     */
    Azure::Core::Nullable<Models::DeleteSnapshotsOption> DeleteSnapshots;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for BlobClient::Undelete.
   */
  struct UndeleteBlobOptions
  {
  };

  /**
   * @brief Optional parameters for BlobLeaseClient::Acquire.
   */
  struct AcquireBlobLeaseOptions
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    LeaseBlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for BlobLeaseClient::Renew.
   */
  struct RenewBlobLeaseOptions
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    LeaseBlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for BlobLeaseClient::Change.
   */
  struct ChangeBlobLeaseOptions
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    LeaseBlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for BlobLeaseClient::Release.
   */
  struct ReleaseBlobLeaseOptions
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    LeaseBlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for BlobLeaseClient::Break.
   */
  struct BreakBlobLeaseOptions
  {
    /**
     * @brief Proposed duration the lease should continue before it is broken, in seconds,
     * between 0 and 60. This break period is only used if it is shorter than the time remaining on
     * the lease. If longer, the time remaining on the lease is used. A new lease will not be
     * available before the break period has expired, but the lease may be held for longer than the
     * break period.
     */
    Azure::Core::Nullable<std::chrono::seconds> BreakPeriod;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    LeaseBlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for BlobClient::SetTags.
   */
  struct SetBlobTagsOptions
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    TagAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for BlobClient::GetTags.
   */
  struct GetBlobTagsOptions
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    TagAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for BlockBlobClient::Upload.
   */
  struct UploadBlockBlobOptions
  {
    /**
     * @brief Hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Core::Nullable<ContentHash> TransactionalContentHash;

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
    Azure::Core::Nullable<Models::AccessTier> Tier;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for BlockBlobClient::UploadFrom.
   */
  struct UploadBlockBlobFromOptions
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
     * @brief Indicates the tier to be set on blob.
     */
    Azure::Core::Nullable<Models::AccessTier> Tier;

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
      int64_t ChunkSize = 4 * 1024 * 1024;

      /**
       * @brief The maximum number of threads that may be used in a parallel transfer.
       */
      int Concurrency = 5;
    } TransferOptions;
  };

  /**
   * @brief Optional parameters for BlockBlobClient::StageBlock.
   */
  struct StageBlockOptions
  {
    /**
     * @brief Hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Core::Nullable<ContentHash> TransactionalContentHash;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for BlockBlobClient::StageBlockFromUri.
   */
  struct StageBlockFromUriOptions
  {
    /**
     * @brief Uploads only the bytes of the source blob in the specified range.
     */
    Azure::Core::Nullable<Core::Http::Range> SourceRange;

    /**
     * @brief Hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Core::Nullable<ContentHash> TransactionalContentHash;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    LeaseAccessConditions AccessConditions;

    /**
     * @brief Optional conditions that the source must meet to perform this operation.
     */
    struct : public ModifiedTimeConditions, public ETagAccessConditions
    {
    } SourceAccessConditions;
  };

  /**
   * @brief Optional parameters for BlockBlobClient::CommitBlockList.
   */
  struct CommitBlockListOptions
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
     * @brief Indicates the tier to be set on blob.
     */
    Azure::Core::Nullable<Models::AccessTier> Tier;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for BlockBlobClient::GetBlockList.
   */
  struct GetBlockListOptions
  {
    /**
     * @brief Specifies whether to return the list of committed blocks, the list of uncommitted
     * blocks, or both lists together.
     */
    Models::BlockListTypeOption ListType = Models::BlockListTypeOption::Committed;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    struct : public LeaseAccessConditions, public TagAccessConditions
    {
    } AccessConditions;
  };

  /**
   * @brief Optional parameters for AppendBlobClient::Create.
   */
  struct CreateAppendBlobOptions
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
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for AppendBlobClient::AppendBlock.
   */
  struct AppendBlockOptions
  {
    /**
     * @brief Hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Core::Nullable<ContentHash> TransactionalContentHash;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    AppendBlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for AppendBlobClient::AppendBlockFromUri.
   */
  struct AppendBlockFromUriOptions
  {
    /**
     * @brief Uploads only the bytes of the source blob in the specified range.
     */
    Azure::Core::Nullable<Core::Http::Range> SourceRange;

    /**
     * @brief Hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Core::Nullable<ContentHash> TransactionalContentHash;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    AppendBlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for AppendBlobClient::Seal.
   */
  struct SealAppendBlobOptions
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    AppendBlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for PageBlobClient::Create.
   */
  struct CreatePageBlobOptions
  {
    /**
     * @brief The sequence number is a user-controlled value that you can use to track requests. The
     * value of the sequence number must be between 0 and 2^63 - 1.
     */
    Azure::Core::Nullable<int64_t> SequenceNumber;

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
    Azure::Core::Nullable<Models::AccessTier> Tier;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for PageBlobClient::UploadPages.
   */
  struct UploadPageBlobPagesOptions
  {
    /**
     * @brief Hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Core::Nullable<ContentHash> TransactionalContentHash;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    PageBlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for PageBlobClient::UploadPagesFromUri.
   */
  struct UploadPageBlobPagesFromUriOptions
  {
    /**
     * @brief Hash of the blob content. This hash is used to verify the integrity of
     * the blob during transport. When this header is specified, the storage service checks the hash
     * that has arrived with the one that was sent.
     */
    Azure::Core::Nullable<ContentHash> TransactionalContentHash;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    PageBlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for PageBlobClient::ClearPages.
   */
  struct ClearPageBlobPagesOptions
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    PageBlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for PageBlobClient::Resize.
   */
  struct ResizePageBlobOptions
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for PageBlobClient::GetPageRanges.
   */
  struct GetPageBlobPageRangesOptions
  {
    /**
     * @brief Optionally specifies the range of bytes over which to list ranges, inclusively. If
     * omitted, then all ranges for the blob are returned.
     */
    Azure::Core::Nullable<Core::Http::Range> Range;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for PageBlobClient::StartCopyIncremental.
   */
  struct StartCopyPageBlobIncrementalOptions
  {
    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

}}} // namespace Azure::Storage::Blobs
