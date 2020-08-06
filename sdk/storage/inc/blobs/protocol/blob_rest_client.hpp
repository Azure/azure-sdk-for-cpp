// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "common/storage_common.hpp"
#include "common/storage_error.hpp"
#include "common/xml_wrapper.hpp"
#include "context.hpp"
#include "http/http.hpp"
#include "http/pipeline.hpp"
#include "nullable.hpp"
#include "response.hpp"

#include <cstring>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace Azure { namespace Storage { namespace Blobs {

  constexpr static const char* c_ApiVersion = "2019-12-12";

  struct AbortCopyBlobInfo
  {
  }; // struct AbortCopyBlobInfo

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

  struct BlobAppendInfo
  {
    std::string ETag;
    std::string LastModified;
    Azure::Core::Nullable<std::string> ContentMd5;
    Azure::Core::Nullable<std::string> ContentCrc64;
    int64_t AppendOffset = 0;
    int64_t CommittedBlockCount = 0;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
  }; // struct BlobAppendInfo

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

  struct BlobContainerInfo
  {
    std::string ETag;
    std::string LastModified;
  }; // struct BlobContainerInfo

  struct BlobContentInfo
  {
    std::string ETag;
    std::string LastModified;
    Azure::Core::Nullable<std::string> ContentMd5;
    Azure::Core::Nullable<std::string> ContentCrc64;
    Azure::Core::Nullable<int64_t> SequenceNumber;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
  }; // struct BlobContentInfo

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

  struct BlobInfo
  {
    std::string ETag;
    std::string LastModified;
    Azure::Core::Nullable<int64_t> SequenceNumber;
  }; // struct BlobInfo

  struct BlobLease
  {
    std::string ETag;
    std::string LastModified;
    std::string LeaseId;
  }; // struct BlobLease

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

  struct BlobSnapshotInfo
  {
    std::string Snapshot;
    std::string ETag;
    std::string LastModified;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
  }; // struct BlobSnapshotInfo

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

  struct BlockInfo
  {
    Azure::Core::Nullable<std::string> ContentMd5;
    Azure::Core::Nullable<std::string> ContentCrc64;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
  }; // struct BlockInfo

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

  struct BrokenLease
  {
    std::string ETag;
    std::string LastModified;
    int32_t LeaseTime = 0;
  }; // struct BrokenLease

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

  struct DeleteBlobInfo
  {
  }; // struct DeleteBlobInfo

  struct DeleteContainerInfo
  {
  }; // struct DeleteContainerInfo

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

  enum class ListBlobContainersIncludeOption
  {
    None = 0,
    Metadata = 1,
  }; // bitwise enum ListBlobContainersIncludeOption

  inline ListBlobContainersIncludeOption operator|(
      ListBlobContainersIncludeOption lhs,
      ListBlobContainersIncludeOption rhs)
  {
    using type = std::underlying_type_t<ListBlobContainersIncludeOption>;
    return static_cast<ListBlobContainersIncludeOption>(
        static_cast<type>(lhs) | static_cast<type>(rhs));
  }

  inline ListBlobContainersIncludeOption& operator|=(
      ListBlobContainersIncludeOption& lhs,
      ListBlobContainersIncludeOption rhs)
  {
    lhs = lhs | rhs;
    return lhs;
  }

  inline ListBlobContainersIncludeOption operator&(
      ListBlobContainersIncludeOption lhs,
      ListBlobContainersIncludeOption rhs)
  {
    using type = std::underlying_type_t<ListBlobContainersIncludeOption>;
    return static_cast<ListBlobContainersIncludeOption>(
        static_cast<type>(lhs) & static_cast<type>(rhs));
  }

  inline ListBlobContainersIncludeOption& operator&=(
      ListBlobContainersIncludeOption& lhs,
      ListBlobContainersIncludeOption rhs)
  {
    lhs = lhs & rhs;
    return lhs;
  }

  inline std::string ListBlobContainersIncludeOptionToString(
      const ListBlobContainersIncludeOption& val)
  {
    ListBlobContainersIncludeOption value_list[] = {
        ListBlobContainersIncludeOption::Metadata,
    };
    const char* string_list[] = {
        "metadata",
    };
    std::string ret;
    for (std::size_t i = 0; i < sizeof(value_list) / sizeof(ListBlobContainersIncludeOption); ++i)
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
    UncomittedBlobs = 16,
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
        ListBlobsIncludeItem::UncomittedBlobs,
    };
    const char* string_list[] = {
        "copy",
        "deleted",
        "metadata",
        "snapshots",
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

  struct PageBlobInfo
  {
    std::string ETag;
    std::string LastModified;
    int64_t SequenceNumber = 0;
  }; // struct PageBlobInfo

  struct PageInfo
  {
    std::string ETag;
    std::string LastModified;
    Azure::Core::Nullable<std::string> ContentMd5;
    Azure::Core::Nullable<std::string> ContentCrc64;
    int64_t SequenceNumber = 0;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
  }; // struct PageInfo

  struct PageRangesInfoInternal
  {
    std::string ETag;
    std::string LastModified;
    int64_t BlobContentLength = 0;
    std::vector<std::pair<int64_t, int64_t>> PageRanges;
    std::vector<std::pair<int64_t, int64_t>> ClearRanges;
  }; // struct PageRangesInfoInternal

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

  struct SetBlobAccessTierInfo
  {
  }; // struct SetBlobAccessTierInfo

  struct SetServicePropertiesInfo
  {
  }; // struct SetServicePropertiesInfo

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

  struct UndeleteBlobInfo
  {
  }; // struct UndeleteBlobInfo

  struct UserDelegationKey
  {
    std::string SignedObjectId;
    std::string SignedTenantId;
    std::string SignedStartsOn;
    std::string SignedExpiresOn;
    std::string SignedService;
    std::string SignedVersion;
    std::string Value;
  }; // struct UserDelegationKey

  struct AccountInfo
  {
    Blobs::SkuName SkuName = Blobs::SkuName::Unknown;
    Blobs::AccountKind AccountKind = Blobs::AccountKind::Unknown;
  }; // struct AccountInfo

  struct BlobAnalyticsLogging
  {
    std::string Version;
    bool Delete = false;
    bool Read = false;
    bool Write = false;
    BlobRetentionPolicy RetentionPolicy;
  }; // struct BlobAnalyticsLogging

  struct BlobBlockListInfo
  {
    std::string ETag;
    std::string LastModified;
    std::string ContentType;
    int64_t ContentLength = 0;
    std::vector<BlobBlock> CommittedBlocks;
    std::vector<BlobBlock> UncommittedBlocks;
  }; // struct BlobBlockListInfo

  struct BlobContainerAccessPolicy
  {
    PublicAccessType AccessType = PublicAccessType::Private;
    std::string ETag;
    std::string LastModified;
    std::vector<BlobSignedIdentifier> SignedIdentifiers;
  }; // struct BlobContainerAccessPolicy

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
  }; // struct BlobContainerItem

  struct BlobContainerProperties
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
  }; // struct BlobContainerProperties

  struct BlobCopyInfo
  {
    std::string ETag;
    std::string LastModified;
    std::string CopyId;
    Blobs::CopyStatus CopyStatus = Blobs::CopyStatus::Unknown;
  }; // struct BlobCopyInfo

  struct BlobDownloadResponse
  {
    std::unique_ptr<Azure::Core::Http::BodyStream> BodyStream;
    std::string ETag;
    std::string LastModified;
    Azure::Core::Nullable<std::string> ContentRange;
    BlobHttpHeaders HttpHeaders;
    std::map<std::string, std::string> Metadata;
    Azure::Core::Nullable<int64_t> SequenceNumber; // only for page blob
    Azure::Core::Nullable<int64_t> CommittedBlockCount; // only for append blob
    Blobs::BlobType BlobType = Blobs::BlobType::Unknown;
    Azure::Core::Nullable<std::string> ContentMd5; // Md5 for the downloaded range
    Azure::Core::Nullable<std::string> ContentCrc64;
    Azure::Core::Nullable<std::string> LeaseDuration;
    Azure::Core::Nullable<BlobLeaseState> LeaseState;
    Azure::Core::Nullable<BlobLeaseStatus> LeaseStatus;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
  }; // struct BlobDownloadResponse

  struct BlobGeoReplication
  {
    BlobGeoReplicationStatus Status = BlobGeoReplicationStatus::Unknown;
    Azure::Core::Nullable<std::string> LastSyncedOn;
  }; // struct BlobGeoReplication

  struct BlobItem
  {
    std::string Name;
    bool Deleted = false;
    std::string Snapshot;
    BlobHttpHeaders HttpHeaders;
    std::map<std::string, std::string> Metadata;
    std::string CreationTime;
    std::string LastModified;
    std::string ETag;
    int64_t ContentLength = 0;
    Blobs::BlobType BlobType = Blobs::BlobType::Unknown;
    AccessTier Tier = AccessTier::Unknown;
    bool AccessTierInferred = true;
    BlobLeaseStatus LeaseStatus = BlobLeaseStatus::Unlocked;
    BlobLeaseState LeaseState = BlobLeaseState::Available;
    Azure::Core::Nullable<std::string> LeaseDuration;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
  }; // struct BlobItem

  struct BlobMetrics
  {
    std::string Version;
    bool Enabled = false;
    BlobRetentionPolicy RetentionPolicy;
    Azure::Core::Nullable<bool> IncludeApis;
  }; // struct BlobMetrics

  struct BlobProperties
  {
    std::string ETag;
    std::string LastModified;
    std::string CreationTime;
    std::map<std::string, std::string> Metadata;
    Blobs::BlobType BlobType = Blobs::BlobType::Unknown;
    Azure::Core::Nullable<std::string> LeaseDuration;
    Azure::Core::Nullable<BlobLeaseState> LeaseState;
    Azure::Core::Nullable<BlobLeaseStatus> LeaseStatus;
    int64_t ContentLength = 0;
    BlobHttpHeaders HttpHeaders;
    Azure::Core::Nullable<int64_t> SequenceNumber; // only for page blob
    Azure::Core::Nullable<int32_t> CommittedBlockCount; // only for append blob
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
    Azure::Core::Nullable<AccessTier> Tier;
    Azure::Core::Nullable<bool> AccessTierInferred;
    Azure::Core::Nullable<BlobArchiveStatus> ArchiveStatus;
    Azure::Core::Nullable<std::string> AccessTierChangeTime;
    Azure::Core::Nullable<std::string> CopyId;
    Azure::Core::Nullable<std::string> CopySource;
    Azure::Core::Nullable<Blobs::CopyStatus> CopyStatus;
    Azure::Core::Nullable<std::string> CopyProgress;
    Azure::Core::Nullable<std::string> CopyCompletionTime;
  }; // struct BlobProperties

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

  struct BlobServiceStatistics
  {
    BlobGeoReplication GeoReplication;
  }; // struct BlobServiceStatistics

  struct BlobsFlatSegment
  {
    std::string ServiceEndpoint;
    std::string Container;
    std::string Prefix;
    std::string Marker;
    std::string NextMarker;
    std::vector<BlobItem> Items;
  }; // struct BlobsFlatSegment

  struct BlobsHierarchySegment
  {
    std::string ServiceEndpoint;
    std::string Container;
    std::string Prefix;
    std::string Delimiter;
    std::string Marker;
    std::string NextMarker;
    std::vector<BlobItem> Items;
    std::vector<BlobPrefix> BlobPrefixes;
  }; // struct BlobsHierarchySegment

  struct ListContainersSegment
  {
    std::string ServiceEndpoint;
    std::string Prefix;
    std::string Marker;
    std::string NextMarker;
    std::vector<BlobContainerItem> Items;
  }; // struct ListContainersSegment

  class BlobRestClient {
  public:
    class Service {
    public:
      struct ListBlobContainersOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::string> Prefix;
        Azure::Core::Nullable<std::string> Marker;
        Azure::Core::Nullable<int32_t> MaxResults;
        ListBlobContainersIncludeOption IncludeMetadata = ListBlobContainersIncludeOption::None;
      }; // struct ListBlobContainersOptions

      static Azure::Core::Response<ListContainersSegment> ListBlobContainers(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const ListBlobContainersOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddQueryParameter("comp", "list");
        if (options.Prefix.HasValue())
        {
          request.AddQueryParameter("prefix", options.Prefix.GetValue());
        }
        if (options.Marker.HasValue())
        {
          request.AddQueryParameter("marker", options.Marker.GetValue());
        }
        if (options.MaxResults.HasValue())
        {
          request.AddQueryParameter("maxresults", std::to_string(options.MaxResults.GetValue()));
        }
        std::string list_blob_containers_include_option
            = ListBlobContainersIncludeOptionToString(options.IncludeMetadata);
        if (!list_blob_containers_include_option.empty())
        {
          request.AddQueryParameter("include", list_blob_containers_include_option);
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        ListContainersSegment response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        {
          const auto& httpResponseBody = httpResponse.GetBody();
          XmlReader reader(
              reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
          response = ListContainersSegmentFromXml(reader);
        }
        return Azure::Core::Response<ListContainersSegment>(
            std::move(response), std::move(pHttpResponse));
      }

      struct GetUserDelegationKeyOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string StartsOn;
        std::string ExpiresOn;
      }; // struct GetUserDelegationKeyOptions

      static Azure::Core::Response<UserDelegationKey> GetUserDelegationKey(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
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
        request.AddQueryParameter("restype", "service");
        request.AddQueryParameter("comp", "userdelegationkey");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        UserDelegationKey response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        {
          const auto& httpResponseBody = httpResponse.GetBody();
          XmlReader reader(
              reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
          response = UserDelegationKeyFromXml(reader);
        }
        return Azure::Core::Response<UserDelegationKey>(
            std::move(response), std::move(pHttpResponse));
      }

      struct GetPropertiesOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
      }; // struct GetPropertiesOptions

      static Azure::Core::Response<BlobServiceProperties> GetProperties(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const GetPropertiesOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddQueryParameter("restype", "service");
        request.AddQueryParameter("comp", "properties");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobServiceProperties response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        {
          const auto& httpResponseBody = httpResponse.GetBody();
          XmlReader reader(
              reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
          response = BlobServicePropertiesFromXml(reader);
        }
        return Azure::Core::Response<BlobServiceProperties>(
            std::move(response), std::move(pHttpResponse));
      }

      struct SetPropertiesOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        BlobServiceProperties Properties;
      }; // struct SetPropertiesOptions

      static Azure::Core::Response<SetServicePropertiesInfo> SetProperties(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const SetPropertiesOptions& options)
      {
        unused(options);
        std::string xml_body;
        {
          XmlWriter writer;
          SetPropertiesOptionsToXml(writer, options);
          xml_body = writer.GetDocument();
          writer.Write(XmlNode{XmlNodeType::End});
        }
        Azure::Core::Http::MemoryBodyStream xml_body_stream(
            reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
        auto request
            = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, &xml_body_stream);
        request.AddHeader("Content-Length", std::to_string(xml_body_stream.Length()));
        request.AddQueryParameter("restype", "service");
        request.AddQueryParameter("comp", "properties");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        SetServicePropertiesInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 202))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        return Azure::Core::Response<SetServicePropertiesInfo>(
            std::move(response), std::move(pHttpResponse));
      }

      struct GetAccountInfoOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
      }; // struct GetAccountInfoOptions

      static Azure::Core::Response<AccountInfo> GetAccountInfo(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const GetAccountInfoOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, url);
        request.AddQueryParameter("restype", "account");
        request.AddQueryParameter("comp", "properties");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        AccountInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.SkuName = SkuNameFromString(httpResponse.GetHeaders().at("x-ms-sku-name"));
        response.AccountKind
            = AccountKindFromString(httpResponse.GetHeaders().at("x-ms-account-kind"));
        return Azure::Core::Response<AccountInfo>(std::move(response), std::move(pHttpResponse));
      }

      struct GetStatisticsOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
      }; // struct GetStatisticsOptions

      static Azure::Core::Response<BlobServiceStatistics> GetStatistics(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const GetStatisticsOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddQueryParameter("restype", "service");
        request.AddQueryParameter("comp", "stats");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobServiceStatistics response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        {
          const auto& httpResponseBody = httpResponse.GetBody();
          XmlReader reader(
              reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
          response = BlobServiceStatisticsFromXml(reader);
        }
        return Azure::Core::Response<BlobServiceStatistics>(
            std::move(response), std::move(pHttpResponse));
      }

    private:
      static BlobServiceProperties BlobServicePropertiesFromXml(XmlReader& reader)
      {
        BlobServiceProperties ret;
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

      static BlobServiceStatistics BlobServiceStatisticsFromXml(XmlReader& reader)
      {
        BlobServiceStatistics ret;
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

      static ListContainersSegment ListContainersSegmentFromXml(XmlReader& reader)
      {
        ListContainersSegment ret;
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
              ret.Marker = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_NextMarker)
            {
              ret.NextMarker = node.Value;
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

      static UserDelegationKey UserDelegationKeyFromXml(XmlReader& reader)
      {
        UserDelegationKey ret;
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
          k_Metadata,
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
            else if (std::strcmp(node.Name, "Metadata") == 0)
            {
              path.emplace_back(XmlTagName::k_Metadata);
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
              ret.LastSyncedOn = node.Value;
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

      static void SetPropertiesOptionsToXml(XmlWriter& writer, const SetPropertiesOptions& options)
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
      struct CreateOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<PublicAccessType> AccessType;
        std::map<std::string, std::string> Metadata;
      }; // struct CreateOptions

      static Azure::Core::Response<BlobContainerInfo> Create(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const CreateOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("restype", "container");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        std::set<std::string> metadataKeys;
        for (const auto& pair : options.Metadata)
        {
          std::string key = pair.first;
          std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
          });
          if (metadataKeys.insert(key).second == false)
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobContainerInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        return Azure::Core::Response<BlobContainerInfo>(
            std::move(response), std::move(pHttpResponse));
      }

      struct DeleteOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
      }; // struct DeleteOptions

      static Azure::Core::Response<DeleteContainerInfo> Delete(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const DeleteOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Delete, url);
        request.AddQueryParameter("restype", "container");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
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
        DeleteContainerInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 202))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        return Azure::Core::Response<DeleteContainerInfo>(
            std::move(response), std::move(pHttpResponse));
      }

      struct GetPropertiesOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<std::string> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> LeaseId;
      }; // struct GetPropertiesOptions

      static Azure::Core::Response<BlobContainerProperties> GetProperties(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const GetPropertiesOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, url);
        request.AddQueryParameter("restype", "container");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
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
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue());
        }
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobContainerProperties response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
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
        return Azure::Core::Response<BlobContainerProperties>(
            std::move(response), std::move(pHttpResponse));
      }

      struct SetMetadataOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::map<std::string, std::string> Metadata;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
      }; // struct SetMetadataOptions

      static Azure::Core::Response<BlobContainerInfo> SetMetadata(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const SetMetadataOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("restype", "container");
        request.AddQueryParameter("comp", "metadata");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        std::set<std::string> metadataKeys;
        for (const auto& pair : options.Metadata)
        {
          std::string key = pair.first;
          std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
          });
          if (metadataKeys.insert(key).second == false)
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
        BlobContainerInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        return Azure::Core::Response<BlobContainerInfo>(
            std::move(response), std::move(pHttpResponse));
      }

      struct ListBlobsFlatOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::string> Prefix;
        Azure::Core::Nullable<std::string> Marker;
        Azure::Core::Nullable<int32_t> MaxResults;
        ListBlobsIncludeItem Include = ListBlobsIncludeItem::None;
      }; // struct ListBlobsFlatOptions

      static Azure::Core::Response<BlobsFlatSegment> ListBlobsFlat(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const ListBlobsFlatOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddQueryParameter("restype", "container");
        request.AddQueryParameter("comp", "list");
        if (options.Prefix.HasValue())
        {
          request.AddQueryParameter("prefix", options.Prefix.GetValue());
        }
        if (options.Marker.HasValue())
        {
          request.AddQueryParameter("marker", options.Marker.GetValue());
        }
        if (options.MaxResults.HasValue())
        {
          request.AddQueryParameter("maxresults", std::to_string(options.MaxResults.GetValue()));
        }
        std::string list_blobs_include_item = ListBlobsIncludeItemToString(options.Include);
        if (!list_blobs_include_item.empty())
        {
          request.AddQueryParameter("include", list_blobs_include_item);
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobsFlatSegment response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        {
          const auto& httpResponseBody = httpResponse.GetBody();
          XmlReader reader(
              reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
          response = BlobsFlatSegmentFromXml(reader);
        }
        return Azure::Core::Response<BlobsFlatSegment>(
            std::move(response), std::move(pHttpResponse));
      }

      struct ListBlobsByHierarchyOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::string> Prefix;
        Azure::Core::Nullable<std::string> Delimiter;
        Azure::Core::Nullable<std::string> Marker;
        Azure::Core::Nullable<int32_t> MaxResults;
        ListBlobsIncludeItem Include = ListBlobsIncludeItem::None;
      }; // struct ListBlobsByHierarchyOptions

      static Azure::Core::Response<BlobsHierarchySegment> ListBlobsByHierarchy(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const ListBlobsByHierarchyOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddQueryParameter("restype", "container");
        request.AddQueryParameter("comp", "list");
        if (options.Prefix.HasValue())
        {
          request.AddQueryParameter("prefix", options.Prefix.GetValue());
        }
        if (options.Delimiter.HasValue())
        {
          request.AddQueryParameter("delimiter", options.Delimiter.GetValue());
        }
        if (options.Marker.HasValue())
        {
          request.AddQueryParameter("marker", options.Marker.GetValue());
        }
        if (options.MaxResults.HasValue())
        {
          request.AddQueryParameter("maxresults", std::to_string(options.MaxResults.GetValue()));
        }
        std::string list_blobs_include_item = ListBlobsIncludeItemToString(options.Include);
        if (!list_blobs_include_item.empty())
        {
          request.AddQueryParameter("include", list_blobs_include_item);
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobsHierarchySegment response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        {
          const auto& httpResponseBody = httpResponse.GetBody();
          XmlReader reader(
              reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
          response = BlobsHierarchySegmentFromXml(reader);
        }
        return Azure::Core::Response<BlobsHierarchySegment>(
            std::move(response), std::move(pHttpResponse));
      }

      struct GetAccessPolicyOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::string> LeaseId;
      }; // struct GetAccessPolicyOptions

      static Azure::Core::Response<BlobContainerAccessPolicy> GetAccessPolicy(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const GetAccessPolicyOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddQueryParameter("restype", "container");
        request.AddQueryParameter("comp", "acl");
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobContainerAccessPolicy response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        {
          const auto& httpResponseBody = httpResponse.GetBody();
          XmlReader reader(
              reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
          response = BlobContainerAccessPolicyFromXml(reader);
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.AccessType
            = PublicAccessTypeFromString(httpResponse.GetHeaders().at("x-ms-blob-public-access"));
        return Azure::Core::Response<BlobContainerAccessPolicy>(
            std::move(response), std::move(pHttpResponse));
      }

      struct SetAccessPolicyOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<PublicAccessType> AccessType;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        std::vector<BlobSignedIdentifier> SignedIdentifiers;
      }; // struct SetAccessPolicyOptions

      static Azure::Core::Response<BlobContainerInfo> SetAccessPolicy(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const SetAccessPolicyOptions& options)
      {
        unused(options);
        std::string xml_body;
        {
          XmlWriter writer;
          SetAccessPolicyOptionsToXml(writer, options);
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
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddQueryParameter("restype", "container");
        request.AddQueryParameter("comp", "acl");
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
        BlobContainerInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        return Azure::Core::Response<BlobContainerInfo>(
            std::move(response), std::move(pHttpResponse));
      }

      struct AcquireLeaseOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        int32_t LeaseDuration = -1;
        Azure::Core::Nullable<std::string> ProposedLeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
      }; // struct AcquireLeaseOptions

      static Azure::Core::Response<BlobLease> AcquireLease(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const AcquireLeaseOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddQueryParameter("restype", "container");
        request.AddQueryParameter("comp", "lease");
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
        BlobLease response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
        return Azure::Core::Response<BlobLease>(std::move(response), std::move(pHttpResponse));
      }

      struct RenewLeaseOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
      }; // struct RenewLeaseOptions

      static Azure::Core::Response<BlobLease> RenewLease(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const RenewLeaseOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddQueryParameter("restype", "container");
        request.AddQueryParameter("comp", "lease");
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
        BlobLease response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
        return Azure::Core::Response<BlobLease>(std::move(response), std::move(pHttpResponse));
      }

      struct ChangeLeaseOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string LeaseId;
        std::string ProposedLeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
      }; // struct ChangeLeaseOptions

      static Azure::Core::Response<BlobLease> ChangeLease(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const ChangeLeaseOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddQueryParameter("restype", "container");
        request.AddQueryParameter("comp", "lease");
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
        BlobLease response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
        return Azure::Core::Response<BlobLease>(std::move(response), std::move(pHttpResponse));
      }

      struct ReleaseLeaseOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
      }; // struct ReleaseLeaseOptions

      static Azure::Core::Response<BlobContainerInfo> ReleaseLease(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const ReleaseLeaseOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddQueryParameter("restype", "container");
        request.AddQueryParameter("comp", "lease");
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
        BlobContainerInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        return Azure::Core::Response<BlobContainerInfo>(
            std::move(response), std::move(pHttpResponse));
      }

      struct BreakLeaseOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<int32_t> BreakPeriod;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
      }; // struct BreakLeaseOptions

      static Azure::Core::Response<BrokenLease> BreakLease(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const BreakLeaseOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddQueryParameter("restype", "container");
        request.AddQueryParameter("comp", "lease");
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
        BrokenLease response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 202))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.LeaseTime = std::stoi(httpResponse.GetHeaders().at("x-ms-lease-time"));
        return Azure::Core::Response<BrokenLease>(std::move(response), std::move(pHttpResponse));
      }

    private:
      static BlobContainerAccessPolicy BlobContainerAccessPolicyFromXml(XmlReader& reader)
      {
        BlobContainerAccessPolicy ret;
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

      static BlobsFlatSegment BlobsFlatSegmentFromXml(XmlReader& reader)
      {
        BlobsFlatSegment ret;
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
              ret.Marker = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_NextMarker)
            {
              ret.NextMarker = node.Value;
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

      static BlobsHierarchySegment BlobsHierarchySegmentFromXml(XmlReader& reader)
      {
        BlobsHierarchySegment ret;
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
              ret.Marker = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_NextMarker)
            {
              ret.NextMarker = node.Value;
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
          k_Properties,
          k_ContentType,
          k_ContentEncoding,
          k_ContentLanguage,
          k_ContentMD5,
          k_CacheControl,
          k_ContentDisposition,
          k_CreationTime,
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
          k_Metadata,
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
            else if (std::strcmp(node.Name, "Metadata") == 0)
            {
              path.emplace_back(XmlTagName::k_Metadata);
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
            else if (path.size() == 1 && path[0] == XmlTagName::k_Deleted)
            {
              ret.Deleted = std::strcmp(node.Value, "true") == 0;
            }
            else if (path.size() == 1 && path[0] == XmlTagName::k_Snapshot)
            {
              ret.Snapshot = node.Value;
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

      static void SetAccessPolicyOptionsToXml(
          XmlWriter& writer,
          const SetAccessPolicyOptions& options)
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
      struct DownloadOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::pair<int64_t, int64_t>> Range;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<std::string> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
      }; // struct DownloadOptions

      static Azure::Core::Response<BlobDownloadResponse> Download(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const DownloadOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url, true);
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
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
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue());
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
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobDownloadResponse response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200 || http_status_code == 206))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.BodyStream = httpResponse.GetBodyStream();
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        auto response_content_md5_iterator = httpResponse.GetHeaders().find("content-md5");
        if (response_content_md5_iterator != httpResponse.GetHeaders().end())
        {
          response.ContentMd5 = response_content_md5_iterator->second;
        }
        auto response_content_crc64_iterator = httpResponse.GetHeaders().find("x-ms-content-crc64");
        if (response_content_crc64_iterator != httpResponse.GetHeaders().end())
        {
          response.ContentCrc64 = response_content_crc64_iterator->second;
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
        response.BlobType = BlobTypeFromString(httpResponse.GetHeaders().at("x-ms-blob-type"));
        return Azure::Core::Response<BlobDownloadResponse>(
            std::move(response), std::move(pHttpResponse));
      }

      struct DeleteOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<DeleteSnapshotsOption> DeleteSnapshots;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
      }; // struct DeleteOptions

      static Azure::Core::Response<DeleteBlobInfo> Delete(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const DeleteOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Delete, url);
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        DeleteBlobInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 202))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        return Azure::Core::Response<DeleteBlobInfo>(std::move(response), std::move(pHttpResponse));
      }

      struct UndeleteOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
      }; // struct UndeleteOptions

      static Azure::Core::Response<UndeleteBlobInfo> Undelete(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const UndeleteOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddQueryParameter("comp", "undelete");
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        UndeleteBlobInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        return Azure::Core::Response<UndeleteBlobInfo>(
            std::move(response), std::move(pHttpResponse));
      }

      struct GetPropertiesOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
      }; // struct GetPropertiesOptions

      static Azure::Core::Response<BlobProperties> GetProperties(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const GetPropertiesOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, url);
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobProperties response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.CreationTime = httpResponse.GetHeaders().at("x-ms-creation-time");
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
        return Azure::Core::Response<BlobProperties>(std::move(response), std::move(pHttpResponse));
      }

      struct SetHttpHeadersOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        BlobHttpHeaders HttpHeaders;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<std::string> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
      }; // struct SetHttpHeadersOptions

      static Azure::Core::Response<BlobInfo> SetHttpHeaders(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const SetHttpHeadersOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "properties");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
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
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue());
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        auto response_sequence_number_iterator
            = httpResponse.GetHeaders().find("x-ms-blob-sequence-number");
        if (response_sequence_number_iterator != httpResponse.GetHeaders().end())
        {
          response.SequenceNumber = std::stoll(response_sequence_number_iterator->second);
        }
        return Azure::Core::Response<BlobInfo>(std::move(response), std::move(pHttpResponse));
      }

      struct SetMetadataOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::map<std::string, std::string> Metadata;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<std::string> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
      }; // struct SetMetadataOptions

      static Azure::Core::Response<BlobInfo> SetMetadata(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const SetMetadataOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "metadata");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        std::set<std::string> metadataKeys;
        for (const auto& pair : options.Metadata)
        {
          std::string key = pair.first;
          std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
          });
          if (metadataKeys.insert(key).second == false)
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
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue());
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        return Azure::Core::Response<BlobInfo>(std::move(response), std::move(pHttpResponse));
      }

      struct SetAccessTierOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        AccessTier Tier = AccessTier::Unknown;
        Azure::Core::Nullable<Blobs::RehydratePriority> RehydratePriority;
      }; // struct SetAccessTierOptions

      static Azure::Core::Response<SetBlobAccessTierInfo> SetAccessTier(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const SetAccessTierOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "tier");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddHeader("x-ms-access-tier", AccessTierToString(options.Tier));
        if (options.RehydratePriority.HasValue())
        {
          request.AddHeader(
              "x-ms-rehydrate-priority",
              RehydratePriorityToString(options.RehydratePriority.GetValue()));
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        SetBlobAccessTierInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200 || http_status_code == 202))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        return Azure::Core::Response<SetBlobAccessTierInfo>(
            std::move(response), std::move(pHttpResponse));
      }

      struct StartCopyFromUriOptions
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
        Azure::Core::Nullable<std::string> SourceIfModifiedSince;
        Azure::Core::Nullable<std::string> SourceIfUnmodifiedSince;
        Azure::Core::Nullable<std::string> SourceIfMatch;
        Azure::Core::Nullable<std::string> SourceIfNoneMatch;
      }; // struct StartCopyFromUriOptions

      static Azure::Core::Response<BlobCopyInfo> StartCopyFromUri(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const StartCopyFromUriOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        std::set<std::string> metadataKeys;
        for (const auto& pair : options.Metadata)
        {
          std::string key = pair.first;
          std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
          });
          if (metadataKeys.insert(key).second == false)
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
        BlobCopyInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 202))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.CopyId = httpResponse.GetHeaders().at("x-ms-copy-id");
        response.CopyStatus
            = CopyStatusFromString(httpResponse.GetHeaders().at("x-ms-copy-status"));
        return Azure::Core::Response<BlobCopyInfo>(std::move(response), std::move(pHttpResponse));
      }

      struct AbortCopyFromUriOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string CopyId;
        Azure::Core::Nullable<std::string> LeaseId;
      }; // struct AbortCopyFromUriOptions

      static Azure::Core::Response<AbortCopyBlobInfo> AbortCopyFromUri(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const AbortCopyFromUriOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddQueryParameter("comp", "copy");
        request.AddQueryParameter("copyid", options.CopyId);
        request.AddHeader("x-ms-copy-action", "abort");
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        AbortCopyBlobInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 204))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        return Azure::Core::Response<AbortCopyBlobInfo>(
            std::move(response), std::move(pHttpResponse));
      }

      struct CreateSnapshotOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::map<std::string, std::string> Metadata;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<std::string> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
      }; // struct CreateSnapshotOptions

      static Azure::Core::Response<BlobSnapshotInfo> CreateSnapshot(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const CreateSnapshotOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "snapshot");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
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
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue());
        }
        std::set<std::string> metadataKeys;
        for (const auto& pair : options.Metadata)
        {
          std::string key = pair.first;
          std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
          });
          if (metadataKeys.insert(key).second == false)
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobSnapshotInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
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
        response.Snapshot = httpResponse.GetHeaders().at("x-ms-snapshot");
        return Azure::Core::Response<BlobSnapshotInfo>(
            std::move(response), std::move(pHttpResponse));
      }

      struct AcquireLeaseOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        int32_t LeaseDuration = -1;
        Azure::Core::Nullable<std::string> ProposedLeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
      }; // struct AcquireLeaseOptions

      static Azure::Core::Response<BlobLease> AcquireLease(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const AcquireLeaseOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddQueryParameter("comp", "lease");
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobLease response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
        return Azure::Core::Response<BlobLease>(std::move(response), std::move(pHttpResponse));
      }

      struct RenewLeaseOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
      }; // struct RenewLeaseOptions

      static Azure::Core::Response<BlobLease> RenewLease(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const RenewLeaseOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddQueryParameter("comp", "lease");
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobLease response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
        return Azure::Core::Response<BlobLease>(std::move(response), std::move(pHttpResponse));
      }

      struct ChangeLeaseOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string LeaseId;
        std::string ProposedLeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
      }; // struct ChangeLeaseOptions

      static Azure::Core::Response<BlobLease> ChangeLease(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const ChangeLeaseOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddQueryParameter("comp", "lease");
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobLease response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
        return Azure::Core::Response<BlobLease>(std::move(response), std::move(pHttpResponse));
      }

      struct ReleaseLeaseOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string LeaseId;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
      }; // struct ReleaseLeaseOptions

      static Azure::Core::Response<BlobInfo> ReleaseLease(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const ReleaseLeaseOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddQueryParameter("comp", "lease");
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        auto response_sequence_number_iterator
            = httpResponse.GetHeaders().find("x-ms-blob-sequence-number");
        if (response_sequence_number_iterator != httpResponse.GetHeaders().end())
        {
          response.SequenceNumber = std::stoll(response_sequence_number_iterator->second);
        }
        return Azure::Core::Response<BlobInfo>(std::move(response), std::move(pHttpResponse));
      }

      struct BreakLeaseOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<int32_t> BreakPeriod;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
      }; // struct BreakLeaseOptions

      static Azure::Core::Response<BrokenLease> BreakLease(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const BreakLeaseOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddQueryParameter("comp", "lease");
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BrokenLease response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 202))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.LeaseTime = std::stoi(httpResponse.GetHeaders().at("x-ms-lease-time"));
        return Azure::Core::Response<BrokenLease>(std::move(response), std::move(pHttpResponse));
      }

    private:
    }; // class Blob

    class BlockBlob {
    public:
      struct UploadOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::string> ContentMd5;
        Azure::Core::Nullable<std::string> ContentCrc64;
        BlobHttpHeaders HttpHeaders;
        std::map<std::string, std::string> Metadata;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<AccessTier> Tier;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<std::string> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
      }; // struct UploadOptions

      static Azure::Core::Response<BlobContentInfo> Upload(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          Azure::Core::Http::BodyStream* requestBody,
          const UploadOptions& options)
      {
        unused(options);
        auto request
            = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, requestBody);
        request.AddHeader("Content-Length", std::to_string(requestBody->Length()));
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
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
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue());
        }
        if (options.ContentMd5.HasValue())
        {
          request.AddHeader("Content-MD5", options.ContentMd5.GetValue());
        }
        if (options.ContentCrc64.HasValue())
        {
          request.AddHeader("x-ms-content-crc64", options.ContentCrc64.GetValue());
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
          std::string key = pair.first;
          std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
          });
          if (metadataKeys.insert(key).second == false)
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobContentInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        auto response_content_md5_iterator = httpResponse.GetHeaders().find("content-md5");
        if (response_content_md5_iterator != httpResponse.GetHeaders().end())
        {
          response.ContentMd5 = response_content_md5_iterator->second;
        }
        auto response_content_crc64_iterator = httpResponse.GetHeaders().find("x-ms-content-crc64");
        if (response_content_crc64_iterator != httpResponse.GetHeaders().end())
        {
          response.ContentCrc64 = response_content_crc64_iterator->second;
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
        return Azure::Core::Response<BlobContentInfo>(
            std::move(response), std::move(pHttpResponse));
      }

      struct StageBlockOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string BlockId;
        Azure::Core::Nullable<std::string> ContentMd5;
        Azure::Core::Nullable<std::string> ContentCrc64;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<std::string> EncryptionAlgorithm;
      }; // struct StageBlockOptions

      static Azure::Core::Response<BlockInfo> StageBlock(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          Azure::Core::Http::BodyStream* requestBody,
          const StageBlockOptions& options)
      {
        unused(options);
        auto request
            = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, requestBody);
        request.AddHeader("Content-Length", std::to_string(requestBody->Length()));
        request.AddQueryParameter("comp", "block");
        request.AddQueryParameter("blockid", options.BlockId);
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        if (options.ContentMd5.HasValue())
        {
          request.AddHeader("Content-MD5", options.ContentMd5.GetValue());
        }
        if (options.ContentCrc64.HasValue())
        {
          request.AddHeader("x-ms-content-crc64", options.ContentCrc64.GetValue());
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
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlockInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        auto response_content_md5_iterator = httpResponse.GetHeaders().find("content-md5");
        if (response_content_md5_iterator != httpResponse.GetHeaders().end())
        {
          response.ContentMd5 = response_content_md5_iterator->second;
        }
        auto response_content_crc64_iterator = httpResponse.GetHeaders().find("x-ms-content-crc64");
        if (response_content_crc64_iterator != httpResponse.GetHeaders().end())
        {
          response.ContentCrc64 = response_content_crc64_iterator->second;
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
        return Azure::Core::Response<BlockInfo>(std::move(response), std::move(pHttpResponse));
      }

      struct StageBlockFromUriOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string BlockId;
        std::string SourceUri;
        Azure::Core::Nullable<std::pair<int64_t, int64_t>> SourceRange;
        Azure::Core::Nullable<std::string> ContentMd5;
        Azure::Core::Nullable<std::string> ContentCrc64;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<std::string> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> SourceIfModifiedSince;
        Azure::Core::Nullable<std::string> SourceIfUnmodifiedSince;
        Azure::Core::Nullable<std::string> SourceIfMatch;
        Azure::Core::Nullable<std::string> SourceIfNoneMatch;
      }; // struct StageBlockFromUriOptions

      static Azure::Core::Response<BlockInfo> StageBlockFromUri(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const StageBlockFromUriOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "block");
        request.AddQueryParameter("blockid", options.BlockId);
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
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
        if (options.ContentMd5.HasValue())
        {
          request.AddHeader("x-ms-source-content-md5", options.ContentMd5.GetValue());
        }
        if (options.ContentCrc64.HasValue())
        {
          request.AddHeader("x-ms-source-content-crc64", options.ContentCrc64.GetValue());
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
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue());
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
        BlockInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        auto response_content_md5_iterator = httpResponse.GetHeaders().find("content-md5");
        if (response_content_md5_iterator != httpResponse.GetHeaders().end())
        {
          response.ContentMd5 = response_content_md5_iterator->second;
        }
        auto response_content_crc64_iterator = httpResponse.GetHeaders().find("x-ms-content-crc64");
        if (response_content_crc64_iterator != httpResponse.GetHeaders().end())
        {
          response.ContentCrc64 = response_content_crc64_iterator->second;
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
        return Azure::Core::Response<BlockInfo>(std::move(response), std::move(pHttpResponse));
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
        Azure::Core::Nullable<std::string> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
        Azure::Core::Nullable<AccessTier> Tier;
      }; // struct CommitBlockListOptions

      static Azure::Core::Response<BlobContentInfo> CommitBlockList(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
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
        request.AddQueryParameter("comp", "blocklist");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
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
          std::string key = pair.first;
          std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
          });
          if (metadataKeys.insert(key).second == false)
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
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue());
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobContentInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
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
        return Azure::Core::Response<BlobContentInfo>(
            std::move(response), std::move(pHttpResponse));
      }

      struct GetBlockListOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<BlockListTypeOption> ListType;
        Azure::Core::Nullable<std::string> LeaseId;
      }; // struct GetBlockListOptions

      static Azure::Core::Response<BlobBlockListInfo> GetBlockList(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const GetBlockListOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddQueryParameter("comp", "blocklist");
        if (options.ListType.HasValue())
        {
          std::string block_list_type_option
              = BlockListTypeOptionToString(options.ListType.GetValue());
          request.AddQueryParameter("blocklisttype", block_list_type_option);
        }
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        if (options.LeaseId.HasValue())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId.GetValue());
        }
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobBlockListInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        {
          const auto& httpResponseBody = httpResponse.GetBody();
          XmlReader reader(
              reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
          response = BlobBlockListInfoFromXml(reader);
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.ContentType = httpResponse.GetHeaders().at("content-type");
        response.ContentLength
            = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-content-length"));
        return Azure::Core::Response<BlobBlockListInfo>(
            std::move(response), std::move(pHttpResponse));
      }

    private:
      static BlobBlockListInfo BlobBlockListInfoFromXml(XmlReader& reader)
      {
        BlobBlockListInfo ret;
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
      struct CreateOptions
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
        Azure::Core::Nullable<std::string> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
      }; // struct CreateOptions

      static Azure::Core::Response<BlobContentInfo> Create(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const CreateOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
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
          std::string key = pair.first;
          std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
          });
          if (metadataKeys.insert(key).second == false)
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
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue());
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobContentInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        auto response_content_md5_iterator = httpResponse.GetHeaders().find("content-md5");
        if (response_content_md5_iterator != httpResponse.GetHeaders().end())
        {
          response.ContentMd5 = response_content_md5_iterator->second;
        }
        auto response_content_crc64_iterator = httpResponse.GetHeaders().find("x-ms-content-crc64");
        if (response_content_crc64_iterator != httpResponse.GetHeaders().end())
        {
          response.ContentCrc64 = response_content_crc64_iterator->second;
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
        return Azure::Core::Response<BlobContentInfo>(
            std::move(response), std::move(pHttpResponse));
      }

      struct UploadPagesOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::pair<int64_t, int64_t> Range;
        Azure::Core::Nullable<std::string> ContentMd5;
        Azure::Core::Nullable<std::string> ContentCrc64;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<int64_t> IfSequenceNumberLessThanOrEqualTo;
        Azure::Core::Nullable<int64_t> IfSequenceNumberLessThan;
        Azure::Core::Nullable<int64_t> IfSequenceNumberEqualTo;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<std::string> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
      }; // struct UploadPagesOptions

      static Azure::Core::Response<PageInfo> UploadPages(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          Azure::Core::Http::BodyStream* requestBody,
          const UploadPagesOptions& options)
      {
        unused(options);
        auto request
            = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, requestBody);
        request.AddHeader("Content-Length", std::to_string(requestBody->Length()));
        request.AddQueryParameter("comp", "page");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        request.AddHeader(
            "x-ms-range",
            "bytes=" + std::to_string(options.Range.first) + "-"
                + std::to_string(options.Range.second));
        if (options.ContentMd5.HasValue())
        {
          request.AddHeader("Content-MD5", options.ContentMd5.GetValue());
        }
        if (options.ContentCrc64.HasValue())
        {
          request.AddHeader("x-ms-content-crc64", options.ContentCrc64.GetValue());
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
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue());
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        PageInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        auto response_content_md5_iterator = httpResponse.GetHeaders().find("content-md5");
        if (response_content_md5_iterator != httpResponse.GetHeaders().end())
        {
          response.ContentMd5 = response_content_md5_iterator->second;
        }
        auto response_content_crc64_iterator = httpResponse.GetHeaders().find("x-ms-content-crc64");
        if (response_content_crc64_iterator != httpResponse.GetHeaders().end())
        {
          response.ContentCrc64 = response_content_crc64_iterator->second;
        }
        response.SequenceNumber
            = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-sequence-number"));
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
        return Azure::Core::Response<PageInfo>(std::move(response), std::move(pHttpResponse));
      }

      struct UploadPagesFromUriOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string SourceUri;
        std::pair<int64_t, int64_t> SourceRange;
        std::pair<int64_t, int64_t> Range;
        Azure::Core::Nullable<std::string> ContentMd5;
        Azure::Core::Nullable<std::string> ContentCrc64;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<int64_t> IfSequenceNumberLessThanOrEqualTo;
        Azure::Core::Nullable<int64_t> IfSequenceNumberLessThan;
        Azure::Core::Nullable<int64_t> IfSequenceNumberEqualTo;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<std::string> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
      }; // struct UploadPagesFromUriOptions

      static Azure::Core::Response<PageInfo> UploadPagesFromUri(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const UploadPagesFromUriOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "page");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
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
        if (options.ContentMd5.HasValue())
        {
          request.AddHeader("x-ms-source-content-md5", options.ContentMd5.GetValue());
        }
        if (options.ContentCrc64.HasValue())
        {
          request.AddHeader("x-ms-source-content-crc64", options.ContentCrc64.GetValue());
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
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue());
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        PageInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        auto response_content_md5_iterator = httpResponse.GetHeaders().find("content-md5");
        if (response_content_md5_iterator != httpResponse.GetHeaders().end())
        {
          response.ContentMd5 = response_content_md5_iterator->second;
        }
        auto response_content_crc64_iterator = httpResponse.GetHeaders().find("x-ms-content-crc64");
        if (response_content_crc64_iterator != httpResponse.GetHeaders().end())
        {
          response.ContentCrc64 = response_content_crc64_iterator->second;
        }
        response.SequenceNumber
            = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-sequence-number"));
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
        return Azure::Core::Response<PageInfo>(std::move(response), std::move(pHttpResponse));
      }

      struct ClearPagesOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::pair<int64_t, int64_t> Range;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<int64_t> IfSequenceNumberLessThanOrEqualTo;
        Azure::Core::Nullable<int64_t> IfSequenceNumberLessThan;
        Azure::Core::Nullable<int64_t> IfSequenceNumberEqualTo;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<std::string> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
      }; // struct ClearPagesOptions

      static Azure::Core::Response<PageInfo> ClearPages(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const ClearPagesOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "page");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
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
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue());
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        PageInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.SequenceNumber
            = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-sequence-number"));
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
        return Azure::Core::Response<PageInfo>(std::move(response), std::move(pHttpResponse));
      }

      struct ResizeOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        int64_t BlobContentLength = -1;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<int64_t> IfSequenceNumberLessThanOrEqualTo;
        Azure::Core::Nullable<int64_t> IfSequenceNumberLessThan;
        Azure::Core::Nullable<int64_t> IfSequenceNumberEqualTo;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<std::string> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
      }; // struct ResizeOptions

      static Azure::Core::Response<PageBlobInfo> Resize(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const ResizeOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "properties");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
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
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue());
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        PageBlobInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.SequenceNumber
            = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-sequence-number"));
        return Azure::Core::Response<PageBlobInfo>(std::move(response), std::move(pHttpResponse));
      }

      struct GetPageRangesOptions
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
      }; // struct GetPageRangesOptions

      static Azure::Core::Response<PageRangesInfoInternal> GetPageRanges(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const GetPageRangesOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddQueryParameter("comp", "pagelist");
        if (options.PreviousSnapshot.HasValue())
        {
          request.AddQueryParameter("prevsnapshot", options.PreviousSnapshot.GetValue());
        }
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        PageRangesInfoInternal response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        {
          const auto& httpResponseBody = httpResponse.GetBody();
          XmlReader reader(
              reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
          response = PageRangesInfoInternalFromXml(reader);
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.BlobContentLength
            = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-content-length"));
        return Azure::Core::Response<PageRangesInfoInternal>(
            std::move(response), std::move(pHttpResponse));
      }

      struct CopyIncrementalOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string CopySource;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
      }; // struct CopyIncrementalOptions

      static Azure::Core::Response<BlobCopyInfo> CopyIncremental(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const CopyIncrementalOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "incrementalcopy");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobCopyInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 202))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        response.CopyId = httpResponse.GetHeaders().at("x-ms-copy-id");
        response.CopyStatus
            = CopyStatusFromString(httpResponse.GetHeaders().at("x-ms-copy-status"));
        return Azure::Core::Response<BlobCopyInfo>(std::move(response), std::move(pHttpResponse));
      }

    private:
      static PageRangesInfoInternal PageRangesInfoInternalFromXml(XmlReader& reader)
      {
        PageRangesInfoInternal ret;
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
      struct CreateOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        BlobHttpHeaders HttpHeaders;
        std::map<std::string, std::string> Metadata;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<std::string> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
      }; // struct CreateOptions

      static Azure::Core::Response<BlobContentInfo> Create(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const CreateOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
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
          std::string key = pair.first;
          std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
          });
          if (metadataKeys.insert(key).second == false)
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
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue());
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobContentInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        auto response_content_md5_iterator = httpResponse.GetHeaders().find("content-md5");
        if (response_content_md5_iterator != httpResponse.GetHeaders().end())
        {
          response.ContentMd5 = response_content_md5_iterator->second;
        }
        auto response_content_crc64_iterator = httpResponse.GetHeaders().find("x-ms-content-crc64");
        if (response_content_crc64_iterator != httpResponse.GetHeaders().end())
        {
          response.ContentCrc64 = response_content_crc64_iterator->second;
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
        return Azure::Core::Response<BlobContentInfo>(
            std::move(response), std::move(pHttpResponse));
      }

      struct AppendBlockOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        Azure::Core::Nullable<std::string> ContentMd5;
        Azure::Core::Nullable<std::string> ContentCrc64;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<int64_t> MaxSize;
        Azure::Core::Nullable<int64_t> AppendPosition;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<std::string> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
      }; // struct AppendBlockOptions

      static Azure::Core::Response<BlobAppendInfo> AppendBlock(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          Azure::Core::Http::BodyStream* requestBody,
          const AppendBlockOptions& options)
      {
        unused(options);
        auto request
            = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, requestBody);
        request.AddHeader("Content-Length", std::to_string(requestBody->Length()));
        request.AddQueryParameter("comp", "appendblock");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
        }
        if (options.ContentMd5.HasValue())
        {
          request.AddHeader("Content-MD5", options.ContentMd5.GetValue());
        }
        if (options.ContentCrc64.HasValue())
        {
          request.AddHeader("x-ms-content-crc64", options.ContentCrc64.GetValue());
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
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue());
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobAppendInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        auto response_content_md5_iterator = httpResponse.GetHeaders().find("content-md5");
        if (response_content_md5_iterator != httpResponse.GetHeaders().end())
        {
          response.ContentMd5 = response_content_md5_iterator->second;
        }
        auto response_content_crc64_iterator = httpResponse.GetHeaders().find("x-ms-content-crc64");
        if (response_content_crc64_iterator != httpResponse.GetHeaders().end())
        {
          response.ContentCrc64 = response_content_crc64_iterator->second;
        }
        response.AppendOffset = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-append-offset"));
        response.CommittedBlockCount
            = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-committed-block-count"));
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
        return Azure::Core::Response<BlobAppendInfo>(std::move(response), std::move(pHttpResponse));
      }

      struct AppendBlockFromUriOptions
      {
        Azure::Core::Nullable<int32_t> Timeout;
        std::string SourceUri;
        Azure::Core::Nullable<std::pair<int64_t, int64_t>> SourceRange;
        Azure::Core::Nullable<std::string> ContentMd5;
        Azure::Core::Nullable<std::string> ContentCrc64;
        Azure::Core::Nullable<std::string> LeaseId;
        Azure::Core::Nullable<int64_t> MaxSize;
        Azure::Core::Nullable<int64_t> AppendPosition;
        Azure::Core::Nullable<std::string> EncryptionKey;
        Azure::Core::Nullable<std::string> EncryptionKeySha256;
        Azure::Core::Nullable<std::string> EncryptionAlgorithm;
        Azure::Core::Nullable<std::string> IfModifiedSince;
        Azure::Core::Nullable<std::string> IfUnmodifiedSince;
        Azure::Core::Nullable<std::string> IfMatch;
        Azure::Core::Nullable<std::string> IfNoneMatch;
      }; // struct AppendBlockFromUriOptions

      static Azure::Core::Response<BlobAppendInfo> AppendBlockFromUri(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const AppendBlockFromUriOptions& options)
      {
        unused(options);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "appendblock");
        request.AddHeader("x-ms-version", c_ApiVersion);
        if (options.Timeout.HasValue())
        {
          request.AddQueryParameter("timeout", std::to_string(options.Timeout.GetValue()));
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
        if (options.ContentMd5.HasValue())
        {
          request.AddHeader("x-ms-source-content-md5", options.ContentMd5.GetValue());
        }
        if (options.ContentCrc64.HasValue())
        {
          request.AddHeader("x-ms-source-content-crc64", options.ContentCrc64.GetValue());
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
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue());
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
        auto pHttpResponse = pipeline.Send(context, request);
        Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
        BlobAppendInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                httpResponse.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw StorageError::CreateFromResponse(context, std::move(pHttpResponse));
        }
        response.ETag = httpResponse.GetHeaders().at("etag");
        response.LastModified = httpResponse.GetHeaders().at("last-modified");
        auto response_content_md5_iterator = httpResponse.GetHeaders().find("content-md5");
        if (response_content_md5_iterator != httpResponse.GetHeaders().end())
        {
          response.ContentMd5 = response_content_md5_iterator->second;
        }
        auto response_content_crc64_iterator = httpResponse.GetHeaders().find("x-ms-content-crc64");
        if (response_content_crc64_iterator != httpResponse.GetHeaders().end())
        {
          response.ContentCrc64 = response_content_crc64_iterator->second;
        }
        response.AppendOffset = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-append-offset"));
        response.CommittedBlockCount
            = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-committed-block-count"));
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
        return Azure::Core::Response<BlobAppendInfo>(std::move(response), std::move(pHttpResponse));
      }

    private:
    }; // class AppendBlob

  }; // class BlobRestClient
}}} // namespace Azure::Storage::Blobs
