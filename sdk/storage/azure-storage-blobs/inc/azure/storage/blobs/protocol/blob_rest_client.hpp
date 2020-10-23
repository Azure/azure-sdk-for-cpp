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
#include "azure/storage/common/storage_error.hpp"
#include "azure/storage/common/xml_wrapper.hpp"

#include <cstring>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace Azure { namespace Storage { namespace Blobs {

  constexpr static const char* c_ApiVersion = "2020-02-10";

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

  inline std::string AccessTierToString(const AccessTier& access_tier)
  {
    switch (access_tier)
    {
      case AccessTier::Unknown:
        return "";
      case AccessTier::P1:
        return "P1";
      case AccessTier::P2:
        return "P2";
      case AccessTier::P3:
        return "P3";
      case AccessTier::P4:
        return "P4";
      case AccessTier::P6:
        return "P6";
      case AccessTier::P10:
        return "P10";
      case AccessTier::P15:
        return "P15";
      case AccessTier::P20:
        return "P20";
      case AccessTier::P30:
        return "P30";
      case AccessTier::P40:
        return "P40";
      case AccessTier::P50:
        return "P50";
      case AccessTier::P60:
        return "P60";
      case AccessTier::P70:
        return "P70";
      case AccessTier::P80:
        return "P80";
      case AccessTier::Hot:
        return "Hot";
      case AccessTier::Cool:
        return "Cool";
      case AccessTier::Archive:
        return "Archive";
      default:
        return std::string();
    }
  }

  inline AccessTier AccessTierFromString(const std::string& access_tier)
  {
    if (access_tier == "")
    {
      return AccessTier::Unknown;
    }
    if (access_tier == "P1")
    {
      return AccessTier::P1;
    }
    if (access_tier == "P2")
    {
      return AccessTier::P2;
    }
    if (access_tier == "P3")
    {
      return AccessTier::P3;
    }
    if (access_tier == "P4")
    {
      return AccessTier::P4;
    }
    if (access_tier == "P6")
    {
      return AccessTier::P6;
    }
    if (access_tier == "P10")
    {
      return AccessTier::P10;
    }
    if (access_tier == "P15")
    {
      return AccessTier::P15;
    }
    if (access_tier == "P20")
    {
      return AccessTier::P20;
    }
    if (access_tier == "P30")
    {
      return AccessTier::P30;
    }
    if (access_tier == "P40")
    {
      return AccessTier::P40;
    }
    if (access_tier == "P50")
    {
      return AccessTier::P50;
    }
    if (access_tier == "P60")
    {
      return AccessTier::P60;
    }
    if (access_tier == "P70")
    {
      return AccessTier::P70;
    }
    if (access_tier == "P80")
    {
      return AccessTier::P80;
    }
    if (access_tier == "Hot")
    {
      return AccessTier::Hot;
    }
    if (access_tier == "Cool")
    {
      return AccessTier::Cool;
    }
    if (access_tier == "Archive")
    {
      return AccessTier::Archive;
    }
    throw std::runtime_error("cannot convert " + access_tier + " to AccessTier");
  }

  enum class AccountKind
  {
    Unknown,
    Storage,
    BlobStorage,
    StorageV2,
    FileStorage,
    BlockBlobStorage,
  }; // enum class AccountKind

  inline std::string AccountKindToString(const AccountKind& account_kind)
  {
    switch (account_kind)
    {
      case AccountKind::Unknown:
        return "";
      case AccountKind::Storage:
        return "Storage";
      case AccountKind::BlobStorage:
        return "BlobStorage";
      case AccountKind::StorageV2:
        return "StorageV2";
      case AccountKind::FileStorage:
        return "FileStorage";
      case AccountKind::BlockBlobStorage:
        return "BlockBlobStorage";
      default:
        return std::string();
    }
  }

  inline AccountKind AccountKindFromString(const std::string& account_kind)
  {
    if (account_kind == "")
    {
      return AccountKind::Unknown;
    }
    if (account_kind == "Storage")
    {
      return AccountKind::Storage;
    }
    if (account_kind == "BlobStorage")
    {
      return AccountKind::BlobStorage;
    }
    if (account_kind == "StorageV2")
    {
      return AccountKind::StorageV2;
    }
    if (account_kind == "FileStorage")
    {
      return AccountKind::FileStorage;
    }
    if (account_kind == "BlockBlobStorage")
    {
      return AccountKind::BlockBlobStorage;
    }
    throw std::runtime_error("cannot convert " + account_kind + " to AccountKind");
  }

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

  inline std::string BlobArchiveStatusToString(const BlobArchiveStatus& blob_archive_status)
  {
    switch (blob_archive_status)
    {
      case BlobArchiveStatus::Unknown:
        return "";
      case BlobArchiveStatus::RehydratePendingToHot:
        return "rehydrate-pending-to-hot";
      case BlobArchiveStatus::RehydratePendingToCool:
        return "rehydrate-pending-to-cool";
      default:
        return std::string();
    }
  }

  inline BlobArchiveStatus BlobArchiveStatusFromString(const std::string& blob_archive_status)
  {
    if (blob_archive_status == "")
    {
      return BlobArchiveStatus::Unknown;
    }
    if (blob_archive_status == "rehydrate-pending-to-hot")
    {
      return BlobArchiveStatus::RehydratePendingToHot;
    }
    if (blob_archive_status == "rehydrate-pending-to-cool")
    {
      return BlobArchiveStatus::RehydratePendingToCool;
    }
    throw std::runtime_error("cannot convert " + blob_archive_status + " to BlobArchiveStatus");
  }

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

  inline std::string BlobGeoReplicationStatusToString(
      const BlobGeoReplicationStatus& blob_geo_replication_status)
  {
    switch (blob_geo_replication_status)
    {
      case BlobGeoReplicationStatus::Unknown:
        return "";
      case BlobGeoReplicationStatus::Live:
        return "live";
      case BlobGeoReplicationStatus::Bootstrap:
        return "bootstrap";
      case BlobGeoReplicationStatus::Unavailable:
        return "unavailable";
      default:
        return std::string();
    }
  }

  inline BlobGeoReplicationStatus BlobGeoReplicationStatusFromString(
      const std::string& blob_geo_replication_status)
  {
    if (blob_geo_replication_status == "")
    {
      return BlobGeoReplicationStatus::Unknown;
    }
    if (blob_geo_replication_status == "live")
    {
      return BlobGeoReplicationStatus::Live;
    }
    if (blob_geo_replication_status == "bootstrap")
    {
      return BlobGeoReplicationStatus::Bootstrap;
    }
    if (blob_geo_replication_status == "unavailable")
    {
      return BlobGeoReplicationStatus::Unavailable;
    }
    throw std::runtime_error(
        "cannot convert " + blob_geo_replication_status + " to BlobGeoReplicationStatus");
  }

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

  inline std::string BlobLeaseStateToString(const BlobLeaseState& blob_lease_state)
  {
    switch (blob_lease_state)
    {
      case BlobLeaseState::Available:
        return "available";
      case BlobLeaseState::Leased:
        return "leased";
      case BlobLeaseState::Expired:
        return "expired";
      case BlobLeaseState::Breaking:
        return "breaking";
      case BlobLeaseState::Broken:
        return "broken";
      default:
        return std::string();
    }
  }

  inline BlobLeaseState BlobLeaseStateFromString(const std::string& blob_lease_state)
  {
    if (blob_lease_state == "available")
    {
      return BlobLeaseState::Available;
    }
    if (blob_lease_state == "leased")
    {
      return BlobLeaseState::Leased;
    }
    if (blob_lease_state == "expired")
    {
      return BlobLeaseState::Expired;
    }
    if (blob_lease_state == "breaking")
    {
      return BlobLeaseState::Breaking;
    }
    if (blob_lease_state == "broken")
    {
      return BlobLeaseState::Broken;
    }
    throw std::runtime_error("cannot convert " + blob_lease_state + " to BlobLeaseState");
  }

  enum class BlobLeaseStatus
  {
    Locked,
    Unlocked,
  }; // enum class BlobLeaseStatus

  inline std::string BlobLeaseStatusToString(const BlobLeaseStatus& blob_lease_status)
  {
    switch (blob_lease_status)
    {
      case BlobLeaseStatus::Locked:
        return "locked";
      case BlobLeaseStatus::Unlocked:
        return "unlocked";
      default:
        return std::string();
    }
  }

  inline BlobLeaseStatus BlobLeaseStatusFromString(const std::string& blob_lease_status)
  {
    if (blob_lease_status == "locked")
    {
      return BlobLeaseStatus::Locked;
    }
    if (blob_lease_status == "unlocked")
    {
      return BlobLeaseStatus::Unlocked;
    }
    throw std::runtime_error("cannot convert " + blob_lease_status + " to BlobLeaseStatus");
  }

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

  inline std::string BlobTypeToString(const BlobType& blob_type)
  {
    switch (blob_type)
    {
      case BlobType::Unknown:
        return "";
      case BlobType::BlockBlob:
        return "BlockBlob";
      case BlobType::PageBlob:
        return "PageBlob";
      case BlobType::AppendBlob:
        return "AppendBlob";
      default:
        return std::string();
    }
  }

  inline BlobType BlobTypeFromString(const std::string& blob_type)
  {
    if (blob_type == "")
    {
      return BlobType::Unknown;
    }
    if (blob_type == "BlockBlob")
    {
      return BlobType::BlockBlob;
    }
    if (blob_type == "PageBlob")
    {
      return BlobType::PageBlob;
    }
    if (blob_type == "AppendBlob")
    {
      return BlobType::AppendBlob;
    }
    throw std::runtime_error("cannot convert " + blob_type + " to BlobType");
  }

  enum class BlockListTypeOption
  {
    Committed,
    Uncommitted,
    All,
  }; // enum class BlockListTypeOption

  inline std::string BlockListTypeOptionToString(const BlockListTypeOption& block_list_type_option)
  {
    switch (block_list_type_option)
    {
      case BlockListTypeOption::Committed:
        return "committed";
      case BlockListTypeOption::Uncommitted:
        return "uncommitted";
      case BlockListTypeOption::All:
        return "all";
      default:
        return std::string();
    }
  }

  inline BlockListTypeOption BlockListTypeOptionFromString(
      const std::string& block_list_type_option)
  {
    if (block_list_type_option == "committed")
    {
      return BlockListTypeOption::Committed;
    }
    if (block_list_type_option == "uncommitted")
    {
      return BlockListTypeOption::Uncommitted;
    }
    if (block_list_type_option == "all")
    {
      return BlockListTypeOption::All;
    }
    throw std::runtime_error(
        "cannot convert " + block_list_type_option + " to BlockListTypeOption");
  }

  enum class BlockType
  {
    Committed,
    Uncommitted,
    Latest,
  }; // enum class BlockType

  inline std::string BlockTypeToString(const BlockType& block_type)
  {
    switch (block_type)
    {
      case BlockType::Committed:
        return "Committed";
      case BlockType::Uncommitted:
        return "Uncommitted";
      case BlockType::Latest:
        return "Latest";
      default:
        return std::string();
    }
  }

  inline BlockType BlockTypeFromString(const std::string& block_type)
  {
    if (block_type == "Committed")
    {
      return BlockType::Committed;
    }
    if (block_type == "Uncommitted")
    {
      return BlockType::Uncommitted;
    }
    if (block_type == "Latest")
    {
      return BlockType::Latest;
    }
    throw std::runtime_error("cannot convert " + block_type + " to BlockType");
  }

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

  inline std::string CopyStatusToString(const CopyStatus& copy_status)
  {
    switch (copy_status)
    {
      case CopyStatus::Unknown:
        return "";
      case CopyStatus::Success:
        return "success";
      case CopyStatus::Pending:
        return "pending";
      default:
        return std::string();
    }
  }

  inline CopyStatus CopyStatusFromString(const std::string& copy_status)
  {
    if (copy_status == "")
    {
      return CopyStatus::Unknown;
    }
    if (copy_status == "success")
    {
      return CopyStatus::Success;
    }
    if (copy_status == "pending")
    {
      return CopyStatus::Pending;
    }
    throw std::runtime_error("cannot convert " + copy_status + " to CopyStatus");
  }

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

  inline std::string DeleteSnapshotsOptionToString(
      const DeleteSnapshotsOption& delete_snapshots_option)
  {
    switch (delete_snapshots_option)
    {
      case DeleteSnapshotsOption::None:
        return "";
      case DeleteSnapshotsOption::IncludeSnapshots:
        return "include";
      case DeleteSnapshotsOption::Only:
        return "only";
      default:
        return std::string();
    }
  }

  inline DeleteSnapshotsOption DeleteSnapshotsOptionFromString(
      const std::string& delete_snapshots_option)
  {
    if (delete_snapshots_option == "")
    {
      return DeleteSnapshotsOption::None;
    }
    if (delete_snapshots_option == "include")
    {
      return DeleteSnapshotsOption::IncludeSnapshots;
    }
    if (delete_snapshots_option == "only")
    {
      return DeleteSnapshotsOption::Only;
    }
    throw std::runtime_error(
        "cannot convert " + delete_snapshots_option + " to DeleteSnapshotsOption");
  }

  enum class EncryptionAlgorithmType
  {
    Unknown,
    Aes256,
  }; // enum class EncryptionAlgorithmType

  inline std::string EncryptionAlgorithmTypeToString(
      const EncryptionAlgorithmType& encryption_algorithm_type)
  {
    switch (encryption_algorithm_type)
    {
      case EncryptionAlgorithmType::Unknown:
        return "";
      case EncryptionAlgorithmType::Aes256:
        return "AES256";
      default:
        return std::string();
    }
  }

  inline EncryptionAlgorithmType EncryptionAlgorithmTypeFromString(
      const std::string& encryption_algorithm_type)
  {
    if (encryption_algorithm_type == "")
    {
      return EncryptionAlgorithmType::Unknown;
    }
    if (encryption_algorithm_type == "AES256")
    {
      return EncryptionAlgorithmType::Aes256;
    }
    throw std::runtime_error(
        "cannot convert " + encryption_algorithm_type + " to EncryptionAlgorithmType");
  }

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

  inline std::string ListBlobContainersIncludeItemToString(const ListBlobContainersIncludeItem& val)
  {
    ListBlobContainersIncludeItem value_list[] = {
        ListBlobContainersIncludeItem::Metadata,
        ListBlobContainersIncludeItem::Deleted,
    };
    const char* string_list[] = {
        "metadata",
        "deleted",
    };
    std::string ret;
    for (std::size_t i = 0; i < sizeof(value_list) / sizeof(ListBlobContainersIncludeItem); ++i)
    {
      if ((val & value_list[i]) == value_list[i])
      {
        if (!ret.empty())
        {
          ret += ",";
        }
        ret += string_list[i];
      }
    }
    return ret;
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

  inline std::string ListBlobsIncludeItemToString(const ListBlobsIncludeItem& val)
  {
    ListBlobsIncludeItem value_list[] = {
        ListBlobsIncludeItem::Copy,
        ListBlobsIncludeItem::Deleted,
        ListBlobsIncludeItem::Metadata,
        ListBlobsIncludeItem::Snapshots,
        ListBlobsIncludeItem::Versions,
        ListBlobsIncludeItem::UncomittedBlobs,
    };
    const char* string_list[] = {
        "copy",
        "deleted",
        "metadata",
        "snapshots",
        "versions",
        "uncommittedblobs",
    };
    std::string ret;
    for (std::size_t i = 0; i < sizeof(value_list) / sizeof(ListBlobsIncludeItem); ++i)
    {
      if ((val & value_list[i]) == value_list[i])
      {
        if (!ret.empty())
        {
          ret += ",";
        }
        ret += string_list[i];
      }
    }
    return ret;
  }

  enum class ObjectReplicationStatus
  {
    Unknown,
    Complete,
    Failed,
  }; // enum class ObjectReplicationStatus

  inline std::string ObjectReplicationStatusToString(
      const ObjectReplicationStatus& object_replication_status)
  {
    switch (object_replication_status)
    {
      case ObjectReplicationStatus::Unknown:
        return "";
      case ObjectReplicationStatus::Complete:
        return "complete";
      case ObjectReplicationStatus::Failed:
        return "failed";
      default:
        return std::string();
    }
  }

  inline ObjectReplicationStatus ObjectReplicationStatusFromString(
      const std::string& object_replication_status)
  {
    if (object_replication_status == "")
    {
      return ObjectReplicationStatus::Unknown;
    }
    if (object_replication_status == "complete")
    {
      return ObjectReplicationStatus::Complete;
    }
    if (object_replication_status == "failed")
    {
      return ObjectReplicationStatus::Failed;
    }
    throw std::runtime_error(
        "cannot convert " + object_replication_status + " to ObjectReplicationStatus");
  }

  enum class PublicAccessType
  {
    Container,
    Blob,
    Private,
  }; // enum class PublicAccessType

  inline std::string PublicAccessTypeToString(const PublicAccessType& public_access_type)
  {
    switch (public_access_type)
    {
      case PublicAccessType::Container:
        return "container";
      case PublicAccessType::Blob:
        return "blob";
      case PublicAccessType::Private:
        return "";
      default:
        return std::string();
    }
  }

  inline PublicAccessType PublicAccessTypeFromString(const std::string& public_access_type)
  {
    if (public_access_type == "container")
    {
      return PublicAccessType::Container;
    }
    if (public_access_type == "blob")
    {
      return PublicAccessType::Blob;
    }
    if (public_access_type == "")
    {
      return PublicAccessType::Private;
    }
    throw std::runtime_error("cannot convert " + public_access_type + " to PublicAccessType");
  }

  enum class RehydratePriority
  {
    Unknown,
    High,
    Standard,
  }; // enum class RehydratePriority

  inline std::string RehydratePriorityToString(const RehydratePriority& rehydrate_priority)
  {
    switch (rehydrate_priority)
    {
      case RehydratePriority::Unknown:
        return "";
      case RehydratePriority::High:
        return "High";
      case RehydratePriority::Standard:
        return "Standard";
      default:
        return std::string();
    }
  }

  inline RehydratePriority RehydratePriorityFromString(const std::string& rehydrate_priority)
  {
    if (rehydrate_priority == "")
    {
      return RehydratePriority::Unknown;
    }
    if (rehydrate_priority == "High")
    {
      return RehydratePriority::High;
    }
    if (rehydrate_priority == "Standard")
    {
      return RehydratePriority::Standard;
    }
    throw std::runtime_error("cannot convert " + rehydrate_priority + " to RehydratePriority");
  }

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

  inline std::string ScheduleBlobExpiryOriginTypeToString(
      const ScheduleBlobExpiryOriginType& schedule_blob_expiry_origin_type)
  {
    switch (schedule_blob_expiry_origin_type)
    {
      case ScheduleBlobExpiryOriginType::Unknown:
        return "";
      case ScheduleBlobExpiryOriginType::NeverExpire:
        return "NeverExpire";
      case ScheduleBlobExpiryOriginType::RelativeToCreation:
        return "RelativeToCreation";
      case ScheduleBlobExpiryOriginType::RelativeToNow:
        return "RelativeToNow";
      case ScheduleBlobExpiryOriginType::Absolute:
        return "Absolute";
      default:
        return std::string();
    }
  }

  inline ScheduleBlobExpiryOriginType ScheduleBlobExpiryOriginTypeFromString(
      const std::string& schedule_blob_expiry_origin_type)
  {
    if (schedule_blob_expiry_origin_type == "")
    {
      return ScheduleBlobExpiryOriginType::Unknown;
    }
    if (schedule_blob_expiry_origin_type == "NeverExpire")
    {
      return ScheduleBlobExpiryOriginType::NeverExpire;
    }
    if (schedule_blob_expiry_origin_type == "RelativeToCreation")
    {
      return ScheduleBlobExpiryOriginType::RelativeToCreation;
    }
    if (schedule_blob_expiry_origin_type == "RelativeToNow")
    {
      return ScheduleBlobExpiryOriginType::RelativeToNow;
    }
    if (schedule_blob_expiry_origin_type == "Absolute")
    {
      return ScheduleBlobExpiryOriginType::Absolute;
    }
    throw std::runtime_error(
        "cannot convert " + schedule_blob_expiry_origin_type + " to ScheduleBlobExpiryOriginType");
  }

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

  inline std::string SkuNameToString(const SkuName& sku_name)
  {
    switch (sku_name)
    {
      case SkuName::Unknown:
        return "";
      case SkuName::StandardLrs:
        return "Standard_LRS";
      case SkuName::StandardGrs:
        return "Standard_GRS";
      case SkuName::StandardRagrs:
        return "Standard_RAGRS";
      case SkuName::StandardZrs:
        return "Standard_ZRS";
      case SkuName::PremiumLrs:
        return "Premium_LRS";
      case SkuName::PremiumZrs:
        return "Premium_ZRS";
      case SkuName::StandardGzrs:
        return "Standard_GZRS";
      case SkuName::StandardRagzrs:
        return "Standard_RAGZRS";
      default:
        return std::string();
    }
  }

  inline SkuName SkuNameFromString(const std::string& sku_name)
  {
    if (sku_name == "")
    {
      return SkuName::Unknown;
    }
    if (sku_name == "Standard_LRS")
    {
      return SkuName::StandardLrs;
    }
    if (sku_name == "Standard_GRS")
    {
      return SkuName::StandardGrs;
    }
    if (sku_name == "Standard_RAGRS")
    {
      return SkuName::StandardRagrs;
    }
    if (sku_name == "Standard_ZRS")
    {
      return SkuName::StandardZrs;
    }
    if (sku_name == "Premium_LRS")
    {
      return SkuName::PremiumLrs;
    }
    if (sku_name == "Premium_ZRS")
    {
      return SkuName::PremiumZrs;
    }
    if (sku_name == "Standard_GZRS")
    {
      return SkuName::StandardGzrs;
    }
    if (sku_name == "Standard_RAGZRS")
    {
      return SkuName::StandardRagzrs;
    }
    throw std::runtime_error("cannot convert " + sku_name + " to SkuName");
  }

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
    Blobs::SkuName SkuName = Blobs::SkuName::Unknown;
    Blobs::AccountKind AccountKind = Blobs::AccountKind::Unknown;
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
    Blobs::CopyStatus CopyStatus = Blobs::CopyStatus::Unknown;
    Azure::Core::Nullable<std::string> VersionId;
  }; // struct StartCopyBlobFromUriResult

  struct StartCopyPageBlobIncrementalResult
  {
    std::string ETag;
    std::string LastModified;
    std::string CopyId;
    Blobs::CopyStatus CopyStatus = Blobs::CopyStatus::Unknown;
    Azure::Core::Nullable<std::string> VersionId;
  }; // struct StartCopyPageBlobIncrementalResult

  struct BlobServiceProperties
  {
    BlobAnalyticsLogging Logging;
    BlobMetrics HourMetrics;
    BlobMetrics MinuteMetrics;
    std::vector<BlobCorsRule> Cors;
    std::string DefaultServiceVersion;
    BlobRetentionPolicy DeleteRetentionPolicy;
    BlobStaticWebsite StaticWebsite;
  }; // struct BlobServiceProperties

  struct GetServicePropertiesResult
  {
    BlobAnalyticsLogging Logging;
    BlobMetrics HourMetrics;
    BlobMetrics MinuteMetrics;
    std::vector<BlobCorsRule> Cors;
    std::string DefaultServiceVersion;
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
    Blobs::BlobType BlobType = Blobs::BlobType::Unknown;
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
    Blobs::BlobType BlobType = Blobs::BlobType::Unknown;
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
    Blobs::BlobType BlobType = Blobs::BlobType::Unknown;
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
    Azure::Core::Nullable<Blobs::CopyStatus> CopyStatus;
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

  class BlobRestClient {
  public:
    class Service {
    public:
      struct ListContainersSegmentOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::string> Prefix;
        Azure::Core::Nullable<std::string> ContinuationToken;
        Azure::Core::Nullable<int32_t> MaxResults;
        ListBlobContainersIncludeItem Include = ListBlobContainersIncludeItem::None;
      }; // struct ListContainersSegmentOptions

      static Azure::Core::Response<ListContainersSegmentResult> ListBlobContainers(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const ListContainersSegmentOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.GetUrl().AppendQueryParameter("comp", "list");
        if (options.Prefix.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "prefix", Details::UrlEncodeQueryParameter(options.Prefix.GetValue()));
        }
        if (options.ContinuationToken.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "marker", Details::UrlEncodeQueryParameter(options.ContinuationToken.GetValue()));
        }
        if (options.MaxResults.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "maxresults", std::to_string(options.MaxResults.GetValue()));
        }
        std::string list_blob_containers_include_item
            = ListBlobContainersIncludeItemToString(options.Include);
        if (!list_blob_containers_include_item.empty())
        {
          request.GetUrl().AppendQueryParameter(
              "include", Details::UrlEncodeQueryParameter(list_blob_containers_include_item));
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        ListContainersSegmentResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        {
          const auto& httpResponseBody = httpResponse.GetBody();
          XmlReader reader(
              reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
          response = ListContainersSegmentResultFromXml(reader);
        }
        return Azure::Core::Response<ListContainersSegmentResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct GetUserDelegationKeyOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string StartsOn;
        std::string ExpiresOn;
      }; // struct GetUserDelegationKeyOptions

      static Azure::Core::Response<GetUserDelegationKeyResult> GetUserDelegationKey(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const GetUserDelegationKeyOptions& options)
      {
        unused(options);
        std::string xml_body;
        {
          XmlWriter writer;
          GetUserDelegationKeyOptionsToXml(writer, options);
          xml_body = writer.GetDocument();
          writer.Write(XmlNode{XmlNodeType::End});
        }
        Azure::Core::Http::MemoryBodyStream xml_body_stream(
            reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
        auto request = Azure::Core::Http::Request(
            Azure::Core::Http::HttpMethod::Post, url, &xml_body_stream);
        request.AddHeader("Content-Length", std::to_string(xml_body_stream.Length()));
        request.GetUrl().AppendQueryParameter("restype", "service");
        request.GetUrl().AppendQueryParameter("comp", "userdelegationkey");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        GetUserDelegationKeyResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        {
          const auto& httpResponseBody = httpResponse.GetBody();
          XmlReader reader(
              reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
          response = GetUserDelegationKeyResultFromXml(reader);
        }
        return Azure::Core::Response<GetUserDelegationKeyResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct GetServicePropertiesOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
      }; // struct GetServicePropertiesOptions

      static Azure::Core::Response<GetServicePropertiesResult> GetProperties(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const GetServicePropertiesOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.GetUrl().AppendQueryParameter("restype", "service");
        request.GetUrl().AppendQueryParameter("comp", "properties");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        GetServicePropertiesResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        {
          const auto& httpResponseBody = httpResponse.GetBody();
          XmlReader reader(
              reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
          response = GetServicePropertiesResultFromXml(reader);
        }
        return Azure::Core::Response<GetServicePropertiesResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct SetServicePropertiesOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        BlobServiceProperties Properties;
      }; // struct SetServicePropertiesOptions

      static Azure::Core::Response<SetServicePropertiesResult> SetProperties(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const SetServicePropertiesOptions& options)
      {
        unused(options);
        std::string xml_body;
        {
          XmlWriter writer;
          SetServicePropertiesOptionsToXml(writer, options);
          xml_body = writer.GetDocument();
          writer.Write(XmlNode{XmlNodeType::End});
        }
        Azure::Core::Http::MemoryBodyStream xml_body_stream(
            reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
        auto request
            = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, &xml_body_stream);
        request.AddHeader("Content-Length", std::to_string(xml_body_stream.Length()));
        request.GetUrl().AppendQueryParameter("restype", "service");
        request.GetUrl().AppendQueryParameter("comp", "properties");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        SetServicePropertiesResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 202))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        return Azure::Core::Response<SetServicePropertiesResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct GetAccountInfoOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
      }; // struct GetAccountInfoOptions

      static Azure::Core::Response<GetAccountInfoResult> GetAccountInfo(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const GetAccountInfoOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, url);
        request.GetUrl().AppendQueryParameter("restype", "account");
        request.GetUrl().AppendQueryParameter("comp", "properties");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        GetAccountInfoResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.SkuName = SkuNameFromString(httpResponse.GetHeaders().at("x-ms-sku-name"));
        response.AccountKind
            = AccountKindFromString(httpResponse.GetHeaders().at("x-ms-account-kind"));
        return Azure::Core::Response<GetAccountInfoResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct GetServiceStatisticsOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
      }; // struct GetServiceStatisticsOptions

      static Azure::Core::Response<GetServiceStatisticsResult> GetStatistics(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const GetServiceStatisticsOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.GetUrl().AppendQueryParameter("restype", "service");
        request.GetUrl().AppendQueryParameter("comp", "stats");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        GetServiceStatisticsResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        {
          const auto& httpResponseBody = httpResponse.GetBody();
          XmlReader reader(
              reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
          response = GetServiceStatisticsResultFromXml(reader);
        }
        return Azure::Core::Response<GetServiceStatisticsResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct FilterBlobsSegmentOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string Where;
        Azure::Core::Nullable<std::string> ContinuationToken;
        Azure::Core::Nullable<int32_t> MaxResults;
      }; // struct FilterBlobsSegmentOptions

      static Azure::Core::Response<FilterBlobsSegmentResult> FilterBlobs(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const FilterBlobsSegmentOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.GetUrl().AppendQueryParameter("comp", "blobs");
        request.GetUrl().AppendQueryParameter(
            "where", Details::UrlEncodeQueryParameter(options.Where));
        if (options.ContinuationToken.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "marker", Details::UrlEncodeQueryParameter(options.ContinuationToken.GetValue()));
        }
        if (options.MaxResults.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "maxresults", std::to_string(options.MaxResults.GetValue()));
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        FilterBlobsSegmentResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        {
          const auto& httpResponseBody = httpResponse.GetBody();
          XmlReader reader(
              reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
          response = FilterBlobsSegmentResultFromXml(reader);
        }
        return Azure::Core::Response<FilterBlobsSegmentResult>(
            std::move(response), std::move(pHttpResponse));
      }

    private:
      static FilterBlobsSegmentResult FilterBlobsSegmentResultFromXml(XmlReader& reader)
      {
        FilterBlobsSegmentResult ret;
        enum class XmlTagName
        {
          k_EnumerationResults,
          k_Where,
          k_NextMarker,
          k_Blobs,
          k_Blob,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "EnumerationResults") == 0)
            {
              path.emplace_back(XmlTagName::k_EnumerationResults);
            }
            else if (std::strcmp(node.Name, "Where") == 0)
            {
              path.emplace_back(XmlTagName::k_Where);
            }
            else if (std::strcmp(node.Name, "NextMarker") == 0)
            {
              path.emplace_back(XmlTagName::k_NextMarker);
            }
            else if (std::strcmp(node.Name, "Blobs") == 0)
            {
              path.emplace_back(XmlTagName::k_Blobs);
            }
            else if (std::strcmp(node.Name, "Blob") == 0)
            {
              path.emplace_back(XmlTagName::k_Blob);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
            if (path.size() == 3 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_Blobs && path[2] == XmlTagName::k_Blob)
            {
              ret.Items.emplace_back(FilterBlobItemFromXml(reader));
              path.pop_back();
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
            if (path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_Where)
            {
              ret.Where = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_NextMarker)
            {
              ret.ContinuationToken = node.Value;
            }
          }
          else if (node.Type == XmlNodeType::Attribute)
          {
            if (path.size() == 1 && path[0] == XmlTagName::k_EnumerationResults
                && std::strcmp(node.Name, "ServiceEndpoint") == 0)
            {
              ret.ServiceEndpoint = node.Value;
            }
          }
        }
        return ret;
      }

      static GetServicePropertiesResult GetServicePropertiesResultFromXml(XmlReader& reader)
      {
        GetServicePropertiesResult ret;
        enum class XmlTagName
        {
          k_StorageServiceProperties,
          k_Logging,
          k_HourMetrics,
          k_MinuteMetrics,
          k_Cors,
          k_CorsRule,
          k_DefaultServiceVersion,
          k_DeleteRetentionPolicy,
          k_StaticWebsite,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "StorageServiceProperties") == 0)
            {
              path.emplace_back(XmlTagName::k_StorageServiceProperties);
            }
            else if (std::strcmp(node.Name, "Logging") == 0)
            {
              path.emplace_back(XmlTagName::k_Logging);
            }
            else if (std::strcmp(node.Name, "HourMetrics") == 0)
            {
              path.emplace_back(XmlTagName::k_HourMetrics);
            }
            else if (std::strcmp(node.Name, "MinuteMetrics") == 0)
            {
              path.emplace_back(XmlTagName::k_MinuteMetrics);
            }
            else if (std::strcmp(node.Name, "Cors") == 0)
            {
              path.emplace_back(XmlTagName::k_Cors);
            }
            else if (std::strcmp(node.Name, "CorsRule") == 0)
            {
              path.emplace_back(XmlTagName::k_CorsRule);
            }
            else if (std::strcmp(node.Name, "DefaultServiceVersion") == 0)
            {
              path.emplace_back(XmlTagName::k_DefaultServiceVersion);
            }
            else if (std::strcmp(node.Name, "DeleteRetentionPolicy") == 0)
            {
              path.emplace_back(XmlTagName::k_DeleteRetentionPolicy);
            }
            else if (std::strcmp(node.Name, "StaticWebsite") == 0)
            {
              path.emplace_back(XmlTagName::k_StaticWebsite);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
            if (path.size() == 2 && path[0] == XmlTagName::k_StorageServiceProperties
                && path[1] == XmlTagName::k_Logging)
            {
              ret.Logging = BlobAnalyticsLoggingFromXml(reader);
              path.pop_back();
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_StorageServiceProperties
                && path[1] == XmlTagName::k_HourMetrics)
            {
              ret.HourMetrics = BlobMetricsFromXml(reader);
              path.pop_back();
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_StorageServiceProperties
                && path[1] == XmlTagName::k_MinuteMetrics)
            {
              ret.MinuteMetrics = BlobMetricsFromXml(reader);
              path.pop_back();
            }
            else if (
                path.size() == 3 && path[0] == XmlTagName::k_StorageServiceProperties
                && path[1] == XmlTagName::k_Cors && path[2] == XmlTagName::k_CorsRule)
            {
              ret.Cors.emplace_back(BlobCorsRuleFromXml(reader));
              path.pop_back();
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_StorageServiceProperties
                && path[1] == XmlTagName::k_DeleteRetentionPolicy)
            {
              ret.DeleteRetentionPolicy = BlobRetentionPolicyFromXml(reader);
              path.pop_back();
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_StorageServiceProperties
                && path[1] == XmlTagName::k_StaticWebsite)
            {
              ret.StaticWebsite = BlobStaticWebsiteFromXml(reader);
              path.pop_back();
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
            if (path.size() == 2 && path[0] == XmlTagName::k_StorageServiceProperties
                && path[1] == XmlTagName::k_DefaultServiceVersion)
            {
              ret.DefaultServiceVersion = node.Value;
            }
          }
        }
        return ret;
      }

      static GetServiceStatisticsResult GetServiceStatisticsResultFromXml(XmlReader& reader)
      {
        GetServiceStatisticsResult ret;
        enum class XmlTagName
        {
          k_StorageServiceStats,
          k_GeoReplication,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "StorageServiceStats") == 0)
            {
              path.emplace_back(XmlTagName::k_StorageServiceStats);
            }
            else if (std::strcmp(node.Name, "GeoReplication") == 0)
            {
              path.emplace_back(XmlTagName::k_GeoReplication);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
            if (path.size() == 2 && path[0] == XmlTagName::k_StorageServiceStats
                && path[1] == XmlTagName::k_GeoReplication)
            {
              ret.GeoReplication = BlobGeoReplicationFromXml(reader);
              path.pop_back();
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
          }
        }
        return ret;
      }

      static GetUserDelegationKeyResult GetUserDelegationKeyResultFromXml(XmlReader& reader)
      {
        GetUserDelegationKeyResult ret;
        enum class XmlTagName
        {
          k_UserDelegationKey,
          k_SignedOid,
          k_SignedTid,
          k_SignedStart,
          k_SignedExpiry,
          k_SignedService,
          k_SignedVersion,
          k_Value,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "UserDelegationKey") == 0)
            {
              path.emplace_back(XmlTagName::k_UserDelegationKey);
            }
            else if (std::strcmp(node.Name, "SignedOid") == 0)
            {
              path.emplace_back(XmlTagName::k_SignedOid);
            }
            else if (std::strcmp(node.Name, "SignedTid") == 0)
            {
              path.emplace_back(XmlTagName::k_SignedTid);
            }
            else if (std::strcmp(node.Name, "SignedStart") == 0)
            {
              path.emplace_back(XmlTagName::k_SignedStart);
            }
            else if (std::strcmp(node.Name, "SignedExpiry") == 0)
            {
              path.emplace_back(XmlTagName::k_SignedExpiry);
            }
            else if (std::strcmp(node.Name, "SignedService") == 0)
            {
              path.emplace_back(XmlTagName::k_SignedService);
            }
            else if (std::strcmp(node.Name, "SignedVersion") == 0)
            {
              path.emplace_back(XmlTagName::k_SignedVersion);
            }
            else if (std::strcmp(node.Name, "Value") == 0)
            {
              path.emplace_back(XmlTagName::k_Value);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
            if (path.size() == 2 && path[0] == XmlTagName::k_UserDelegationKey
                && path[1] == XmlTagName::k_SignedOid)
            {
              ret.SignedObjectId = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_UserDelegationKey
                && path[1] == XmlTagName::k_SignedTid)
            {
              ret.SignedTenantId = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_UserDelegationKey
                && path[1] == XmlTagName::k_SignedStart)
            {
              ret.SignedStartsOn = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_UserDelegationKey
                && path[1] == XmlTagName::k_SignedExpiry)
            {
              ret.SignedExpiresOn = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_UserDelegationKey
                && path[1] == XmlTagName::k_SignedService)
            {
              ret.SignedService = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_UserDelegationKey
                && path[1] == XmlTagName::k_SignedVersion)
            {
              ret.SignedVersion = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_UserDelegationKey
                && path[1] == XmlTagName::k_Value)
            {
              ret.Value = node.Value;
            }
          }
        }
        return ret;
      }

      static ListContainersSegmentResult ListContainersSegmentResultFromXml(XmlReader& reader)
      {
        ListContainersSegmentResult ret;
        enum class XmlTagName
        {
          k_EnumerationResults,
          k_Prefix,
          k_Marker,
          k_NextMarker,
          k_Containers,
          k_Container,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "EnumerationResults") == 0)
            {
              path.emplace_back(XmlTagName::k_EnumerationResults);
            }
            else if (std::strcmp(node.Name, "Prefix") == 0)
            {
              path.emplace_back(XmlTagName::k_Prefix);
            }
            else if (std::strcmp(node.Name, "Marker") == 0)
            {
              path.emplace_back(XmlTagName::k_Marker);
            }
            else if (std::strcmp(node.Name, "NextMarker") == 0)
            {
              path.emplace_back(XmlTagName::k_NextMarker);
            }
            else if (std::strcmp(node.Name, "Containers") == 0)
            {
              path.emplace_back(XmlTagName::k_Containers);
            }
            else if (std::strcmp(node.Name, "Container") == 0)
            {
              path.emplace_back(XmlTagName::k_Container);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
            if (path.size() == 3 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_Containers && path[2] == XmlTagName::k_Container)
            {
              ret.Items.emplace_back(BlobContainerItemFromXml(reader));
              path.pop_back();
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
            if (path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_Prefix)
            {
              ret.Prefix = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_Marker)
            {
              ret.PreviousContinuationToken = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_NextMarker)
            {
              ret.ContinuationToken = node.Value;
            }
          }
          else if (node.Type == XmlNodeType::Attribute)
          {
            if (path.size() == 1 && path[0] == XmlTagName::k_EnumerationResults
                && std::strcmp(node.Name, "ServiceEndpoint") == 0)
            {
              ret.ServiceEndpoint = node.Value;
            }
          }
        }
        return ret;
      }

      static BlobAnalyticsLogging BlobAnalyticsLoggingFromXml(XmlReader& reader)
      {
        BlobAnalyticsLogging ret;
        enum class XmlTagName
        {
          k_Version,
          k_Delete,
          k_Read,
          k_Write,
          k_RetentionPolicy,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "Version") == 0)
            {
              path.emplace_back(XmlTagName::k_Version);
            }
            else if (std::strcmp(node.Name, "Delete") == 0)
            {
              path.emplace_back(XmlTagName::k_Delete);
            }
            else if (std::strcmp(node.Name, "Read") == 0)
            {
              path.emplace_back(XmlTagName::k_Read);
            }
            else if (std::strcmp(node.Name, "Write") == 0)
            {
              path.emplace_back(XmlTagName::k_Write);
            }
            else if (std::strcmp(node.Name, "RetentionPolicy") == 0)
            {
              path.emplace_back(XmlTagName::k_RetentionPolicy);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
            if (path.size() == 1 && path[0] == XmlTagName::k_RetentionPolicy)
            {
              ret.RetentionPolicy = BlobRetentionPolicyFromXml(reader);
              path.pop_back();
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
            if (path.size() == 1 && path[0] == XmlTagName::k_Version)
            {
              ret.Version = node.Value;
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_Delete)
            {
              ret.Delete = std::strcmp(node.Value, "true") == 0;
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_Read)
            {
              ret.Read = std::strcmp(node.Value, "true") == 0;
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_Write)
            {
              ret.Write = std::strcmp(node.Value, "true") == 0;
            }
          }
        }
        return ret;
      }

      static BlobContainerItem BlobContainerItemFromXml(XmlReader& reader)
      {
        BlobContainerItem ret;
        enum class XmlTagName
        {
          k_Name,
          k_Properties,
          k_Etag,
          k_LastModified,
          k_PublicAccess,
          k_HasImmutabilityPolicy,
          k_HasLegalHold,
          k_LeaseStatus,
          k_LeaseState,
          k_LeaseDuration,
          k_DefaultEncryptionScope,
          k_DenyEncryptionScopeOverride,
          k_Metadata,
          k_Deleted,
          k_Version,
          k_DeletedTime,
          k_RemainingRetentionDays,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "Name") == 0)
            {
              path.emplace_back(XmlTagName::k_Name);
            }
            else if (std::strcmp(node.Name, "Properties") == 0)
            {
              path.emplace_back(XmlTagName::k_Properties);
            }
            else if (std::strcmp(node.Name, "Etag") == 0)
            {
              path.emplace_back(XmlTagName::k_Etag);
            }
            else if (std::strcmp(node.Name, "Last-Modified") == 0)
            {
              path.emplace_back(XmlTagName::k_LastModified);
            }
            else if (std::strcmp(node.Name, "PublicAccess") == 0)
            {
              path.emplace_back(XmlTagName::k_PublicAccess);
            }
            else if (std::strcmp(node.Name, "HasImmutabilityPolicy") == 0)
            {
              path.emplace_back(XmlTagName::k_HasImmutabilityPolicy);
            }
            else if (std::strcmp(node.Name, "HasLegalHold") == 0)
            {
              path.emplace_back(XmlTagName::k_HasLegalHold);
            }
            else if (std::strcmp(node.Name, "LeaseStatus") == 0)
            {
              path.emplace_back(XmlTagName::k_LeaseStatus);
            }
            else if (std::strcmp(node.Name, "LeaseState") == 0)
            {
              path.emplace_back(XmlTagName::k_LeaseState);
            }
            else if (std::strcmp(node.Name, "LeaseDuration") == 0)
            {
              path.emplace_back(XmlTagName::k_LeaseDuration);
            }
            else if (std::strcmp(node.Name, "DefaultEncryptionScope") == 0)
            {
              path.emplace_back(XmlTagName::k_DefaultEncryptionScope);
            }
            else if (std::strcmp(node.Name, "DenyEncryptionScopeOverride") == 0)
            {
              path.emplace_back(XmlTagName::k_DenyEncryptionScopeOverride);
            }
            else if (std::strcmp(node.Name, "Metadata") == 0)
            {
              path.emplace_back(XmlTagName::k_Metadata);
            }
            else if (std::strcmp(node.Name, "Deleted") == 0)
            {
              path.emplace_back(XmlTagName::k_Deleted);
            }
            else if (std::strcmp(node.Name, "Version") == 0)
            {
              path.emplace_back(XmlTagName::k_Version);
            }
            else if (std::strcmp(node.Name, "DeletedTime") == 0)
            {
              path.emplace_back(XmlTagName::k_DeletedTime);
            }
            else if (std::strcmp(node.Name, "RemainingRetentionDays") == 0)
            {
              path.emplace_back(XmlTagName::k_RemainingRetentionDays);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
            if (path.size() == 1 && path[0] == XmlTagName::k_Metadata)
            {
              ret.Metadata = MetadataFromXml(reader);
              path.pop_back();
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
            if (path.size() == 1 && path[0] == XmlTagName::k_Name)
            {
              ret.Name = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_Etag)
            {
              ret.ETag = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_LastModified)
            {
              ret.LastModified = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_PublicAccess)
            {
              ret.AccessType = PublicAccessTypeFromString(node.Value);
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_HasImmutabilityPolicy)
            {
              ret.HasImmutabilityPolicy = std::strcmp(node.Value, "true") == 0;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_HasLegalHold)
            {
              ret.HasLegalHold = std::strcmp(node.Value, "true") == 0;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_LeaseStatus)
            {
              ret.LeaseStatus = BlobLeaseStatusFromString(node.Value);
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_LeaseState)
            {
              ret.LeaseState = BlobLeaseStateFromString(node.Value);
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_LeaseDuration)
            {
              ret.LeaseDuration = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_DefaultEncryptionScope)
            {
              ret.DefaultEncryptionScope = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_DenyEncryptionScopeOverride)
            {
              ret.PreventEncryptionScopeOverride = std::strcmp(node.Value, "true") == 0;
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_Deleted)
            {
              ret.IsDeleted = std::strcmp(node.Value, "true") == 0;
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_Version)
            {
              ret.VersionId = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_DeletedTime)
            {
              ret.DeletedTime = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_RemainingRetentionDays)
            {
              ret.RemainingRetentionDays = std::stoi(node.Value);
            }
          }
        }
        return ret;
      }

      static BlobCorsRule BlobCorsRuleFromXml(XmlReader& reader)
      {
        BlobCorsRule ret;
        enum class XmlTagName
        {
          k_AllowedOrigins,
          k_AllowedMethods,
          k_MaxAgeInSeconds,
          k_ExposedHeaders,
          k_AllowedHeaders,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "AllowedOrigins") == 0)
            {
              path.emplace_back(XmlTagName::k_AllowedOrigins);
            }
            else if (std::strcmp(node.Name, "AllowedMethods") == 0)
            {
              path.emplace_back(XmlTagName::k_AllowedMethods);
            }
            else if (std::strcmp(node.Name, "MaxAgeInSeconds") == 0)
            {
              path.emplace_back(XmlTagName::k_MaxAgeInSeconds);
            }
            else if (std::strcmp(node.Name, "ExposedHeaders") == 0)
            {
              path.emplace_back(XmlTagName::k_ExposedHeaders);
            }
            else if (std::strcmp(node.Name, "AllowedHeaders") == 0)
            {
              path.emplace_back(XmlTagName::k_AllowedHeaders);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
            if (path.size() == 1 && path[0] == XmlTagName::k_AllowedOrigins)
            {
              ret.AllowedOrigins = node.Value;
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_AllowedMethods)
            {
              ret.AllowedMethods = node.Value;
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_MaxAgeInSeconds)
            {
              ret.MaxAgeInSeconds = std::stoi(node.Value);
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_ExposedHeaders)
            {
              ret.ExposedHeaders = node.Value;
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_AllowedHeaders)
            {
              ret.AllowedHeaders = node.Value;
            }
          }
        }
        return ret;
      }

      static BlobGeoReplication BlobGeoReplicationFromXml(XmlReader& reader)
      {
        BlobGeoReplication ret;
        enum class XmlTagName
        {
          k_Status,
          k_LastSyncTime,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "Status") == 0)
            {
              path.emplace_back(XmlTagName::k_Status);
            }
            else if (std::strcmp(node.Name, "LastSyncTime") == 0)
            {
              path.emplace_back(XmlTagName::k_LastSyncTime);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
            if (path.size() == 1 && path[0] == XmlTagName::k_Status)
            {
              ret.Status = BlobGeoReplicationStatusFromString(node.Value);
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_LastSyncTime)
            {
              ret.LastSyncTime = node.Value;
            }
          }
        }
        return ret;
      }

      static BlobMetrics BlobMetricsFromXml(XmlReader& reader)
      {
        BlobMetrics ret;
        enum class XmlTagName
        {
          k_Version,
          k_Enabled,
          k_IncludeAPIs,
          k_RetentionPolicy,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "Version") == 0)
            {
              path.emplace_back(XmlTagName::k_Version);
            }
            else if (std::strcmp(node.Name, "Enabled") == 0)
            {
              path.emplace_back(XmlTagName::k_Enabled);
            }
            else if (std::strcmp(node.Name, "IncludeAPIs") == 0)
            {
              path.emplace_back(XmlTagName::k_IncludeAPIs);
            }
            else if (std::strcmp(node.Name, "RetentionPolicy") == 0)
            {
              path.emplace_back(XmlTagName::k_RetentionPolicy);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
            if (path.size() == 1 && path[0] == XmlTagName::k_RetentionPolicy)
            {
              ret.RetentionPolicy = BlobRetentionPolicyFromXml(reader);
              path.pop_back();
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
            if (path.size() == 1 && path[0] == XmlTagName::k_Version)
            {
              ret.Version = node.Value;
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_Enabled)
            {
              ret.Enabled = std::strcmp(node.Value, "true") == 0;
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_IncludeAPIs)
            {
              ret.IncludeApis = std::strcmp(node.Value, "true") == 0;
            }
          }
        }
        return ret;
      }

      static BlobRetentionPolicy BlobRetentionPolicyFromXml(XmlReader& reader)
      {
        BlobRetentionPolicy ret;
        enum class XmlTagName
        {
          k_Enabled,
          k_Days,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "Enabled") == 0)
            {
              path.emplace_back(XmlTagName::k_Enabled);
            }
            else if (std::strcmp(node.Name, "Days") == 0)
            {
              path.emplace_back(XmlTagName::k_Days);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
            if (path.size() == 1 && path[0] == XmlTagName::k_Enabled)
            {
              ret.Enabled = std::strcmp(node.Value, "true") == 0;
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_Days)
            {
              ret.Days = std::stoi(node.Value);
            }
          }
        }
        return ret;
      }

      static BlobStaticWebsite BlobStaticWebsiteFromXml(XmlReader& reader)
      {
        BlobStaticWebsite ret;
        enum class XmlTagName
        {
          k_Enabled,
          k_IndexDocument,
          k_DefaultIndexDocumentPath,
          k_ErrorDocument404Path,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "Enabled") == 0)
            {
              path.emplace_back(XmlTagName::k_Enabled);
            }
            else if (std::strcmp(node.Name, "IndexDocument") == 0)
            {
              path.emplace_back(XmlTagName::k_IndexDocument);
            }
            else if (std::strcmp(node.Name, "DefaultIndexDocumentPath") == 0)
            {
              path.emplace_back(XmlTagName::k_DefaultIndexDocumentPath);
            }
            else if (std::strcmp(node.Name, "ErrorDocument404Path") == 0)
            {
              path.emplace_back(XmlTagName::k_ErrorDocument404Path);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
            if (path.size() == 1 && path[0] == XmlTagName::k_Enabled)
            {
              ret.Enabled = std::strcmp(node.Value, "true") == 0;
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_IndexDocument)
            {
              ret.IndexDocument = node.Value;
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_DefaultIndexDocumentPath)
            {
              ret.DefaultIndexDocumentPath = node.Value;
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_ErrorDocument404Path)
            {
              ret.ErrorDocument404Path = node.Value;
            }
          }
        }
        return ret;
      }

      static FilterBlobItem FilterBlobItemFromXml(XmlReader& reader)
      {
        FilterBlobItem ret;
        enum class XmlTagName
        {
          k_Name,
          k_ContainerName,
          k_TagValue,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "Name") == 0)
            {
              path.emplace_back(XmlTagName::k_Name);
            }
            else if (std::strcmp(node.Name, "ContainerName") == 0)
            {
              path.emplace_back(XmlTagName::k_ContainerName);
            }
            else if (std::strcmp(node.Name, "TagValue") == 0)
            {
              path.emplace_back(XmlTagName::k_TagValue);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
            if (path.size() == 1 && path[0] == XmlTagName::k_Name)
            {
              ret.BlobName = node.Value;
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_ContainerName)
            {
              ret.ContainerName = node.Value;
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_TagValue)
            {
              ret.TagValue = node.Value;
            }
          }
        }
        return ret;
      }

      static std::map<std::string, std::string> MetadataFromXml(XmlReader& reader)
      {
        std::map<std::string, std::string> ret;
        int depth = 0;
        std::string key;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (depth++ == 0)
            {
              key = node.Name;
            }
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (depth-- == 0)
            {
              break;
            }
          }
          else if (depth == 1 && node.Type == XmlNodeType::Text)
          {
            ret.emplace(std::move(key), std::string(node.Value));
          }
        }
        return ret;
      }

      static void GetUserDelegationKeyOptionsToXml(
          XmlWriter& writer,
          const GetUserDelegationKeyOptions& options)
      {
        writer.Write(XmlNode{XmlNodeType::StartTag, "KeyInfo"});
        writer.Write(XmlNode{XmlNodeType::StartTag, "Start"});
        writer.Write(XmlNode{XmlNodeType::Text, nullptr, options.StartsOn.data()});
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::StartTag, "Expiry"});
        writer.Write(XmlNode{XmlNodeType::Text, nullptr, options.ExpiresOn.data()});
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::EndTag});
      }

      static void SetServicePropertiesOptionsToXml(
          XmlWriter& writer,
          const SetServicePropertiesOptions& options)
      {
        writer.Write(XmlNode{XmlNodeType::StartTag, "StorageServiceProperties"});
        BlobServicePropertiesToXml(writer, options.Properties);
        writer.Write(XmlNode{XmlNodeType::EndTag});
      }

      static void BlobServicePropertiesToXml(
          XmlWriter& writer,
          const BlobServiceProperties& options)
      {
        writer.Write(XmlNode{XmlNodeType::StartTag, "Logging"});
        BlobAnalyticsLoggingToXml(writer, options.Logging);
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::StartTag, "HourMetrics"});
        BlobMetricsToXml(writer, options.HourMetrics);
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::StartTag, "MinuteMetrics"});
        BlobMetricsToXml(writer, options.MinuteMetrics);
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::StartTag, "Cors"});
        for (const auto& i : options.Cors)
        {
          BlobCorsRuleToXml(writer, i);
        }
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::StartTag, "DefaultServiceVersion"});
        writer.Write(XmlNode{XmlNodeType::Text, nullptr, options.DefaultServiceVersion.data()});
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::StartTag, "DeleteRetentionPolicy"});
        BlobRetentionPolicyToXml(writer, options.DeleteRetentionPolicy);
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::StartTag, "StaticWebsite"});
        BlobStaticWebsiteToXml(writer, options.StaticWebsite);
        writer.Write(XmlNode{XmlNodeType::EndTag});
      }

      static void BlobAnalyticsLoggingToXml(XmlWriter& writer, const BlobAnalyticsLogging& options)
      {
        writer.Write(XmlNode{XmlNodeType::StartTag, "Version"});
        writer.Write(XmlNode{XmlNodeType::Text, nullptr, options.Version.data()});
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::StartTag, "Delete"});
        writer.Write(XmlNode{XmlNodeType::Text, nullptr, options.Delete ? "true" : "false"});
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::StartTag, "Read"});
        writer.Write(XmlNode{XmlNodeType::Text, nullptr, options.Read ? "true" : "false"});
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::StartTag, "Write"});
        writer.Write(XmlNode{XmlNodeType::Text, nullptr, options.Write ? "true" : "false"});
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::StartTag, "RetentionPolicy"});
        BlobRetentionPolicyToXml(writer, options.RetentionPolicy);
        writer.Write(XmlNode{XmlNodeType::EndTag});
      }

      static void BlobCorsRuleToXml(XmlWriter& writer, const BlobCorsRule& options)
      {
        writer.Write(XmlNode{XmlNodeType::StartTag, "CorsRule"});
        writer.Write(XmlNode{XmlNodeType::StartTag, "AllowedOrigins"});
        writer.Write(XmlNode{XmlNodeType::Text, nullptr, options.AllowedOrigins.data()});
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::StartTag, "AllowedMethods"});
        writer.Write(XmlNode{XmlNodeType::Text, nullptr, options.AllowedMethods.data()});
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::StartTag, "AllowedHeaders"});
        writer.Write(XmlNode{XmlNodeType::Text, nullptr, options.AllowedHeaders.data()});
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::StartTag, "ExposedHeaders"});
        writer.Write(XmlNode{XmlNodeType::Text, nullptr, options.ExposedHeaders.data()});
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::StartTag, "MaxAgeInSeconds"});
        writer.Write(
            XmlNode{XmlNodeType::Text, nullptr, std::to_string(options.MaxAgeInSeconds).data()});
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::EndTag});
      }

      static void BlobMetricsToXml(XmlWriter& writer, const BlobMetrics& options)
      {
        writer.Write(XmlNode{XmlNodeType::StartTag, "Version"});
        writer.Write(XmlNode{XmlNodeType::Text, nullptr, options.Version.data()});
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::StartTag, "Enabled"});
        writer.Write(XmlNode{XmlNodeType::Text, nullptr, options.Enabled ? "true" : "false"});
        writer.Write(XmlNode{XmlNodeType::EndTag});
        if (options.IncludeApis.HasValue())
        {
          writer.Write(XmlNode{XmlNodeType::StartTag, "IncludeAPIs"});
          writer.Write(XmlNode{
              XmlNodeType::Text, nullptr, options.IncludeApis.GetValue() ? "true" : "false"});
          writer.Write(XmlNode{XmlNodeType::EndTag});
        }
        writer.Write(XmlNode{XmlNodeType::StartTag, "RetentionPolicy"});
        BlobRetentionPolicyToXml(writer, options.RetentionPolicy);
        writer.Write(XmlNode{XmlNodeType::EndTag});
      }

      static void BlobRetentionPolicyToXml(XmlWriter& writer, const BlobRetentionPolicy& options)
      {
        writer.Write(XmlNode{XmlNodeType::StartTag, "Enabled"});
        writer.Write(XmlNode{XmlNodeType::Text, nullptr, options.Enabled ? "true" : "false"});
        writer.Write(XmlNode{XmlNodeType::EndTag});
        if (options.Days.HasValue())
        {
          writer.Write(XmlNode{XmlNodeType::StartTag, "Days"});
          writer.Write(
              XmlNode{XmlNodeType::Text, nullptr, std::to_string(options.Days.GetValue()).data()});
          writer.Write(XmlNode{XmlNodeType::EndTag});
        }
      }

      static void BlobStaticWebsiteToXml(XmlWriter& writer, const BlobStaticWebsite& options)
      {
        writer.Write(XmlNode{XmlNodeType::StartTag, "Enabled"});
        writer.Write(XmlNode{XmlNodeType::Text, nullptr, options.Enabled ? "true" : "false"});
        writer.Write(XmlNode{XmlNodeType::EndTag});
        if (options.IndexDocument.HasValue())
        {
          writer.Write(XmlNode{XmlNodeType::StartTag, "IndexDocument"});
          writer.Write(
              XmlNode{XmlNodeType::Text, nullptr, options.IndexDocument.GetValue().data()});
          writer.Write(XmlNode{XmlNodeType::EndTag});
        }
        if (options.DefaultIndexDocumentPath.HasValue())
        {
          writer.Write(XmlNode{XmlNodeType::StartTag, "DefaultIndexDocumentPath"});
          writer.Write(XmlNode{
              XmlNodeType::Text, nullptr, options.DefaultIndexDocumentPath.GetValue().data()});
          writer.Write(XmlNode{XmlNodeType::EndTag});
        }
        if (options.ErrorDocument404Path.HasValue())
        {
          writer.Write(XmlNode{XmlNodeType::StartTag, "ErrorDocument404Path"});
          writer.Write(
              XmlNode{XmlNodeType::Text, nullptr, options.ErrorDocument404Path.GetValue().data()});
          writer.Write(XmlNode{XmlNodeType::EndTag});
        }
      }

    }; // class Service

    class Container {
    public:
      struct CreateContainerOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<PublicAccessType> AccessType;
        std::map<std::string, std::string> Metadata;
        Azure::Core::Nullable<std::string> DefaultEncryptionScope;
        Azure::Core::Nullable<bool> PreventEncryptionScopeOverride;
      }; // struct CreateContainerOptions

      static Azure::Core::Response<CreateContainerResult> Create(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const CreateContainerOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.GetUrl().AppendQueryParameter("restype", "container");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        std::set<std::string> metadataKeys;
        for (const auto& pair : options.Metadata)
        {
          if (metadataKeys.insert(Azure::Core::Strings::ToLower(pair.first)).second == false)
          {
            throw std::runtime_error("duplicate keys in metadata");
          }
          request.AddHeader("x-ms-meta-" + pair.first, pair.second);
        }
        metadataKeys.clear();
        if (options.AccessType.HasValue())
        {
          request.AddHeader(
              "x-ms-blob-public-access", PublicAccessTypeToString(options.AccessType.GetValue()));
        }
        if (options.DefaultEncryptionScope.HasValue())
        {
          request.AddHeader(
              "x-ms-default-encryption-scope", options.DefaultEncryptionScope.GetValue());
        }
        if (options.PreventEncryptionScopeOverride.HasValue())
        {
          request.AddHeader(
              "x-ms-deny-encryption-scope-override",
              options.PreventEncryptionScopeOverride.GetValue() ? "true" : "false");
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        CreateContainerResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        return Azure::Core::Response<CreateContainerResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct DeleteContainerOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
      }; // struct DeleteContainerOptions

      static Azure::Core::Response<DeleteContainerResult> Delete(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const DeleteContainerOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Delete, url);
        request.GetUrl().AppendQueryParameter("restype", "container");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        DeleteContainerResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 202))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        return Azure::Core::Response<DeleteContainerResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct UndeleteContainerOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string DeletedContainerName;
        std::string DeletedContainerVersion;
      }; // struct UndeleteContainerOptions

      static Azure::Core::Response<UndeleteContainerResult> Undelete(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const UndeleteContainerOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.GetUrl().AppendQueryParameter("restype", "container");
        request.GetUrl().AppendQueryParameter("comp", "undelete");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddHeader("x-ms-deleted-container-name", options.DeletedContainerName);
        request.AddHeader("x-ms-deleted-container-version", options.DeletedContainerVersion);
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        UndeleteContainerResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        return Azure::Core::Response<UndeleteContainerResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct GetContainerPropertiesOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::string> LeaseId;
      }; // struct GetContainerPropertiesOptions

      static Azure::Core::Response<GetContainerPropertiesResult> GetProperties(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const GetContainerPropertiesOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, url);
        request.GetUrl().AppendQueryParameter("restype", "container");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        GetContainerPropertiesResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        for (auto i = httpResponse.GetHeaders().lower_bound("x-ms-meta-");
             i != httpResponse.GetHeaders().end() && i->first.substr(0, 10) == "x-ms-meta-";
             ++i)
        {
          response.Metadata.emplace(i->first.substr(10), i->second);
        }
        auto response_access_type_iterator
            = httpResponse.GetHeaders().find("x-ms-blob-public-access");
        if (response_access_type_iterator != httpResponse.GetHeaders().end())
        {
          response.AccessType = PublicAccessTypeFromString(response_access_type_iterator->second);
        }
        response.HasImmutabilityPolicy
            = httpResponse.GetHeaders().at("x-ms-has-immutability-policy") == "true";
        response.HasLegalHold = httpResponse.GetHeaders().at("x-ms-has-legal-hold") == "true";
        response.LeaseStatus
            = BlobLeaseStatusFromString(httpResponse.GetHeaders().at("x-ms-lease-status"));
        response.LeaseState
            = BlobLeaseStateFromString(httpResponse.GetHeaders().at("x-ms-lease-state"));
        auto response_lease_duration_iterator
            = httpResponse.GetHeaders().find("x-ms-lease-duration");
        if (response_lease_duration_iterator != httpResponse.GetHeaders().end())
        {
          response.LeaseDuration = response_lease_duration_iterator->second;
        }
        response.DefaultEncryptionScope
            = httpResponse.GetHeaders().at("x-ms-default-encryption-scope");
        response.PreventEncryptionScopeOverride
            = httpResponse.GetHeaders().at("x-ms-deny-encryption-scope-override") == "true";
        return Azure::Core::Response<GetContainerPropertiesResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct SetContainerMetadataOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::map<std::string, std::string> Metadata;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
      }; // struct SetContainerMetadataOptions

      static Azure::Core::Response<SetContainerMetadataResult> SetMetadata(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const SetContainerMetadataOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.GetUrl().AppendQueryParameter("restype", "container");
        request.GetUrl().AppendQueryParameter("comp", "metadata");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        std::set<std::string> metadataKeys;
        for (const auto& pair : options.Metadata)
        {
          if (metadataKeys.insert(Azure::Core::Strings::ToLower(pair.first)).second == false)
          {
            throw std::runtime_error("duplicate keys in metadata");
          }
          request.AddHeader("x-ms-meta-" + pair.first, pair.second);
        }
        metadataKeys.clear();
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        SetContainerMetadataResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        return Azure::Core::Response<SetContainerMetadataResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct ListBlobsFlatSegmentOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::string> Prefix;
        Azure::Core::Nullable<std::string> ContinuationToken;
        Azure::Core::Nullable<int32_t> MaxResults;
        ListBlobsIncludeItem Include = ListBlobsIncludeItem::None;
      }; // struct ListBlobsFlatSegmentOptions

      static Azure::Core::Response<ListBlobsFlatSegmentResult> ListBlobsFlat(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const ListBlobsFlatSegmentOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.GetUrl().AppendQueryParameter("restype", "container");
        request.GetUrl().AppendQueryParameter("comp", "list");
        if (options.Prefix.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "prefix", Details::UrlEncodeQueryParameter(options.Prefix.GetValue()));
        }
        if (options.ContinuationToken.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "marker", Details::UrlEncodeQueryParameter(options.ContinuationToken.GetValue()));
        }
        if (options.MaxResults.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "maxresults", std::to_string(options.MaxResults.GetValue()));
        }
        std::string list_blobs_include_item = ListBlobsIncludeItemToString(options.Include);
        if (!list_blobs_include_item.empty())
        {
          request.GetUrl().AppendQueryParameter(
              "include", Details::UrlEncodeQueryParameter(list_blobs_include_item));
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        ListBlobsFlatSegmentResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        {
          const auto& httpResponseBody = httpResponse.GetBody();
          XmlReader reader(
              reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
          response = ListBlobsFlatSegmentResultFromXml(reader);
        }
        return Azure::Core::Response<ListBlobsFlatSegmentResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct ListBlobsByHierarchySegmentOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::string> Prefix;
        Azure::Core::Nullable<std::string> Delimiter;
        Azure::Core::Nullable<std::string> ContinuationToken;
        Azure::Core::Nullable<int32_t> MaxResults;
        ListBlobsIncludeItem Include = ListBlobsIncludeItem::None;
      }; // struct ListBlobsByHierarchySegmentOptions

      static Azure::Core::Response<ListBlobsByHierarchySegmentResult> ListBlobsByHierarchy(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const ListBlobsByHierarchySegmentOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.GetUrl().AppendQueryParameter("restype", "container");
        request.GetUrl().AppendQueryParameter("comp", "list");
        if (options.Prefix.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "prefix", Details::UrlEncodeQueryParameter(options.Prefix.GetValue()));
        }
        if (options.Delimiter.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "delimiter", Details::UrlEncodeQueryParameter(options.Delimiter.GetValue()));
        }
        if (options.ContinuationToken.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "marker", Details::UrlEncodeQueryParameter(options.ContinuationToken.GetValue()));
        }
        if (options.MaxResults.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "maxresults", std::to_string(options.MaxResults.GetValue()));
        }
        std::string list_blobs_include_item = ListBlobsIncludeItemToString(options.Include);
        if (!list_blobs_include_item.empty())
        {
          request.GetUrl().AppendQueryParameter(
              "include", Details::UrlEncodeQueryParameter(list_blobs_include_item));
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        ListBlobsByHierarchySegmentResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        {
          const auto& httpResponseBody = httpResponse.GetBody();
          XmlReader reader(
              reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
          response = ListBlobsByHierarchySegmentResultFromXml(reader);
        }
        return Azure::Core::Response<ListBlobsByHierarchySegmentResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct GetContainerAccessPolicyOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::string> LeaseId;
      }; // struct GetContainerAccessPolicyOptions

      static Azure::Core::Response<GetContainerAccessPolicyResult> GetAccessPolicy(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const GetContainerAccessPolicyOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.GetUrl().AppendQueryParameter("restype", "container");
        request.GetUrl().AppendQueryParameter("comp", "acl");
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        GetContainerAccessPolicyResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        {
          const auto& httpResponseBody = httpResponse.GetBody();
          XmlReader reader(
              reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
          response = GetContainerAccessPolicyResultFromXml(reader);
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.AccessType
            = PublicAccessTypeFromString(httpResponse.GetHeaders().at("x-ms-blob-public-access"));
        return Azure::Core::Response<GetContainerAccessPolicyResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct SetContainerAccessPolicyOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<PublicAccessType> AccessType;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        std::vector<BlobSignedIdentifier> SignedIdentifiers;
      }; // struct SetContainerAccessPolicyOptions

      static Azure::Core::Response<SetContainerAccessPolicyResult> SetAccessPolicy(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const SetContainerAccessPolicyOptions& options)
      {
        unused(options);
        std::string xml_body;
        {
          XmlWriter writer;
          SetContainerAccessPolicyOptionsToXml(writer, options);
          xml_body = writer.GetDocument();
          writer.Write(XmlNode{XmlNodeType::End});
        }
        Azure::Core::Http::MemoryBodyStream xml_body_stream(
            reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
        auto request
            = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, &xml_body_stream);
        request.AddHeader("Content-Length", std::to_string(xml_body_stream.Length()));
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.GetUrl().AppendQueryParameter("restype", "container");
        request.GetUrl().AppendQueryParameter("comp", "acl");
        if (options.AccessType.HasValue())
        {
          request.AddHeader(
              "x-ms-blob-public-access", PublicAccessTypeToString(options.AccessType.GetValue()));
        }
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        SetContainerAccessPolicyResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        return Azure::Core::Response<SetContainerAccessPolicyResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct AcquireContainerLeaseOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        int32_t LeaseDuration = -1;
        Azure::Core::Nullable<std::string> ProposedLeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
      }; // struct AcquireContainerLeaseOptions

      static Azure::Core::Response<AcquireContainerLeaseResult> AcquireLease(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const AcquireContainerLeaseOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.GetUrl().AppendQueryParameter("restype", "container");
        request.GetUrl().AppendQueryParameter("comp", "lease");
        request.AddHeader("x-ms-lease-action", "acquire");
        request.AddHeader("x-ms-lease-duration", std::to_string(options.LeaseDuration));
        if (options.ProposedLeaseId.HasValue())
        {
          request.AddHeader("x-ms-proposed-lease-id", options.ProposedLeaseId.GetValue());
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        AcquireContainerLeaseResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
        return Azure::Core::Response<AcquireContainerLeaseResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct RenewContainerLeaseOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
      }; // struct RenewContainerLeaseOptions

      static Azure::Core::Response<RenewContainerLeaseResult> RenewLease(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const RenewContainerLeaseOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.GetUrl().AppendQueryParameter("restype", "container");
        request.GetUrl().AppendQueryParameter("comp", "lease");
        request.AddHeader("x-ms-lease-action", "renew");
        request.AddHeader("x-ms-lease-id", options.LeaseId);
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        RenewContainerLeaseResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
        return Azure::Core::Response<RenewContainerLeaseResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct ChangeContainerLeaseOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string LeaseId;
        std::string ProposedLeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
      }; // struct ChangeContainerLeaseOptions

      static Azure::Core::Response<ChangeContainerLeaseResult> ChangeLease(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const ChangeContainerLeaseOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.GetUrl().AppendQueryParameter("restype", "container");
        request.GetUrl().AppendQueryParameter("comp", "lease");
        request.AddHeader("x-ms-lease-action", "change");
        request.AddHeader("x-ms-lease-id", options.LeaseId);
        request.AddHeader("x-ms-proposed-lease-id", options.ProposedLeaseId);
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        ChangeContainerLeaseResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
        return Azure::Core::Response<ChangeContainerLeaseResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct ReleaseContainerLeaseOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
      }; // struct ReleaseContainerLeaseOptions

      static Azure::Core::Response<ReleaseContainerLeaseResult> ReleaseLease(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const ReleaseContainerLeaseOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.GetUrl().AppendQueryParameter("restype", "container");
        request.GetUrl().AppendQueryParameter("comp", "lease");
        request.AddHeader("x-ms-lease-action", "release");
        request.AddHeader("x-ms-lease-id", options.LeaseId);
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        ReleaseContainerLeaseResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        return Azure::Core::Response<ReleaseContainerLeaseResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct BreakContainerLeaseOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<int32_t> BreakPeriod;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
      }; // struct BreakContainerLeaseOptions

      static Azure::Core::Response<BreakContainerLeaseResult> BreakLease(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const BreakContainerLeaseOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.GetUrl().AppendQueryParameter("restype", "container");
        request.GetUrl().AppendQueryParameter("comp", "lease");
        request.AddHeader("x-ms-lease-action", "break");
        if (options.BreakPeriod.HasValue())
        {
          request.AddHeader(
              "x-ms-lease-break-period", std::to_string(options.BreakPeriod.GetValue()));
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BreakContainerLeaseResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 202))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.LeaseTime = std::stoi(httpResponse.GetHeaders().at("x-ms-lease-time"));
        return Azure::Core::Response<BreakContainerLeaseResult>(
            std::move(response), std::move(pHttpResponse));
      }

    private:
      static GetContainerAccessPolicyResult GetContainerAccessPolicyResultFromXml(XmlReader& reader)
      {
        GetContainerAccessPolicyResult ret;
        enum class XmlTagName
        {
          k_SignedIdentifiers,
          k_SignedIdentifier,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "SignedIdentifiers") == 0)
            {
              path.emplace_back(XmlTagName::k_SignedIdentifiers);
            }
            else if (std::strcmp(node.Name, "SignedIdentifier") == 0)
            {
              path.emplace_back(XmlTagName::k_SignedIdentifier);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
            if (path.size() == 2 && path[0] == XmlTagName::k_SignedIdentifiers
                && path[1] == XmlTagName::k_SignedIdentifier)
            {
              ret.SignedIdentifiers.emplace_back(BlobSignedIdentifierFromXml(reader));
              path.pop_back();
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
          }
        }
        return ret;
      }

      static ListBlobsByHierarchySegmentResult ListBlobsByHierarchySegmentResultFromXml(
          XmlReader& reader)
      {
        ListBlobsByHierarchySegmentResult ret;
        enum class XmlTagName
        {
          k_EnumerationResults,
          k_Prefix,
          k_Delimiter,
          k_Marker,
          k_NextMarker,
          k_Blobs,
          k_Blob,
          k_BlobPrefix,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "EnumerationResults") == 0)
            {
              path.emplace_back(XmlTagName::k_EnumerationResults);
            }
            else if (std::strcmp(node.Name, "Prefix") == 0)
            {
              path.emplace_back(XmlTagName::k_Prefix);
            }
            else if (std::strcmp(node.Name, "Delimiter") == 0)
            {
              path.emplace_back(XmlTagName::k_Delimiter);
            }
            else if (std::strcmp(node.Name, "Marker") == 0)
            {
              path.emplace_back(XmlTagName::k_Marker);
            }
            else if (std::strcmp(node.Name, "NextMarker") == 0)
            {
              path.emplace_back(XmlTagName::k_NextMarker);
            }
            else if (std::strcmp(node.Name, "Blobs") == 0)
            {
              path.emplace_back(XmlTagName::k_Blobs);
            }
            else if (std::strcmp(node.Name, "Blob") == 0)
            {
              path.emplace_back(XmlTagName::k_Blob);
            }
            else if (std::strcmp(node.Name, "BlobPrefix") == 0)
            {
              path.emplace_back(XmlTagName::k_BlobPrefix);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
            if (path.size() == 3 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_Blobs && path[2] == XmlTagName::k_Blob)
            {
              ret.Items.emplace_back(BlobItemFromXml(reader));
              path.pop_back();
            }
            else if (
                path.size() == 3 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_Blobs && path[2] == XmlTagName::k_BlobPrefix)
            {
              ret.BlobPrefixes.emplace_back(BlobPrefixFromXml(reader));
              path.pop_back();
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
            if (path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_Prefix)
            {
              ret.Prefix = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_Delimiter)
            {
              ret.Delimiter = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_Marker)
            {
              ret.PreviousContinuationToken = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_NextMarker)
            {
              ret.ContinuationToken = node.Value;
            }
          }
          else if (node.Type == XmlNodeType::Attribute)
          {
            if (path.size() == 1 && path[0] == XmlTagName::k_EnumerationResults
                && std::strcmp(node.Name, "ServiceEndpoint") == 0)
            {
              ret.ServiceEndpoint = node.Value;
            }
            else if (
                path.size() == 1 && path[0] == XmlTagName::k_EnumerationResults
                && std::strcmp(node.Name, "ContainerName") == 0)
            {
              ret.Container = node.Value;
            }
          }
        }
        return ret;
      }

      static ListBlobsFlatSegmentResult ListBlobsFlatSegmentResultFromXml(XmlReader& reader)
      {
        ListBlobsFlatSegmentResult ret;
        enum class XmlTagName
        {
          k_EnumerationResults,
          k_Prefix,
          k_Marker,
          k_NextMarker,
          k_Blobs,
          k_Blob,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "EnumerationResults") == 0)
            {
              path.emplace_back(XmlTagName::k_EnumerationResults);
            }
            else if (std::strcmp(node.Name, "Prefix") == 0)
            {
              path.emplace_back(XmlTagName::k_Prefix);
            }
            else if (std::strcmp(node.Name, "Marker") == 0)
            {
              path.emplace_back(XmlTagName::k_Marker);
            }
            else if (std::strcmp(node.Name, "NextMarker") == 0)
            {
              path.emplace_back(XmlTagName::k_NextMarker);
            }
            else if (std::strcmp(node.Name, "Blobs") == 0)
            {
              path.emplace_back(XmlTagName::k_Blobs);
            }
            else if (std::strcmp(node.Name, "Blob") == 0)
            {
              path.emplace_back(XmlTagName::k_Blob);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
            if (path.size() == 3 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_Blobs && path[2] == XmlTagName::k_Blob)
            {
              ret.Items.emplace_back(BlobItemFromXml(reader));
              path.pop_back();
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
            if (path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_Prefix)
            {
              ret.Prefix = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_Marker)
            {
              ret.PreviousContinuationToken = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_NextMarker)
            {
              ret.ContinuationToken = node.Value;
            }
          }
          else if (node.Type == XmlNodeType::Attribute)
          {
            if (path.size() == 1 && path[0] == XmlTagName::k_EnumerationResults
                && std::strcmp(node.Name, "ServiceEndpoint") == 0)
            {
              ret.ServiceEndpoint = node.Value;
            }
            else if (
                path.size() == 1 && path[0] == XmlTagName::k_EnumerationResults
                && std::strcmp(node.Name, "ContainerName") == 0)
            {
              ret.Container = node.Value;
            }
          }
        }
        return ret;
      }

      static BlobItem BlobItemFromXml(XmlReader& reader)
      {
        BlobItem ret;
        enum class XmlTagName
        {
          k_Name,
          k_Deleted,
          k_Snapshot,
          k_VersionId,
          k_IsCurrentVersion,
          k_Properties,
          k_ContentType,
          k_ContentEncoding,
          k_ContentLanguage,
          k_ContentMD5,
          k_CacheControl,
          k_ContentDisposition,
          k_CreationTime,
          k_ExpiryTime,
          k_LastAccessTime,
          k_LastModified,
          k_Etag,
          k_ContentLength,
          k_BlobType,
          k_AccessTier,
          k_AccessTierInferred,
          k_LeaseStatus,
          k_LeaseState,
          k_LeaseDuration,
          k_ServerEncrypted,
          k_EncryptionKeySHA256,
          k_Sealed,
          k_xmsblobsequencenumber,
          k_Metadata,
          k_OrMetadata,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "Name") == 0)
            {
              path.emplace_back(XmlTagName::k_Name);
            }
            else if (std::strcmp(node.Name, "Deleted") == 0)
            {
              path.emplace_back(XmlTagName::k_Deleted);
            }
            else if (std::strcmp(node.Name, "Snapshot") == 0)
            {
              path.emplace_back(XmlTagName::k_Snapshot);
            }
            else if (std::strcmp(node.Name, "VersionId") == 0)
            {
              path.emplace_back(XmlTagName::k_VersionId);
            }
            else if (std::strcmp(node.Name, "IsCurrentVersion") == 0)
            {
              path.emplace_back(XmlTagName::k_IsCurrentVersion);
            }
            else if (std::strcmp(node.Name, "Properties") == 0)
            {
              path.emplace_back(XmlTagName::k_Properties);
            }
            else if (std::strcmp(node.Name, "Content-Type") == 0)
            {
              path.emplace_back(XmlTagName::k_ContentType);
            }
            else if (std::strcmp(node.Name, "Content-Encoding") == 0)
            {
              path.emplace_back(XmlTagName::k_ContentEncoding);
            }
            else if (std::strcmp(node.Name, "Content-Language") == 0)
            {
              path.emplace_back(XmlTagName::k_ContentLanguage);
            }
            else if (std::strcmp(node.Name, "Content-MD5") == 0)
            {
              path.emplace_back(XmlTagName::k_ContentMD5);
            }
            else if (std::strcmp(node.Name, "Cache-Control") == 0)
            {
              path.emplace_back(XmlTagName::k_CacheControl);
            }
            else if (std::strcmp(node.Name, "Content-Disposition") == 0)
            {
              path.emplace_back(XmlTagName::k_ContentDisposition);
            }
            else if (std::strcmp(node.Name, "Creation-Time") == 0)
            {
              path.emplace_back(XmlTagName::k_CreationTime);
            }
            else if (std::strcmp(node.Name, "Expiry-Time") == 0)
            {
              path.emplace_back(XmlTagName::k_ExpiryTime);
            }
            else if (std::strcmp(node.Name, "LastAccessTime") == 0)
            {
              path.emplace_back(XmlTagName::k_LastAccessTime);
            }
            else if (std::strcmp(node.Name, "Last-Modified") == 0)
            {
              path.emplace_back(XmlTagName::k_LastModified);
            }
            else if (std::strcmp(node.Name, "Etag") == 0)
            {
              path.emplace_back(XmlTagName::k_Etag);
            }
            else if (std::strcmp(node.Name, "Content-Length") == 0)
            {
              path.emplace_back(XmlTagName::k_ContentLength);
            }
            else if (std::strcmp(node.Name, "BlobType") == 0)
            {
              path.emplace_back(XmlTagName::k_BlobType);
            }
            else if (std::strcmp(node.Name, "AccessTier") == 0)
            {
              path.emplace_back(XmlTagName::k_AccessTier);
            }
            else if (std::strcmp(node.Name, "AccessTierInferred") == 0)
            {
              path.emplace_back(XmlTagName::k_AccessTierInferred);
            }
            else if (std::strcmp(node.Name, "LeaseStatus") == 0)
            {
              path.emplace_back(XmlTagName::k_LeaseStatus);
            }
            else if (std::strcmp(node.Name, "LeaseState") == 0)
            {
              path.emplace_back(XmlTagName::k_LeaseState);
            }
            else if (std::strcmp(node.Name, "LeaseDuration") == 0)
            {
              path.emplace_back(XmlTagName::k_LeaseDuration);
            }
            else if (std::strcmp(node.Name, "ServerEncrypted") == 0)
            {
              path.emplace_back(XmlTagName::k_ServerEncrypted);
            }
            else if (std::strcmp(node.Name, "EncryptionKeySHA256") == 0)
            {
              path.emplace_back(XmlTagName::k_EncryptionKeySHA256);
            }
            else if (std::strcmp(node.Name, "Sealed") == 0)
            {
              path.emplace_back(XmlTagName::k_Sealed);
            }
            else if (std::strcmp(node.Name, "x-ms-blob-sequence-number") == 0)
            {
              path.emplace_back(XmlTagName::k_xmsblobsequencenumber);
            }
            else if (std::strcmp(node.Name, "Metadata") == 0)
            {
              path.emplace_back(XmlTagName::k_Metadata);
            }
            else if (std::strcmp(node.Name, "OrMetadata") == 0)
            {
              path.emplace_back(XmlTagName::k_OrMetadata);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
            if (path.size() == 1 && path[0] == XmlTagName::k_Metadata)
            {
              ret.Metadata = MetadataFromXml(reader);
              path.pop_back();
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_OrMetadata)
            {
              ret.ObjectReplicationSourceProperties
                  = ObjectReplicationSourcePropertiesFromXml(reader);
              path.pop_back();
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
            if (path.size() == 1 && path[0] == XmlTagName::k_Name)
            {
              ret.Name = node.Value;
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_Deleted)
            {
              ret.Deleted = std::strcmp(node.Value, "true") == 0;
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_Snapshot)
            {
              ret.Snapshot = node.Value;
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_VersionId)
            {
              ret.VersionId = node.Value;
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_IsCurrentVersion)
            {
              ret.IsCurrentVersion = std::strcmp(node.Value, "true") == 0;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_ContentType)
            {
              ret.HttpHeaders.ContentType = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_ContentEncoding)
            {
              ret.HttpHeaders.ContentEncoding = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_ContentLanguage)
            {
              ret.HttpHeaders.ContentLanguage = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_ContentMD5)
            {
              ret.HttpHeaders.ContentMd5 = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_CacheControl)
            {
              ret.HttpHeaders.CacheControl = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_ContentDisposition)
            {
              ret.HttpHeaders.ContentDisposition = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_CreationTime)
            {
              ret.CreationTime = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_ExpiryTime)
            {
              ret.ExpiryTime = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_LastAccessTime)
            {
              ret.LastAccessTime = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_LastModified)
            {
              ret.LastModified = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_Etag)
            {
              ret.ETag = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_ContentLength)
            {
              ret.ContentLength = std::stoll(node.Value);
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_BlobType)
            {
              ret.BlobType = BlobTypeFromString(node.Value);
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_AccessTier)
            {
              ret.Tier = AccessTierFromString(node.Value);
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_AccessTierInferred)
            {
              ret.AccessTierInferred = std::strcmp(node.Value, "true") == 0;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_LeaseStatus)
            {
              ret.LeaseStatus = BlobLeaseStatusFromString(node.Value);
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_LeaseState)
            {
              ret.LeaseState = BlobLeaseStateFromString(node.Value);
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_LeaseDuration)
            {
              ret.LeaseDuration = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_ServerEncrypted)
            {
              ret.ServerEncrypted = std::strcmp(node.Value, "true") == 0;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_EncryptionKeySHA256)
            {
              ret.EncryptionKeySha256 = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_Sealed)
            {
              ret.IsSealed = std::strcmp(node.Value, "true") == 0;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_xmsblobsequencenumber)
            {
              ret.SequenceNumber = std::stoll(node.Value);
            }
          }
        }
        return ret;
      }

      static BlobPrefix BlobPrefixFromXml(XmlReader& reader)
      {
        BlobPrefix ret;
        enum class XmlTagName
        {
          k_Name,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "Name") == 0)
            {
              path.emplace_back(XmlTagName::k_Name);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
            if (path.size() == 1 && path[0] == XmlTagName::k_Name)
            {
              ret.Name = node.Value;
            }
          }
        }
        return ret;
      }

      static BlobSignedIdentifier BlobSignedIdentifierFromXml(XmlReader& reader)
      {
        BlobSignedIdentifier ret;
        enum class XmlTagName
        {
          k_Id,
          k_AccessPolicy,
          k_Start,
          k_Expiry,
          k_Permission,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "Id") == 0)
            {
              path.emplace_back(XmlTagName::k_Id);
            }
            else if (std::strcmp(node.Name, "AccessPolicy") == 0)
            {
              path.emplace_back(XmlTagName::k_AccessPolicy);
            }
            else if (std::strcmp(node.Name, "Start") == 0)
            {
              path.emplace_back(XmlTagName::k_Start);
            }
            else if (std::strcmp(node.Name, "Expiry") == 0)
            {
              path.emplace_back(XmlTagName::k_Expiry);
            }
            else if (std::strcmp(node.Name, "Permission") == 0)
            {
              path.emplace_back(XmlTagName::k_Permission);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
            if (path.size() == 1 && path[0] == XmlTagName::k_Id)
            {
              ret.Id = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_AccessPolicy
                && path[1] == XmlTagName::k_Start)
            {
              ret.StartsOn = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_AccessPolicy
                && path[1] == XmlTagName::k_Expiry)
            {
              ret.ExpiresOn = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_AccessPolicy
                && path[1] == XmlTagName::k_Permission)
            {
              ret.Permissions = node.Value;
            }
          }
        }
        return ret;
      }

      static std::map<std::string, std::string> MetadataFromXml(XmlReader& reader)
      {
        std::map<std::string, std::string> ret;
        int depth = 0;
        std::string key;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (depth++ == 0)
            {
              key = node.Name;
            }
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (depth-- == 0)
            {
              break;
            }
          }
          else if (depth == 1 && node.Type == XmlNodeType::Text)
          {
            ret.emplace(std::move(key), std::string(node.Value));
          }
        }
        return ret;
      }

      static std::vector<ObjectReplicationPolicy> ObjectReplicationSourcePropertiesFromXml(
          XmlReader& reader)
      {
        int depth = 0;
        std::map<std::string, std::vector<ObjectReplicationRule>> orPropertiesMap;
        std::string policyId;
        std::string ruleId;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            ++depth;
            std::string startTagName = node.Name;
            if (startTagName.substr(0, 3) == "or-")
            {
              auto underscorePos = startTagName.find('_', 3);
              policyId
                  = std::string(startTagName.begin() + 3, startTagName.begin() + underscorePos);
              ruleId = startTagName.substr(underscorePos + 1);
            }
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (depth-- == 0)
            {
              break;
            }
          }
          if (depth == 1 && node.Type == XmlNodeType::Text)
          {
            ObjectReplicationRule rule;
            rule.RuleId = std::move(ruleId);
            rule.ReplicationStatus = ObjectReplicationStatusFromString(node.Value);
            orPropertiesMap[policyId].emplace_back(std::move(rule));
          }
        }
        std::vector<ObjectReplicationPolicy> ret;
        for (auto& property : orPropertiesMap)
        {
          ObjectReplicationPolicy policy;
          policy.PolicyId = property.first;
          policy.Rules = std::move(property.second);
          ret.emplace_back(std::move(policy));
        }
        return ret;
      }

      static void SetContainerAccessPolicyOptionsToXml(
          XmlWriter& writer,
          const SetContainerAccessPolicyOptions& options)
      {
        writer.Write(XmlNode{XmlNodeType::StartTag, "SignedIdentifiers"});
        for (const auto& i : options.SignedIdentifiers)
        {
          BlobSignedIdentifierToXml(writer, i);
        }
        writer.Write(XmlNode{XmlNodeType::EndTag});
      }

      static void BlobSignedIdentifierToXml(XmlWriter& writer, const BlobSignedIdentifier& options)
      {
        writer.Write(XmlNode{XmlNodeType::StartTag, "SignedIdentifier"});
        writer.Write(XmlNode{XmlNodeType::StartTag, "Id"});
        writer.Write(XmlNode{XmlNodeType::Text, nullptr, options.Id.data()});
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::StartTag, "AccessPolicy"});
        writer.Write(XmlNode{XmlNodeType::StartTag, "Start"});
        writer.Write(XmlNode{XmlNodeType::Text, nullptr, options.StartsOn.data()});
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::StartTag, "Expiry"});
        writer.Write(XmlNode{XmlNodeType::Text, nullptr, options.ExpiresOn.data()});
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::StartTag, "Permission"});
        writer.Write(XmlNode{XmlNodeType::Text, nullptr, options.Permissions.data()});
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::EndTag});
      }

    }; // class Container

    class Blob {
    public:
      struct DownloadBlobOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::pair<int64_t, int64_t>> Range;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct DownloadBlobOptions

      static Azure::Core::Response<DownloadBlobResult> Download(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const DownloadBlobOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url, true);
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        if (options.Range.HasValue())
        {
          auto startOffset = options.Range.GetValue().first;
          auto endOffset = options.Range.GetValue().second;
          if (endOffset != std::numeric_limits<decltype(endOffset)>::max())
          {
            request.AddHeader(
                "x-ms-range",
                "bytes=" + std::to_string(startOffset) + "-" + std::to_string(endOffset));
          }
          else
          {
            request.AddHeader("x-ms-range", "bytes=" + std::to_string(startOffset) + "-");
          }
        }
        if (options.EncryptionKey.HasValue())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
        }
        if (options.EncryptionKeySha256.HasValue())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySha256.GetValue());
        }
        if (options.EncryptionAlgorithm.HasValue())
        {
          request.AddHeader(
              "x-ms-encryption-algorithm",
              EncryptionAlgorithmTypeToString(options.EncryptionAlgorithm.GetValue()));
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        DownloadBlobResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200 || http_status_code == 206))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.BodyStream = httpResponse.GetBodyStream();
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        auto response_transactional_content_md5_iterator
            = httpResponse.GetHeaders().find("content-md5");
        if (response_transactional_content_md5_iterator != httpResponse.GetHeaders().end())
        {
          response.TransactionalContentMd5 = response_transactional_content_md5_iterator->second;
        }
        auto response_transactional_content_crc64_iterator
            = httpResponse.GetHeaders().find("x-ms-content-crc64");
        if (response_transactional_content_crc64_iterator != httpResponse.GetHeaders().end())
        {
          response.TransactionalContentCrc64
              = response_transactional_content_crc64_iterator->second;
        }
        auto response_http_headers_content_type_iterator
            = httpResponse.GetHeaders().find("content-type");
        if (response_http_headers_content_type_iterator != httpResponse.GetHeaders().end())
        {
          response.HttpHeaders.ContentType = response_http_headers_content_type_iterator->second;
        }
        auto response_http_headers_content_encoding_iterator
            = httpResponse.GetHeaders().find("content-encoding");
        if (response_http_headers_content_encoding_iterator != httpResponse.GetHeaders().end())
        {
          response.HttpHeaders.ContentEncoding
              = response_http_headers_content_encoding_iterator->second;
        }
        auto response_http_headers_content_language_iterator
            = httpResponse.GetHeaders().find("content-language");
        if (response_http_headers_content_language_iterator != httpResponse.GetHeaders().end())
        {
          response.HttpHeaders.ContentLanguage
              = response_http_headers_content_language_iterator->second;
        }
        auto response_http_headers_cache_control_iterator
            = httpResponse.GetHeaders().find("cache-control");
        if (response_http_headers_cache_control_iterator != httpResponse.GetHeaders().end())
        {
          response.HttpHeaders.CacheControl = response_http_headers_cache_control_iterator->second;
        }
        auto response_http_headers_content_md5_iterator
            = httpResponse.GetHeaders().find("content-md5");
        if (response_http_headers_content_md5_iterator != httpResponse.GetHeaders().end())
        {
          response.HttpHeaders.ContentMd5 = response_http_headers_content_md5_iterator->second;
        }
        auto x_ms_blob_content_md5_iterator
            = httpResponse.GetHeaders().find("x-ms-blob-content-md5");
        if (x_ms_blob_content_md5_iterator != httpResponse.GetHeaders().end())
        {
          response.HttpHeaders.ContentMd5 = x_ms_blob_content_md5_iterator->second;
        }
        auto response_http_headers_content_disposition_iterator
            = httpResponse.GetHeaders().find("content-disposition");
        if (response_http_headers_content_disposition_iterator != httpResponse.GetHeaders().end())
        {
          response.HttpHeaders.ContentDisposition
              = response_http_headers_content_disposition_iterator->second;
        }
        for (auto i = httpResponse.GetHeaders().lower_bound("x-ms-meta-");
             i != httpResponse.GetHeaders().end() && i->first.substr(0, 10) == "x-ms-meta-";
             ++i)
        {
          response.Metadata.emplace(i->first.substr(10), i->second);
        }
        auto response_server_encrypted_iterator
            = httpResponse.GetHeaders().find("x-ms-server-encrypted");
        if (response_server_encrypted_iterator != httpResponse.GetHeaders().end())
        {
          response.ServerEncrypted = response_server_encrypted_iterator->second == "true";
        }
        auto response_encryption_key_sha256_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryption_key_sha256_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionKeySha256 = response_encryption_key_sha256_iterator->second;
        }
        auto response_encryption_scope_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-scope");
        if (response_encryption_scope_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionScope = response_encryption_scope_iterator->second;
        }
        auto response_lease_status_iterator = httpResponse.GetHeaders().find("x-ms-lease-status");
        if (response_lease_status_iterator != httpResponse.GetHeaders().end())
        {
          response.LeaseStatus = BlobLeaseStatusFromString(response_lease_status_iterator->second);
        }
        auto response_lease_state_iterator = httpResponse.GetHeaders().find("x-ms-lease-state");
        if (response_lease_state_iterator != httpResponse.GetHeaders().end())
        {
          response.LeaseState = BlobLeaseStateFromString(response_lease_state_iterator->second);
        }
        auto response_lease_duration_iterator
            = httpResponse.GetHeaders().find("x-ms-lease-duration");
        if (response_lease_duration_iterator != httpResponse.GetHeaders().end())
        {
          response.LeaseDuration = response_lease_duration_iterator->second;
        }
        response.CreationTime = httpResponse.GetHeaders().at("x-ms-creation-time");
        auto response_expiry_time_iterator = httpResponse.GetHeaders().find("x-ms-expiry-time");
        if (response_expiry_time_iterator != httpResponse.GetHeaders().end())
        {
          response.ExpiryTime = response_expiry_time_iterator->second;
        }
        auto response_last_access_time_iterator
            = httpResponse.GetHeaders().find("x-ms-last-access-time");
        if (response_last_access_time_iterator != httpResponse.GetHeaders().end())
        {
          response.LastAccessTime = response_last_access_time_iterator->second;
        }
        auto response_content_range_iterator = httpResponse.GetHeaders().find("content-range");
        if (response_content_range_iterator != httpResponse.GetHeaders().end())
        {
          response.ContentRange = response_content_range_iterator->second;
        }
        auto response_sequence_number_iterator
            = httpResponse.GetHeaders().find("x-ms-blob-sequence-number");
        if (response_sequence_number_iterator != httpResponse.GetHeaders().end())
        {
          response.SequenceNumber = std::stoll(response_sequence_number_iterator->second);
        }
        auto response_committed_block_count_iterator
            = httpResponse.GetHeaders().find("x-ms-blob-committed-block-count");
        if (response_committed_block_count_iterator != httpResponse.GetHeaders().end())
        {
          response.CommittedBlockCount
              = std::stoll(response_committed_block_count_iterator->second);
        }
        auto response_is_sealed_iterator = httpResponse.GetHeaders().find("x-ms-blob-sealed");
        if (response_is_sealed_iterator != httpResponse.GetHeaders().end())
        {
          response.IsSealed = response_is_sealed_iterator->second == "true";
        }
        response.BlobType = BlobTypeFromString(httpResponse.GetHeaders().at("x-ms-blob-type"));
        auto response_object_replication_destination_policy_id_iterator
            = httpResponse.GetHeaders().find("x-ms-or-policy-id");
        if (response_object_replication_destination_policy_id_iterator
            != httpResponse.GetHeaders().end())
        {
          response.ObjectReplicationDestinationPolicyId
              = response_object_replication_destination_policy_id_iterator->second;
        }
        {
          std::map<std::string, std::vector<ObjectReplicationRule>> orPropertiesMap;
          for (auto i = httpResponse.GetHeaders().lower_bound("x-ms-or-");
               i != httpResponse.GetHeaders().end() && i->first.substr(0, 8) == "x-ms-or-";
               ++i)
          {
            const std::string& header = i->first;
            auto underscorePos = header.find('_', 8);
            if (underscorePos == std::string::npos)
            {
              continue;
            }
            std::string policyId = std::string(header.begin() + 8, header.begin() + underscorePos);
            std::string ruleId = header.substr(underscorePos + 1);

            ObjectReplicationRule rule;
            rule.RuleId = std::move(ruleId);
            rule.ReplicationStatus = ObjectReplicationStatusFromString(i->second);
            orPropertiesMap[policyId].emplace_back(std::move(rule));
          }
          for (auto& property : orPropertiesMap)
          {
            ObjectReplicationPolicy policy;
            policy.PolicyId = property.first;
            policy.Rules = std::move(property.second);
            response.ObjectReplicationSourceProperties.emplace_back(std::move(policy));
          }
        }
        auto response_tag_count_iterator = httpResponse.GetHeaders().find("x-ms-tag-count");
        if (response_tag_count_iterator != httpResponse.GetHeaders().end())
        {
          response.TagCount = std::stoi(response_tag_count_iterator->second);
        }
        return Azure::Core::Response<DownloadBlobResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct DeleteBlobOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<DeleteSnapshotsOption> DeleteSnapshots;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct DeleteBlobOptions

      static Azure::Core::Http::Request DeleteCreateMessage(
          const Azure::Core::Http::Url& url,
          const DeleteBlobOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Delete, url);
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        if (options.DeleteSnapshots.HasValue())
        {
          request.AddHeader(
              "x-ms-delete-snapshots",
              DeleteSnapshotsOptionToString(options.DeleteSnapshots.GetValue()));
        }
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        return request;
      }

      static Azure::Core::Response<DeleteBlobResult> DeleteCreateResponse(
          const Azure::Core::Context& context,
          std::unique_ptr<Azure::Core::Http::RawResponse> pHttpResponse)
      {
        unused(context);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        DeleteBlobResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 202))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        return Azure::Core::Response<DeleteBlobResult>(
            std::move(response), std::move(pHttpResponse));
      }

      static Azure::Core::Response<DeleteBlobResult> Delete(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const DeleteBlobOptions& options)
      {
        auto request = DeleteCreateMessage(url, options);
        auto pHttpResponse = pipeline.Send(context, request);
        return DeleteCreateResponse(context, std::move(pHttpResponse));
      }

      struct SetBlobExpiryOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        ScheduleBlobExpiryOriginType ExpiryOrigin;
        Azure::Core::Nullable<std::string> ExpiryTime;
      }; // struct SetBlobExpiryOptions

      static Azure::Core::Response<SetBlobExpiryResult> ScheduleDeletion(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const SetBlobExpiryOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.GetUrl().AppendQueryParameter("comp", "expiry");
        request.AddHeader(
            "x-ms-expiry-option", ScheduleBlobExpiryOriginTypeToString(options.ExpiryOrigin));
        if (options.ExpiryTime.HasValue())
        {
          request.AddHeader("x-ms-expiry-time", options.ExpiryTime.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        SetBlobExpiryResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        return Azure::Core::Response<SetBlobExpiryResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct UndeleteBlobOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
      }; // struct UndeleteBlobOptions

      static Azure::Core::Response<UndeleteBlobResult> Undelete(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const UndeleteBlobOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.GetUrl().AppendQueryParameter("comp", "undelete");
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        UndeleteBlobResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        return Azure::Core::Response<UndeleteBlobResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct GetBlobPropertiesOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct GetBlobPropertiesOptions

      static Azure::Core::Response<GetBlobPropertiesResult> GetProperties(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const GetBlobPropertiesOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, url);
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        if (options.EncryptionKey.HasValue())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
        }
        if (options.EncryptionKeySha256.HasValue())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySha256.GetValue());
        }
        if (options.EncryptionAlgorithm.HasValue())
        {
          request.AddHeader(
              "x-ms-encryption-algorithm",
              EncryptionAlgorithmTypeToString(options.EncryptionAlgorithm.GetValue()));
        }
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        GetBlobPropertiesResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.CreationTime = httpResponse.GetHeaders().at("x-ms-creation-time");
        auto response_expiry_time_iterator = httpResponse.GetHeaders().find("x-ms-expiry-time");
        if (response_expiry_time_iterator != httpResponse.GetHeaders().end())
        {
          response.ExpiryTime = response_expiry_time_iterator->second;
        }
        auto response_last_access_time_iterator
            = httpResponse.GetHeaders().find("x-ms-last-access-time");
        if (response_last_access_time_iterator != httpResponse.GetHeaders().end())
        {
          response.LastAccessTime = response_last_access_time_iterator->second;
        }
        for (auto i = httpResponse.GetHeaders().lower_bound("x-ms-meta-");
             i != httpResponse.GetHeaders().end() && i->first.substr(0, 10) == "x-ms-meta-";
             ++i)
        {
          response.Metadata.emplace(i->first.substr(10), i->second);
        }
        response.BlobType = BlobTypeFromString(httpResponse.GetHeaders().at("x-ms-blob-type"));
        auto response_lease_status_iterator = httpResponse.GetHeaders().find("x-ms-lease-status");
        if (response_lease_status_iterator != httpResponse.GetHeaders().end())
        {
          response.LeaseStatus = BlobLeaseStatusFromString(response_lease_status_iterator->second);
        }
        auto response_lease_state_iterator = httpResponse.GetHeaders().find("x-ms-lease-state");
        if (response_lease_state_iterator != httpResponse.GetHeaders().end())
        {
          response.LeaseState = BlobLeaseStateFromString(response_lease_state_iterator->second);
        }
        auto response_lease_duration_iterator
            = httpResponse.GetHeaders().find("x-ms-lease-duration");
        if (response_lease_duration_iterator != httpResponse.GetHeaders().end())
        {
          response.LeaseDuration = response_lease_duration_iterator->second;
        }
        response.ContentLength = std::stoll(httpResponse.GetHeaders().at("content-length"));
        auto response_http_headers_content_type_iterator
            = httpResponse.GetHeaders().find("content-type");
        if (response_http_headers_content_type_iterator != httpResponse.GetHeaders().end())
        {
          response.HttpHeaders.ContentType = response_http_headers_content_type_iterator->second;
        }
        auto response_http_headers_content_encoding_iterator
            = httpResponse.GetHeaders().find("content-encoding");
        if (response_http_headers_content_encoding_iterator != httpResponse.GetHeaders().end())
        {
          response.HttpHeaders.ContentEncoding
              = response_http_headers_content_encoding_iterator->second;
        }
        auto response_http_headers_content_language_iterator
            = httpResponse.GetHeaders().find("content-language");
        if (response_http_headers_content_language_iterator != httpResponse.GetHeaders().end())
        {
          response.HttpHeaders.ContentLanguage
              = response_http_headers_content_language_iterator->second;
        }
        auto response_http_headers_cache_control_iterator
            = httpResponse.GetHeaders().find("cache-control");
        if (response_http_headers_cache_control_iterator != httpResponse.GetHeaders().end())
        {
          response.HttpHeaders.CacheControl = response_http_headers_cache_control_iterator->second;
        }
        auto response_http_headers_content_md5_iterator
            = httpResponse.GetHeaders().find("content-md5");
        if (response_http_headers_content_md5_iterator != httpResponse.GetHeaders().end())
        {
          response.HttpHeaders.ContentMd5 = response_http_headers_content_md5_iterator->second;
        }
        auto x_ms_blob_content_md5_iterator
            = httpResponse.GetHeaders().find("x-ms-blob-content-md5");
        if (x_ms_blob_content_md5_iterator != httpResponse.GetHeaders().end())
        {
          response.HttpHeaders.ContentMd5 = x_ms_blob_content_md5_iterator->second;
        }
        auto response_http_headers_content_disposition_iterator
            = httpResponse.GetHeaders().find("content-disposition");
        if (response_http_headers_content_disposition_iterator != httpResponse.GetHeaders().end())
        {
          response.HttpHeaders.ContentDisposition
              = response_http_headers_content_disposition_iterator->second;
        }
        auto response_sequence_number_iterator
            = httpResponse.GetHeaders().find("x-ms-blob-sequence-number");
        if (response_sequence_number_iterator != httpResponse.GetHeaders().end())
        {
          response.SequenceNumber = std::stoll(response_sequence_number_iterator->second);
        }
        auto response_committed_block_count_iterator
            = httpResponse.GetHeaders().find("x-ms-blob-committed-block-count");
        if (response_committed_block_count_iterator != httpResponse.GetHeaders().end())
        {
          response.CommittedBlockCount = std::stoi(response_committed_block_count_iterator->second);
        }
        auto response_is_sealed_iterator = httpResponse.GetHeaders().find("x-ms-blob-sealed");
        if (response_is_sealed_iterator != httpResponse.GetHeaders().end())
        {
          response.IsSealed = response_is_sealed_iterator->second == "true";
        }
        auto response_server_encrypted_iterator
            = httpResponse.GetHeaders().find("x-ms-server-encrypted");
        if (response_server_encrypted_iterator != httpResponse.GetHeaders().end())
        {
          response.ServerEncrypted = response_server_encrypted_iterator->second == "true";
        }
        auto response_encryption_key_sha256_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryption_key_sha256_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionKeySha256 = response_encryption_key_sha256_iterator->second;
        }
        auto response_encryption_scope_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-scope");
        if (response_encryption_scope_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionScope = response_encryption_scope_iterator->second;
        }
        auto response_tier_iterator = httpResponse.GetHeaders().find("x-ms-access-tier");
        if (response_tier_iterator != httpResponse.GetHeaders().end())
        {
          response.Tier = AccessTierFromString(response_tier_iterator->second);
        }
        auto response_access_tier_inferred_iterator
            = httpResponse.GetHeaders().find("x-ms-access-tier-inferred");
        if (response_access_tier_inferred_iterator != httpResponse.GetHeaders().end())
        {
          response.AccessTierInferred = response_access_tier_inferred_iterator->second == "true";
        }
        auto response_archive_status_iterator
            = httpResponse.GetHeaders().find("x-ms-archive-status");
        if (response_archive_status_iterator != httpResponse.GetHeaders().end())
        {
          response.ArchiveStatus
              = BlobArchiveStatusFromString(response_archive_status_iterator->second);
        }
        auto response_access_tier_change_time_iterator
            = httpResponse.GetHeaders().find("x-ms-access-tier-change-time");
        if (response_access_tier_change_time_iterator != httpResponse.GetHeaders().end())
        {
          response.AccessTierChangeTime = response_access_tier_change_time_iterator->second;
        }
        auto response_copy_id_iterator = httpResponse.GetHeaders().find("x-ms-copy-id");
        if (response_copy_id_iterator != httpResponse.GetHeaders().end())
        {
          response.CopyId = response_copy_id_iterator->second;
        }
        auto response_copy_source_iterator = httpResponse.GetHeaders().find("x-ms-copy-source");
        if (response_copy_source_iterator != httpResponse.GetHeaders().end())
        {
          response.CopySource = response_copy_source_iterator->second;
        }
        auto response_copy_status_iterator = httpResponse.GetHeaders().find("x-ms-copy-status");
        if (response_copy_status_iterator != httpResponse.GetHeaders().end())
        {
          response.CopyStatus = CopyStatusFromString(response_copy_status_iterator->second);
        }
        auto response_copy_progress_iterator = httpResponse.GetHeaders().find("x-ms-copy-progress");
        if (response_copy_progress_iterator != httpResponse.GetHeaders().end())
        {
          response.CopyProgress = response_copy_progress_iterator->second;
        }
        auto response_copy_completion_time_iterator
            = httpResponse.GetHeaders().find("x-ms-copy-completion-time");
        if (response_copy_completion_time_iterator != httpResponse.GetHeaders().end())
        {
          response.CopyCompletionTime = response_copy_completion_time_iterator->second;
        }
        auto response_object_replication_destination_policy_id_iterator
            = httpResponse.GetHeaders().find("x-ms-or-policy-id");
        if (response_object_replication_destination_policy_id_iterator
            != httpResponse.GetHeaders().end())
        {
          response.ObjectReplicationDestinationPolicyId
              = response_object_replication_destination_policy_id_iterator->second;
        }
        {
          std::map<std::string, std::vector<ObjectReplicationRule>> orPropertiesMap;
          for (auto i = httpResponse.GetHeaders().lower_bound("x-ms-or-");
               i != httpResponse.GetHeaders().end() && i->first.substr(0, 8) == "x-ms-or-";
               ++i)
          {
            const std::string& header = i->first;
            auto underscorePos = header.find('_', 8);
            if (underscorePos == std::string::npos)
            {
              continue;
            }
            std::string policyId = std::string(header.begin() + 8, header.begin() + underscorePos);
            std::string ruleId = header.substr(underscorePos + 1);

            ObjectReplicationRule rule;
            rule.RuleId = std::move(ruleId);
            rule.ReplicationStatus = ObjectReplicationStatusFromString(i->second);
            orPropertiesMap[policyId].emplace_back(std::move(rule));
          }
          for (auto& property : orPropertiesMap)
          {
            ObjectReplicationPolicy policy;
            policy.PolicyId = property.first;
            policy.Rules = std::move(property.second);
            response.ObjectReplicationSourceProperties.emplace_back(std::move(policy));
          }
        }
        auto response_tag_count_iterator = httpResponse.GetHeaders().find("x-ms-tag-count");
        if (response_tag_count_iterator != httpResponse.GetHeaders().end())
        {
          response.TagCount = std::stoi(response_tag_count_iterator->second);
        }
        return Azure::Core::Response<GetBlobPropertiesResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct SetBlobHttpHeadersOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        BlobHttpHeaders HttpHeaders;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct SetBlobHttpHeadersOptions

      static Azure::Core::Response<SetBlobHttpHeadersResult> SetHttpHeaders(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const SetBlobHttpHeadersOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.GetUrl().AppendQueryParameter("comp", "properties");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        if (!options.HttpHeaders.ContentType.empty())
        {
          request.AddHeader("x-ms-blob-content-type", options.HttpHeaders.ContentType);
        }
        if (!options.HttpHeaders.ContentEncoding.empty())
        {
          request.AddHeader("x-ms-blob-content-encoding", options.HttpHeaders.ContentEncoding);
        }
        if (!options.HttpHeaders.ContentLanguage.empty())
        {
          request.AddHeader("x-ms-blob-content-language", options.HttpHeaders.ContentLanguage);
        }
        if (!options.HttpHeaders.CacheControl.empty())
        {
          request.AddHeader("x-ms-blob-cache-control", options.HttpHeaders.CacheControl);
        }
        if (!options.HttpHeaders.ContentMd5.empty())
        {
          request.AddHeader("x-ms-blob-content-md5", options.HttpHeaders.ContentMd5);
        }
        if (!options.HttpHeaders.ContentDisposition.empty())
        {
          request.AddHeader(
              "x-ms-blob-content-disposition", options.HttpHeaders.ContentDisposition);
        }
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        SetBlobHttpHeadersResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        auto response_sequence_number_iterator
            = httpResponse.GetHeaders().find("x-ms-blob-sequence-number");
        if (response_sequence_number_iterator != httpResponse.GetHeaders().end())
        {
          response.SequenceNumber = std::stoll(response_sequence_number_iterator->second);
        }
        return Azure::Core::Response<SetBlobHttpHeadersResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct SetBlobMetadataOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::map<std::string, std::string> Metadata;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> EncryptionScope;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct SetBlobMetadataOptions

      static Azure::Core::Response<SetBlobMetadataResult> SetMetadata(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const SetBlobMetadataOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.GetUrl().AppendQueryParameter("comp", "metadata");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        std::set<std::string> metadataKeys;
        for (const auto& pair : options.Metadata)
        {
          if (metadataKeys.insert(Azure::Core::Strings::ToLower(pair.first)).second == false)
          {
            throw std::runtime_error("duplicate keys in metadata");
          }
          request.AddHeader("x-ms-meta-" + pair.first, pair.second);
        }
        metadataKeys.clear();
        if (options.EncryptionKey.HasValue())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
        }
        if (options.EncryptionKeySha256.HasValue())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySha256.GetValue());
        }
        if (options.EncryptionAlgorithm.HasValue())
        {
          request.AddHeader(
              "x-ms-encryption-algorithm",
              EncryptionAlgorithmTypeToString(options.EncryptionAlgorithm.GetValue()));
        }
        if (options.EncryptionScope.HasValue())
        {
          request.AddHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
        }
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        SetBlobMetadataResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        return Azure::Core::Response<SetBlobMetadataResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct SetBlobAccessTierOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        AccessTier Tier = AccessTier::Unknown;
        Azure::Core::Nullable<Blobs::RehydratePriority> RehydratePriority;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct SetBlobAccessTierOptions

      static Azure::Core::Http::Request SetAccessTierCreateMessage(
          const Azure::Core::Http::Url& url,
          const SetBlobAccessTierOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.GetUrl().AppendQueryParameter("comp", "tier");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddHeader("x-ms-access-tier", AccessTierToString(options.Tier));
        if (options.RehydratePriority.HasValue())
        {
          request.AddHeader(
              "x-ms-rehydrate-priority",
              RehydratePriorityToString(options.RehydratePriority.GetValue()));
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        return request;
      }

      static Azure::Core::Response<SetBlobAccessTierResult> SetAccessTierCreateResponse(
          const Azure::Core::Context& context,
          std::unique_ptr<Azure::Core::Http::RawResponse> pHttpResponse)
      {
        unused(context);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        SetBlobAccessTierResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200 || http_status_code == 202))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        return Azure::Core::Response<SetBlobAccessTierResult>(
            std::move(response), std::move(pHttpResponse));
      }

      static Azure::Core::Response<SetBlobAccessTierResult> SetAccessTier(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const SetBlobAccessTierOptions& options)
      {
        auto request = SetAccessTierCreateMessage(url, options);
        auto pHttpResponse = pipeline.Send(context, request);
        return SetAccessTierCreateResponse(context, std::move(pHttpResponse));
      }

      struct StartCopyBlobFromUriOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::map<std::string, std::string> Metadata;
        std::string SourceUri;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> SourceLeaseId;
        Azure::Core::Nullable<AccessTier> Tier;
        Azure::Core::Nullable<Blobs::RehydratePriority> RehydratePriority;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
        Azure::Core::Nullable<std::string> SourceIfModifiedSince;
        Azure::Core::Nullable<std::string> SourceIfUnmodifiedSince;
        Azure::Core::Nullable<std::string> SourceIfMatch;
        Azure::Core::Nullable<std::string> SourceIfNoneMatch;
        Azure::Core::Nullable<std::string> SourceIfTags;
        Azure::Core::Nullable<bool> ShouldSealDestination;
      }; // struct StartCopyBlobFromUriOptions

      static Azure::Core::Response<StartCopyBlobFromUriResult> StartCopyFromUri(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const StartCopyBlobFromUriOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        std::set<std::string> metadataKeys;
        for (const auto& pair : options.Metadata)
        {
          if (metadataKeys.insert(Azure::Core::Strings::ToLower(pair.first)).second == false)
          {
            throw std::runtime_error("duplicate keys in metadata");
          }
          request.AddHeader("x-ms-meta-" + pair.first, pair.second);
        }
        metadataKeys.clear();
        request.AddHeader("x-ms-copy-source", options.SourceUri);
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        if (options.SourceLeaseId.HasValue())
        {
          request.AddHeader("x-ms-source-lease-id", options.SourceLeaseId.GetValue());
        }
        if (options.Tier.HasValue())
        {
          request.AddHeader("x-ms-access-tier", AccessTierToString(options.Tier.GetValue()));
        }
        if (options.RehydratePriority.HasValue())
        {
          request.AddHeader(
              "x-ms-rehydrate-priority",
              RehydratePriorityToString(options.RehydratePriority.GetValue()));
        }
        if (options.ShouldSealDestination.HasValue())
        {
          request.AddHeader(
              "x-ms-seal-blob", options.ShouldSealDestination.GetValue() ? "true" : "false");
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        if (options.SourceIfModifiedSince.HasValue())
        {
          request.AddHeader(
              "x-ms-source-if-modified-since", options.SourceIfModifiedSince.GetValue());
        }
        if (options.SourceIfUnmodifiedSince.HasValue())
        {
          request.AddHeader(
              "x-ms-source-if-unmodified-since", options.SourceIfUnmodifiedSince.GetValue());
        }
        if (options.SourceIfMatch.HasValue())
        {
          request.AddHeader("x-ms-source-if-match", options.SourceIfMatch.GetValue());
        }
        if (options.SourceIfNoneMatch.HasValue())
        {
          request.AddHeader("x-ms-source-if-none-match", options.SourceIfNoneMatch.GetValue());
        }
        if (options.SourceIfTags.HasValue())
        {
          request.AddHeader("x-ms-source-if-tags", options.SourceIfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        StartCopyBlobFromUriResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 202))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.CopyId = httpResponse.GetHeaders().at("x-ms-copy-id");
        response.CopyStatus
            = CopyStatusFromString(httpResponse.GetHeaders().at("x-ms-copy-status"));
        auto response_version_id_iterator = httpResponse.GetHeaders().find("x-ms-version-id");
        if (response_version_id_iterator != httpResponse.GetHeaders().end())
        {
          response.VersionId = response_version_id_iterator->second;
        }
        return Azure::Core::Response<StartCopyBlobFromUriResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct AbortCopyBlobFromUriOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string CopyId;
        Azure::Core::Nullable<std::string> LeaseId;
      }; // struct AbortCopyBlobFromUriOptions

      static Azure::Core::Response<AbortCopyBlobFromUriResult> AbortCopyFromUri(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const AbortCopyBlobFromUriOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.GetUrl().AppendQueryParameter("comp", "copy");
        request.GetUrl().AppendQueryParameter(
            "copyid", Details::UrlEncodeQueryParameter(options.CopyId));
        request.AddHeader("x-ms-copy-action", "abort");
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        AbortCopyBlobFromUriResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 204))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        return Azure::Core::Response<AbortCopyBlobFromUriResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct CreateBlobSnapshotOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::map<std::string, std::string> Metadata;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> EncryptionScope;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct CreateBlobSnapshotOptions

      static Azure::Core::Response<CreateBlobSnapshotResult> CreateSnapshot(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const CreateBlobSnapshotOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.GetUrl().AppendQueryParameter("comp", "snapshot");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        if (options.EncryptionKey.HasValue())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
        }
        if (options.EncryptionKeySha256.HasValue())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySha256.GetValue());
        }
        if (options.EncryptionAlgorithm.HasValue())
        {
          request.AddHeader(
              "x-ms-encryption-algorithm",
              EncryptionAlgorithmTypeToString(options.EncryptionAlgorithm.GetValue()));
        }
        if (options.EncryptionScope.HasValue())
        {
          request.AddHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
        }
        std::set<std::string> metadataKeys;
        for (const auto& pair : options.Metadata)
        {
          if (metadataKeys.insert(Azure::Core::Strings::ToLower(pair.first)).second == false)
          {
            throw std::runtime_error("duplicate keys in metadata");
          }
          request.AddHeader("x-ms-meta-" + pair.first, pair.second);
        }
        metadataKeys.clear();
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        CreateBlobSnapshotResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        auto response_server_encrypted_iterator
            = httpResponse.GetHeaders().find("x-ms-request-server-encrypted");
        if (response_server_encrypted_iterator != httpResponse.GetHeaders().end())
        {
          response.ServerEncrypted = response_server_encrypted_iterator->second == "true";
        }
        auto response_encryption_key_sha256_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryption_key_sha256_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionKeySha256 = response_encryption_key_sha256_iterator->second;
        }
        auto response_encryption_scope_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-scope");
        if (response_encryption_scope_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionScope = response_encryption_scope_iterator->second;
        }
        response.Snapshot = httpResponse.GetHeaders().at("x-ms-snapshot");
        auto response_version_id_iterator = httpResponse.GetHeaders().find("x-ms-version-id");
        if (response_version_id_iterator != httpResponse.GetHeaders().end())
        {
          response.VersionId = response_version_id_iterator->second;
        }
        return Azure::Core::Response<CreateBlobSnapshotResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct GetBlobTagsOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct GetBlobTagsOptions

      static Azure::Core::Response<GetBlobTagsResult> GetTags(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const GetBlobTagsOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.GetUrl().AppendQueryParameter("comp", "tags");
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        GetBlobTagsResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        {
          const auto& httpResponseBody = httpResponse.GetBody();
          XmlReader reader(
              reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
          response = GetBlobTagsResultFromXml(reader);
        }
        return Azure::Core::Response<GetBlobTagsResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct SetBlobTagsOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::map<std::string, std::string> Tags;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct SetBlobTagsOptions

      static Azure::Core::Response<SetBlobTagsResult> SetTags(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const SetBlobTagsOptions& options)
      {
        unused(options);
        std::string xml_body;
        {
          XmlWriter writer;
          SetBlobTagsOptionsToXml(writer, options);
          xml_body = writer.GetDocument();
          writer.Write(XmlNode{XmlNodeType::End});
        }
        Azure::Core::Http::MemoryBodyStream xml_body_stream(
            reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
        auto request
            = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, &xml_body_stream);
        request.AddHeader("Content-Length", std::to_string(xml_body_stream.Length()));
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.GetUrl().AppendQueryParameter("comp", "tags");
        request.AddHeader("Content-Type", "application/xml; charset=UTF-8");
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        SetBlobTagsResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 204))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        return Azure::Core::Response<SetBlobTagsResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct AcquireBlobLeaseOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        int32_t LeaseDuration = -1;
        Azure::Core::Nullable<std::string> ProposedLeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct AcquireBlobLeaseOptions

      static Azure::Core::Response<AcquireBlobLeaseResult> AcquireLease(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const AcquireBlobLeaseOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.GetUrl().AppendQueryParameter("comp", "lease");
        request.AddHeader("x-ms-lease-action", "acquire");
        request.AddHeader("x-ms-lease-duration", std::to_string(options.LeaseDuration));
        if (options.ProposedLeaseId.HasValue())
        {
          request.AddHeader("x-ms-proposed-lease-id", options.ProposedLeaseId.GetValue());
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        AcquireBlobLeaseResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
        return Azure::Core::Response<AcquireBlobLeaseResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct RenewBlobLeaseOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct RenewBlobLeaseOptions

      static Azure::Core::Response<RenewBlobLeaseResult> RenewLease(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const RenewBlobLeaseOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.GetUrl().AppendQueryParameter("comp", "lease");
        request.AddHeader("x-ms-lease-action", "renew");
        request.AddHeader("x-ms-lease-id", options.LeaseId);
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        RenewBlobLeaseResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
        return Azure::Core::Response<RenewBlobLeaseResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct ChangeBlobLeaseOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string LeaseId;
        std::string ProposedLeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct ChangeBlobLeaseOptions

      static Azure::Core::Response<ChangeBlobLeaseResult> ChangeLease(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const ChangeBlobLeaseOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.GetUrl().AppendQueryParameter("comp", "lease");
        request.AddHeader("x-ms-lease-action", "change");
        request.AddHeader("x-ms-lease-id", options.LeaseId);
        request.AddHeader("x-ms-proposed-lease-id", options.ProposedLeaseId);
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        ChangeBlobLeaseResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
        return Azure::Core::Response<ChangeBlobLeaseResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct ReleaseBlobLeaseOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct ReleaseBlobLeaseOptions

      static Azure::Core::Response<ReleaseBlobLeaseResult> ReleaseLease(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const ReleaseBlobLeaseOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.GetUrl().AppendQueryParameter("comp", "lease");
        request.AddHeader("x-ms-lease-action", "release");
        request.AddHeader("x-ms-lease-id", options.LeaseId);
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        ReleaseBlobLeaseResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        auto response_sequence_number_iterator
            = httpResponse.GetHeaders().find("x-ms-blob-sequence-number");
        if (response_sequence_number_iterator != httpResponse.GetHeaders().end())
        {
          response.SequenceNumber = std::stoll(response_sequence_number_iterator->second);
        }
        return Azure::Core::Response<ReleaseBlobLeaseResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct BreakBlobLeaseOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<int32_t> BreakPeriod;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct BreakBlobLeaseOptions

      static Azure::Core::Response<BreakBlobLeaseResult> BreakLease(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const BreakBlobLeaseOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.GetUrl().AppendQueryParameter("comp", "lease");
        request.AddHeader("x-ms-lease-action", "break");
        if (options.BreakPeriod.HasValue())
        {
          request.AddHeader(
              "x-ms-lease-break-period", std::to_string(options.BreakPeriod.GetValue()));
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BreakBlobLeaseResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 202))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.LeaseTime = std::stoi(httpResponse.GetHeaders().at("x-ms-lease-time"));
        return Azure::Core::Response<BreakBlobLeaseResult>(
            std::move(response), std::move(pHttpResponse));
      }

    private:
      static GetBlobTagsResult GetBlobTagsResultFromXml(XmlReader& reader)
      {
        GetBlobTagsResult ret;
        enum class XmlTagName
        {
          k_Tags,
          k_TagSet,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "Tags") == 0)
            {
              path.emplace_back(XmlTagName::k_Tags);
            }
            else if (std::strcmp(node.Name, "TagSet") == 0)
            {
              path.emplace_back(XmlTagName::k_TagSet);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
            if (path.size() == 2 && path[0] == XmlTagName::k_Tags
                && path[1] == XmlTagName::k_TagSet)
            {
              ret.Tags = TagsFromXml(reader);
              path.pop_back();
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
          }
        }
        return ret;
      }

      static std::map<std::string, std::string> TagsFromXml(XmlReader& reader)
      {
        std::map<std::string, std::string> ret;
        int depth = 0;
        std::string key;
        bool is_key = false;
        bool is_value = false;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            ++depth;
            if (strcmp(node.Name, "Key") == 0)
            {
              is_key = true;
            }
            else if (strcmp(node.Name, "Value") == 0)
            {
              is_value = true;
            }
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (depth-- == 0)
            {
              break;
            }
          }
          if (depth == 2 && node.Type == XmlNodeType::Text)
          {
            if (is_key)
            {
              key = node.Value;
              is_key = false;
            }
            else if (is_value)
            {
              ret.emplace(std::move(key), node.Value);
              is_value = false;
            }
          }
        }
        return ret;
      }

      static void SetBlobTagsOptionsToXml(XmlWriter& writer, const SetBlobTagsOptions& options)
      {
        writer.Write(XmlNode{XmlNodeType::StartTag, "Tags"});
        writer.Write(XmlNode{XmlNodeType::StartTag, "TagSet"});
        for (const auto& i : options.Tags)
        {
          writer.Write(XmlNode{XmlNodeType::StartTag, "Tag"});
          writer.Write(XmlNode{XmlNodeType::StartTag, "Key"});
          writer.Write(XmlNode{XmlNodeType::Text, nullptr, i.first.data()});
          writer.Write(XmlNode{XmlNodeType::EndTag});
          writer.Write(XmlNode{XmlNodeType::StartTag, "Value"});
          writer.Write(XmlNode{XmlNodeType::Text, nullptr, i.second.data()});
          writer.Write(XmlNode{XmlNodeType::EndTag});
          writer.Write(XmlNode{XmlNodeType::EndTag});
        }
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::EndTag});
      }

    }; // class Blob

    class BlockBlob {
    public:
      struct UploadBlockBlobOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::string> TransactionalContentMd5;
        Azure::Core::Nullable<std::string> TransactionalContentCrc64;
        BlobHttpHeaders HttpHeaders;
        std::map<std::string, std::string> Metadata;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<AccessTier> Tier;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> EncryptionScope;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct UploadBlockBlobOptions

      static Azure::Core::Response<UploadBlockBlobResult> Upload(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          Azure::Core::Http::BodyStream* requestBody,
          const UploadBlockBlobOptions& options)
      {
        unused(options);
        auto request
            = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, requestBody);
        request.AddHeader("Content-Length", std::to_string(requestBody->Length()));
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        if (options.EncryptionKey.HasValue())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
        }
        if (options.EncryptionKeySha256.HasValue())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySha256.GetValue());
        }
        if (options.EncryptionAlgorithm.HasValue())
        {
          request.AddHeader(
              "x-ms-encryption-algorithm",
              EncryptionAlgorithmTypeToString(options.EncryptionAlgorithm.GetValue()));
        }
        if (options.EncryptionScope.HasValue())
        {
          request.AddHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
        }
        if (options.TransactionalContentMd5.HasValue())
        {
          request.AddHeader("Content-MD5", options.TransactionalContentMd5.GetValue());
        }
        if (options.TransactionalContentCrc64.HasValue())
        {
          request.AddHeader("x-ms-content-crc64", options.TransactionalContentCrc64.GetValue());
        }
        if (!options.HttpHeaders.ContentType.empty())
        {
          request.AddHeader("x-ms-blob-content-type", options.HttpHeaders.ContentType);
        }
        if (!options.HttpHeaders.ContentEncoding.empty())
        {
          request.AddHeader("x-ms-blob-content-encoding", options.HttpHeaders.ContentEncoding);
        }
        if (!options.HttpHeaders.ContentLanguage.empty())
        {
          request.AddHeader("x-ms-blob-content-language", options.HttpHeaders.ContentLanguage);
        }
        if (!options.HttpHeaders.CacheControl.empty())
        {
          request.AddHeader("x-ms-blob-cache-control", options.HttpHeaders.CacheControl);
        }
        if (!options.HttpHeaders.ContentMd5.empty())
        {
          request.AddHeader("x-ms-blob-content-md5", options.HttpHeaders.ContentMd5);
        }
        if (!options.HttpHeaders.ContentDisposition.empty())
        {
          request.AddHeader(
              "x-ms-blob-content-disposition", options.HttpHeaders.ContentDisposition);
        }
        std::set<std::string> metadataKeys;
        for (const auto& pair : options.Metadata)
        {
          if (metadataKeys.insert(Azure::Core::Strings::ToLower(pair.first)).second == false)
          {
            throw std::runtime_error("duplicate keys in metadata");
          }
          request.AddHeader("x-ms-meta-" + pair.first, pair.second);
        }
        metadataKeys.clear();
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        request.AddHeader("x-ms-blob-type", "BlockBlob");
        if (options.Tier.HasValue())
        {
          request.AddHeader("x-ms-access-tier", AccessTierToString(options.Tier.GetValue()));
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        UploadBlockBlobResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        auto response_transactional_content_md5_iterator
            = httpResponse.GetHeaders().find("content-md5");
        if (response_transactional_content_md5_iterator != httpResponse.GetHeaders().end())
        {
          response.TransactionalContentMd5 = response_transactional_content_md5_iterator->second;
        }
        auto response_transactional_content_crc64_iterator
            = httpResponse.GetHeaders().find("x-ms-content-crc64");
        if (response_transactional_content_crc64_iterator != httpResponse.GetHeaders().end())
        {
          response.TransactionalContentCrc64
              = response_transactional_content_crc64_iterator->second;
        }
        auto response_version_id_iterator = httpResponse.GetHeaders().find("x-ms-version-id");
        if (response_version_id_iterator != httpResponse.GetHeaders().end())
        {
          response.VersionId = response_version_id_iterator->second;
        }
        auto response_server_encrypted_iterator
            = httpResponse.GetHeaders().find("x-ms-request-server-encrypted");
        if (response_server_encrypted_iterator != httpResponse.GetHeaders().end())
        {
          response.ServerEncrypted = response_server_encrypted_iterator->second == "true";
        }
        auto response_encryption_key_sha256_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryption_key_sha256_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionKeySha256 = response_encryption_key_sha256_iterator->second;
        }
        auto response_encryption_scope_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-scope");
        if (response_encryption_scope_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionScope = response_encryption_scope_iterator->second;
        }
        return Azure::Core::Response<UploadBlockBlobResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct StageBlockOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string BlockId;
        Azure::Core::Nullable<std::string> TransactionalContentMd5;
        Azure::Core::Nullable<std::string> TransactionalContentCrc64;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> EncryptionScope;
      }; // struct StageBlockOptions

      static Azure::Core::Response<StageBlockResult> StageBlock(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          Azure::Core::Http::BodyStream* requestBody,
          const StageBlockOptions& options)
      {
        unused(options);
        auto request
            = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, requestBody);
        request.AddHeader("Content-Length", std::to_string(requestBody->Length()));
        request.GetUrl().AppendQueryParameter("comp", "block");
        request.GetUrl().AppendQueryParameter(
            "blockid", Details::UrlEncodeQueryParameter(options.BlockId));
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        if (options.TransactionalContentMd5.HasValue())
        {
          request.AddHeader("Content-MD5", options.TransactionalContentMd5.GetValue());
        }
        if (options.TransactionalContentCrc64.HasValue())
        {
          request.AddHeader("x-ms-content-crc64", options.TransactionalContentCrc64.GetValue());
        }
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        if (options.EncryptionKey.HasValue())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
        }
        if (options.EncryptionKeySha256.HasValue())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySha256.GetValue());
        }
        if (options.EncryptionAlgorithm.HasValue())
        {
          request.AddHeader(
              "x-ms-encryption-algorithm",
              EncryptionAlgorithmTypeToString(options.EncryptionAlgorithm.GetValue()));
        }
        if (options.EncryptionScope.HasValue())
        {
          request.AddHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        StageBlockResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        auto response_transactional_content_md5_iterator
            = httpResponse.GetHeaders().find("content-md5");
        if (response_transactional_content_md5_iterator != httpResponse.GetHeaders().end())
        {
          response.TransactionalContentMd5 = response_transactional_content_md5_iterator->second;
        }
        auto response_transactional_content_crc64_iterator
            = httpResponse.GetHeaders().find("x-ms-content-crc64");
        if (response_transactional_content_crc64_iterator != httpResponse.GetHeaders().end())
        {
          response.TransactionalContentCrc64
              = response_transactional_content_crc64_iterator->second;
        }
        auto response_server_encrypted_iterator
            = httpResponse.GetHeaders().find("x-ms-request-server-encrypted");
        if (response_server_encrypted_iterator != httpResponse.GetHeaders().end())
        {
          response.ServerEncrypted = response_server_encrypted_iterator->second == "true";
        }
        auto response_encryption_key_sha256_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryption_key_sha256_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionKeySha256 = response_encryption_key_sha256_iterator->second;
        }
        auto response_encryption_scope_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-scope");
        if (response_encryption_scope_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionScope = response_encryption_scope_iterator->second;
        }
        return Azure::Core::Response<StageBlockResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct StageBlockFromUriOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string BlockId;
        std::string SourceUri;
        Azure::Core::Nullable<std::pair<int64_t, int64_t>> SourceRange;
        Azure::Core::Nullable<std::string> TransactionalContentMd5;
        Azure::Core::Nullable<std::string> TransactionalContentCrc64;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> EncryptionScope;
        Azure::Core::Nullable<std::string> SourceIfModifiedSince;
        Azure::Core::Nullable<std::string> SourceIfUnmodifiedSince;
        Azure::Core::Nullable<std::string> SourceIfMatch;
        Azure::Core::Nullable<std::string> SourceIfNoneMatch;
      }; // struct StageBlockFromUriOptions

      static Azure::Core::Response<StageBlockFromUriResult> StageBlockFromUri(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const StageBlockFromUriOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.GetUrl().AppendQueryParameter("comp", "block");
        request.GetUrl().AppendQueryParameter(
            "blockid", Details::UrlEncodeQueryParameter(options.BlockId));
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddHeader("x-ms-copy-source", options.SourceUri);
        if (options.SourceRange.HasValue())
        {
          auto startOffset = options.SourceRange.GetValue().first;
          auto endOffset = options.SourceRange.GetValue().second;
          if (endOffset != std::numeric_limits<decltype(endOffset)>::max())
          {
            request.AddHeader(
                "x-ms-source_range",
                "bytes=" + std::to_string(startOffset) + "-" + std::to_string(endOffset));
          }
          else
          {
            request.AddHeader("x-ms-source_range", "bytes=" + std::to_string(startOffset) + "-");
          }
        }
        if (options.TransactionalContentMd5.HasValue())
        {
          request.AddHeader("x-ms-source-content-md5", options.TransactionalContentMd5.GetValue());
        }
        if (options.TransactionalContentCrc64.HasValue())
        {
          request.AddHeader(
              "x-ms-source-content-crc64", options.TransactionalContentCrc64.GetValue());
        }
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        if (options.EncryptionKey.HasValue())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
        }
        if (options.EncryptionKeySha256.HasValue())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySha256.GetValue());
        }
        if (options.EncryptionAlgorithm.HasValue())
        {
          request.AddHeader(
              "x-ms-encryption-algorithm",
              EncryptionAlgorithmTypeToString(options.EncryptionAlgorithm.GetValue()));
        }
        if (options.EncryptionScope.HasValue())
        {
          request.AddHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
        }
        if (options.SourceIfModifiedSince.HasValue())
        {
          request.AddHeader(
              "x-ms-source-if-modified-since", options.SourceIfModifiedSince.GetValue());
        }
        if (options.SourceIfUnmodifiedSince.HasValue())
        {
          request.AddHeader(
              "x-ms-source-if-unmodified-since", options.SourceIfUnmodifiedSince.GetValue());
        }
        if (options.SourceIfMatch.HasValue())
        {
          request.AddHeader("x-ms-source-if-match", options.SourceIfMatch.GetValue());
        }
        if (options.SourceIfNoneMatch.HasValue())
        {
          request.AddHeader("x-ms-source-if-none-match", options.SourceIfNoneMatch.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        StageBlockFromUriResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        auto response_transactional_content_md5_iterator
            = httpResponse.GetHeaders().find("content-md5");
        if (response_transactional_content_md5_iterator != httpResponse.GetHeaders().end())
        {
          response.TransactionalContentMd5 = response_transactional_content_md5_iterator->second;
        }
        auto response_transactional_content_crc64_iterator
            = httpResponse.GetHeaders().find("x-ms-content-crc64");
        if (response_transactional_content_crc64_iterator != httpResponse.GetHeaders().end())
        {
          response.TransactionalContentCrc64
              = response_transactional_content_crc64_iterator->second;
        }
        auto response_server_encrypted_iterator
            = httpResponse.GetHeaders().find("x-ms-request-server-encrypted");
        if (response_server_encrypted_iterator != httpResponse.GetHeaders().end())
        {
          response.ServerEncrypted = response_server_encrypted_iterator->second == "true";
        }
        auto response_encryption_key_sha256_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryption_key_sha256_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionKeySha256 = response_encryption_key_sha256_iterator->second;
        }
        auto response_encryption_scope_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-scope");
        if (response_encryption_scope_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionScope = response_encryption_scope_iterator->second;
        }
        return Azure::Core::Response<StageBlockFromUriResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct CommitBlockListOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::vector<std::pair<BlockType, std::string>> BlockList;
        BlobHttpHeaders HttpHeaders;
        std::map<std::string, std::string> Metadata;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> EncryptionScope;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
        Azure::Core::Nullable<AccessTier> Tier;
      }; // struct CommitBlockListOptions

      static Azure::Core::Response<CommitBlockListResult> CommitBlockList(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const CommitBlockListOptions& options)
      {
        unused(options);
        std::string xml_body;
        {
          XmlWriter writer;
          CommitBlockListOptionsToXml(writer, options);
          xml_body = writer.GetDocument();
          writer.Write(XmlNode{XmlNodeType::End});
        }
        Azure::Core::Http::MemoryBodyStream xml_body_stream(
            reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
        auto request
            = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, &xml_body_stream);
        request.AddHeader("Content-Length", std::to_string(xml_body_stream.Length()));
        request.GetUrl().AppendQueryParameter("comp", "blocklist");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        if (!options.HttpHeaders.ContentType.empty())
        {
          request.AddHeader("x-ms-blob-content-type", options.HttpHeaders.ContentType);
        }
        if (!options.HttpHeaders.ContentEncoding.empty())
        {
          request.AddHeader("x-ms-blob-content-encoding", options.HttpHeaders.ContentEncoding);
        }
        if (!options.HttpHeaders.ContentLanguage.empty())
        {
          request.AddHeader("x-ms-blob-content-language", options.HttpHeaders.ContentLanguage);
        }
        if (!options.HttpHeaders.CacheControl.empty())
        {
          request.AddHeader("x-ms-blob-cache-control", options.HttpHeaders.CacheControl);
        }
        if (!options.HttpHeaders.ContentMd5.empty())
        {
          request.AddHeader("x-ms-blob-content-md5", options.HttpHeaders.ContentMd5);
        }
        if (!options.HttpHeaders.ContentDisposition.empty())
        {
          request.AddHeader(
              "x-ms-blob-content-disposition", options.HttpHeaders.ContentDisposition);
        }
        std::set<std::string> metadataKeys;
        for (const auto& pair : options.Metadata)
        {
          if (metadataKeys.insert(Azure::Core::Strings::ToLower(pair.first)).second == false)
          {
            throw std::runtime_error("duplicate keys in metadata");
          }
          request.AddHeader("x-ms-meta-" + pair.first, pair.second);
        }
        metadataKeys.clear();
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        if (options.EncryptionKey.HasValue())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
        }
        if (options.EncryptionKeySha256.HasValue())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySha256.GetValue());
        }
        if (options.EncryptionAlgorithm.HasValue())
        {
          request.AddHeader(
              "x-ms-encryption-algorithm",
              EncryptionAlgorithmTypeToString(options.EncryptionAlgorithm.GetValue()));
        }
        if (options.EncryptionScope.HasValue())
        {
          request.AddHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
        }
        if (options.Tier.HasValue())
        {
          request.AddHeader("x-ms-access-tier", AccessTierToString(options.Tier.GetValue()));
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        CommitBlockListResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        auto response_version_id_iterator = httpResponse.GetHeaders().find("x-ms-version-id");
        if (response_version_id_iterator != httpResponse.GetHeaders().end())
        {
          response.VersionId = response_version_id_iterator->second;
        }
        auto response_server_encrypted_iterator
            = httpResponse.GetHeaders().find("x-ms-request-server-encrypted");
        if (response_server_encrypted_iterator != httpResponse.GetHeaders().end())
        {
          response.ServerEncrypted = response_server_encrypted_iterator->second == "true";
        }
        auto response_encryption_key_sha256_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryption_key_sha256_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionKeySha256 = response_encryption_key_sha256_iterator->second;
        }
        auto response_encryption_scope_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-scope");
        if (response_encryption_scope_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionScope = response_encryption_scope_iterator->second;
        }
        return Azure::Core::Response<CommitBlockListResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct GetBlockListOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<BlockListTypeOption> ListType;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct GetBlockListOptions

      static Azure::Core::Response<GetBlockListResult> GetBlockList(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const GetBlockListOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.GetUrl().AppendQueryParameter("comp", "blocklist");
        if (options.ListType.HasValue())
        {
          std::string block_list_type_option
              = BlockListTypeOptionToString(options.ListType.GetValue());
          request.GetUrl().AppendQueryParameter(
              "blocklisttype", Details::UrlEncodeQueryParameter(block_list_type_option));
        }
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        GetBlockListResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        {
          const auto& httpResponseBody = httpResponse.GetBody();
          XmlReader reader(
              reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
          response = GetBlockListResultFromXml(reader);
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.ContentType = httpResponse.GetHeaders().at("content-type");
        response.ContentLength
            = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-content-length"));
        return Azure::Core::Response<GetBlockListResult>(
            std::move(response), std::move(pHttpResponse));
      }

    private:
      static GetBlockListResult GetBlockListResultFromXml(XmlReader& reader)
      {
        GetBlockListResult ret;
        enum class XmlTagName
        {
          k_BlockList,
          k_CommittedBlocks,
          k_Block,
          k_UncommittedBlocks,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "BlockList") == 0)
            {
              path.emplace_back(XmlTagName::k_BlockList);
            }
            else if (std::strcmp(node.Name, "CommittedBlocks") == 0)
            {
              path.emplace_back(XmlTagName::k_CommittedBlocks);
            }
            else if (std::strcmp(node.Name, "Block") == 0)
            {
              path.emplace_back(XmlTagName::k_Block);
            }
            else if (std::strcmp(node.Name, "UncommittedBlocks") == 0)
            {
              path.emplace_back(XmlTagName::k_UncommittedBlocks);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
            if (path.size() == 3 && path[0] == XmlTagName::k_BlockList
                && path[1] == XmlTagName::k_CommittedBlocks && path[2] == XmlTagName::k_Block)
            {
              ret.CommittedBlocks.emplace_back(BlobBlockFromXml(reader));
              path.pop_back();
            }
            else if (
                path.size() == 3 && path[0] == XmlTagName::k_BlockList
                && path[1] == XmlTagName::k_UncommittedBlocks && path[2] == XmlTagName::k_Block)
            {
              ret.UncommittedBlocks.emplace_back(BlobBlockFromXml(reader));
              path.pop_back();
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
          }
        }
        return ret;
      }

      static BlobBlock BlobBlockFromXml(XmlReader& reader)
      {
        BlobBlock ret;
        enum class XmlTagName
        {
          k_Name,
          k_Size,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "Name") == 0)
            {
              path.emplace_back(XmlTagName::k_Name);
            }
            else if (std::strcmp(node.Name, "Size") == 0)
            {
              path.emplace_back(XmlTagName::k_Size);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
            if (path.size() == 1 && path[0] == XmlTagName::k_Name)
            {
              ret.Name = node.Value;
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_Size)
            {
              ret.Size = std::stoll(node.Value);
            }
          }
        }
        return ret;
      }

      static void CommitBlockListOptionsToXml(
          XmlWriter& writer,
          const CommitBlockListOptions& options)
      {
        writer.Write(XmlNode{XmlNodeType::StartTag, "BlockList"});
        for (const auto& i : options.BlockList)
        {
          writer.Write(
              XmlNode{XmlNodeType::StartTag, BlockTypeToString(i.first).data(), i.second.data()});
        }
        writer.Write(XmlNode{XmlNodeType::EndTag});
      }

    }; // class BlockBlob

    class PageBlob {
    public:
      struct CreatePageBlobOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        int64_t BlobContentLength = -1;
        Azure::Core::Nullable<int64_t> SequenceNumber;
        BlobHttpHeaders HttpHeaders;
        std::map<std::string, std::string> Metadata;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<AccessTier> Tier;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> EncryptionScope;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct CreatePageBlobOptions

      static Azure::Core::Response<CreatePageBlobResult> Create(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const CreatePageBlobOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        if (!options.HttpHeaders.ContentType.empty())
        {
          request.AddHeader("x-ms-blob-content-type", options.HttpHeaders.ContentType);
        }
        if (!options.HttpHeaders.ContentEncoding.empty())
        {
          request.AddHeader("x-ms-blob-content-encoding", options.HttpHeaders.ContentEncoding);
        }
        if (!options.HttpHeaders.ContentLanguage.empty())
        {
          request.AddHeader("x-ms-blob-content-language", options.HttpHeaders.ContentLanguage);
        }
        if (!options.HttpHeaders.CacheControl.empty())
        {
          request.AddHeader("x-ms-blob-cache-control", options.HttpHeaders.CacheControl);
        }
        if (!options.HttpHeaders.ContentMd5.empty())
        {
          request.AddHeader("x-ms-blob-content-md5", options.HttpHeaders.ContentMd5);
        }
        if (!options.HttpHeaders.ContentDisposition.empty())
        {
          request.AddHeader(
              "x-ms-blob-content-disposition", options.HttpHeaders.ContentDisposition);
        }
        std::set<std::string> metadataKeys;
        for (const auto& pair : options.Metadata)
        {
          if (metadataKeys.insert(Azure::Core::Strings::ToLower(pair.first)).second == false)
          {
            throw std::runtime_error("duplicate keys in metadata");
          }
          request.AddHeader("x-ms-meta-" + pair.first, pair.second);
        }
        metadataKeys.clear();
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        request.AddHeader("x-ms-blob-type", "PageBlob");
        request.AddHeader("x-ms-blob-content-length", std::to_string(options.BlobContentLength));
        if (options.SequenceNumber.HasValue())
        {
          request.AddHeader(
              "x-ms-blob-sequence-number", std::to_string(options.SequenceNumber.GetValue()));
        }
        if (options.Tier.HasValue())
        {
          request.AddHeader("x-ms-access-tier", AccessTierToString(options.Tier.GetValue()));
        }
        if (options.EncryptionKey.HasValue())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
        }
        if (options.EncryptionKeySha256.HasValue())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySha256.GetValue());
        }
        if (options.EncryptionAlgorithm.HasValue())
        {
          request.AddHeader(
              "x-ms-encryption-algorithm",
              EncryptionAlgorithmTypeToString(options.EncryptionAlgorithm.GetValue()));
        }
        if (options.EncryptionScope.HasValue())
        {
          request.AddHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        CreatePageBlobResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        auto response_version_id_iterator = httpResponse.GetHeaders().find("x-ms-version-id");
        if (response_version_id_iterator != httpResponse.GetHeaders().end())
        {
          response.VersionId = response_version_id_iterator->second;
        }
        auto response_server_encrypted_iterator
            = httpResponse.GetHeaders().find("x-ms-request-server-encrypted");
        if (response_server_encrypted_iterator != httpResponse.GetHeaders().end())
        {
          response.ServerEncrypted = response_server_encrypted_iterator->second == "true";
        }
        auto response_encryption_key_sha256_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryption_key_sha256_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionKeySha256 = response_encryption_key_sha256_iterator->second;
        }
        auto response_encryption_scope_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-scope");
        if (response_encryption_scope_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionScope = response_encryption_scope_iterator->second;
        }
        return Azure::Core::Response<CreatePageBlobResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct UploadPageBlobPagesOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::pair<int64_t, int64_t> Range;
        Azure::Core::Nullable<std::string> TransactionalContentMd5;
        Azure::Core::Nullable<std::string> TransactionalContentCrc64;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<int64_t> IfSequenceNumberLessThanOrEqualTo;
        Azure::Core::Nullable<int64_t> IfSequenceNumberLessThan;
        Azure::Core::Nullable<int64_t> IfSequenceNumberEqualTo;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> EncryptionScope;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct UploadPageBlobPagesOptions

      static Azure::Core::Response<UploadPageBlobPagesResult> UploadPages(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          Azure::Core::Http::BodyStream* requestBody,
          const UploadPageBlobPagesOptions& options)
      {
        unused(options);
        auto request
            = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, requestBody);
        request.AddHeader("Content-Length", std::to_string(requestBody->Length()));
        request.GetUrl().AppendQueryParameter("comp", "page");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddHeader(
            "x-ms-range",
            "bytes=" + std::to_string(options.Range.first) + "-"
                + std::to_string(options.Range.second));
        if (options.TransactionalContentMd5.HasValue())
        {
          request.AddHeader("Content-MD5", options.TransactionalContentMd5.GetValue());
        }
        if (options.TransactionalContentCrc64.HasValue())
        {
          request.AddHeader("x-ms-content-crc64", options.TransactionalContentCrc64.GetValue());
        }
        request.AddHeader("x-ms-page-write", "update");
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        if (options.IfSequenceNumberLessThanOrEqualTo.HasValue())
        {
          request.AddHeader(
              "x-ms-if-sequence-number-le",
              std::to_string(options.IfSequenceNumberLessThanOrEqualTo.GetValue()));
        }
        if (options.IfSequenceNumberLessThan.HasValue())
        {
          request.AddHeader(
              "x-ms-if-sequence-number-lt",
              std::to_string(options.IfSequenceNumberLessThan.GetValue()));
        }
        if (options.IfSequenceNumberEqualTo.HasValue())
        {
          request.AddHeader(
              "x-ms-if-sequence-number-eq",
              std::to_string(options.IfSequenceNumberEqualTo.GetValue()));
        }
        if (options.EncryptionKey.HasValue())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
        }
        if (options.EncryptionKeySha256.HasValue())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySha256.GetValue());
        }
        if (options.EncryptionAlgorithm.HasValue())
        {
          request.AddHeader(
              "x-ms-encryption-algorithm",
              EncryptionAlgorithmTypeToString(options.EncryptionAlgorithm.GetValue()));
        }
        if (options.EncryptionScope.HasValue())
        {
          request.AddHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        UploadPageBlobPagesResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        auto response_transactional_content_md5_iterator
            = httpResponse.GetHeaders().find("content-md5");
        if (response_transactional_content_md5_iterator != httpResponse.GetHeaders().end())
        {
          response.TransactionalContentMd5 = response_transactional_content_md5_iterator->second;
        }
        auto response_transactional_content_crc64_iterator
            = httpResponse.GetHeaders().find("x-ms-content-crc64");
        if (response_transactional_content_crc64_iterator != httpResponse.GetHeaders().end())
        {
          response.TransactionalContentCrc64
              = response_transactional_content_crc64_iterator->second;
        }
        response.SequenceNumber
            = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-sequence-number"));
        auto response_server_encrypted_iterator
            = httpResponse.GetHeaders().find("x-ms-request-server-encrypted");
        if (response_server_encrypted_iterator != httpResponse.GetHeaders().end())
        {
          response.ServerEncrypted = response_server_encrypted_iterator->second == "true";
        }
        auto response_encryption_key_sha256_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryption_key_sha256_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionKeySha256 = response_encryption_key_sha256_iterator->second;
        }
        auto response_encryption_scope_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-scope");
        if (response_encryption_scope_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionScope = response_encryption_scope_iterator->second;
        }
        return Azure::Core::Response<UploadPageBlobPagesResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct UploadPageBlobPagesFromUriOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string SourceUri;
        std::pair<int64_t, int64_t> SourceRange;
        std::pair<int64_t, int64_t> Range;
        Azure::Core::Nullable<std::string> TransactionalContentMd5;
        Azure::Core::Nullable<std::string> TransactionalContentCrc64;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<int64_t> IfSequenceNumberLessThanOrEqualTo;
        Azure::Core::Nullable<int64_t> IfSequenceNumberLessThan;
        Azure::Core::Nullable<int64_t> IfSequenceNumberEqualTo;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> EncryptionScope;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct UploadPageBlobPagesFromUriOptions

      static Azure::Core::Response<UploadPageBlobPagesFromUriResult> UploadPagesFromUri(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const UploadPageBlobPagesFromUriOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.GetUrl().AppendQueryParameter("comp", "page");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddHeader(
            "x-ms-range",
            "bytes=" + std::to_string(options.Range.first) + "-"
                + std::to_string(options.Range.second));
        request.AddHeader("x-ms-copy-source", options.SourceUri);
        request.AddHeader(
            "x-ms-source-range",
            "bytes=" + std::to_string(options.SourceRange.first) + "-"
                + std::to_string(options.SourceRange.second));
        if (options.TransactionalContentMd5.HasValue())
        {
          request.AddHeader("x-ms-source-content-md5", options.TransactionalContentMd5.GetValue());
        }
        if (options.TransactionalContentCrc64.HasValue())
        {
          request.AddHeader(
              "x-ms-source-content-crc64", options.TransactionalContentCrc64.GetValue());
        }
        request.AddHeader("x-ms-page-write", "update");
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        if (options.IfSequenceNumberLessThanOrEqualTo.HasValue())
        {
          request.AddHeader(
              "x-ms-if-sequence-number-le",
              std::to_string(options.IfSequenceNumberLessThanOrEqualTo.GetValue()));
        }
        if (options.IfSequenceNumberLessThan.HasValue())
        {
          request.AddHeader(
              "x-ms-if-sequence-number-lt",
              std::to_string(options.IfSequenceNumberLessThan.GetValue()));
        }
        if (options.IfSequenceNumberEqualTo.HasValue())
        {
          request.AddHeader(
              "x-ms-if-sequence-number-eq",
              std::to_string(options.IfSequenceNumberEqualTo.GetValue()));
        }
        if (options.EncryptionKey.HasValue())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
        }
        if (options.EncryptionKeySha256.HasValue())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySha256.GetValue());
        }
        if (options.EncryptionAlgorithm.HasValue())
        {
          request.AddHeader(
              "x-ms-encryption-algorithm",
              EncryptionAlgorithmTypeToString(options.EncryptionAlgorithm.GetValue()));
        }
        if (options.EncryptionScope.HasValue())
        {
          request.AddHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        UploadPageBlobPagesFromUriResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        auto response_transactional_content_md5_iterator
            = httpResponse.GetHeaders().find("content-md5");
        if (response_transactional_content_md5_iterator != httpResponse.GetHeaders().end())
        {
          response.TransactionalContentMd5 = response_transactional_content_md5_iterator->second;
        }
        auto response_transactional_content_crc64_iterator
            = httpResponse.GetHeaders().find("x-ms-content-crc64");
        if (response_transactional_content_crc64_iterator != httpResponse.GetHeaders().end())
        {
          response.TransactionalContentCrc64
              = response_transactional_content_crc64_iterator->second;
        }
        response.SequenceNumber
            = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-sequence-number"));
        auto response_server_encrypted_iterator
            = httpResponse.GetHeaders().find("x-ms-request-server-encrypted");
        if (response_server_encrypted_iterator != httpResponse.GetHeaders().end())
        {
          response.ServerEncrypted = response_server_encrypted_iterator->second == "true";
        }
        auto response_encryption_key_sha256_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryption_key_sha256_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionKeySha256 = response_encryption_key_sha256_iterator->second;
        }
        auto response_encryption_scope_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-scope");
        if (response_encryption_scope_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionScope = response_encryption_scope_iterator->second;
        }
        return Azure::Core::Response<UploadPageBlobPagesFromUriResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct ClearPageBlobPagesOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::pair<int64_t, int64_t> Range;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<int64_t> IfSequenceNumberLessThanOrEqualTo;
        Azure::Core::Nullable<int64_t> IfSequenceNumberLessThan;
        Azure::Core::Nullable<int64_t> IfSequenceNumberEqualTo;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> EncryptionScope;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct ClearPageBlobPagesOptions

      static Azure::Core::Response<ClearPageBlobPagesResult> ClearPages(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const ClearPageBlobPagesOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.GetUrl().AppendQueryParameter("comp", "page");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddHeader(
            "x-ms-range",
            "bytes=" + std::to_string(options.Range.first) + "-"
                + std::to_string(options.Range.second));
        request.AddHeader("x-ms-page-write", "clear");
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        if (options.IfSequenceNumberLessThanOrEqualTo.HasValue())
        {
          request.AddHeader(
              "x-ms-if-sequence-number-le",
              std::to_string(options.IfSequenceNumberLessThanOrEqualTo.GetValue()));
        }
        if (options.IfSequenceNumberLessThan.HasValue())
        {
          request.AddHeader(
              "x-ms-if-sequence-number-lt",
              std::to_string(options.IfSequenceNumberLessThan.GetValue()));
        }
        if (options.IfSequenceNumberEqualTo.HasValue())
        {
          request.AddHeader(
              "x-ms-if-sequence-number-eq",
              std::to_string(options.IfSequenceNumberEqualTo.GetValue()));
        }
        if (options.EncryptionKey.HasValue())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
        }
        if (options.EncryptionKeySha256.HasValue())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySha256.GetValue());
        }
        if (options.EncryptionAlgorithm.HasValue())
        {
          request.AddHeader(
              "x-ms-encryption-algorithm",
              EncryptionAlgorithmTypeToString(options.EncryptionAlgorithm.GetValue()));
        }
        if (options.EncryptionScope.HasValue())
        {
          request.AddHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        ClearPageBlobPagesResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.SequenceNumber
            = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-sequence-number"));
        auto response_server_encrypted_iterator
            = httpResponse.GetHeaders().find("x-ms-request-server-encrypted");
        if (response_server_encrypted_iterator != httpResponse.GetHeaders().end())
        {
          response.ServerEncrypted = response_server_encrypted_iterator->second == "true";
        }
        auto response_encryption_key_sha256_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryption_key_sha256_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionKeySha256 = response_encryption_key_sha256_iterator->second;
        }
        auto response_encryption_scope_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-scope");
        if (response_encryption_scope_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionScope = response_encryption_scope_iterator->second;
        }
        return Azure::Core::Response<ClearPageBlobPagesResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct ResizePageBlobOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        int64_t BlobContentLength = -1;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<int64_t> IfSequenceNumberLessThanOrEqualTo;
        Azure::Core::Nullable<int64_t> IfSequenceNumberLessThan;
        Azure::Core::Nullable<int64_t> IfSequenceNumberEqualTo;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> EncryptionScope;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct ResizePageBlobOptions

      static Azure::Core::Response<ResizePageBlobResult> Resize(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const ResizePageBlobOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.GetUrl().AppendQueryParameter("comp", "properties");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddHeader("x-ms-blob-content-length", std::to_string(options.BlobContentLength));
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        if (options.IfSequenceNumberLessThanOrEqualTo.HasValue())
        {
          request.AddHeader(
              "x-ms-if-sequence-number-le",
              std::to_string(options.IfSequenceNumberLessThanOrEqualTo.GetValue()));
        }
        if (options.IfSequenceNumberLessThan.HasValue())
        {
          request.AddHeader(
              "x-ms-if-sequence-number-lt",
              std::to_string(options.IfSequenceNumberLessThan.GetValue()));
        }
        if (options.IfSequenceNumberEqualTo.HasValue())
        {
          request.AddHeader(
              "x-ms-if-sequence-number-eq",
              std::to_string(options.IfSequenceNumberEqualTo.GetValue()));
        }
        if (options.EncryptionKey.HasValue())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
        }
        if (options.EncryptionKeySha256.HasValue())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySha256.GetValue());
        }
        if (options.EncryptionAlgorithm.HasValue())
        {
          request.AddHeader(
              "x-ms-encryption-algorithm",
              EncryptionAlgorithmTypeToString(options.EncryptionAlgorithm.GetValue()));
        }
        if (options.EncryptionScope.HasValue())
        {
          request.AddHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        ResizePageBlobResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.SequenceNumber
            = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-sequence-number"));
        return Azure::Core::Response<ResizePageBlobResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct GetPageBlobPageRangesOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::string> PreviousSnapshot;
        Azure::Core::Nullable<std::string> PreviousSnapshotUrl;
        Azure::Core::Nullable<std::pair<int64_t, int64_t>> Range;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct GetPageBlobPageRangesOptions

      static Azure::Core::Response<GetPageBlobPageRangesResultInternal> GetPageRanges(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const GetPageBlobPageRangesOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.GetUrl().AppendQueryParameter("comp", "pagelist");
        if (options.PreviousSnapshot.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "prevsnapshot",
              Details::UrlEncodeQueryParameter(options.PreviousSnapshot.GetValue()));
        }
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        if (options.Range.HasValue())
        {
          auto startOffset = options.Range.GetValue().first;
          auto endOffset = options.Range.GetValue().second;
          if (endOffset != std::numeric_limits<decltype(endOffset)>::max())
          {
            request.AddHeader(
                "x-ms-range",
                "bytes=" + std::to_string(startOffset) + "-" + std::to_string(endOffset));
          }
          else
          {
            request.AddHeader("x-ms-range", "bytes=" + std::to_string(startOffset) + "-");
          }
        }
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        if (options.PreviousSnapshotUrl.HasValue())
        {
          request.AddHeader("x-ms-previous-snapshot-url", options.PreviousSnapshotUrl.GetValue());
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        GetPageBlobPageRangesResultInternal response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        {
          const auto& httpResponseBody = httpResponse.GetBody();
          XmlReader reader(
              reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
          response = GetPageBlobPageRangesResultInternalFromXml(reader);
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.BlobContentLength
            = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-content-length"));
        return Azure::Core::Response<GetPageBlobPageRangesResultInternal>(
            std::move(response), std::move(pHttpResponse));
      }

      struct StartCopyPageBlobIncrementalOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string CopySource;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct StartCopyPageBlobIncrementalOptions

      static Azure::Core::Response<StartCopyPageBlobIncrementalResult> StartCopyIncremental(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const StartCopyPageBlobIncrementalOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.GetUrl().AppendQueryParameter("comp", "incrementalcopy");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddHeader("x-ms-copy-source", options.CopySource);
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        StartCopyPageBlobIncrementalResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 202))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.CopyId = httpResponse.GetHeaders().at("x-ms-copy-id");
        response.CopyStatus
            = CopyStatusFromString(httpResponse.GetHeaders().at("x-ms-copy-status"));
        auto response_version_id_iterator = httpResponse.GetHeaders().find("x-ms-version-id");
        if (response_version_id_iterator != httpResponse.GetHeaders().end())
        {
          response.VersionId = response_version_id_iterator->second;
        }
        return Azure::Core::Response<StartCopyPageBlobIncrementalResult>(
            std::move(response), std::move(pHttpResponse));
      }

    private:
      static GetPageBlobPageRangesResultInternal GetPageBlobPageRangesResultInternalFromXml(
          XmlReader& reader)
      {
        GetPageBlobPageRangesResultInternal ret;
        enum class XmlTagName
        {
          k_PageList,
          k_PageRange,
          k_ClearRange,
          k_Unknown,
        };
        std::vector<XmlTagName> path;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "PageList") == 0)
            {
              path.emplace_back(XmlTagName::k_PageList);
            }
            else if (std::strcmp(node.Name, "PageRange") == 0)
            {
              path.emplace_back(XmlTagName::k_PageRange);
            }
            else if (std::strcmp(node.Name, "ClearRange") == 0)
            {
              path.emplace_back(XmlTagName::k_ClearRange);
            }
            else
            {
              path.emplace_back(XmlTagName::k_Unknown);
            }
            if (path.size() == 2 && path[0] == XmlTagName::k_PageList
                && path[1] == XmlTagName::k_PageRange)
            {
              ret.PageRanges.emplace_back(PageRangesFromXml(reader));
              path.pop_back();
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_PageList
                && path[1] == XmlTagName::k_ClearRange)
            {
              ret.ClearRanges.emplace_back(ClearRangesFromXml(reader));
              path.pop_back();
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
          }
        }
        return ret;
      }

      static std::pair<int64_t, int64_t> ClearRangesFromXml(XmlReader& reader)
      {
        int depth = 0;
        bool is_start = false;
        bool is_end = false;
        int64_t start = 0;
        int64_t end = 0;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::StartTag && strcmp(node.Name, "Start") == 0)
          {
            ++depth;
            is_start = true;
          }
          else if (node.Type == XmlNodeType::StartTag && strcmp(node.Name, "End") == 0)
          {
            ++depth;
            is_end = true;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            is_start = false;
            is_end = false;
            if (depth-- == 0)
            {
              break;
            }
          }
          if (depth == 1 && node.Type == XmlNodeType::Text)
          {
            if (is_start)
            {
              start = std::stoll(node.Value);
            }
            else if (is_end)
            {
              end = std::stoll(node.Value);
            }
          }
        }
        return std::make_pair(start, end);
      }

      static std::pair<int64_t, int64_t> PageRangesFromXml(XmlReader& reader)
      {
        int depth = 0;
        bool is_start = false;
        bool is_end = false;
        int64_t start = 0;
        int64_t end = 0;
        while (true)
        {
          auto node = reader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::StartTag && strcmp(node.Name, "Start") == 0)
          {
            ++depth;
            is_start = true;
          }
          else if (node.Type == XmlNodeType::StartTag && strcmp(node.Name, "End") == 0)
          {
            ++depth;
            is_end = true;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            is_start = false;
            is_end = false;
            if (depth-- == 0)
            {
              break;
            }
          }
          if (depth == 1 && node.Type == XmlNodeType::Text)
          {
            if (is_start)
            {
              start = std::stoll(node.Value);
            }
            else if (is_end)
            {
              end = std::stoll(node.Value);
            }
          }
        }
        return std::make_pair(start, end);
      }

    }; // class PageBlob

    class AppendBlob {
    public:
      struct CreateAppendBlobOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        BlobHttpHeaders HttpHeaders;
        std::map<std::string, std::string> Metadata;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> EncryptionScope;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct CreateAppendBlobOptions

      static Azure::Core::Response<CreateAppendBlobResult> Create(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const CreateAppendBlobOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        if (!options.HttpHeaders.ContentType.empty())
        {
          request.AddHeader("x-ms-blob-content-type", options.HttpHeaders.ContentType);
        }
        if (!options.HttpHeaders.ContentEncoding.empty())
        {
          request.AddHeader("x-ms-blob-content-encoding", options.HttpHeaders.ContentEncoding);
        }
        if (!options.HttpHeaders.ContentLanguage.empty())
        {
          request.AddHeader("x-ms-blob-content-language", options.HttpHeaders.ContentLanguage);
        }
        if (!options.HttpHeaders.CacheControl.empty())
        {
          request.AddHeader("x-ms-blob-cache-control", options.HttpHeaders.CacheControl);
        }
        if (!options.HttpHeaders.ContentMd5.empty())
        {
          request.AddHeader("x-ms-blob-content-md5", options.HttpHeaders.ContentMd5);
        }
        if (!options.HttpHeaders.ContentDisposition.empty())
        {
          request.AddHeader(
              "x-ms-blob-content-disposition", options.HttpHeaders.ContentDisposition);
        }
        std::set<std::string> metadataKeys;
        for (const auto& pair : options.Metadata)
        {
          if (metadataKeys.insert(Azure::Core::Strings::ToLower(pair.first)).second == false)
          {
            throw std::runtime_error("duplicate keys in metadata");
          }
          request.AddHeader("x-ms-meta-" + pair.first, pair.second);
        }
        metadataKeys.clear();
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        request.AddHeader("x-ms-blob-type", "AppendBlob");
        if (options.EncryptionKey.HasValue())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
        }
        if (options.EncryptionKeySha256.HasValue())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySha256.GetValue());
        }
        if (options.EncryptionAlgorithm.HasValue())
        {
          request.AddHeader(
              "x-ms-encryption-algorithm",
              EncryptionAlgorithmTypeToString(options.EncryptionAlgorithm.GetValue()));
        }
        if (options.EncryptionScope.HasValue())
        {
          request.AddHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        CreateAppendBlobResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        auto response_version_id_iterator = httpResponse.GetHeaders().find("x-ms-version-id");
        if (response_version_id_iterator != httpResponse.GetHeaders().end())
        {
          response.VersionId = response_version_id_iterator->second;
        }
        auto response_server_encrypted_iterator
            = httpResponse.GetHeaders().find("x-ms-request-server-encrypted");
        if (response_server_encrypted_iterator != httpResponse.GetHeaders().end())
        {
          response.ServerEncrypted = response_server_encrypted_iterator->second == "true";
        }
        auto response_encryption_key_sha256_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryption_key_sha256_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionKeySha256 = response_encryption_key_sha256_iterator->second;
        }
        auto response_encryption_scope_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-scope");
        if (response_encryption_scope_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionScope = response_encryption_scope_iterator->second;
        }
        return Azure::Core::Response<CreateAppendBlobResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct AppendBlockOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::string> TransactionalContentMd5;
        Azure::Core::Nullable<std::string> TransactionalContentCrc64;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<int64_t> MaxSize;
        Azure::Core::Nullable<int64_t> AppendPosition;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> EncryptionScope;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct AppendBlockOptions

      static Azure::Core::Response<AppendBlockResult> AppendBlock(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          Azure::Core::Http::BodyStream* requestBody,
          const AppendBlockOptions& options)
      {
        unused(options);
        auto request
            = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, requestBody);
        request.AddHeader("Content-Length", std::to_string(requestBody->Length()));
        request.GetUrl().AppendQueryParameter("comp", "appendblock");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        if (options.TransactionalContentMd5.HasValue())
        {
          request.AddHeader("Content-MD5", options.TransactionalContentMd5.GetValue());
        }
        if (options.TransactionalContentCrc64.HasValue())
        {
          request.AddHeader("x-ms-content-crc64", options.TransactionalContentCrc64.GetValue());
        }
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        if (options.MaxSize.HasValue())
        {
          request.AddHeader(
              "x-ms-blob-condition-maxsize", std::to_string(options.MaxSize.GetValue()));
        }
        if (options.AppendPosition.HasValue())
        {
          request.AddHeader(
              "x-ms-blob-condition-appendpos", std::to_string(options.AppendPosition.GetValue()));
        }
        if (options.EncryptionKey.HasValue())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
        }
        if (options.EncryptionKeySha256.HasValue())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySha256.GetValue());
        }
        if (options.EncryptionAlgorithm.HasValue())
        {
          request.AddHeader(
              "x-ms-encryption-algorithm",
              EncryptionAlgorithmTypeToString(options.EncryptionAlgorithm.GetValue()));
        }
        if (options.EncryptionScope.HasValue())
        {
          request.AddHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        AppendBlockResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        auto response_transactional_content_md5_iterator
            = httpResponse.GetHeaders().find("content-md5");
        if (response_transactional_content_md5_iterator != httpResponse.GetHeaders().end())
        {
          response.TransactionalContentMd5 = response_transactional_content_md5_iterator->second;
        }
        auto response_transactional_content_crc64_iterator
            = httpResponse.GetHeaders().find("x-ms-content-crc64");
        if (response_transactional_content_crc64_iterator != httpResponse.GetHeaders().end())
        {
          response.TransactionalContentCrc64
              = response_transactional_content_crc64_iterator->second;
        }
        response.AppendOffset = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-append-offset"));
        response.CommittedBlockCount
            = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-committed-block-count"));
        auto response_server_encrypted_iterator
            = httpResponse.GetHeaders().find("x-ms-request-server-encrypted");
        if (response_server_encrypted_iterator != httpResponse.GetHeaders().end())
        {
          response.ServerEncrypted = response_server_encrypted_iterator->second == "true";
        }
        auto response_encryption_key_sha256_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryption_key_sha256_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionKeySha256 = response_encryption_key_sha256_iterator->second;
        }
        auto response_encryption_scope_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-scope");
        if (response_encryption_scope_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionScope = response_encryption_scope_iterator->second;
        }
        return Azure::Core::Response<AppendBlockResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct AppendBlockFromUriOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string SourceUri;
        Azure::Core::Nullable<std::pair<int64_t, int64_t>> SourceRange;
        Azure::Core::Nullable<std::string> TransactionalContentMd5;
        Azure::Core::Nullable<std::string> TransactionalContentCrc64;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<int64_t> MaxSize;
        Azure::Core::Nullable<int64_t> AppendPosition;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> EncryptionScope;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
      }; // struct AppendBlockFromUriOptions

      static Azure::Core::Response<AppendBlockFromUriResult> AppendBlockFromUri(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const AppendBlockFromUriOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.GetUrl().AppendQueryParameter("comp", "appendblock");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddHeader("x-ms-copy-source", options.SourceUri);
        if (options.SourceRange.HasValue())
        {
          auto startOffset = options.SourceRange.GetValue().first;
          auto endOffset = options.SourceRange.GetValue().second;
          if (endOffset != std::numeric_limits<decltype(endOffset)>::max())
          {
            request.AddHeader(
                "x-ms-source-range",
                "bytes=" + std::to_string(startOffset) + "-" + std::to_string(endOffset));
          }
          else
          {
            request.AddHeader("x-ms-source-range", "bytes=" + std::to_string(startOffset) + "-");
          }
        }
        if (options.TransactionalContentMd5.HasValue())
        {
          request.AddHeader("x-ms-source-content-md5", options.TransactionalContentMd5.GetValue());
        }
        if (options.TransactionalContentCrc64.HasValue())
        {
          request.AddHeader(
              "x-ms-source-content-crc64", options.TransactionalContentCrc64.GetValue());
        }
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        if (options.MaxSize.HasValue())
        {
          request.AddHeader(
              "x-ms-blob-condition-maxsize", std::to_string(options.MaxSize.GetValue()));
        }
        if (options.AppendPosition.HasValue())
        {
          request.AddHeader(
              "x-ms-blob-condition-appendpos", std::to_string(options.AppendPosition.GetValue()));
        }
        if (options.EncryptionKey.HasValue())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
        }
        if (options.EncryptionKeySha256.HasValue())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySha256.GetValue());
        }
        if (options.EncryptionAlgorithm.HasValue())
        {
          request.AddHeader(
              "x-ms-encryption-algorithm",
              EncryptionAlgorithmTypeToString(options.EncryptionAlgorithm.GetValue()));
        }
        if (options.EncryptionScope.HasValue())
        {
          request.AddHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        AppendBlockFromUriResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        auto response_transactional_content_md5_iterator
            = httpResponse.GetHeaders().find("content-md5");
        if (response_transactional_content_md5_iterator != httpResponse.GetHeaders().end())
        {
          response.TransactionalContentMd5 = response_transactional_content_md5_iterator->second;
        }
        auto response_transactional_content_crc64_iterator
            = httpResponse.GetHeaders().find("x-ms-content-crc64");
        if (response_transactional_content_crc64_iterator != httpResponse.GetHeaders().end())
        {
          response.TransactionalContentCrc64
              = response_transactional_content_crc64_iterator->second;
        }
        response.AppendOffset = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-append-offset"));
        response.CommittedBlockCount
            = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-committed-block-count"));
        auto response_server_encrypted_iterator
            = httpResponse.GetHeaders().find("x-ms-request-server-encrypted");
        if (response_server_encrypted_iterator != httpResponse.GetHeaders().end())
        {
          response.ServerEncrypted = response_server_encrypted_iterator->second == "true";
        }
        auto response_encryption_key_sha256_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryption_key_sha256_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionKeySha256 = response_encryption_key_sha256_iterator->second;
        }
        auto response_encryption_scope_iterator
            = httpResponse.GetHeaders().find("x-ms-encryption-scope");
        if (response_encryption_scope_iterator != httpResponse.GetHeaders().end())
        {
          response.EncryptionScope = response_encryption_scope_iterator->second;
        }
        return Azure::Core::Response<AppendBlockFromUriResult>(
            std::move(response), std::move(pHttpResponse));
      }

      struct SealAppendBlobOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<std::string> IfTags;
        Azure::Core::Nullable<int64_t> AppendPosition;
      }; // struct SealAppendBlobOptions

      static Azure::Core::Response<SealAppendBlobResult> Seal(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          const SealAppendBlobOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.GetUrl().AppendQueryParameter("comp", "seal");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        if (options.IfModifiedSince.HasValue())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince.GetValue());
        }
        if (options.IfUnmodifiedSince.HasValue())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince.GetValue());
        }
        if (options.IfMatch.HasValue())
        {
          request.AddHeader("If-Match", options.IfMatch.GetValue());
        }
        if (options.IfNoneMatch.HasValue())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch.GetValue());
        }
        if (options.IfTags.HasValue())
        {
          request.AddHeader("x-ms-if-tags", options.IfTags.GetValue());
        }
        if (options.AppendPosition.HasValue())
        {
          request.AddHeader(
              "x-ms-blob-condition-appendpos", std::to_string(options.AppendPosition.GetValue()));
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        SealAppendBlobResult response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        return Azure::Core::Response<SealAppendBlobResult>(
            std::move(response), std::move(pHttpResponse));
      }

    private:
    }; // class AppendBlob

    class BlobBatch {
    public:
      struct SubmitBlobBatchOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string ContentType;
      }; // struct SubmitBlobBatchOptions

      static Azure::Core::Response<SubmitBlobBatchResultInternal> SubmitBatch(
          const Azure::Core::Context& context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const Azure::Core::Http::Url& url,
          Azure::Core::Http::BodyStream* requestBody,
          const SubmitBlobBatchOptions& options)
      {
        unused(options);
        auto request
            = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Post, url, requestBody);
        request.AddHeader("Content-Length", std::to_string(requestBody->Length()));
        request.GetUrl().AppendQueryParameter("comp", "batch");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.GetUrl().AppendQueryParameter(
              "timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddHeader("Content-Type", options.ContentType);
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        SubmitBlobBatchResultInternal response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 202))
        {
          throw StorageError::CreateFromResponse(std::move(pHttpResponse));
        }
        response.ContentType = httpResponse.GetHeaders().at("content-type");
        return Azure::Core::Response<SubmitBlobBatchResultInternal>(
            std::move(response), std::move(pHttpResponse));
      }

    private:
    }; // class BlobBatch

  }; // class BlobRestClient
}}} // namespace Azure::Storage::Blobs
