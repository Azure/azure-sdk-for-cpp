
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "common/storage_common.hpp"
#include "common/xml_wrapper.hpp"
#include "context.hpp"
#include "http/http.hpp"
#include "http/pipeline.hpp"

#include <cstring>
#include <functional>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace Azure { namespace Storage { namespace Blobs {
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

  struct BasicResponse
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
  }; // struct BasicResponse

  struct BlobAppendInfo
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ETag;
    std::string LastModified;
    std::string ContentMD5;
    std::string ContentCRC64;
    uint64_t AppendOffset = std::numeric_limits<uint64_t>::max();
    uint64_t CommittedBlockCount = std::numeric_limits<uint64_t>::max();
    bool ServerEncrypted = true;
    std::string EncryptionKeySHA256;
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
    uint64_t Size = 0;
  }; // struct BlobBlock

  struct BlobContainerInfo
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ETag;
    std::string LastModified;
  }; // struct BlobContainerInfo

  struct BlobContentInfo
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ETag;
    std::string LastModified;
    std::string ContentMD5;
    std::string ContentCRC64;
    uint64_t SequenceNumber = 0;
    bool ServerEncrypted = true;
    std::string EncryptionKeySHA256;
  }; // struct BlobContentInfo

  struct BlobHttpHeaders
  {
    std::string ContentType;
    std::string ContentEncoding;
    std::string ContentLanguage;
    std::string ContentMD5;
    std::string CacheControl;
    std::string ContentDisposition;
  }; // struct BlobHttpHeaders

  struct BlobInfo
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ETag;
    std::string LastModified;
    uint64_t SequenceNumber = 0;
  }; // struct BlobInfo

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

  struct BlobSnapshotInfo
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string Snapshot;
    std::string ETag;
    std::string LastModified;
    bool ServerEncrypted = true;
    std::string EncryptionKeySHA256;
  }; // struct BlobSnapshotInfo

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
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ContentMD5;
    std::string ContentCRC64;
    bool ServerEncrypted = true;
    std::string EncryptionKeySHA256;
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
    None,
    Metadata,
  }; // enum class ListBlobContainersIncludeOption

  inline std::string ListBlobContainersIncludeOptionToString(
      const ListBlobContainersIncludeOption& list_blob_containers_include_option)
  {
    switch (list_blob_containers_include_option)
    {
      case ListBlobContainersIncludeOption::None:
        return "";
      case ListBlobContainersIncludeOption::Metadata:
        return "metadata";
      default:
        return std::string();
    }
  }

  inline ListBlobContainersIncludeOption ListBlobContainersIncludeOptionFromString(
      const std::string& list_blob_containers_include_option)
  {
    if (list_blob_containers_include_option == "")
    {
      return ListBlobContainersIncludeOption::None;
    }
    if (list_blob_containers_include_option == "metadata")
    {
      return ListBlobContainersIncludeOption::Metadata;
    }
    throw std::runtime_error(
        "cannot convert " + list_blob_containers_include_option
        + " to ListBlobContainersIncludeOption");
  }

  enum class ListBlobsIncludeItem
  {
    Copy,
    Deleted,
    Metadata,
    Snapshots,
    UncomittedBlobs,
  }; // enum class ListBlobsIncludeItem

  inline std::string ListBlobsIncludeItemToString(
      const ListBlobsIncludeItem& list_blobs_include_item)
  {
    switch (list_blobs_include_item)
    {
      case ListBlobsIncludeItem::Copy:
        return "copy";
      case ListBlobsIncludeItem::Deleted:
        return "deleted";
      case ListBlobsIncludeItem::Metadata:
        return "metadata";
      case ListBlobsIncludeItem::Snapshots:
        return "snapshots";
      case ListBlobsIncludeItem::UncomittedBlobs:
        return "uncommittedblobs";
      default:
        return std::string();
    }
  }

  inline ListBlobsIncludeItem ListBlobsIncludeItemFromString(
      const std::string& list_blobs_include_item)
  {
    if (list_blobs_include_item == "copy")
    {
      return ListBlobsIncludeItem::Copy;
    }
    if (list_blobs_include_item == "deleted")
    {
      return ListBlobsIncludeItem::Deleted;
    }
    if (list_blobs_include_item == "metadata")
    {
      return ListBlobsIncludeItem::Metadata;
    }
    if (list_blobs_include_item == "snapshots")
    {
      return ListBlobsIncludeItem::Snapshots;
    }
    if (list_blobs_include_item == "uncommittedblobs")
    {
      return ListBlobsIncludeItem::UncomittedBlobs;
    }
    throw std::runtime_error(
        "cannot convert " + list_blobs_include_item + " to ListBlobsIncludeItem");
  }

  struct PageBlobInfo
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ETag;
    std::string LastModified;
    uint64_t SequenceNumber = 0;
  }; // struct PageBlobInfo

  struct PageInfo
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ETag;
    std::string LastModified;
    std::string ContentMD5;
    std::string ContentCRC64;
    uint64_t SequenceNumber = 0;
    bool ServerEncrypted = true;
    std::string EncryptionKeySHA256;
  }; // struct PageInfo

  struct PageRangesInfoInternal
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ETag;
    std::string LastModified;
    uint64_t BlobContentLength = 0;
    std::vector<std::pair<uint64_t, uint64_t>> PageRanges;
    std::vector<std::pair<uint64_t, uint64_t>> ClearRanges;
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

  struct UserDelegationKey
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string SignedObjectId;
    std::string SignedTenantId;
    std::string SignedStartsOn;
    std::string SignedExpiresOn;
    std::string SignedService;
    std::string SignedVersion;
    std::string Value;
  }; // struct UserDelegationKey

  struct BlobBlockListInfo
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ETag;
    std::string LastModified;
    std::string ContentType;
    uint64_t ContentLength = 0;
    std::vector<BlobBlock> CommittedBlocks;
    std::vector<BlobBlock> UncommittedBlocks;
  }; // struct BlobBlockListInfo

  struct BlobContainerItem
  {
    std::string Name;
    std::string ETag;
    std::string LastModified;
    std::map<std::string, std::string> Metadata;
    PublicAccessType AccessType = PublicAccessType::Private;
    bool HasImmutabilityPolicy = false;
    bool HasLegalHold = false;
    std::string LeaseDuration;
    BlobLeaseState LeaseState = BlobLeaseState::Available;
    BlobLeaseStatus LeaseStatus = BlobLeaseStatus::Unlocked;
  }; // struct BlobContainerItem

  struct BlobContainerProperties
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ETag;
    std::string LastModified;
    std::map<std::string, std::string> Metadata;
    PublicAccessType AccessType = PublicAccessType::Private;
    bool HasImmutabilityPolicy = false;
    bool HasLegalHold = false;
    std::string LeaseDuration;
    BlobLeaseState LeaseState = BlobLeaseState::Available;
    BlobLeaseStatus LeaseStatus = BlobLeaseStatus::Unlocked;
  }; // struct BlobContainerProperties

  struct BlobCopyInfo
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ETag;
    std::string LastModified;
    std::string CopyId;
    Blobs::CopyStatus CopyStatus = Blobs::CopyStatus::Unknown;
  }; // struct BlobCopyInfo

  struct BlobItem
  {
    std::string Name;
    bool Deleted = false;
    std::string Snapshot;
    BlobHttpHeaders Properties;
    std::map<std::string, std::string> Metadata;
    std::string CreationTime;
    std::string LastModified;
    std::string ETag;
    uint64_t ContentLength = 0;
    Blobs::BlobType BlobType = Blobs::BlobType::Unknown;
    AccessTier Tier = AccessTier::Unknown;
    bool AccessTierInferred = true;
    BlobLeaseStatus LeaseStatus = BlobLeaseStatus::Unlocked;
    BlobLeaseState LeaseState = BlobLeaseState::Available;
    std::string LeaseDuration;
    bool ServerEncrypted = true;
    std::string EncryptionKeySHA256;
  }; // struct BlobItem

  struct BlobProperties
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ETag;
    std::string LastModified;
    std::string CreationTime;
    std::map<std::string, std::string> Metadata;
    Blobs::BlobType BlobType = Blobs::BlobType::Unknown;
    std::string LeaseDuration;
    BlobLeaseState LeaseState = BlobLeaseState::Available;
    BlobLeaseStatus LeaseStatus = BlobLeaseStatus::Unlocked;
    uint64_t ContentLength = 0;
    std::string ContentType;
    std::string ContentEncoding;
    std::string ContentLanguage;
    std::string ContentMD5;
    std::string CacheControl;
    std::string ContentDisposition;
    uint64_t SequenceNumber = 0; // only for page blob
    int CommittedBlockCount = 0; // only for append blob
    bool ServerEncrypted = true;
    std::string EncryptionKeySHA256;
    AccessTier Tier = AccessTier::Unknown;
    bool AccessTierInferred = true;
    BlobArchiveStatus ArchiveStatus = BlobArchiveStatus::Unknown;
    std::string AccessTierChangeTime;
  }; // struct BlobProperties

  struct FlattenedDownloadProperties
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::unique_ptr<Azure::Core::Http::BodyStream> BodyStream = nullptr;
    std::string ETag;
    std::string LastModified;
    std::string ContentRange;
    BlobHttpHeaders Properties;
    std::map<std::string, std::string> Metadata;
    uint64_t SequenceNumber = 0; // only for page blob
    uint64_t CommittedBlockCount = 0; // only for append blob
    Blobs::BlobType BlobType = Blobs::BlobType::Unknown;
    std::string ContentMD5; // MD5 for the downloaded range
    std::string ContentCRC64;
    std::string LeaseDuration;
    BlobLeaseState LeaseState = BlobLeaseState::Available;
    BlobLeaseStatus LeaseStatus = BlobLeaseStatus::Unlocked;
    bool ServerEncrypted = true;
    std::string EncryptionKeySHA256;
  }; // struct FlattenedDownloadProperties

  struct BlobsFlatSegment
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ServiceEndpoint;
    std::string Container;
    std::string Prefix;
    std::string Marker;
    std::string NextMarker;
    int MaxResults = 0;
    std::string Delimiter;
    std::vector<BlobItem> BlobItems;
  }; // struct BlobsFlatSegment

  struct ListContainersSegment
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ServiceEndpoint;
    std::string Prefix;
    std::string Marker;
    std::string NextMarker;
    int MaxResults = 0;
    std::vector<BlobContainerItem> BlobContainerItems;
  }; // struct ListContainersSegment

  class BlobRestClient {
  public:
    class Service {
    public:
      struct ListBlobContainersOptions
      {
        std::string Prefix;
        std::string Marker;
        int MaxResults = 0;
        ListBlobContainersIncludeOption IncludeMetadata = ListBlobContainersIncludeOption::None;
      }; // struct ListBlobContainersOptions

      static Azure::Core::Http::Request ListBlobContainersConstructRequest(
          const std::string& url,
          const ListBlobContainersOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", "2019-07-07");
        request.AddQueryParameter("comp", "list");
        if (!options.Prefix.empty())
        {
          request.AddQueryParameter("prefix", options.Prefix);
        }
        if (!options.Marker.empty())
        {
          request.AddQueryParameter("marker", options.Marker);
        }
        if (options.MaxResults != 0)
        {
          request.AddQueryParameter("maxresults", std::to_string(options.MaxResults));
        }
        std::string list_blob_containers_include_option
            = ListBlobContainersIncludeOptionToString(options.IncludeMetadata);
        if (!list_blob_containers_include_option.empty())
        {
          request.AddQueryParameter("include", list_blob_containers_include_option);
        }
        return request;
      }

      static ListContainersSegment ListBlobContainersParseResponse(
          Azure::Core::Http::Response& http_response)
      {
        ListContainersSegment response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        auto bodyStream = http_response.GetBodyStream();
        std::vector<uint8_t> bodyContent(static_cast<std::size_t>(bodyStream->Length()));
        bodyStream->Read(&bodyContent[0], bodyContent.size());
        XmlReader reader(reinterpret_cast<const char*>(bodyContent.data()), bodyContent.size());
        response = ListContainersSegmentFromXml(reader);
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        return response;
      }

      static ListContainersSegment ListBlobContainers(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const ListBlobContainersOptions& options)
      {
        auto request = ListBlobContainersConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return ListBlobContainersParseResponse(*response);
      }

      struct GetUserDelegationKeyOptions
      {
        std::string StartsOn;
        std::string ExpiresOn;
      }; // struct GetUserDelegationKeyOptions

      static Azure::Core::Http::Request GetUserDelegationKeyConstructRequest(
          const std::string& url,
          const GetUserDelegationKeyOptions& options)
      {
        XmlWriter writer;
        GetUserDelegationKeyOptionsToXml(writer, options);
        std::string xml_body = writer.GetDocument();
        std::vector<uint8_t> body_buffer(xml_body.begin(), xml_body.end());
        uint64_t body_buffer_length = body_buffer.size();
        auto request = Azure::Core::Http::Request(
            Azure::Core::Http::HttpMethod::Post,
            url,
            std::make_unique<Azure::Core::Http::MemoryBodyStream>(std::move(body_buffer)));
        request.AddHeader("Content-Length", std::to_string(body_buffer_length));
        request.AddQueryParameter("restype", "service");
        request.AddQueryParameter("comp", "userdelegationkey");
        request.AddHeader("x-ms-version", "2019-07-07");
        unused(options);
        return request;
      }

      static UserDelegationKey GetUserDelegationKeyParseResponse(
          Azure::Core::Http::Response& http_response)
      {
        UserDelegationKey response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        auto bodyStream = http_response.GetBodyStream();
        std::vector<uint8_t> bodyContent(static_cast<std::size_t>(bodyStream->Length()));
        bodyStream->Read(&bodyContent[0], bodyContent.size());
        XmlReader reader(reinterpret_cast<const char*>(bodyContent.data()), bodyContent.size());
        response = UserDelegationKeyFromXml(reader);
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        return response;
      }

      static UserDelegationKey GetUserDelegationKey(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const GetUserDelegationKeyOptions& options)
      {
        auto request = GetUserDelegationKeyConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return GetUserDelegationKeyParseResponse(*response);
      }

    private:
      static ListContainersSegment ListContainersSegmentFromXml(XmlReader& reader)
      {
        ListContainersSegment ret;
        enum class XmlTagName
        {
          k_EnumerationResults,
          k_Prefix,
          k_Marker,
          k_NextMarker,
          k_MaxResults,
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
            else if (std::strcmp(node.Name, "MaxResults") == 0)
            {
              path.emplace_back(XmlTagName::k_MaxResults);
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
              ret.BlobContainerItems.emplace_back(BlobContainerItemFromXml(reader));
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
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_MaxResults)
            {
              ret.MaxResults = std::stoi(node.Value);
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
        writer.Write(XmlNode{XmlNodeType::End});
      }

    }; // class Service

    class Container {
    public:
      struct CreateOptions
      {
        PublicAccessType AccessType = PublicAccessType::Private;
        std::map<std::string, std::string> Metadata;
      }; // struct CreateOptions

      static Azure::Core::Http::Request CreateConstructRequest(
          const std::string& url,
          const CreateOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("restype", "container");
        request.AddHeader("x-ms-version", "2019-07-07");
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
        auto options_accesstype_str = PublicAccessTypeToString(options.AccessType);
        if (!options_accesstype_str.empty())
        {
          request.AddHeader("x-ms-blob-public-access", options_accesstype_str);
        }
        return request;
      }

      static BlobContainerInfo CreateParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlobContainerInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        return response;
      }

      static BlobContainerInfo Create(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const CreateOptions& options)
      {
        auto request = CreateConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return CreateParseResponse(*response);
      }

      struct DeleteOptions
      {
        std::string IfModifiedSince;
        std::string IfUnmodifiedSince;
      }; // struct DeleteOptions

      static Azure::Core::Http::Request DeleteConstructRequest(
          const std::string& url,
          const DeleteOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Delete, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("restype", "container");
        request.AddHeader("x-ms-version", "2019-07-07");
        if (!options.IfModifiedSince.empty())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince);
        }
        if (!options.IfUnmodifiedSince.empty())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince);
        }
        return request;
      }

      static BasicResponse DeleteParseResponse(Azure::Core::Http::Response& http_response)
      {
        BasicResponse response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 202))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        return response;
      }

      static BasicResponse Delete(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const DeleteOptions& options)
      {
        auto request = DeleteConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return DeleteParseResponse(*response);
      }

      struct GetPropertiesOptions
      {
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
      }; // struct GetPropertiesOptions

      static Azure::Core::Http::Request GetPropertiesConstructRequest(
          const std::string& url,
          const GetPropertiesOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("restype", "container");
        request.AddHeader("x-ms-version", "2019-07-07");
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        return request;
      }

      static BlobContainerProperties GetPropertiesParseResponse(
          Azure::Core::Http::Response& http_response)
      {
        BlobContainerProperties response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        for (auto i = http_response.GetHeaders().lower_bound("x-ms-meta-");
             i != http_response.GetHeaders().end() && i->first.substr(0, 10) == "x-ms-meta-";
             ++i)
        {
          response.Metadata.emplace(i->first.substr(10), i->second);
        }
        auto response_accesstype_iterator
            = http_response.GetHeaders().find("x-ms-blob-public-access");
        if (response_accesstype_iterator != http_response.GetHeaders().end())
        {
          response.AccessType = PublicAccessTypeFromString(response_accesstype_iterator->second);
        }
        response.HasImmutabilityPolicy
            = http_response.GetHeaders().at("x-ms-has-immutability-policy") == "true";
        response.HasLegalHold = http_response.GetHeaders().at("x-ms-has-legal-hold") == "true";
        auto response_leasestatus_iterator = http_response.GetHeaders().find("x-ms-lease-status");
        if (response_leasestatus_iterator != http_response.GetHeaders().end())
        {
          response.LeaseStatus = BlobLeaseStatusFromString(response_leasestatus_iterator->second);
        }
        auto response_leasestate_iterator = http_response.GetHeaders().find("x-ms-lease-state");
        if (response_leasestate_iterator != http_response.GetHeaders().end())
        {
          response.LeaseState = BlobLeaseStateFromString(response_leasestate_iterator->second);
        }
        auto response_leaseduration_iterator
            = http_response.GetHeaders().find("x-ms-lease-duration");
        if (response_leaseduration_iterator != http_response.GetHeaders().end())
        {
          response.LeaseDuration = response_leaseduration_iterator->second;
        }
        return response;
      }

      static BlobContainerProperties GetProperties(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const GetPropertiesOptions& options)
      {
        auto request = GetPropertiesConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return GetPropertiesParseResponse(*response);
      }

      struct SetMetadataOptions
      {
        std::map<std::string, std::string> Metadata;
        std::string IfModifiedSince;
      }; // struct SetMetadataOptions

      static Azure::Core::Http::Request SetMetadataConstructRequest(
          const std::string& url,
          const SetMetadataOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("restype", "container");
        request.AddQueryParameter("comp", "metadata");
        request.AddHeader("x-ms-version", "2019-07-07");
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
        if (!options.IfModifiedSince.empty())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince);
        }
        return request;
      }

      static BlobContainerInfo SetMetadataParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlobContainerInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        return response;
      }

      static BlobContainerInfo SetMetadata(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const SetMetadataOptions& options)
      {
        auto request = SetMetadataConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return SetMetadataParseResponse(*response);
      }

      struct ListBlobsOptions
      {
        std::string Prefix;
        std::string Delimiter;
        std::string Marker;
        int MaxResults = 0;
        std::vector<ListBlobsIncludeItem> Include;
      }; // struct ListBlobsOptions

      static Azure::Core::Http::Request ListBlobsConstructRequest(
          const std::string& url,
          const ListBlobsOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", "2019-07-07");
        request.AddQueryParameter("restype", "container");
        request.AddQueryParameter("comp", "list");
        if (!options.Prefix.empty())
        {
          request.AddQueryParameter("prefix", options.Prefix);
        }
        if (!options.Delimiter.empty())
        {
          request.AddQueryParameter("delimiter", options.Delimiter);
        }
        if (!options.Marker.empty())
        {
          request.AddQueryParameter("marker", options.Marker);
        }
        if (options.MaxResults != 0)
        {
          request.AddQueryParameter("maxresults", std::to_string(options.MaxResults));
        }
        std::string options_include_str;
        for (auto i : options.Include)
        {
          if (!options_include_str.empty())
          {
            options_include_str += ",";
          }
          options_include_str += ListBlobsIncludeItemToString(i);
        }
        if (!options_include_str.empty())
        {
          request.AddQueryParameter("include", options_include_str);
        }
        return request;
      }

      static BlobsFlatSegment ListBlobsParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlobsFlatSegment response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        auto bodyStream = http_response.GetBodyStream();
        std::vector<uint8_t> bodyContent(static_cast<std::size_t>(bodyStream->Length()));
        bodyStream->Read(&bodyContent[0], bodyContent.size());
        XmlReader reader(reinterpret_cast<const char*>(bodyContent.data()), bodyContent.size());
        response = BlobsFlatSegmentFromXml(reader);
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        return response;
      }

      static BlobsFlatSegment ListBlobs(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const ListBlobsOptions& options)
      {
        auto request = ListBlobsConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return ListBlobsParseResponse(*response);
      }

    private:
      static BlobsFlatSegment BlobsFlatSegmentFromXml(XmlReader& reader)
      {
        BlobsFlatSegment ret;
        enum class XmlTagName
        {
          k_EnumerationResults,
          k_Prefix,
          k_Marker,
          k_NextMarker,
          k_MaxResults,
          k_Delimiter,
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
            else if (std::strcmp(node.Name, "MaxResults") == 0)
            {
              path.emplace_back(XmlTagName::k_MaxResults);
            }
            else if (std::strcmp(node.Name, "Delimiter") == 0)
            {
              path.emplace_back(XmlTagName::k_Delimiter);
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
              ret.BlobItems.emplace_back(BlobItemFromXml(reader));
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
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_MaxResults)
            {
              ret.MaxResults = std::stoi(node.Value);
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                && path[1] == XmlTagName::k_Delimiter)
            {
              ret.Delimiter = node.Value;
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
              ret.Properties.ContentType = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_ContentEncoding)
            {
              ret.Properties.ContentEncoding = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_ContentLanguage)
            {
              ret.Properties.ContentLanguage = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_ContentMD5)
            {
              ret.Properties.ContentMD5 = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_CacheControl)
            {
              ret.Properties.CacheControl = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::k_Properties
                && path[1] == XmlTagName::k_ContentDisposition)
            {
              ret.Properties.ContentDisposition = node.Value;
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
              ret.ContentLength = std::stoull(node.Value);
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
              ret.EncryptionKeySHA256 = node.Value;
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

    }; // class Container

    class Blob {
    public:
      struct DownloadOptions
      {
        std::pair<uint64_t, uint64_t> Range;
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
        std::string IfModifiedSince;
        std::string IfUnmodifiedSince;
        std::string IfMatch;
        std::string IfNoneMatch;
      }; // struct DownloadOptions

      static Azure::Core::Http::Request DownloadConstructRequest(
          const std::string& url,
          const DownloadOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", "2019-07-07");
        if (options.Range.first == std::numeric_limits<decltype(options.Range.first)>::max())
        {
          // do nothing
        }
        else if (options.Range.second == std::numeric_limits<decltype(options.Range.second)>::max())
        {
          request.AddHeader("x-ms-range", "bytes=" + std::to_string(options.Range.first) + "-");
        }
        else
        {
          request.AddHeader(
              "x-ms-range",
              "bytes=" + std::to_string(options.Range.first) + "-"
                  + std::to_string(options.Range.second));
        }
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        if (!options.IfModifiedSince.empty())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince);
        }
        if (!options.IfUnmodifiedSince.empty())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince);
        }
        if (!options.IfMatch.empty())
        {
          request.AddHeader("If-Match", options.IfMatch);
        }
        if (!options.IfNoneMatch.empty())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch);
        }
        return request;
      }

      static FlattenedDownloadProperties DownloadParseResponse(
          Azure::Core::Http::Response& http_response)
      {
        FlattenedDownloadProperties response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200 || http_status_code == 206))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        auto response_contentmd5_iterator = http_response.GetHeaders().find("Content-MD5");
        if (response_contentmd5_iterator != http_response.GetHeaders().end())
        {
          response.ContentMD5 = response_contentmd5_iterator->second;
        }
        auto response_contentcrc64_iterator = http_response.GetHeaders().find("x-ms-content-crc64");
        if (response_contentcrc64_iterator != http_response.GetHeaders().end())
        {
          response.ContentCRC64 = response_contentcrc64_iterator->second;
        }
        auto response_properties_contenttype_iterator
            = http_response.GetHeaders().find("Content-Type");
        if (response_properties_contenttype_iterator != http_response.GetHeaders().end())
        {
          response.Properties.ContentType = response_properties_contenttype_iterator->second;
        }
        auto response_properties_contentencoding_iterator
            = http_response.GetHeaders().find("Content-Encoding");
        if (response_properties_contentencoding_iterator != http_response.GetHeaders().end())
        {
          response.Properties.ContentEncoding
              = response_properties_contentencoding_iterator->second;
        }
        auto response_properties_contentlanguage_iterator
            = http_response.GetHeaders().find("Content-Language");
        if (response_properties_contentlanguage_iterator != http_response.GetHeaders().end())
        {
          response.Properties.ContentLanguage
              = response_properties_contentlanguage_iterator->second;
        }
        auto response_properties_cachecontrol_iterator
            = http_response.GetHeaders().find("Cache-Control");
        if (response_properties_cachecontrol_iterator != http_response.GetHeaders().end())
        {
          response.Properties.CacheControl = response_properties_cachecontrol_iterator->second;
        }
        auto response_properties_contentmd5_iterator
            = http_response.GetHeaders().find("Content-MD5");
        if (response_properties_contentmd5_iterator != http_response.GetHeaders().end())
        {
          response.Properties.ContentMD5 = response_properties_contentmd5_iterator->second;
        }
        auto response_properties_contentdisposition_iterator
            = http_response.GetHeaders().find("Content-Disposition");
        if (response_properties_contentdisposition_iterator != http_response.GetHeaders().end())
        {
          response.Properties.ContentDisposition
              = response_properties_contentdisposition_iterator->second;
        }
        for (auto i = http_response.GetHeaders().lower_bound("x-ms-meta-");
             i != http_response.GetHeaders().end() && i->first.substr(0, 10) == "x-ms-meta-";
             ++i)
        {
          response.Metadata.emplace(i->first.substr(10), i->second);
        }
        auto response_serverencrypted_iterator
            = http_response.GetHeaders().find("x-ms-server-encrypted");
        if (response_serverencrypted_iterator != http_response.GetHeaders().end())
        {
          response.ServerEncrypted = response_serverencrypted_iterator->second == "true";
        }
        auto response_encryptionkeysha256_iterator
            = http_response.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryptionkeysha256_iterator != http_response.GetHeaders().end())
        {
          response.EncryptionKeySHA256 = response_encryptionkeysha256_iterator->second;
        }
        auto response_leasestatus_iterator = http_response.GetHeaders().find("x-ms-lease-status");
        if (response_leasestatus_iterator != http_response.GetHeaders().end())
        {
          response.LeaseStatus = BlobLeaseStatusFromString(response_leasestatus_iterator->second);
        }
        auto response_leasestate_iterator = http_response.GetHeaders().find("x-ms-lease-state");
        if (response_leasestate_iterator != http_response.GetHeaders().end())
        {
          response.LeaseState = BlobLeaseStateFromString(response_leasestate_iterator->second);
        }
        auto response_leaseduration_iterator
            = http_response.GetHeaders().find("x-ms-lease-duration");
        if (response_leaseduration_iterator != http_response.GetHeaders().end())
        {
          response.LeaseDuration = response_leaseduration_iterator->second;
        }
        auto response_contentrange_iterator = http_response.GetHeaders().find("Content-Range");
        if (response_contentrange_iterator != http_response.GetHeaders().end())
        {
          response.ContentRange = response_contentrange_iterator->second;
        }
        auto response_sequencenumber_iterator
            = http_response.GetHeaders().find("x-ms-blob-sequence-number");
        if (response_sequencenumber_iterator != http_response.GetHeaders().end())
        {
          response.SequenceNumber = std::stoull(response_sequencenumber_iterator->second);
        }
        auto response_committedblockcount_iterator
            = http_response.GetHeaders().find("x-ms-blob-committed-block-count");
        if (response_committedblockcount_iterator != http_response.GetHeaders().end())
        {
          response.CommittedBlockCount = std::stoull(response_committedblockcount_iterator->second);
        }
        response.BlobType = BlobTypeFromString(http_response.GetHeaders().at("x-ms-blob-type"));
        response.BodyStream = std::move(http_response.GetBodyStream());
        return response;
      }

      static FlattenedDownloadProperties Download(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const DownloadOptions& options)
      {
        auto request = DownloadConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return DownloadParseResponse(*response);
      }

      struct DeleteOptions
      {
        DeleteSnapshotsOption DeleteSnapshots = DeleteSnapshotsOption::None;
        std::string IfModifiedSince;
        std::string IfUnmodifiedSince;
        std::string IfMatch;
        std::string IfNoneMatch;
      }; // struct DeleteOptions

      static Azure::Core::Http::Request DeleteConstructRequest(
          const std::string& url,
          const DeleteOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Delete, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", "2019-07-07");
        auto options_deletesnapshots_str = DeleteSnapshotsOptionToString(options.DeleteSnapshots);
        if (!options_deletesnapshots_str.empty())
        {
          request.AddHeader("x-ms-delete-snapshots", options_deletesnapshots_str);
        }
        if (!options.IfModifiedSince.empty())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince);
        }
        if (!options.IfUnmodifiedSince.empty())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince);
        }
        if (!options.IfMatch.empty())
        {
          request.AddHeader("If-Match", options.IfMatch);
        }
        if (!options.IfNoneMatch.empty())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch);
        }
        return request;
      }

      static BasicResponse DeleteParseResponse(Azure::Core::Http::Response& http_response)
      {
        BasicResponse response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 202))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        return response;
      }

      static BasicResponse Delete(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const DeleteOptions& options)
      {
        auto request = DeleteConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return DeleteParseResponse(*response);
      }

      struct UndeleteOptions
      {
      }; // struct UndeleteOptions

      static Azure::Core::Http::Request UndeleteConstructRequest(
          const std::string& url,
          const UndeleteOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", "2019-07-07");
        request.AddQueryParameter("comp", "undelete");
        unused(options);
        return request;
      }

      static BasicResponse UndeleteParseResponse(Azure::Core::Http::Response& http_response)
      {
        BasicResponse response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        return response;
      }

      static BasicResponse Undelete(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const UndeleteOptions& options)
      {
        auto request = UndeleteConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return UndeleteParseResponse(*response);
      }

      struct GetPropertiesOptions
      {
        std::string IfModifiedSince;
        std::string IfUnmodifiedSince;
        std::string IfMatch;
        std::string IfNoneMatch;
      }; // struct GetPropertiesOptions

      static Azure::Core::Http::Request GetPropertiesConstructRequest(
          const std::string& url,
          const GetPropertiesOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", "2019-07-07");
        if (!options.IfModifiedSince.empty())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince);
        }
        if (!options.IfUnmodifiedSince.empty())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince);
        }
        if (!options.IfMatch.empty())
        {
          request.AddHeader("If-Match", options.IfMatch);
        }
        if (!options.IfNoneMatch.empty())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch);
        }
        return request;
      }

      static BlobProperties GetPropertiesParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlobProperties response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        response.CreationTime = http_response.GetHeaders().at("x-ms-creation-time");
        for (auto i = http_response.GetHeaders().lower_bound("x-ms-meta-");
             i != http_response.GetHeaders().end() && i->first.substr(0, 10) == "x-ms-meta-";
             ++i)
        {
          response.Metadata.emplace(i->first.substr(10), i->second);
        }
        response.BlobType = BlobTypeFromString(http_response.GetHeaders().at("x-ms-blob-type"));
        auto response_leasestatus_iterator = http_response.GetHeaders().find("x-ms-lease-status");
        if (response_leasestatus_iterator != http_response.GetHeaders().end())
        {
          response.LeaseStatus = BlobLeaseStatusFromString(response_leasestatus_iterator->second);
        }
        auto response_leasestate_iterator = http_response.GetHeaders().find("x-ms-lease-state");
        if (response_leasestate_iterator != http_response.GetHeaders().end())
        {
          response.LeaseState = BlobLeaseStateFromString(response_leasestate_iterator->second);
        }
        auto response_leaseduration_iterator
            = http_response.GetHeaders().find("x-ms-lease-duration");
        if (response_leaseduration_iterator != http_response.GetHeaders().end())
        {
          response.LeaseDuration = response_leaseduration_iterator->second;
        }
        response.ContentLength = std::stoull(http_response.GetHeaders().at("Content-Length"));
        auto response_contenttype_iterator = http_response.GetHeaders().find("Content-Type");
        if (response_contenttype_iterator != http_response.GetHeaders().end())
        {
          response.ContentType = response_contenttype_iterator->second;
        }
        auto response_contentencoding_iterator
            = http_response.GetHeaders().find("Content-Encoding");
        if (response_contentencoding_iterator != http_response.GetHeaders().end())
        {
          response.ContentEncoding = response_contentencoding_iterator->second;
        }
        auto response_contentlanguage_iterator
            = http_response.GetHeaders().find("Content-Language");
        if (response_contentlanguage_iterator != http_response.GetHeaders().end())
        {
          response.ContentLanguage = response_contentlanguage_iterator->second;
        }
        auto response_cachecontrol_iterator = http_response.GetHeaders().find("Cache-Control");
        if (response_cachecontrol_iterator != http_response.GetHeaders().end())
        {
          response.CacheControl = response_cachecontrol_iterator->second;
        }
        auto response_contentmd5_iterator = http_response.GetHeaders().find("Content-MD5");
        if (response_contentmd5_iterator != http_response.GetHeaders().end())
        {
          response.ContentMD5 = response_contentmd5_iterator->second;
        }
        auto response_contentdisposition_iterator
            = http_response.GetHeaders().find("Content-Disposition");
        if (response_contentdisposition_iterator != http_response.GetHeaders().end())
        {
          response.ContentDisposition = response_contentdisposition_iterator->second;
        }
        auto response_sequencenumber_iterator
            = http_response.GetHeaders().find("x-ms-blob-sequence-number");
        if (response_sequencenumber_iterator != http_response.GetHeaders().end())
        {
          response.SequenceNumber = std::stoull(response_sequencenumber_iterator->second);
        }
        auto response_committedblockcount_iterator
            = http_response.GetHeaders().find("x-ms-blob-committed-block-count");
        if (response_committedblockcount_iterator != http_response.GetHeaders().end())
        {
          response.CommittedBlockCount = std::stoi(response_committedblockcount_iterator->second);
        }
        auto response_serverencrypted_iterator
            = http_response.GetHeaders().find("x-ms-server-encrypted");
        if (response_serverencrypted_iterator != http_response.GetHeaders().end())
        {
          response.ServerEncrypted = response_serverencrypted_iterator->second == "true";
        }
        auto response_encryptionkeysha256_iterator
            = http_response.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryptionkeysha256_iterator != http_response.GetHeaders().end())
        {
          response.EncryptionKeySHA256 = response_encryptionkeysha256_iterator->second;
        }
        auto response_tier_iterator = http_response.GetHeaders().find("x-ms-access-tier");
        if (response_tier_iterator != http_response.GetHeaders().end())
        {
          response.Tier = AccessTierFromString(response_tier_iterator->second);
        }
        auto response_accesstierinferred_iterator
            = http_response.GetHeaders().find("x-ms-access-tier-inferred");
        if (response_accesstierinferred_iterator != http_response.GetHeaders().end())
        {
          response.AccessTierInferred = response_accesstierinferred_iterator->second == "true";
        }
        auto response_archivestatus_iterator
            = http_response.GetHeaders().find("x-ms-archive-status");
        if (response_archivestatus_iterator != http_response.GetHeaders().end())
        {
          response.ArchiveStatus
              = BlobArchiveStatusFromString(response_archivestatus_iterator->second);
        }
        auto response_accesstierchangetime_iterator
            = http_response.GetHeaders().find("x-ms-access-tier-change-time");
        if (response_accesstierchangetime_iterator != http_response.GetHeaders().end())
        {
          response.AccessTierChangeTime = response_accesstierchangetime_iterator->second;
        }
        return response;
      }

      static BlobProperties GetProperties(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const GetPropertiesOptions& options)
      {
        auto request = GetPropertiesConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return GetPropertiesParseResponse(*response);
      }

      struct SetHttpHeadersOptions
      {
        std::string ContentType;
        std::string ContentEncoding;
        std::string ContentLanguage;
        std::string ContentMD5;
        std::string CacheControl;
        std::string ContentDisposition;
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
        std::string IfModifiedSince;
        std::string IfUnmodifiedSince;
        std::string IfMatch;
        std::string IfNoneMatch;
      }; // struct SetHttpHeadersOptions

      static Azure::Core::Http::Request SetHttpHeadersConstructRequest(
          const std::string& url,
          const SetHttpHeadersOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "properties");
        request.AddHeader("x-ms-version", "2019-07-07");
        if (!options.ContentType.empty())
        {
          request.AddHeader("x-ms-blob-content-type", options.ContentType);
        }
        if (!options.ContentEncoding.empty())
        {
          request.AddHeader("x-ms-blob-content-encoding", options.ContentEncoding);
        }
        if (!options.ContentLanguage.empty())
        {
          request.AddHeader("x-ms-blob-content-language", options.ContentLanguage);
        }
        if (!options.CacheControl.empty())
        {
          request.AddHeader("x-ms-blob-cache-control", options.CacheControl);
        }
        if (!options.ContentMD5.empty())
        {
          request.AddHeader("x-ms-blob-content-md5", options.ContentMD5);
        }
        if (!options.ContentDisposition.empty())
        {
          request.AddHeader("x-ms-blob-content-disposition", options.ContentDisposition);
        }
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        if (!options.IfModifiedSince.empty())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince);
        }
        if (!options.IfUnmodifiedSince.empty())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince);
        }
        if (!options.IfMatch.empty())
        {
          request.AddHeader("If-Match", options.IfMatch);
        }
        if (!options.IfNoneMatch.empty())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch);
        }
        return request;
      }

      static BlobInfo SetHttpHeadersParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlobInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        auto response_sequencenumber_iterator
            = http_response.GetHeaders().find("x-ms-blob-sequence-number");
        if (response_sequencenumber_iterator != http_response.GetHeaders().end())
        {
          response.SequenceNumber = std::stoull(response_sequencenumber_iterator->second);
        }
        return response;
      }

      static BlobInfo SetHttpHeaders(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const SetHttpHeadersOptions& options)
      {
        auto request = SetHttpHeadersConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return SetHttpHeadersParseResponse(*response);
      }

      struct SetMetadataOptions
      {
        std::map<std::string, std::string> Metadata;
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
        std::string IfModifiedSince;
        std::string IfUnmodifiedSince;
        std::string IfMatch;
        std::string IfNoneMatch;
      }; // struct SetMetadataOptions

      static Azure::Core::Http::Request SetMetadataConstructRequest(
          const std::string& url,
          const SetMetadataOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "metadata");
        request.AddHeader("x-ms-version", "2019-07-07");
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
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        if (!options.IfModifiedSince.empty())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince);
        }
        if (!options.IfUnmodifiedSince.empty())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince);
        }
        if (!options.IfMatch.empty())
        {
          request.AddHeader("If-Match", options.IfMatch);
        }
        if (!options.IfNoneMatch.empty())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch);
        }
        return request;
      }

      static BlobInfo SetMetadataParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlobInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        return response;
      }

      static BlobInfo SetMetadata(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const SetMetadataOptions& options)
      {
        auto request = SetMetadataConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return SetMetadataParseResponse(*response);
      }

      struct SetAccessTierOptions
      {
        AccessTier Tier = AccessTier::Unknown;
        Blobs::RehydratePriority RehydratePriority = Blobs::RehydratePriority::Unknown;
      }; // struct SetAccessTierOptions

      static Azure::Core::Http::Request SetAccessTierConstructRequest(
          const std::string& url,
          const SetAccessTierOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "tier");
        request.AddHeader("x-ms-version", "2019-07-07");
        request.AddHeader("x-ms-access-tier", AccessTierToString(options.Tier));
        auto options_rehydratepriority_str = RehydratePriorityToString(options.RehydratePriority);
        if (!options_rehydratepriority_str.empty())
        {
          request.AddHeader("x-ms-rehydrate-priority", options_rehydratepriority_str);
        }
        return request;
      }

      static BasicResponse SetAccessTierParseResponse(Azure::Core::Http::Response& http_response)
      {
        BasicResponse response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200 || http_status_code == 202))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        return response;
      }

      static BasicResponse SetAccessTier(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const SetAccessTierOptions& options)
      {
        auto request = SetAccessTierConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return SetAccessTierParseResponse(*response);
      }

      struct StartCopyFromUriOptions
      {
        std::map<std::string, std::string> Metadata;
        std::string SourceUri;
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
      }; // struct StartCopyFromUriOptions

      static Azure::Core::Http::Request StartCopyFromUriConstructRequest(
          const std::string& url,
          const StartCopyFromUriOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", "2019-07-07");
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
        if (!options.LeaseId.empty())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId);
        }
        if (!options.SourceLeaseId.empty())
        {
          request.AddHeader("x-ms-source-lease-id", options.SourceLeaseId);
        }
        auto options_tier_str = AccessTierToString(options.Tier);
        if (!options_tier_str.empty())
        {
          request.AddHeader("x-ms-access-tier", options_tier_str);
        }
        auto options_rehydratepriority_str = RehydratePriorityToString(options.RehydratePriority);
        if (!options_rehydratepriority_str.empty())
        {
          request.AddHeader("x-ms-rehydrate-priority", options_rehydratepriority_str);
        }
        if (!options.IfModifiedSince.empty())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince);
        }
        if (!options.IfUnmodifiedSince.empty())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince);
        }
        if (!options.IfMatch.empty())
        {
          request.AddHeader("If-Match", options.IfMatch);
        }
        if (!options.IfNoneMatch.empty())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch);
        }
        if (!options.SourceIfModifiedSince.empty())
        {
          request.AddHeader("x-ms-source-if-modified-since", options.SourceIfModifiedSince);
        }
        if (!options.SourceIfUnmodifiedSince.empty())
        {
          request.AddHeader("x-ms-source-if-unmodified-since", options.SourceIfUnmodifiedSince);
        }
        if (!options.SourceIfMatch.empty())
        {
          request.AddHeader("x-ms-source-if-match", options.SourceIfMatch);
        }
        if (!options.SourceIfNoneMatch.empty())
        {
          request.AddHeader("x-ms-source-if-none-match", options.SourceIfNoneMatch);
        }
        return request;
      }

      static BlobCopyInfo StartCopyFromUriParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlobCopyInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 202))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        response.CopyId = http_response.GetHeaders().at("x-ms-copy-id");
        response.CopyStatus
            = CopyStatusFromString(http_response.GetHeaders().at("x-ms-copy-status"));
        return response;
      }

      static BlobCopyInfo StartCopyFromUri(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const StartCopyFromUriOptions& options)
      {
        auto request = StartCopyFromUriConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return StartCopyFromUriParseResponse(*response);
      }

      struct AbortCopyFromUriOptions
      {
        std::string CopyId;
        std::string LeaseId;
      }; // struct AbortCopyFromUriOptions

      static Azure::Core::Http::Request AbortCopyFromUriConstructRequest(
          const std::string& url,
          const AbortCopyFromUriOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", "2019-07-07");
        request.AddQueryParameter("comp", "copy");
        request.AddQueryParameter("copyid", options.CopyId);
        request.AddHeader("x-ms-copy-action", "abort");
        if (!options.LeaseId.empty())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId);
        }
        return request;
      }

      static BasicResponse AbortCopyFromUriParseResponse(Azure::Core::Http::Response& http_response)
      {
        BasicResponse response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 204))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        return response;
      }

      static BasicResponse AbortCopyFromUri(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const AbortCopyFromUriOptions& options)
      {
        auto request = AbortCopyFromUriConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return AbortCopyFromUriParseResponse(*response);
      }

      struct CreateSnapshotOptions
      {
        std::map<std::string, std::string> Metadata;
        std::string LeaseId;
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
        std::string IfModifiedSince;
        std::string IfUnmodifiedSince;
        std::string IfMatch;
        std::string IfNoneMatch;
      }; // struct CreateSnapshotOptions

      static Azure::Core::Http::Request CreateSnapshotConstructRequest(
          const std::string& url,
          const CreateSnapshotOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "snapshot");
        request.AddHeader("x-ms-version", "2019-07-07");
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
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
        if (!options.LeaseId.empty())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId);
        }
        if (!options.IfModifiedSince.empty())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince);
        }
        if (!options.IfUnmodifiedSince.empty())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince);
        }
        if (!options.IfMatch.empty())
        {
          request.AddHeader("If-Match", options.IfMatch);
        }
        if (!options.IfNoneMatch.empty())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch);
        }
        return request;
      }

      static BlobSnapshotInfo CreateSnapshotParseResponse(
          Azure::Core::Http::Response& http_response)
      {
        BlobSnapshotInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        auto response_serverencrypted_iterator
            = http_response.GetHeaders().find("x-ms-server-encrypted");
        if (response_serverencrypted_iterator != http_response.GetHeaders().end())
        {
          response.ServerEncrypted = response_serverencrypted_iterator->second == "true";
        }
        auto response_encryptionkeysha256_iterator
            = http_response.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryptionkeysha256_iterator != http_response.GetHeaders().end())
        {
          response.EncryptionKeySHA256 = response_encryptionkeysha256_iterator->second;
        }
        response.Snapshot = http_response.GetHeaders().at("x-ms-snapshot");
        return response;
      }

      static BlobSnapshotInfo CreateSnapshot(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const CreateSnapshotOptions& options)
      {
        auto request = CreateSnapshotConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return CreateSnapshotParseResponse(*response);
      }

    private:
    }; // class Blob

    class BlockBlob {
    public:
      struct UploadOptions
      {
        std::string ContentMD5;
        std::string ContentCRC64;
        BlobHttpHeaders Properties;
        std::map<std::string, std::string> Metadata;
        std::string LeaseId;
        AccessTier Tier = AccessTier::Unknown;
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
        std::string IfModifiedSince;
        std::string IfUnmodifiedSince;
        std::string IfMatch;
        std::string IfNoneMatch;
      }; // struct UploadOptions

      static Azure::Core::Http::Request UploadConstructRequest(
          const std::string& url,
          std::unique_ptr<Azure::Core::Http::BodyStream> content,
          const UploadOptions& options)
      {
        uint64_t body_stream_length = content == nullptr ? 0 : content->Length();
        auto request = Azure::Core::Http::Request(
            Azure::Core::Http::HttpMethod::Put, url, std::move(content));
        request.AddHeader("Content-Length", std::to_string(body_stream_length));
        request.AddHeader("x-ms-version", "2019-07-07");
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        if (!options.ContentMD5.empty())
        {
          request.AddHeader("Content-MD5", options.ContentMD5);
        }
        if (!options.ContentCRC64.empty())
        {
          request.AddHeader("x-ms-content-crc64", options.ContentCRC64);
        }
        if (!options.Properties.ContentType.empty())
        {
          request.AddHeader("x-ms-blob-content-type", options.Properties.ContentType);
        }
        if (!options.Properties.ContentEncoding.empty())
        {
          request.AddHeader("x-ms-blob-content-encoding", options.Properties.ContentEncoding);
        }
        if (!options.Properties.ContentLanguage.empty())
        {
          request.AddHeader("x-ms-blob-content-language", options.Properties.ContentLanguage);
        }
        if (!options.Properties.CacheControl.empty())
        {
          request.AddHeader("x-ms-blob-cache-control", options.Properties.CacheControl);
        }
        if (!options.Properties.ContentMD5.empty())
        {
          request.AddHeader("x-ms-blob-content-md5", options.Properties.ContentMD5);
        }
        if (!options.Properties.ContentDisposition.empty())
        {
          request.AddHeader("x-ms-blob-content-disposition", options.Properties.ContentDisposition);
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
        if (!options.LeaseId.empty())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId);
        }
        request.AddHeader("x-ms-blob-type", "BlockBlob");
        auto options_tier_str = AccessTierToString(options.Tier);
        if (!options_tier_str.empty())
        {
          request.AddHeader("x-ms-access-tier", options_tier_str);
        }
        if (!options.IfModifiedSince.empty())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince);
        }
        if (!options.IfUnmodifiedSince.empty())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince);
        }
        if (!options.IfMatch.empty())
        {
          request.AddHeader("If-Match", options.IfMatch);
        }
        if (!options.IfNoneMatch.empty())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch);
        }
        return request;
      }

      static BlobContentInfo UploadParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlobContentInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        auto response_contentmd5_iterator = http_response.GetHeaders().find("Content-MD5");
        if (response_contentmd5_iterator != http_response.GetHeaders().end())
        {
          response.ContentMD5 = response_contentmd5_iterator->second;
        }
        auto response_contentcrc64_iterator = http_response.GetHeaders().find("x-ms-content-crc64");
        if (response_contentcrc64_iterator != http_response.GetHeaders().end())
        {
          response.ContentCRC64 = response_contentcrc64_iterator->second;
        }
        auto response_serverencrypted_iterator
            = http_response.GetHeaders().find("x-ms-server-encrypted");
        if (response_serverencrypted_iterator != http_response.GetHeaders().end())
        {
          response.ServerEncrypted = response_serverencrypted_iterator->second == "true";
        }
        auto response_encryptionkeysha256_iterator
            = http_response.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryptionkeysha256_iterator != http_response.GetHeaders().end())
        {
          response.EncryptionKeySHA256 = response_encryptionkeysha256_iterator->second;
        }
        return response;
      }

      static BlobContentInfo Upload(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          std::unique_ptr<Azure::Core::Http::BodyStream> content,
          const UploadOptions& options)
      {
        auto request = UploadConstructRequest(url, std::move(content), options);
        auto response = pipeline.Send(context, request);
        return UploadParseResponse(*response);
      }

      struct StageBlockOptions
      {
        std::string BlockId;
        std::string ContentMD5;
        std::string ContentCRC64;
        std::string LeaseId;
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
      }; // struct StageBlockOptions

      static Azure::Core::Http::Request StageBlockConstructRequest(
          const std::string& url,
          std::unique_ptr<Azure::Core::Http::BodyStream> content,
          const StageBlockOptions& options)
      {
        uint64_t body_stream_length = content == nullptr ? 0 : content->Length();
        auto request = Azure::Core::Http::Request(
            Azure::Core::Http::HttpMethod::Put, url, std::move(content));
        request.AddHeader("Content-Length", std::to_string(body_stream_length));
        request.AddQueryParameter("comp", "block");
        request.AddQueryParameter("blockid", options.BlockId);
        request.AddHeader("x-ms-version", "2019-07-07");
        if (!options.ContentMD5.empty())
        {
          request.AddHeader("Content-MD5", options.ContentMD5);
        }
        if (!options.ContentCRC64.empty())
        {
          request.AddHeader("x-ms-content-crc64", options.ContentCRC64);
        }
        if (!options.LeaseId.empty())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId);
        }
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        return request;
      }

      static BlockInfo StageBlockParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlockInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        auto response_contentmd5_iterator = http_response.GetHeaders().find("Content-MD5");
        if (response_contentmd5_iterator != http_response.GetHeaders().end())
        {
          response.ContentMD5 = response_contentmd5_iterator->second;
        }
        auto response_contentcrc64_iterator = http_response.GetHeaders().find("x-ms-content-crc64");
        if (response_contentcrc64_iterator != http_response.GetHeaders().end())
        {
          response.ContentCRC64 = response_contentcrc64_iterator->second;
        }
        auto response_serverencrypted_iterator
            = http_response.GetHeaders().find("x-ms-server-encrypted");
        if (response_serverencrypted_iterator != http_response.GetHeaders().end())
        {
          response.ServerEncrypted = response_serverencrypted_iterator->second == "true";
        }
        auto response_encryptionkeysha256_iterator
            = http_response.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryptionkeysha256_iterator != http_response.GetHeaders().end())
        {
          response.EncryptionKeySHA256 = response_encryptionkeysha256_iterator->second;
        }
        return response;
      }

      static BlockInfo StageBlock(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          std::unique_ptr<Azure::Core::Http::BodyStream> content,
          const StageBlockOptions& options)
      {
        auto request = StageBlockConstructRequest(url, std::move(content), options);
        auto response = pipeline.Send(context, request);
        return StageBlockParseResponse(*response);
      }

      struct StageBlockFromUriOptions
      {
        std::string BlockId;
        std::string SourceUri;
        std::pair<uint64_t, uint64_t> SourceRange;
        std::string ContentMD5;
        std::string ContentCRC64;
        std::string LeaseId;
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
        std::string SourceIfModifiedSince;
        std::string SourceIfUnmodifiedSince;
        std::string SourceIfMatch;
        std::string SourceIfNoneMatch;
      }; // struct StageBlockFromUriOptions

      static Azure::Core::Http::Request StageBlockFromUriConstructRequest(
          const std::string& url,
          const StageBlockFromUriOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "block");
        request.AddQueryParameter("blockid", options.BlockId);
        request.AddHeader("x-ms-version", "2019-07-07");
        request.AddHeader("x-ms-copy-source", options.SourceUri);
        if (options.SourceRange.first
            == std::numeric_limits<decltype(options.SourceRange.first)>::max())
        {
          // do nothing
        }
        else if (
            options.SourceRange.second
            == std::numeric_limits<decltype(options.SourceRange.second)>::max())
        {
          request.AddHeader(
              "x-ms-source_range", "bytes=" + std::to_string(options.SourceRange.first) + "-");
        }
        else
        {
          request.AddHeader(
              "x-ms-source_range",
              "bytes=" + std::to_string(options.SourceRange.first) + "-"
                  + std::to_string(options.SourceRange.second));
        }
        if (!options.ContentMD5.empty())
        {
          request.AddHeader("x-ms-source-content-md5", options.ContentMD5);
        }
        if (!options.ContentCRC64.empty())
        {
          request.AddHeader("x-ms-source-content-crc64", options.ContentCRC64);
        }
        if (!options.LeaseId.empty())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId);
        }
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        if (!options.SourceIfModifiedSince.empty())
        {
          request.AddHeader("x-ms-source-if-modified-since", options.SourceIfModifiedSince);
        }
        if (!options.SourceIfUnmodifiedSince.empty())
        {
          request.AddHeader("x-ms-source-if-unmodified-since", options.SourceIfUnmodifiedSince);
        }
        if (!options.SourceIfMatch.empty())
        {
          request.AddHeader("x-ms-source-if-match", options.SourceIfMatch);
        }
        if (!options.SourceIfNoneMatch.empty())
        {
          request.AddHeader("x-ms-source-if-none-match", options.SourceIfNoneMatch);
        }
        return request;
      }

      static BlockInfo StageBlockFromUriParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlockInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        auto response_contentmd5_iterator = http_response.GetHeaders().find("Content-MD5");
        if (response_contentmd5_iterator != http_response.GetHeaders().end())
        {
          response.ContentMD5 = response_contentmd5_iterator->second;
        }
        auto response_contentcrc64_iterator = http_response.GetHeaders().find("x-ms-content-crc64");
        if (response_contentcrc64_iterator != http_response.GetHeaders().end())
        {
          response.ContentCRC64 = response_contentcrc64_iterator->second;
        }
        auto response_serverencrypted_iterator
            = http_response.GetHeaders().find("x-ms-server-encrypted");
        if (response_serverencrypted_iterator != http_response.GetHeaders().end())
        {
          response.ServerEncrypted = response_serverencrypted_iterator->second == "true";
        }
        auto response_encryptionkeysha256_iterator
            = http_response.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryptionkeysha256_iterator != http_response.GetHeaders().end())
        {
          response.EncryptionKeySHA256 = response_encryptionkeysha256_iterator->second;
        }
        return response;
      }

      static BlockInfo StageBlockFromUri(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const StageBlockFromUriOptions& options)
      {
        auto request = StageBlockFromUriConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return StageBlockFromUriParseResponse(*response);
      }

      struct CommitBlockListOptions
      {
        std::vector<std::pair<BlockType, std::string>> BlockList;
        BlobHttpHeaders Properties;
        std::map<std::string, std::string> Metadata;
        std::string LeaseId;
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
        std::string IfModifiedSince;
        std::string IfUnmodifiedSince;
        std::string IfMatch;
        std::string IfNoneMatch;
        AccessTier Tier = AccessTier::Unknown;
      }; // struct CommitBlockListOptions

      static Azure::Core::Http::Request CommitBlockListConstructRequest(
          const std::string& url,
          const CommitBlockListOptions& options)
      {
        XmlWriter writer;
        CommitBlockListOptionsToXml(writer, options);
        std::string xml_body = writer.GetDocument();
        std::vector<uint8_t> body_buffer(xml_body.begin(), xml_body.end());
        uint64_t body_buffer_length = body_buffer.size();
        auto request = Azure::Core::Http::Request(
            Azure::Core::Http::HttpMethod::Put,
            url,
            std::make_unique<Azure::Core::Http::MemoryBodyStream>(std::move(body_buffer)));
        request.AddHeader("Content-Length", std::to_string(body_buffer_length));
        request.AddQueryParameter("comp", "blocklist");
        request.AddHeader("x-ms-version", "2019-07-07");
        if (!options.Properties.ContentType.empty())
        {
          request.AddHeader("x-ms-blob-content-type", options.Properties.ContentType);
        }
        if (!options.Properties.ContentEncoding.empty())
        {
          request.AddHeader("x-ms-blob-content-encoding", options.Properties.ContentEncoding);
        }
        if (!options.Properties.ContentLanguage.empty())
        {
          request.AddHeader("x-ms-blob-content-language", options.Properties.ContentLanguage);
        }
        if (!options.Properties.CacheControl.empty())
        {
          request.AddHeader("x-ms-blob-cache-control", options.Properties.CacheControl);
        }
        if (!options.Properties.ContentMD5.empty())
        {
          request.AddHeader("x-ms-blob-content-md5", options.Properties.ContentMD5);
        }
        if (!options.Properties.ContentDisposition.empty())
        {
          request.AddHeader("x-ms-blob-content-disposition", options.Properties.ContentDisposition);
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
        if (!options.LeaseId.empty())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId);
        }
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        auto options_tier_str = AccessTierToString(options.Tier);
        if (!options_tier_str.empty())
        {
          request.AddHeader("x-ms-access-tier", options_tier_str);
        }
        if (!options.IfModifiedSince.empty())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince);
        }
        if (!options.IfUnmodifiedSince.empty())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince);
        }
        if (!options.IfMatch.empty())
        {
          request.AddHeader("If-Match", options.IfMatch);
        }
        if (!options.IfNoneMatch.empty())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch);
        }
        return request;
      }

      static BlobContentInfo CommitBlockListParseResponse(
          Azure::Core::Http::Response& http_response)
      {
        BlobContentInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        auto response_serverencrypted_iterator
            = http_response.GetHeaders().find("x-ms-server-encrypted");
        if (response_serverencrypted_iterator != http_response.GetHeaders().end())
        {
          response.ServerEncrypted = response_serverencrypted_iterator->second == "true";
        }
        auto response_encryptionkeysha256_iterator
            = http_response.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryptionkeysha256_iterator != http_response.GetHeaders().end())
        {
          response.EncryptionKeySHA256 = response_encryptionkeysha256_iterator->second;
        }
        return response;
      }

      static BlobContentInfo CommitBlockList(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const CommitBlockListOptions& options)
      {
        auto request = CommitBlockListConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return CommitBlockListParseResponse(*response);
      }

      struct GetBlockListOptions
      {
        BlockListTypeOption ListType = BlockListTypeOption::All;
        std::string IfModifiedSince;
        std::string IfUnmodifiedSince;
        std::string IfMatch;
        std::string IfNoneMatch;
      }; // struct GetBlockListOptions

      static Azure::Core::Http::Request GetBlockListConstructRequest(
          const std::string& url,
          const GetBlockListOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "blocklist");
        std::string block_list_type_option = BlockListTypeOptionToString(options.ListType);
        if (!block_list_type_option.empty())
        {
          request.AddQueryParameter("blocklisttype", block_list_type_option);
        }
        request.AddHeader("x-ms-version", "2019-07-07");
        if (!options.IfModifiedSince.empty())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince);
        }
        if (!options.IfUnmodifiedSince.empty())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince);
        }
        if (!options.IfMatch.empty())
        {
          request.AddHeader("If-Match", options.IfMatch);
        }
        if (!options.IfNoneMatch.empty())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch);
        }
        return request;
      }

      static BlobBlockListInfo GetBlockListParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlobBlockListInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        auto bodyStream = http_response.GetBodyStream();
        std::vector<uint8_t> bodyContent(static_cast<std::size_t>(bodyStream->Length()));
        bodyStream->Read(&bodyContent[0], bodyContent.size());
        XmlReader reader(reinterpret_cast<const char*>(bodyContent.data()), bodyContent.size());
        response = BlobBlockListInfoFromXml(reader);
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        response.ContentType = http_response.GetHeaders().at("Content-Type");
        response.ContentLength
            = std::stoull(http_response.GetHeaders().at("x-ms-blob-content-length"));
        return response;
      }

      static BlobBlockListInfo GetBlockList(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const GetBlockListOptions& options)
      {
        auto request = GetBlockListConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return GetBlockListParseResponse(*response);
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
              ret.Size = std::stoull(node.Value);
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
        writer.Write(XmlNode{XmlNodeType::End});
      }

    }; // class BlockBlob

    class PageBlob {
    public:
      struct CreateOptions
      {
        uint64_t BlobContentLength;
        uint64_t SequenceNumber = 0;
        BlobHttpHeaders Properties;
        std::map<std::string, std::string> Metadata;
        std::string LeaseId;
        AccessTier Tier = AccessTier::Unknown;
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
        std::string IfModifiedSince;
        std::string IfUnmodifiedSince;
        std::string IfMatch;
        std::string IfNoneMatch;
      }; // struct CreateOptions

      static Azure::Core::Http::Request CreateConstructRequest(
          const std::string& url,
          const CreateOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", "2019-07-07");
        if (!options.Properties.ContentType.empty())
        {
          request.AddHeader("x-ms-blob-content-type", options.Properties.ContentType);
        }
        if (!options.Properties.ContentEncoding.empty())
        {
          request.AddHeader("x-ms-blob-content-encoding", options.Properties.ContentEncoding);
        }
        if (!options.Properties.ContentLanguage.empty())
        {
          request.AddHeader("x-ms-blob-content-language", options.Properties.ContentLanguage);
        }
        if (!options.Properties.CacheControl.empty())
        {
          request.AddHeader("x-ms-blob-cache-control", options.Properties.CacheControl);
        }
        if (!options.Properties.ContentMD5.empty())
        {
          request.AddHeader("x-ms-blob-content-md5", options.Properties.ContentMD5);
        }
        if (!options.Properties.ContentDisposition.empty())
        {
          request.AddHeader("x-ms-blob-content-disposition", options.Properties.ContentDisposition);
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
        if (!options.LeaseId.empty())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId);
        }
        request.AddHeader("x-ms-blob-type", "PageBlob");
        request.AddHeader("x-ms-blob-content-length", std::to_string(options.BlobContentLength));
        request.AddHeader("x-ms-blob-sequence-number", std::to_string(options.SequenceNumber));
        auto options_tier_str = AccessTierToString(options.Tier);
        if (!options_tier_str.empty())
        {
          request.AddHeader("x-ms-access-tier", options_tier_str);
        }
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        if (!options.IfModifiedSince.empty())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince);
        }
        if (!options.IfUnmodifiedSince.empty())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince);
        }
        if (!options.IfMatch.empty())
        {
          request.AddHeader("If-Match", options.IfMatch);
        }
        if (!options.IfNoneMatch.empty())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch);
        }
        return request;
      }

      static BlobContentInfo CreateParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlobContentInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        auto response_contentmd5_iterator = http_response.GetHeaders().find("Content-MD5");
        if (response_contentmd5_iterator != http_response.GetHeaders().end())
        {
          response.ContentMD5 = response_contentmd5_iterator->second;
        }
        auto response_contentcrc64_iterator = http_response.GetHeaders().find("x-ms-content-crc64");
        if (response_contentcrc64_iterator != http_response.GetHeaders().end())
        {
          response.ContentCRC64 = response_contentcrc64_iterator->second;
        }
        auto response_serverencrypted_iterator
            = http_response.GetHeaders().find("x-ms-server-encrypted");
        if (response_serverencrypted_iterator != http_response.GetHeaders().end())
        {
          response.ServerEncrypted = response_serverencrypted_iterator->second == "true";
        }
        auto response_encryptionkeysha256_iterator
            = http_response.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryptionkeysha256_iterator != http_response.GetHeaders().end())
        {
          response.EncryptionKeySHA256 = response_encryptionkeysha256_iterator->second;
        }
        return response;
      }

      static BlobContentInfo Create(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const CreateOptions& options)
      {
        auto request = CreateConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return CreateParseResponse(*response);
      }

      struct UploadPagesOptions
      {
        std::pair<uint64_t, uint64_t> Range;
        std::string ContentMD5;
        std::string ContentCRC64;
        std::string LeaseId;
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
        std::string IfModifiedSince;
        std::string IfUnmodifiedSince;
        std::string IfMatch;
        std::string IfNoneMatch;
      }; // struct UploadPagesOptions

      static Azure::Core::Http::Request UploadPagesConstructRequest(
          const std::string& url,
          std::unique_ptr<Azure::Core::Http::BodyStream> content,
          const UploadPagesOptions& options)
      {
        uint64_t body_stream_length = content == nullptr ? 0 : content->Length();
        auto request = Azure::Core::Http::Request(
            Azure::Core::Http::HttpMethod::Put, url, std::move(content));
        request.AddHeader("Content-Length", std::to_string(body_stream_length));
        request.AddQueryParameter("comp", "page");
        request.AddHeader("x-ms-version", "2019-07-07");
        if (options.Range.first == std::numeric_limits<decltype(options.Range.first)>::max())
        {
          // do nothing
        }
        else if (options.Range.second == std::numeric_limits<decltype(options.Range.second)>::max())
        {
          request.AddHeader("x-ms-range", "bytes=" + std::to_string(options.Range.first) + "-");
        }
        else
        {
          request.AddHeader(
              "x-ms-range",
              "bytes=" + std::to_string(options.Range.first) + "-"
                  + std::to_string(options.Range.second));
        }
        if (!options.ContentMD5.empty())
        {
          request.AddHeader("Content-MD5", options.ContentMD5);
        }
        if (!options.ContentCRC64.empty())
        {
          request.AddHeader("x-ms-content-crc64", options.ContentCRC64);
        }
        request.AddHeader("x-ms-page-write", "update");
        if (!options.LeaseId.empty())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId);
        }
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        if (!options.IfModifiedSince.empty())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince);
        }
        if (!options.IfUnmodifiedSince.empty())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince);
        }
        if (!options.IfMatch.empty())
        {
          request.AddHeader("If-Match", options.IfMatch);
        }
        if (!options.IfNoneMatch.empty())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch);
        }
        return request;
      }

      static PageInfo UploadPagesParseResponse(Azure::Core::Http::Response& http_response)
      {
        PageInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        auto response_contentmd5_iterator = http_response.GetHeaders().find("Content-MD5");
        if (response_contentmd5_iterator != http_response.GetHeaders().end())
        {
          response.ContentMD5 = response_contentmd5_iterator->second;
        }
        auto response_contentcrc64_iterator = http_response.GetHeaders().find("x-ms-content-crc64");
        if (response_contentcrc64_iterator != http_response.GetHeaders().end())
        {
          response.ContentCRC64 = response_contentcrc64_iterator->second;
        }
        auto response_sequencenumber_iterator
            = http_response.GetHeaders().find("x-ms-blob-sequence-number");
        if (response_sequencenumber_iterator != http_response.GetHeaders().end())
        {
          response.SequenceNumber = std::stoull(response_sequencenumber_iterator->second);
        }
        auto response_serverencrypted_iterator
            = http_response.GetHeaders().find("x-ms-server-encrypted");
        if (response_serverencrypted_iterator != http_response.GetHeaders().end())
        {
          response.ServerEncrypted = response_serverencrypted_iterator->second == "true";
        }
        auto response_encryptionkeysha256_iterator
            = http_response.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryptionkeysha256_iterator != http_response.GetHeaders().end())
        {
          response.EncryptionKeySHA256 = response_encryptionkeysha256_iterator->second;
        }
        return response;
      }

      static PageInfo UploadPages(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          std::unique_ptr<Azure::Core::Http::BodyStream> content,
          const UploadPagesOptions& options)
      {
        auto request = UploadPagesConstructRequest(url, std::move(content), options);
        auto response = pipeline.Send(context, request);
        return UploadPagesParseResponse(*response);
      }

      struct UploadPagesFromUriOptions
      {
        std::string SourceUri;
        std::pair<uint64_t, uint64_t> SourceRange;
        std::pair<uint64_t, uint64_t> Range;
        std::string ContentMD5;
        std::string ContentCRC64;
        std::string LeaseId;
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
        std::string IfModifiedSince;
        std::string IfUnmodifiedSince;
        std::string IfMatch;
        std::string IfNoneMatch;
      }; // struct UploadPagesFromUriOptions

      static Azure::Core::Http::Request UploadPagesFromUriConstructRequest(
          const std::string& url,
          const UploadPagesFromUriOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "page");
        request.AddHeader("x-ms-version", "2019-07-07");
        if (options.Range.first == std::numeric_limits<decltype(options.Range.first)>::max())
        {
          // do nothing
        }
        else if (options.Range.second == std::numeric_limits<decltype(options.Range.second)>::max())
        {
          request.AddHeader("x-ms-range", "bytes=" + std::to_string(options.Range.first) + "-");
        }
        else
        {
          request.AddHeader(
              "x-ms-range",
              "bytes=" + std::to_string(options.Range.first) + "-"
                  + std::to_string(options.Range.second));
        }
        request.AddHeader("x-ms-copy-source", options.SourceUri);
        if (options.SourceRange.first
            == std::numeric_limits<decltype(options.SourceRange.first)>::max())
        {
          // do nothing
        }
        else if (
            options.SourceRange.second
            == std::numeric_limits<decltype(options.SourceRange.second)>::max())
        {
          request.AddHeader(
              "x-ms-source-range", "bytes=" + std::to_string(options.SourceRange.first) + "-");
        }
        else
        {
          request.AddHeader(
              "x-ms-source-range",
              "bytes=" + std::to_string(options.SourceRange.first) + "-"
                  + std::to_string(options.SourceRange.second));
        }
        if (!options.ContentMD5.empty())
        {
          request.AddHeader("x-ms-source-content-md5", options.ContentMD5);
        }
        if (!options.ContentCRC64.empty())
        {
          request.AddHeader("x-ms-source-content-crc64", options.ContentCRC64);
        }
        request.AddHeader("x-ms-page-write", "update");
        if (!options.LeaseId.empty())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId);
        }
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        if (!options.IfModifiedSince.empty())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince);
        }
        if (!options.IfUnmodifiedSince.empty())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince);
        }
        if (!options.IfMatch.empty())
        {
          request.AddHeader("If-Match", options.IfMatch);
        }
        if (!options.IfNoneMatch.empty())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch);
        }
        return request;
      }

      static PageInfo UploadPagesFromUriParseResponse(Azure::Core::Http::Response& http_response)
      {
        PageInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        auto response_contentmd5_iterator = http_response.GetHeaders().find("Content-MD5");
        if (response_contentmd5_iterator != http_response.GetHeaders().end())
        {
          response.ContentMD5 = response_contentmd5_iterator->second;
        }
        auto response_contentcrc64_iterator = http_response.GetHeaders().find("x-ms-content-crc64");
        if (response_contentcrc64_iterator != http_response.GetHeaders().end())
        {
          response.ContentCRC64 = response_contentcrc64_iterator->second;
        }
        auto response_sequencenumber_iterator
            = http_response.GetHeaders().find("x-ms-blob-sequence-number");
        if (response_sequencenumber_iterator != http_response.GetHeaders().end())
        {
          response.SequenceNumber = std::stoull(response_sequencenumber_iterator->second);
        }
        auto response_serverencrypted_iterator
            = http_response.GetHeaders().find("x-ms-server-encrypted");
        if (response_serverencrypted_iterator != http_response.GetHeaders().end())
        {
          response.ServerEncrypted = response_serverencrypted_iterator->second == "true";
        }
        auto response_encryptionkeysha256_iterator
            = http_response.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryptionkeysha256_iterator != http_response.GetHeaders().end())
        {
          response.EncryptionKeySHA256 = response_encryptionkeysha256_iterator->second;
        }
        return response;
      }

      static PageInfo UploadPagesFromUri(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const UploadPagesFromUriOptions& options)
      {
        auto request = UploadPagesFromUriConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return UploadPagesFromUriParseResponse(*response);
      }

      struct ClearPagesOptions
      {
        std::pair<uint64_t, uint64_t> Range;
        std::string LeaseId;
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
        std::string IfModifiedSince;
        std::string IfUnmodifiedSince;
        std::string IfMatch;
        std::string IfNoneMatch;
      }; // struct ClearPagesOptions

      static Azure::Core::Http::Request ClearPagesConstructRequest(
          const std::string& url,
          const ClearPagesOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "page");
        request.AddHeader("x-ms-version", "2019-07-07");
        if (options.Range.first == std::numeric_limits<decltype(options.Range.first)>::max())
        {
          // do nothing
        }
        else if (options.Range.second == std::numeric_limits<decltype(options.Range.second)>::max())
        {
          request.AddHeader("x-ms-range", "bytes=" + std::to_string(options.Range.first) + "-");
        }
        else
        {
          request.AddHeader(
              "x-ms-range",
              "bytes=" + std::to_string(options.Range.first) + "-"
                  + std::to_string(options.Range.second));
        }
        request.AddHeader("x-ms-page-write", "clear");
        if (!options.LeaseId.empty())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId);
        }
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        if (!options.IfModifiedSince.empty())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince);
        }
        if (!options.IfUnmodifiedSince.empty())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince);
        }
        if (!options.IfMatch.empty())
        {
          request.AddHeader("If-Match", options.IfMatch);
        }
        if (!options.IfNoneMatch.empty())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch);
        }
        return request;
      }

      static PageInfo ClearPagesParseResponse(Azure::Core::Http::Response& http_response)
      {
        PageInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        auto response_sequencenumber_iterator
            = http_response.GetHeaders().find("x-ms-blob-sequence-number");
        if (response_sequencenumber_iterator != http_response.GetHeaders().end())
        {
          response.SequenceNumber = std::stoull(response_sequencenumber_iterator->second);
        }
        auto response_serverencrypted_iterator
            = http_response.GetHeaders().find("x-ms-server-encrypted");
        if (response_serverencrypted_iterator != http_response.GetHeaders().end())
        {
          response.ServerEncrypted = response_serverencrypted_iterator->second == "true";
        }
        auto response_encryptionkeysha256_iterator
            = http_response.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryptionkeysha256_iterator != http_response.GetHeaders().end())
        {
          response.EncryptionKeySHA256 = response_encryptionkeysha256_iterator->second;
        }
        return response;
      }

      static PageInfo ClearPages(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const ClearPagesOptions& options)
      {
        auto request = ClearPagesConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return ClearPagesParseResponse(*response);
      }

      struct ResizeOptions
      {
        uint64_t BlobContentLength;
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
        std::string IfModifiedSince;
        std::string IfUnmodifiedSince;
        std::string IfMatch;
        std::string IfNoneMatch;
      }; // struct ResizeOptions

      static Azure::Core::Http::Request ResizeConstructRequest(
          const std::string& url,
          const ResizeOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "properties");
        request.AddHeader("x-ms-version", "2019-07-07");
        request.AddHeader("x-ms-blob-content-length", std::to_string(options.BlobContentLength));
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        if (!options.IfModifiedSince.empty())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince);
        }
        if (!options.IfUnmodifiedSince.empty())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince);
        }
        if (!options.IfMatch.empty())
        {
          request.AddHeader("If-Match", options.IfMatch);
        }
        if (!options.IfNoneMatch.empty())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch);
        }
        return request;
      }

      static PageBlobInfo ResizeParseResponse(Azure::Core::Http::Response& http_response)
      {
        PageBlobInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        auto response_sequencenumber_iterator
            = http_response.GetHeaders().find("x-ms-blob-sequence-number");
        if (response_sequencenumber_iterator != http_response.GetHeaders().end())
        {
          response.SequenceNumber = std::stoull(response_sequencenumber_iterator->second);
        }
        return response;
      }

      static PageBlobInfo Resize(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const ResizeOptions& options)
      {
        auto request = ResizeConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return ResizeParseResponse(*response);
      }

      struct GetPageRangesOptions
      {
        std::string PreviousSnapshot;
        std::string PreviousSnapshotUrl;
        std::pair<uint64_t, uint64_t> Range;
        std::string LeaseId;
        std::string IfModifiedSince;
        std::string IfUnmodifiedSince;
        std::string IfMatch;
        std::string IfNoneMatch;
      }; // struct GetPageRangesOptions

      static Azure::Core::Http::Request GetPageRangesConstructRequest(
          const std::string& url,
          const GetPageRangesOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "pagelist");
        if (!options.PreviousSnapshot.empty())
        {
          request.AddQueryParameter("prevsnapshot", options.PreviousSnapshot);
        }
        request.AddHeader("x-ms-version", "2019-07-07");
        if (options.Range.first == std::numeric_limits<decltype(options.Range.first)>::max())
        {
          // do nothing
        }
        else if (options.Range.second == std::numeric_limits<decltype(options.Range.second)>::max())
        {
          request.AddHeader("x-ms-range", "bytes=" + std::to_string(options.Range.first) + "-");
        }
        else
        {
          request.AddHeader(
              "x-ms-range",
              "bytes=" + std::to_string(options.Range.first) + "-"
                  + std::to_string(options.Range.second));
        }
        if (!options.LeaseId.empty())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId);
        }
        if (!options.PreviousSnapshotUrl.empty())
        {
          request.AddHeader("x-ms-previous-snapshot-url", options.PreviousSnapshotUrl);
        }
        if (!options.IfModifiedSince.empty())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince);
        }
        if (!options.IfUnmodifiedSince.empty())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince);
        }
        if (!options.IfMatch.empty())
        {
          request.AddHeader("If-Match", options.IfMatch);
        }
        if (!options.IfNoneMatch.empty())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch);
        }
        return request;
      }

      static PageRangesInfoInternal GetPageRangesParseResponse(
          Azure::Core::Http::Response& http_response)
      {
        PageRangesInfoInternal response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        auto bodyStream = http_response.GetBodyStream();
        std::vector<uint8_t> bodyContent(static_cast<std::size_t>(bodyStream->Length()));
        bodyStream->Read(&bodyContent[0], bodyContent.size());
        XmlReader reader(reinterpret_cast<const char*>(bodyContent.data()), bodyContent.size());
        response = PageRangesInfoInternalFromXml(reader);
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        response.BlobContentLength
            = std::stoull(http_response.GetHeaders().at("x-ms-blob-content-length"));
        return response;
      }

      static PageRangesInfoInternal GetPageRanges(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const GetPageRangesOptions& options)
      {
        auto request = GetPageRangesConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return GetPageRangesParseResponse(*response);
      }

      struct CopyIncrementalOptions
      {
        std::string CopySource;
        std::string IfModifiedSince;
        std::string IfUnmodifiedSince;
        std::string IfMatch;
        std::string IfNoneMatch;
      }; // struct CopyIncrementalOptions

      static Azure::Core::Http::Request CopyIncrementalConstructRequest(
          const std::string& url,
          const CopyIncrementalOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "incrementalcopy");
        request.AddHeader("x-ms-version", "2019-07-07");
        request.AddHeader("x-ms-copy-source", options.CopySource);
        if (!options.IfModifiedSince.empty())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince);
        }
        if (!options.IfUnmodifiedSince.empty())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince);
        }
        if (!options.IfMatch.empty())
        {
          request.AddHeader("If-Match", options.IfMatch);
        }
        if (!options.IfNoneMatch.empty())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch);
        }
        return request;
      }

      static BlobCopyInfo CopyIncrementalParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlobCopyInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 202))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        response.CopyId = http_response.GetHeaders().at("x-ms-copy-id");
        response.CopyStatus
            = CopyStatusFromString(http_response.GetHeaders().at("x-ms-copy-status"));
        return response;
      }

      static BlobCopyInfo CopyIncremental(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const CopyIncrementalOptions& options)
      {
        auto request = CopyIncrementalConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return CopyIncrementalParseResponse(*response);
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

      static std::pair<uint64_t, uint64_t> ClearRangesFromXml(XmlReader& reader)
      {
        int depth = 0;
        bool is_start = false;
        bool is_end = false;
        uint64_t start;
        uint64_t end;
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
              start = std::stoull(node.Value);
            }
            else if (is_end)
            {
              end = std::stoull(node.Value);
            }
          }
        }
        return std::make_pair(start, end);
      }

      static std::pair<uint64_t, uint64_t> PageRangesFromXml(XmlReader& reader)
      {
        int depth = 0;
        bool is_start = false;
        bool is_end = false;
        uint64_t start;
        uint64_t end;
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
              start = std::stoull(node.Value);
            }
            else if (is_end)
            {
              end = std::stoull(node.Value);
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
        BlobHttpHeaders Properties;
        std::map<std::string, std::string> Metadata;
        std::string LeaseId;
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
        std::string IfModifiedSince;
        std::string IfUnmodifiedSince;
        std::string IfMatch;
        std::string IfNoneMatch;
      }; // struct CreateOptions

      static Azure::Core::Http::Request CreateConstructRequest(
          const std::string& url,
          const CreateOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", "2019-07-07");
        if (!options.Properties.ContentType.empty())
        {
          request.AddHeader("x-ms-blob-content-type", options.Properties.ContentType);
        }
        if (!options.Properties.ContentEncoding.empty())
        {
          request.AddHeader("x-ms-blob-content-encoding", options.Properties.ContentEncoding);
        }
        if (!options.Properties.ContentLanguage.empty())
        {
          request.AddHeader("x-ms-blob-content-language", options.Properties.ContentLanguage);
        }
        if (!options.Properties.CacheControl.empty())
        {
          request.AddHeader("x-ms-blob-cache-control", options.Properties.CacheControl);
        }
        if (!options.Properties.ContentMD5.empty())
        {
          request.AddHeader("x-ms-blob-content-md5", options.Properties.ContentMD5);
        }
        if (!options.Properties.ContentDisposition.empty())
        {
          request.AddHeader("x-ms-blob-content-disposition", options.Properties.ContentDisposition);
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
        if (!options.LeaseId.empty())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId);
        }
        request.AddHeader("x-ms-blob-type", "AppendBlob");
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        if (!options.IfModifiedSince.empty())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince);
        }
        if (!options.IfUnmodifiedSince.empty())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince);
        }
        if (!options.IfMatch.empty())
        {
          request.AddHeader("If-Match", options.IfMatch);
        }
        if (!options.IfNoneMatch.empty())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch);
        }
        return request;
      }

      static BlobContentInfo CreateParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlobContentInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        auto response_contentmd5_iterator = http_response.GetHeaders().find("Content-MD5");
        if (response_contentmd5_iterator != http_response.GetHeaders().end())
        {
          response.ContentMD5 = response_contentmd5_iterator->second;
        }
        auto response_contentcrc64_iterator = http_response.GetHeaders().find("x-ms-content-crc64");
        if (response_contentcrc64_iterator != http_response.GetHeaders().end())
        {
          response.ContentCRC64 = response_contentcrc64_iterator->second;
        }
        auto response_serverencrypted_iterator
            = http_response.GetHeaders().find("x-ms-server-encrypted");
        if (response_serverencrypted_iterator != http_response.GetHeaders().end())
        {
          response.ServerEncrypted = response_serverencrypted_iterator->second == "true";
        }
        auto response_encryptionkeysha256_iterator
            = http_response.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryptionkeysha256_iterator != http_response.GetHeaders().end())
        {
          response.EncryptionKeySHA256 = response_encryptionkeysha256_iterator->second;
        }
        return response;
      }

      static BlobContentInfo Create(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const CreateOptions& options)
      {
        auto request = CreateConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return CreateParseResponse(*response);
      }

      struct AppendBlockOptions
      {
        Azure::Core::Http::BodyStream* BodyStream = nullptr;
        std::string ContentMD5;
        std::string ContentCRC64;
        std::string LeaseId;
        uint64_t MaxSize = std::numeric_limits<uint64_t>::max();
        uint64_t AppendPosition = std::numeric_limits<uint64_t>::max();
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
        std::string IfModifiedSince;
        std::string IfUnmodifiedSince;
        std::string IfMatch;
        std::string IfNoneMatch;
      }; // struct AppendBlockOptions

      static Azure::Core::Http::Request AppendBlockConstructRequest(
          const std::string& url,
          std::unique_ptr<Azure::Core::Http::BodyStream> content,
          const AppendBlockOptions& options)
      {
        uint64_t body_stream_length = content == nullptr ? 0 : content->Length();
        auto request = Azure::Core::Http::Request(
            Azure::Core::Http::HttpMethod::Put, url, std::move(content));
        request.AddHeader("Content-Length", std::to_string(body_stream_length));
        request.AddQueryParameter("comp", "appendblock");
        request.AddHeader("x-ms-version", "2019-07-07");
        if (!options.ContentMD5.empty())
        {
          request.AddHeader("Content-MD5", options.ContentMD5);
        }
        if (!options.ContentCRC64.empty())
        {
          request.AddHeader("x-ms-content-crc64", options.ContentCRC64);
        }
        if (!options.LeaseId.empty())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId);
        }
        if (options.MaxSize != std::numeric_limits<uint64_t>::max())
        {
          request.AddHeader("x-ms-blob-condition-maxsize", std::to_string(options.MaxSize));
        }
        if (options.AppendPosition != std::numeric_limits<uint64_t>::max())
        {
          request.AddHeader(
              "x-ms-blob-condition-appendpos", std::to_string(options.AppendPosition));
        }
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        if (!options.IfModifiedSince.empty())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince);
        }
        if (!options.IfUnmodifiedSince.empty())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince);
        }
        if (!options.IfMatch.empty())
        {
          request.AddHeader("If-Match", options.IfMatch);
        }
        if (!options.IfNoneMatch.empty())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch);
        }
        return request;
      }

      static BlobAppendInfo AppendBlockParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlobAppendInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        auto response_contentmd5_iterator = http_response.GetHeaders().find("Content-MD5");
        if (response_contentmd5_iterator != http_response.GetHeaders().end())
        {
          response.ContentMD5 = response_contentmd5_iterator->second;
        }
        auto response_contentcrc64_iterator = http_response.GetHeaders().find("x-ms-content-crc64");
        if (response_contentcrc64_iterator != http_response.GetHeaders().end())
        {
          response.ContentCRC64 = response_contentcrc64_iterator->second;
        }
        response.AppendOffset
            = std::stoull(http_response.GetHeaders().at("x-ms-blob-append-offset"));
        response.CommittedBlockCount
            = std::stoull(http_response.GetHeaders().at("x-ms-blob-committed-block-count"));
        auto response_serverencrypted_iterator
            = http_response.GetHeaders().find("x-ms-server-encrypted");
        if (response_serverencrypted_iterator != http_response.GetHeaders().end())
        {
          response.ServerEncrypted = response_serverencrypted_iterator->second == "true";
        }
        auto response_encryptionkeysha256_iterator
            = http_response.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryptionkeysha256_iterator != http_response.GetHeaders().end())
        {
          response.EncryptionKeySHA256 = response_encryptionkeysha256_iterator->second;
        }
        return response;
      }

      static BlobAppendInfo AppendBlock(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          std::unique_ptr<Azure::Core::Http::BodyStream> content,
          const AppendBlockOptions& options)
      {
        auto request = AppendBlockConstructRequest(url, std::move(content), options);
        auto response = pipeline.Send(context, request);
        return AppendBlockParseResponse(*response);
      }

      struct AppendBlockFromUriOptions
      {
        std::string SourceUri;
        std::pair<uint64_t, uint64_t> SourceRange;
        std::string ContentMD5;
        std::string ContentCRC64;
        std::string LeaseId;
        uint64_t MaxSize = std::numeric_limits<uint64_t>::max();
        uint64_t AppendPosition = std::numeric_limits<uint64_t>::max();
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
        std::string IfModifiedSince;
        std::string IfUnmodifiedSince;
        std::string IfMatch;
        std::string IfNoneMatch;
      }; // struct AppendBlockFromUriOptions

      static Azure::Core::Http::Request AppendBlockFromUriConstructRequest(
          const std::string& url,
          const AppendBlockFromUriOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "appendblock");
        request.AddHeader("x-ms-version", "2019-07-07");
        request.AddHeader("x-ms-copy-source", options.SourceUri);
        if (options.SourceRange.first
            == std::numeric_limits<decltype(options.SourceRange.first)>::max())
        {
          // do nothing
        }
        else if (
            options.SourceRange.second
            == std::numeric_limits<decltype(options.SourceRange.second)>::max())
        {
          request.AddHeader(
              "x-ms-source-range", "bytes=" + std::to_string(options.SourceRange.first) + "-");
        }
        else
        {
          request.AddHeader(
              "x-ms-source-range",
              "bytes=" + std::to_string(options.SourceRange.first) + "-"
                  + std::to_string(options.SourceRange.second));
        }
        if (!options.ContentMD5.empty())
        {
          request.AddHeader("x-ms-source-content-md5", options.ContentMD5);
        }
        if (!options.ContentCRC64.empty())
        {
          request.AddHeader("x-ms-source-content-crc64", options.ContentCRC64);
        }
        if (!options.LeaseId.empty())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId);
        }
        if (options.MaxSize != std::numeric_limits<uint64_t>::max())
        {
          request.AddHeader("x-ms-blob-condition-maxsize", std::to_string(options.MaxSize));
        }
        if (options.AppendPosition != std::numeric_limits<uint64_t>::max())
        {
          request.AddHeader(
              "x-ms-blob-condition-appendpos", std::to_string(options.AppendPosition));
        }
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        if (!options.IfModifiedSince.empty())
        {
          request.AddHeader("If-Modified-Since", options.IfModifiedSince);
        }
        if (!options.IfUnmodifiedSince.empty())
        {
          request.AddHeader("If-Unmodified-Since", options.IfUnmodifiedSince);
        }
        if (!options.IfMatch.empty())
        {
          request.AddHeader("If-Match", options.IfMatch);
        }
        if (!options.IfNoneMatch.empty())
        {
          request.AddHeader("If-None-Match", options.IfNoneMatch);
        }
        return request;
      }

      static BlobAppendInfo AppendBlockFromUriParseResponse(
          Azure::Core::Http::Response& http_response)
      {
        BlobAppendInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        auto response_contentmd5_iterator = http_response.GetHeaders().find("Content-MD5");
        if (response_contentmd5_iterator != http_response.GetHeaders().end())
        {
          response.ContentMD5 = response_contentmd5_iterator->second;
        }
        auto response_contentcrc64_iterator = http_response.GetHeaders().find("x-ms-content-crc64");
        if (response_contentcrc64_iterator != http_response.GetHeaders().end())
        {
          response.ContentCRC64 = response_contentcrc64_iterator->second;
        }
        response.AppendOffset
            = std::stoull(http_response.GetHeaders().at("x-ms-blob-append-offset"));
        response.CommittedBlockCount
            = std::stoull(http_response.GetHeaders().at("x-ms-blob-committed-block-count"));
        auto response_serverencrypted_iterator
            = http_response.GetHeaders().find("x-ms-server-encrypted");
        if (response_serverencrypted_iterator != http_response.GetHeaders().end())
        {
          response.ServerEncrypted = response_serverencrypted_iterator->second == "true";
        }
        auto response_encryptionkeysha256_iterator
            = http_response.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryptionkeysha256_iterator != http_response.GetHeaders().end())
        {
          response.EncryptionKeySHA256 = response_encryptionkeysha256_iterator->second;
        }
        return response;
      }

      static BlobAppendInfo AppendBlockFromUri(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const AppendBlockFromUriOptions& options)
      {
        auto request = AppendBlockFromUriConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return AppendBlockFromUriParseResponse(*response);
      }

    private:
    }; // class AppendBlob

  }; // class BlobRestClient
}}} // namespace Azure::Storage::Blobs
