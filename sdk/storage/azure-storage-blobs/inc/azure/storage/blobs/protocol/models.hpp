// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/pipeline.hpp"
#include "azure/core/nullable.hpp"
#include "azure/core/response.hpp"
#include "azure/core/strings.hpp"
#include "azure/storage/common/crypt.hpp"
#include "azure/storage/common/storage_common.hpp"
#include "azure/storage/common/storage_exception.hpp"
#include "azure/storage/common/xml_wrapper.hpp"

#include <cstring>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace Azure { namespace Storage { namespace Blobs { namespace Models {

  struct AbortCopyBlobFromUriResult
  {
  }; // struct AbortCopyBlobFromUriResult

  enum class AccessTier
  {
    Unknown,
    P1,
    P2,
    P3,
    P4,
    P6,
    P10,
    P15,
    P20,
    P30,
    P40,
    P50,
    P60,
    P70,
    P80,
    Hot,
    Cool,
    Archive,
  }; // enum class AccessTier

  enum class AccountKind
  {
    Unknown,
    Storage,
    BlobStorage,
    StorageV2,
    FileStorage,
    BlockBlobStorage,
  }; // enum class AccountKind

  struct AcquireBlobLeaseResult
  {
    std::string ETag;
    std::string LastModified;
    std::string LeaseId;
  }; // struct AcquireBlobLeaseResult

  struct AcquireContainerLeaseResult
  {
    std::string ETag;
    std::string LastModified;
    std::string LeaseId;
  }; // struct AcquireContainerLeaseResult

  struct AppendBlockFromUriResult
  {
    std::string ETag;
    std::string LastModified;
    Azure::Core::Nullable<std::string> TransactionalContentMd5;
    Azure::Core::Nullable<std::string> TransactionalContentCrc64;
    int64_t AppendOffset = 0;
    int64_t CommittedBlockCount = 0;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
    Azure::Core::Nullable<std::string> EncryptionScope;
  }; // struct AppendBlockFromUriResult

  struct AppendBlockResult
  {
    std::string ETag;
    std::string LastModified;
    Azure::Core::Nullable<std::string> TransactionalContentMd5;
    Azure::Core::Nullable<std::string> TransactionalContentCrc64;
    int64_t AppendOffset = 0;
    int64_t CommittedBlockCount = 0;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
    Azure::Core::Nullable<std::string> EncryptionScope;
  }; // struct AppendBlockResult

  enum class BlobArchiveStatus
  {
    Unknown,
    RehydratePendingToHot,
    RehydratePendingToCool,
  }; // enum class BlobArchiveStatus

  struct BlobBlock
  {
    std::string Name;
    int64_t Size = 0;
  }; // struct BlobBlock

  struct BlobCorsRule
  {
    std::string AllowedOrigins;
    std::string AllowedMethods;
    std::string AllowedHeaders;
    std::string ExposedHeaders;
    int32_t MaxAgeInSeconds = 0;
  }; // struct BlobCorsRule

  enum class BlobGeoReplicationStatus
  {
    Unknown,
    Live,
    Bootstrap,
    Unavailable,
  }; // enum class BlobGeoReplicationStatus

  struct BlobHttpHeaders
  {
    std::string ContentType;
    std::string ContentEncoding;
    std::string ContentLanguage;
    std::string ContentMd5;
    std::string CacheControl;
    std::string ContentDisposition;
  }; // struct BlobHttpHeaders

  enum class BlobLeaseState
  {
    Available,
    Leased,
    Expired,
    Breaking,
    Broken,
  }; // enum class BlobLeaseState

  enum class BlobLeaseStatus
  {
    Locked,
    Unlocked,
  }; // enum class BlobLeaseStatus

  struct BlobPrefix
  {
    std::string Name;
  }; // struct BlobPrefix

  struct BlobRetentionPolicy
  {
    bool Enabled = false;
    Azure::Core::Nullable<int32_t> Days;
  }; // struct BlobRetentionPolicy

  struct BlobSignedIdentifier
  {
    std::string Id;
    std::string StartsOn;
    std::string ExpiresOn;
    std::string Permissions;
  }; // struct BlobSignedIdentifier

  struct BlobStaticWebsite
  {
    bool Enabled = false;
    Azure::Core::Nullable<std::string> IndexDocument;
    Azure::Core::Nullable<std::string> DefaultIndexDocumentPath;
    Azure::Core::Nullable<std::string> ErrorDocument404Path;
  }; // struct BlobStaticWebsite

  enum class BlobType
  {
    Unknown,
    BlockBlob,
    PageBlob,
    AppendBlob,
  }; // enum class BlobType

  enum class BlockListTypeOption
  {
    Committed,
    Uncommitted,
    All,
  }; // enum class BlockListTypeOption

  enum class BlockType
  {
    Committed,
    Uncommitted,
    Latest,
  }; // enum class BlockType

  struct BreakBlobLeaseResult
  {
    std::string ETag;
    std::string LastModified;
    int32_t LeaseTime = 0;
  }; // struct BreakBlobLeaseResult

  struct BreakContainerLeaseResult
  {
    std::string ETag;
    std::string LastModified;
    int32_t LeaseTime = 0;
  }; // struct BreakContainerLeaseResult

  struct ChangeBlobLeaseResult
  {
    std::string ETag;
    std::string LastModified;
    std::string LeaseId;
  }; // struct ChangeBlobLeaseResult

  struct ChangeContainerLeaseResult
  {
    std::string ETag;
    std::string LastModified;
    std::string LeaseId;
  }; // struct ChangeContainerLeaseResult

  struct ClearPageBlobPagesResult
  {
    std::string ETag;
    std::string LastModified;
    int64_t SequenceNumber = 0;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
    Azure::Core::Nullable<std::string> EncryptionScope;
  }; // struct ClearPageBlobPagesResult

  struct CommitBlockListResult
  {
    std::string ETag;
    std::string LastModified;
    Azure::Core::Nullable<std::string> VersionId;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
    Azure::Core::Nullable<std::string> EncryptionScope;
    Azure::Core::Nullable<std::string> TransactionalContentMd5;
    Azure::Core::Nullable<std::string> TransactionalContentCrc64;
  }; // struct CommitBlockListResult

  enum class CopyStatus
  {
    Unknown,
    Success,
    Pending,
  }; // enum class CopyStatus

  struct CreateAppendBlobResult
  {
    std::string ETag;
    std::string LastModified;
    Azure::Core::Nullable<std::string> VersionId;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
    Azure::Core::Nullable<std::string> EncryptionScope;
  }; // struct CreateAppendBlobResult

  struct CreateBlobSnapshotResult
  {
    std::string Snapshot;
    std::string ETag;
    std::string LastModified;
    Azure::Core::Nullable<std::string> VersionId;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
    Azure::Core::Nullable<std::string> EncryptionScope;
  }; // struct CreateBlobSnapshotResult

  struct CreateContainerResult
  {
    std::string ETag;
    std::string LastModified;
  }; // struct CreateContainerResult

  struct CreatePageBlobResult
  {
    std::string ETag;
    std::string LastModified;
    Azure::Core::Nullable<std::string> VersionId;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
    Azure::Core::Nullable<std::string> EncryptionScope;
    Azure::Core::Nullable<int64_t> SequenceNumber;
  }; // struct CreatePageBlobResult

  struct DeleteBlobResult
  {
  }; // struct DeleteBlobResult

  struct DeleteContainerResult
  {
  }; // struct DeleteContainerResult

  enum class DeleteSnapshotsOption
  {
    None,
    IncludeSnapshots,
    Only,
  }; // enum class DeleteSnapshotsOption

  enum class EncryptionAlgorithmType
  {
    Unknown,
    Aes256,
  }; // enum class EncryptionAlgorithmType

  struct FilterBlobItem
  {
    std::string BlobName;
    std::string ContainerName;
    std::string TagValue;
  }; // struct FilterBlobItem

  struct GetBlobTagsResult
  {
    std::map<std::string, std::string> Tags;
  }; // struct GetBlobTagsResult

  struct GetPageBlobPageRangesResultInternal
  {
    std::string ETag;
    std::string LastModified;
    int64_t BlobContentLength = 0;
    std::vector<std::pair<int64_t, int64_t>> PageRanges;
    std::vector<std::pair<int64_t, int64_t>> ClearRanges;
  }; // struct GetPageBlobPageRangesResultInternal

  struct GetUserDelegationKeyResult
  {
    std::string SignedObjectId;
    std::string SignedTenantId;
    std::string SignedStartsOn;
    std::string SignedExpiresOn;
    std::string SignedService;
    std::string SignedVersion;
    std::string Value;
  }; // struct GetUserDelegationKeyResult

  enum class ListBlobContainersIncludeItem
  {
    None = 0,
    Metadata = 1,
    Deleted = 2,
  }; // bitwise enum ListBlobContainersIncludeItem

  inline ListBlobContainersIncludeItem operator|(
      ListBlobContainersIncludeItem lhs,
      ListBlobContainersIncludeItem rhs)
  {
    using type = std::underlying_type_t<ListBlobContainersIncludeItem>;
    return static_cast<ListBlobContainersIncludeItem>(
        static_cast<type>(lhs) | static_cast<type>(rhs));
  }

  inline ListBlobContainersIncludeItem& operator|=(
      ListBlobContainersIncludeItem& lhs,
      ListBlobContainersIncludeItem rhs)
  {
    lhs = lhs | rhs;
    return lhs;
  }

  inline ListBlobContainersIncludeItem operator&(
      ListBlobContainersIncludeItem lhs,
      ListBlobContainersIncludeItem rhs)
  {
    using type = std::underlying_type_t<ListBlobContainersIncludeItem>;
    return static_cast<ListBlobContainersIncludeItem>(
        static_cast<type>(lhs) & static_cast<type>(rhs));
  }

  inline ListBlobContainersIncludeItem& operator&=(
      ListBlobContainersIncludeItem& lhs,
      ListBlobContainersIncludeItem rhs)
  {
    lhs = lhs & rhs;
    return lhs;
  }

  enum class ListBlobsIncludeItem
  {
    None = 0,
    Copy = 1,
    Deleted = 2,
    Metadata = 4,
    Snapshots = 8,
    Versions = 16,
    UncomittedBlobs = 32,
  }; // bitwise enum ListBlobsIncludeItem

  inline ListBlobsIncludeItem operator|(ListBlobsIncludeItem lhs, ListBlobsIncludeItem rhs)
  {
    using type = std::underlying_type_t<ListBlobsIncludeItem>;
    return static_cast<ListBlobsIncludeItem>(static_cast<type>(lhs) | static_cast<type>(rhs));
  }

  inline ListBlobsIncludeItem& operator|=(ListBlobsIncludeItem& lhs, ListBlobsIncludeItem rhs)
  {
    lhs = lhs | rhs;
    return lhs;
  }

  inline ListBlobsIncludeItem operator&(ListBlobsIncludeItem lhs, ListBlobsIncludeItem rhs)
  {
    using type = std::underlying_type_t<ListBlobsIncludeItem>;
    return static_cast<ListBlobsIncludeItem>(static_cast<type>(lhs) & static_cast<type>(rhs));
  }

  inline ListBlobsIncludeItem& operator&=(ListBlobsIncludeItem& lhs, ListBlobsIncludeItem rhs)
  {
    lhs = lhs & rhs;
    return lhs;
  }

  enum class ObjectReplicationStatus
  {
    Unknown,
    Complete,
    Failed,
  }; // enum class ObjectReplicationStatus

  enum class PublicAccessType
  {
    Container,
    Blob,
    Private,
  }; // enum class PublicAccessType

  enum class RehydratePriority
  {
    Unknown,
    High,
    Standard,
  }; // enum class RehydratePriority

  struct ReleaseBlobLeaseResult
  {
    std::string ETag;
    std::string LastModified;
    Azure::Core::Nullable<int64_t> SequenceNumber;
  }; // struct ReleaseBlobLeaseResult

  struct ReleaseContainerLeaseResult
  {
    std::string ETag;
    std::string LastModified;
  }; // struct ReleaseContainerLeaseResult

  struct RenewBlobLeaseResult
  {
    std::string ETag;
    std::string LastModified;
    std::string LeaseId;
  }; // struct RenewBlobLeaseResult

  struct RenewContainerLeaseResult
  {
    std::string ETag;
    std::string LastModified;
    std::string LeaseId;
  }; // struct RenewContainerLeaseResult

  struct ResizePageBlobResult
  {
    std::string ETag;
    std::string LastModified;
    int64_t SequenceNumber = 0;
  }; // struct ResizePageBlobResult

  enum class ScheduleBlobExpiryOriginType
  {
    Unknown,
    NeverExpire,
    RelativeToCreation,
    RelativeToNow,
    Absolute,
  }; // enum class ScheduleBlobExpiryOriginType

  struct SealAppendBlobResult
  {
    std::string ETag;
    std::string LastModified;
    bool IsSealed = true;
  }; // struct SealAppendBlobResult

  struct SetBlobAccessTierResult
  {
  }; // struct SetBlobAccessTierResult

  struct SetBlobExpiryResult
  {
  }; // struct SetBlobExpiryResult

  struct SetBlobHttpHeadersResult
  {
    std::string ETag;
    std::string LastModified;
    Azure::Core::Nullable<int64_t> SequenceNumber;
  }; // struct SetBlobHttpHeadersResult

  struct SetBlobMetadataResult
  {
    std::string ETag;
    std::string LastModified;
    Azure::Core::Nullable<int64_t> SequenceNumber;
  }; // struct SetBlobMetadataResult

  struct SetBlobTagsResult
  {
  }; // struct SetBlobTagsResult

  struct SetContainerAccessPolicyResult
  {
    std::string ETag;
    std::string LastModified;
  }; // struct SetContainerAccessPolicyResult

  struct SetContainerMetadataResult
  {
    std::string ETag;
    std::string LastModified;
  }; // struct SetContainerMetadataResult

  struct SetServicePropertiesResult
  {
  }; // struct SetServicePropertiesResult

  enum class SkuName
  {
    Unknown,
    StandardLrs,
    StandardGrs,
    StandardRagrs,
    StandardZrs,
    PremiumLrs,
    PremiumZrs,
    StandardGzrs,
    StandardRagzrs,
  }; // enum class SkuName

  struct StageBlockFromUriResult
  {
    Azure::Core::Nullable<std::string> TransactionalContentMd5;
    Azure::Core::Nullable<std::string> TransactionalContentCrc64;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
    Azure::Core::Nullable<std::string> EncryptionScope;
  }; // struct StageBlockFromUriResult

  struct StageBlockResult
  {
    Azure::Core::Nullable<std::string> TransactionalContentMd5;
    Azure::Core::Nullable<std::string> TransactionalContentCrc64;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
    Azure::Core::Nullable<std::string> EncryptionScope;
  }; // struct StageBlockResult

  struct SubmitBlobBatchResultInternal
  {
    std::string ContentType;
  }; // struct SubmitBlobBatchResultInternal

  struct UndeleteBlobResult
  {
  }; // struct UndeleteBlobResult

  struct UndeleteContainerResult
  {
  }; // struct UndeleteContainerResult

  struct UploadBlockBlobResult
  {
    std::string ETag;
    std::string LastModified;
    Azure::Core::Nullable<std::string> VersionId;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
    Azure::Core::Nullable<std::string> EncryptionScope;
    Azure::Core::Nullable<std::string> TransactionalContentMd5;
    Azure::Core::Nullable<std::string> TransactionalContentCrc64;
  }; // struct UploadBlockBlobResult

  struct UploadPageBlobPagesFromUriResult
  {
    std::string ETag;
    std::string LastModified;
    Azure::Core::Nullable<std::string> TransactionalContentMd5;
    Azure::Core::Nullable<std::string> TransactionalContentCrc64;
    int64_t SequenceNumber = 0;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
    Azure::Core::Nullable<std::string> EncryptionScope;
  }; // struct UploadPageBlobPagesFromUriResult

  struct UploadPageBlobPagesResult
  {
    std::string ETag;
    std::string LastModified;
    Azure::Core::Nullable<std::string> TransactionalContentMd5;
    Azure::Core::Nullable<std::string> TransactionalContentCrc64;
    int64_t SequenceNumber = 0;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
    Azure::Core::Nullable<std::string> EncryptionScope;
  }; // struct UploadPageBlobPagesResult

  struct BlobAnalyticsLogging
  {
    std::string Version;
    bool Delete = false;
    bool Read = false;
    bool Write = false;
    BlobRetentionPolicy RetentionPolicy;
  }; // struct BlobAnalyticsLogging

  struct BlobContainerItem
  {
    std::string Name;
    std::string ETag;
    std::string LastModified;
    std::map<std::string, std::string> Metadata;
    PublicAccessType AccessType = PublicAccessType::Private;
    bool HasImmutabilityPolicy = false;
    bool HasLegalHold = false;
    Azure::Core::Nullable<std::string> LeaseDuration;
    BlobLeaseState LeaseState = BlobLeaseState::Available;
    BlobLeaseStatus LeaseStatus = BlobLeaseStatus::Unlocked;
    std::string DefaultEncryptionScope;
    bool PreventEncryptionScopeOverride = false;
    bool IsDeleted = false;
    Azure::Core::Nullable<std::string> VersionId;
    Azure::Core::Nullable<std::string> DeletedTime;
    Azure::Core::Nullable<int32_t> RemainingRetentionDays;
  }; // struct BlobContainerItem

  struct BlobGeoReplication
  {
    BlobGeoReplicationStatus Status = BlobGeoReplicationStatus::Unknown;
    Azure::Core::Nullable<std::string> LastSyncTime;
  }; // struct BlobGeoReplication

  struct BlobMetrics
  {
    std::string Version;
    bool Enabled = false;
    BlobRetentionPolicy RetentionPolicy;
    Azure::Core::Nullable<bool> IncludeApis;
  }; // struct BlobMetrics

  struct FilterBlobsSegmentResult
  {
    std::string ServiceEndpoint;
    std::string Where;
    std::string ContinuationToken;
    std::vector<FilterBlobItem> Items;
  }; // struct FilterBlobsSegmentResult

  struct GetAccountInfoResult
  {
    Blobs::Models::SkuName SkuName = Blobs::Models::SkuName::Unknown;
    Blobs::Models::AccountKind AccountKind = Blobs::Models::AccountKind::Unknown;
  }; // struct GetAccountInfoResult

  struct GetBlockListResult
  {
    std::string ETag;
    std::string LastModified;
    std::string ContentType;
    int64_t ContentLength = 0;
    std::vector<BlobBlock> CommittedBlocks;
    std::vector<BlobBlock> UncommittedBlocks;
  }; // struct GetBlockListResult

  struct GetContainerAccessPolicyResult
  {
    PublicAccessType AccessType = PublicAccessType::Private;
    std::string ETag;
    std::string LastModified;
    std::vector<BlobSignedIdentifier> SignedIdentifiers;
  }; // struct GetContainerAccessPolicyResult

  struct GetContainerPropertiesResult
  {
    std::string ETag;
    std::string LastModified;
    std::map<std::string, std::string> Metadata;
    PublicAccessType AccessType = PublicAccessType::Private;
    bool HasImmutabilityPolicy = false;
    bool HasLegalHold = false;
    Azure::Core::Nullable<std::string> LeaseDuration;
    BlobLeaseState LeaseState = BlobLeaseState::Available;
    BlobLeaseStatus LeaseStatus = BlobLeaseStatus::Unlocked;
    std::string DefaultEncryptionScope;
    bool PreventEncryptionScopeOverride = false;
  }; // struct GetContainerPropertiesResult

  struct ObjectReplicationRule
  {
    std::string RuleId;
    ObjectReplicationStatus ReplicationStatus = ObjectReplicationStatus::Unknown;
  }; // struct ObjectReplicationRule

  struct StartCopyBlobFromUriResult
  {
    std::string ETag;
    std::string LastModified;
    std::string CopyId;
    Blobs::Models::CopyStatus CopyStatus = Blobs::Models::CopyStatus::Unknown;
    Azure::Core::Nullable<std::string> VersionId;
  }; // struct StartCopyBlobFromUriResult

  struct StartCopyPageBlobIncrementalResult
  {
    std::string ETag;
    std::string LastModified;
    std::string CopyId;
    Blobs::Models::CopyStatus CopyStatus = Blobs::Models::CopyStatus::Unknown;
    Azure::Core::Nullable<std::string> VersionId;
  }; // struct StartCopyPageBlobIncrementalResult

  struct BlobServiceProperties
  {
    BlobAnalyticsLogging Logging;
    BlobMetrics HourMetrics;
    BlobMetrics MinuteMetrics;
    std::vector<BlobCorsRule> Cors;
    Azure::Core::Nullable<std::string> DefaultServiceVersion;
    BlobRetentionPolicy DeleteRetentionPolicy;
    BlobStaticWebsite StaticWebsite;
  }; // struct BlobServiceProperties

  struct GetServicePropertiesResult
  {
    BlobAnalyticsLogging Logging;
    BlobMetrics HourMetrics;
    BlobMetrics MinuteMetrics;
    std::vector<BlobCorsRule> Cors;
    Azure::Core::Nullable<std::string> DefaultServiceVersion;
    BlobRetentionPolicy DeleteRetentionPolicy;
    BlobStaticWebsite StaticWebsite;
  }; // struct GetServicePropertiesResult

  struct GetServiceStatisticsResult
  {
    BlobGeoReplication GeoReplication;
  }; // struct GetServiceStatisticsResult

  struct ListContainersSegmentResult
  {
    std::string ServiceEndpoint;
    std::string Prefix;
    std::string PreviousContinuationToken;
    std::string ContinuationToken;
    std::vector<BlobContainerItem> Items;
  }; // struct ListContainersSegmentResult

  struct ObjectReplicationPolicy
  {
    std::string PolicyId;
    std::vector<ObjectReplicationRule> Rules;
  }; // struct ObjectReplicationPolicy

  struct BlobItem
  {
    std::string Name;
    bool Deleted = false;
    std::string Snapshot;
    Azure::Core::Nullable<std::string> VersionId;
    Azure::Core::Nullable<bool> IsCurrentVersion;
    BlobHttpHeaders HttpHeaders;
    std::map<std::string, std::string> Metadata;
    std::string CreationTime;
    Azure::Core::Nullable<std::string> ExpiryTime;
    Azure::Core::Nullable<std::string> LastAccessTime;
    std::string LastModified;
    std::string ETag;
    int64_t ContentLength = 0;
    Blobs::Models::BlobType BlobType = Blobs::Models::BlobType::Unknown;
    Azure::Core::Nullable<AccessTier> Tier;
    Azure::Core::Nullable<bool> AccessTierInferred;
    BlobLeaseStatus LeaseStatus = BlobLeaseStatus::Unlocked;
    BlobLeaseState LeaseState = BlobLeaseState::Available;
    Azure::Core::Nullable<std::string> LeaseDuration;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
    Azure::Core::Nullable<std::string> EncryptionScope;
    Azure::Core::Nullable<int64_t> SequenceNumber; // only for page blobd
    Azure::Core::Nullable<bool> IsSealed; // only for append blob
    std::vector<ObjectReplicationPolicy>
        ObjectReplicationSourceProperties; // only valid for replication source blob
  }; // struct BlobItem

  struct DownloadBlobResult
  {
    std::unique_ptr<Azure::Core::Http::BodyStream> BodyStream;
    std::string ETag;
    std::string LastModified;
    std::string CreationTime;
    Azure::Core::Nullable<std::string> ExpiryTime;
    Azure::Core::Nullable<std::string> LastAccessTime;
    Azure::Core::Nullable<std::string> ContentRange;
    BlobHttpHeaders HttpHeaders;
    std::map<std::string, std::string> Metadata;
    Azure::Core::Nullable<int64_t> SequenceNumber; // only for page blob
    Azure::Core::Nullable<int64_t> CommittedBlockCount; // only for append blob
    Azure::Core::Nullable<bool> IsSealed; // only for append blob
    Blobs::Models::BlobType BlobType = Blobs::Models::BlobType::Unknown;
    Azure::Core::Nullable<std::string> TransactionalContentMd5; // Md5 for the downloaded range
    Azure::Core::Nullable<std::string> TransactionalContentCrc64;
    Azure::Core::Nullable<std::string> LeaseDuration;
    Azure::Core::Nullable<BlobLeaseState> LeaseState;
    Azure::Core::Nullable<BlobLeaseStatus> LeaseStatus;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
    Azure::Core::Nullable<std::string> EncryptionScope;
    Azure::Core::Nullable<std::string>
        ObjectReplicationDestinationPolicyId; // only valid for replication destination blob
    std::vector<ObjectReplicationPolicy>
        ObjectReplicationSourceProperties; // only valid for replication source blob
    Azure::Core::Nullable<int32_t> TagCount;
  }; // struct DownloadBlobResult

  struct GetBlobPropertiesResult
  {
    std::string ETag;
    std::string LastModified;
    std::string CreationTime;
    Azure::Core::Nullable<std::string> ExpiryTime;
    Azure::Core::Nullable<std::string> LastAccessTime;
    std::map<std::string, std::string> Metadata;
    Blobs::Models::BlobType BlobType = Blobs::Models::BlobType::Unknown;
    Azure::Core::Nullable<std::string> LeaseDuration;
    Azure::Core::Nullable<BlobLeaseState> LeaseState;
    Azure::Core::Nullable<BlobLeaseStatus> LeaseStatus;
    int64_t ContentLength = 0;
    BlobHttpHeaders HttpHeaders;
    Azure::Core::Nullable<int64_t> SequenceNumber; // only for page blob
    Azure::Core::Nullable<int32_t> CommittedBlockCount; // only for append blob
    Azure::Core::Nullable<bool> IsSealed; // only for append blob
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
    Azure::Core::Nullable<std::string> EncryptionScope;
    Azure::Core::Nullable<AccessTier> Tier;
    Azure::Core::Nullable<bool> AccessTierInferred;
    Azure::Core::Nullable<BlobArchiveStatus> ArchiveStatus;
    Azure::Core::Nullable<std::string> AccessTierChangeTime;
    Azure::Core::Nullable<std::string> CopyId;
    Azure::Core::Nullable<std::string> CopySource;
    Azure::Core::Nullable<Blobs::Models::CopyStatus> CopyStatus;
    Azure::Core::Nullable<std::string> CopyProgress;
    Azure::Core::Nullable<std::string> CopyCompletionTime;
    Azure::Core::Nullable<std::string>
        ObjectReplicationDestinationPolicyId; // only valid for replication destination blob
    std::vector<ObjectReplicationPolicy>
        ObjectReplicationSourceProperties; // only valid for replication source blob
    Azure::Core::Nullable<int32_t> TagCount;
  }; // struct GetBlobPropertiesResult

  struct ListBlobsByHierarchySegmentResult
  {
    std::string ServiceEndpoint;
    std::string Container;
    std::string Prefix;
    std::string Delimiter;
    std::string PreviousContinuationToken;
    std::string ContinuationToken;
    std::vector<BlobItem> Items;
    std::vector<BlobPrefix> BlobPrefixes;
  }; // struct ListBlobsByHierarchySegmentResult

  struct ListBlobsFlatSegmentResult
  {
    std::string ServiceEndpoint;
    std::string Container;
    std::string Prefix;
    std::string PreviousContinuationToken;
    std::string ContinuationToken;
    std::vector<BlobItem> Items;
  }; // struct ListBlobsFlatSegmentResult

}}}} // namespace Azure::Storage::Blobs::Models
