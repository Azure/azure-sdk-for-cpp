// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstring>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include <azure/core/context.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/etag.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/internal/strings.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/response.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_exception.hpp>
#include <azure/storage/common/xml_wrapper.hpp>

#include "azure/storage/blobs/dll_import_export.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  namespace _detail {
    constexpr static const char* ApiVersion = "2020-02-10";
  } // namespace _detail

  namespace Models {

    struct AbortCopyBlobFromUriResult
    {
      std::string RequestId;
    }; // struct AbortCopyBlobFromUriResult

    class AccessTier {
    public:
      AccessTier() = default;
      explicit AccessTier(std::string value) : m_value(std::move(value)) {}
      bool operator==(const AccessTier& other) const { return m_value == other.m_value; }
      bool operator!=(const AccessTier& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P1;
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P2;
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P3;
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P4;
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P6;
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P10;
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P15;
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P20;
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P30;
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P40;
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P50;
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P60;
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P70;
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P80;
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier Hot;
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier Cool;
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier Archive;

    private:
      std::string m_value;
    }; // extensible enum AccessTier

    class AccountKind {
    public:
      AccountKind() = default;
      explicit AccountKind(std::string value) : m_value(std::move(value)) {}
      bool operator==(const AccountKind& other) const { return m_value == other.m_value; }
      bool operator!=(const AccountKind& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccountKind Storage;
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccountKind BlobStorage;
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccountKind StorageV2;
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccountKind FileStorage;
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccountKind BlockBlobStorage;

    private:
      std::string m_value;
    }; // extensible enum AccountKind

    namespace _detail {
      struct AcquireBlobContainerLeaseResult
      {
        std::string RequestId;
        Azure::ETag ETag;
        Azure::DateTime LastModified;
        std::string LeaseId;
      }; // struct AcquireBlobContainerLeaseResult
    } // namespace _detail

    namespace _detail {
      struct AcquireBlobLeaseResult
      {
        std::string RequestId;
        Azure::ETag ETag;
        Azure::DateTime LastModified;
        std::string LeaseId;
      }; // struct AcquireBlobLeaseResult
    } // namespace _detail

    class BlobArchiveStatus {
    public:
      BlobArchiveStatus() = default;
      explicit BlobArchiveStatus(std::string value) : m_value(std::move(value)) {}
      bool operator==(const BlobArchiveStatus& other) const { return m_value == other.m_value; }
      bool operator!=(const BlobArchiveStatus& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlobArchiveStatus RehydratePendingToHot;
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlobArchiveStatus RehydratePendingToCool;

    private:
      std::string m_value;
    }; // extensible enum BlobArchiveStatus

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

    class BlobGeoReplicationStatus {
    public:
      BlobGeoReplicationStatus() = default;
      explicit BlobGeoReplicationStatus(std::string value) : m_value(std::move(value)) {}
      bool operator==(const BlobGeoReplicationStatus& other) const
      {
        return m_value == other.m_value;
      }
      bool operator!=(const BlobGeoReplicationStatus& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlobGeoReplicationStatus Live;
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlobGeoReplicationStatus Bootstrap;
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlobGeoReplicationStatus Unavailable;

    private:
      std::string m_value;
    }; // extensible enum BlobGeoReplicationStatus

    class BlobLeaseDurationType {
    public:
      BlobLeaseDurationType() = default;
      explicit BlobLeaseDurationType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const BlobLeaseDurationType& other) const { return m_value == other.m_value; }
      bool operator!=(const BlobLeaseDurationType& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlobLeaseDurationType Infinite;
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlobLeaseDurationType Fixed;

    private:
      std::string m_value;
    }; // extensible enum BlobLeaseDurationType

    class BlobLeaseState {
    public:
      BlobLeaseState() = default;
      explicit BlobLeaseState(std::string value) : m_value(std::move(value)) {}
      bool operator==(const BlobLeaseState& other) const { return m_value == other.m_value; }
      bool operator!=(const BlobLeaseState& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlobLeaseState Available;
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlobLeaseState Leased;
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlobLeaseState Expired;
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlobLeaseState Breaking;
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlobLeaseState Broken;

    private:
      std::string m_value;
    }; // extensible enum BlobLeaseState

    class BlobLeaseStatus {
    public:
      BlobLeaseStatus() = default;
      explicit BlobLeaseStatus(std::string value) : m_value(std::move(value)) {}
      bool operator==(const BlobLeaseStatus& other) const { return m_value == other.m_value; }
      bool operator!=(const BlobLeaseStatus& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlobLeaseStatus Locked;
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlobLeaseStatus Unlocked;

    private:
      std::string m_value;
    }; // extensible enum BlobLeaseStatus

    struct BlobRetentionPolicy
    {
      bool IsEnabled = false;
      Azure::Nullable<int32_t> Days;
    }; // struct BlobRetentionPolicy

    struct BlobSignedIdentifier
    {
      std::string Id;
      Azure::DateTime StartsOn;
      Azure::DateTime ExpiresOn;
      std::string Permissions;
    }; // struct BlobSignedIdentifier

    struct BlobStaticWebsite
    {
      bool IsEnabled = false;
      Azure::Nullable<std::string> IndexDocument;
      Azure::Nullable<std::string> DefaultIndexDocumentPath;
      Azure::Nullable<std::string> ErrorDocument404Path;
    }; // struct BlobStaticWebsite

    class BlobType {
    public:
      BlobType() = default;
      explicit BlobType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const BlobType& other) const { return m_value == other.m_value; }
      bool operator!=(const BlobType& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlobType BlockBlob;
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlobType PageBlob;
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlobType AppendBlob;

    private:
      std::string m_value;
    }; // extensible enum BlobType

    class BlockListTypeOption {
    public:
      BlockListTypeOption() = default;
      explicit BlockListTypeOption(std::string value) : m_value(std::move(value)) {}
      bool operator==(const BlockListTypeOption& other) const { return m_value == other.m_value; }
      bool operator!=(const BlockListTypeOption& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlockListTypeOption Committed;
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlockListTypeOption Uncommitted;
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlockListTypeOption All;

    private:
      std::string m_value;
    }; // extensible enum BlockListTypeOption

    class BlockType {
    public:
      BlockType() = default;
      explicit BlockType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const BlockType& other) const { return m_value == other.m_value; }
      bool operator!=(const BlockType& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlockType Committed;
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlockType Uncommitted;
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlockType Latest;

    private:
      std::string m_value;
    }; // extensible enum BlockType

    namespace _detail {
      struct BreakBlobContainerLeaseResult
      {
        std::string RequestId;
        Azure::ETag ETag;
        Azure::DateTime LastModified;
        int32_t LeaseTime = 0;
      }; // struct BreakBlobContainerLeaseResult
    } // namespace _detail

    namespace _detail {
      struct BreakBlobLeaseResult
      {
        std::string RequestId;
        Azure::ETag ETag;
        Azure::DateTime LastModified;
        int32_t LeaseTime = 0;
      }; // struct BreakBlobLeaseResult
    } // namespace _detail

    namespace _detail {
      struct ChangeBlobContainerLeaseResult
      {
        std::string RequestId;
        Azure::ETag ETag;
        Azure::DateTime LastModified;
        std::string LeaseId;
      }; // struct ChangeBlobContainerLeaseResult
    } // namespace _detail

    namespace _detail {
      struct ChangeBlobLeaseResult
      {
        std::string RequestId;
        Azure::ETag ETag;
        Azure::DateTime LastModified;
        std::string LeaseId;
      }; // struct ChangeBlobLeaseResult
    } // namespace _detail

    struct ClearPageBlobPagesResult
    {
      std::string RequestId;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      int64_t SequenceNumber = 0;
    }; // struct ClearPageBlobPagesResult

    class CopyStatus {
    public:
      CopyStatus() = default;
      explicit CopyStatus(std::string value) : m_value(std::move(value)) {}
      bool operator==(const CopyStatus& other) const { return m_value == other.m_value; }
      bool operator!=(const CopyStatus& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      AZ_STORAGE_BLOBS_DLLEXPORT const static CopyStatus Success;
      AZ_STORAGE_BLOBS_DLLEXPORT const static CopyStatus Pending;

    private:
      std::string m_value;
    }; // extensible enum CopyStatus

    struct CreateAppendBlobResult
    {
      std::string RequestId;
      bool Created = true;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      Azure::Nullable<std::string> VersionId;
      bool IsServerEncrypted = false;
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      Azure::Nullable<std::string> EncryptionScope;
    }; // struct CreateAppendBlobResult

    struct CreateBlobContainerResult
    {
      std::string RequestId;
      bool Created = true;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
    }; // struct CreateBlobContainerResult

    struct CreateBlobSnapshotResult
    {
      std::string RequestId;
      std::string Snapshot;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      Azure::Nullable<std::string> VersionId;
      bool IsServerEncrypted = false;
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      Azure::Nullable<std::string> EncryptionScope;
    }; // struct CreateBlobSnapshotResult

    struct CreatePageBlobResult
    {
      std::string RequestId;
      bool Created = true;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      Azure::Nullable<std::string> VersionId;
      bool IsServerEncrypted = false;
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      Azure::Nullable<std::string> EncryptionScope;
      Azure::Nullable<int64_t> SequenceNumber;
    }; // struct CreatePageBlobResult

    struct DeleteBlobContainerResult
    {
      std::string RequestId;
      bool Deleted = true;
    }; // struct DeleteBlobContainerResult

    struct DeleteBlobResult
    {
      std::string RequestId;
      bool Deleted = true;
    }; // struct DeleteBlobResult

    class DeleteSnapshotsOption {
    public:
      DeleteSnapshotsOption() = default;
      explicit DeleteSnapshotsOption(std::string value) : m_value(std::move(value)) {}
      bool operator==(const DeleteSnapshotsOption& other) const { return m_value == other.m_value; }
      bool operator!=(const DeleteSnapshotsOption& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      AZ_STORAGE_BLOBS_DLLEXPORT const static DeleteSnapshotsOption IncludeSnapshots;
      AZ_STORAGE_BLOBS_DLLEXPORT const static DeleteSnapshotsOption OnlySnapshots;

    private:
      std::string m_value;
    }; // extensible enum DeleteSnapshotsOption

    class EncryptionAlgorithmType {
    public:
      EncryptionAlgorithmType() = default;
      explicit EncryptionAlgorithmType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const EncryptionAlgorithmType& other) const
      {
        return m_value == other.m_value;
      }
      bool operator!=(const EncryptionAlgorithmType& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      AZ_STORAGE_BLOBS_DLLEXPORT const static EncryptionAlgorithmType Aes256;

    private:
      std::string m_value;
    }; // extensible enum EncryptionAlgorithmType

    struct FilterBlobItem
    {
      std::string BlobName;
      std::string BlobContainerName;
    }; // struct FilterBlobItem

    struct GetBlobTagsResult
    {
      std::string RequestId;
      std::map<std::string, std::string> Tags;
    }; // struct GetBlobTagsResult

    struct GetPageBlobPageRangesResult
    {
      std::string RequestId;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      int64_t BlobSize = 0;
      std::vector<Azure::Core::Http::HttpRange> PageRanges;
      std::vector<Azure::Core::Http::HttpRange> ClearRanges;
    }; // struct GetPageBlobPageRangesResult

    enum class ListBlobContainersIncludeFlags
    {
      None = 0,
      Metadata = 1,
      Deleted = 2,
    }; // bitwise enum ListBlobContainersIncludeFlags

    inline ListBlobContainersIncludeFlags operator|(
        ListBlobContainersIncludeFlags lhs,
        ListBlobContainersIncludeFlags rhs)
    {
      using type = std::underlying_type_t<ListBlobContainersIncludeFlags>;
      return static_cast<ListBlobContainersIncludeFlags>(
          static_cast<type>(lhs) | static_cast<type>(rhs));
    }

    inline ListBlobContainersIncludeFlags& operator|=(
        ListBlobContainersIncludeFlags& lhs,
        ListBlobContainersIncludeFlags rhs)
    {
      lhs = lhs | rhs;
      return lhs;
    }

    inline ListBlobContainersIncludeFlags operator&(
        ListBlobContainersIncludeFlags lhs,
        ListBlobContainersIncludeFlags rhs)
    {
      using type = std::underlying_type_t<ListBlobContainersIncludeFlags>;
      return static_cast<ListBlobContainersIncludeFlags>(
          static_cast<type>(lhs) & static_cast<type>(rhs));
    }

    inline ListBlobContainersIncludeFlags& operator&=(
        ListBlobContainersIncludeFlags& lhs,
        ListBlobContainersIncludeFlags rhs)
    {
      lhs = lhs & rhs;
      return lhs;
    }

    enum class ListBlobsIncludeFlags
    {
      None = 0,
      Copy = 1,
      Deleted = 2,
      Metadata = 4,
      Snapshots = 8,
      Versions = 16,
      UncomittedBlobs = 32,
    }; // bitwise enum ListBlobsIncludeFlags

    inline ListBlobsIncludeFlags operator|(ListBlobsIncludeFlags lhs, ListBlobsIncludeFlags rhs)
    {
      using type = std::underlying_type_t<ListBlobsIncludeFlags>;
      return static_cast<ListBlobsIncludeFlags>(static_cast<type>(lhs) | static_cast<type>(rhs));
    }

    inline ListBlobsIncludeFlags& operator|=(ListBlobsIncludeFlags& lhs, ListBlobsIncludeFlags rhs)
    {
      lhs = lhs | rhs;
      return lhs;
    }

    inline ListBlobsIncludeFlags operator&(ListBlobsIncludeFlags lhs, ListBlobsIncludeFlags rhs)
    {
      using type = std::underlying_type_t<ListBlobsIncludeFlags>;
      return static_cast<ListBlobsIncludeFlags>(static_cast<type>(lhs) & static_cast<type>(rhs));
    }

    inline ListBlobsIncludeFlags& operator&=(ListBlobsIncludeFlags& lhs, ListBlobsIncludeFlags rhs)
    {
      lhs = lhs & rhs;
      return lhs;
    }

    class ObjectReplicationStatus {
    public:
      ObjectReplicationStatus() = default;
      explicit ObjectReplicationStatus(std::string value) : m_value(std::move(value)) {}
      bool operator==(const ObjectReplicationStatus& other) const
      {
        return m_value == other.m_value;
      }
      bool operator!=(const ObjectReplicationStatus& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      AZ_STORAGE_BLOBS_DLLEXPORT const static ObjectReplicationStatus Complete;
      AZ_STORAGE_BLOBS_DLLEXPORT const static ObjectReplicationStatus Failed;

    private:
      std::string m_value;
    }; // extensible enum ObjectReplicationStatus

    class PublicAccessType {
    public:
      PublicAccessType() = default;
      explicit PublicAccessType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const PublicAccessType& other) const { return m_value == other.m_value; }
      bool operator!=(const PublicAccessType& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      AZ_STORAGE_BLOBS_DLLEXPORT const static PublicAccessType BlobContainer;
      AZ_STORAGE_BLOBS_DLLEXPORT const static PublicAccessType Blob;
      AZ_STORAGE_BLOBS_DLLEXPORT const static PublicAccessType None;

    private:
      std::string m_value;
    }; // extensible enum PublicAccessType

    class RehydratePriority {
    public:
      RehydratePriority() = default;
      explicit RehydratePriority(std::string value) : m_value(std::move(value)) {}
      bool operator==(const RehydratePriority& other) const { return m_value == other.m_value; }
      bool operator!=(const RehydratePriority& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      AZ_STORAGE_BLOBS_DLLEXPORT const static RehydratePriority High;
      AZ_STORAGE_BLOBS_DLLEXPORT const static RehydratePriority Standard;

    private:
      std::string m_value;
    }; // extensible enum RehydratePriority

    namespace _detail {
      struct ReleaseBlobContainerLeaseResult
      {
        std::string RequestId;
        Azure::ETag ETag;
        Azure::DateTime LastModified;
      }; // struct ReleaseBlobContainerLeaseResult
    } // namespace _detail

    namespace _detail {
      struct ReleaseBlobLeaseResult
      {
        std::string RequestId;
        Azure::ETag ETag;
        Azure::DateTime LastModified;
        Azure::Nullable<int64_t> SequenceNumber;
      }; // struct ReleaseBlobLeaseResult
    } // namespace _detail

    namespace _detail {
      struct RenewBlobContainerLeaseResult
      {
        std::string RequestId;
        Azure::ETag ETag;
        Azure::DateTime LastModified;
        std::string LeaseId;
      }; // struct RenewBlobContainerLeaseResult
    } // namespace _detail

    namespace _detail {
      struct RenewBlobLeaseResult
      {
        std::string RequestId;
        Azure::ETag ETag;
        Azure::DateTime LastModified;
        std::string LeaseId;
      }; // struct RenewBlobLeaseResult
    } // namespace _detail

    struct ResizePageBlobResult
    {
      std::string RequestId;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      int64_t SequenceNumber = 0;
    }; // struct ResizePageBlobResult

    class ScheduleBlobExpiryOriginType {
    public:
      ScheduleBlobExpiryOriginType() = default;
      explicit ScheduleBlobExpiryOriginType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const ScheduleBlobExpiryOriginType& other) const
      {
        return m_value == other.m_value;
      }
      bool operator!=(const ScheduleBlobExpiryOriginType& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      AZ_STORAGE_BLOBS_DLLEXPORT const static ScheduleBlobExpiryOriginType NeverExpire;
      AZ_STORAGE_BLOBS_DLLEXPORT const static ScheduleBlobExpiryOriginType RelativeToCreation;
      AZ_STORAGE_BLOBS_DLLEXPORT const static ScheduleBlobExpiryOriginType RelativeToNow;
      AZ_STORAGE_BLOBS_DLLEXPORT const static ScheduleBlobExpiryOriginType Absolute;

    private:
      std::string m_value;
    }; // extensible enum ScheduleBlobExpiryOriginType

    struct SealAppendBlobResult
    {
      std::string RequestId;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      bool IsSealed = true;
    }; // struct SealAppendBlobResult

    struct SetBlobAccessTierResult
    {
      std::string RequestId;
    }; // struct SetBlobAccessTierResult

    struct SetBlobContainerAccessPolicyResult
    {
      std::string RequestId;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
    }; // struct SetBlobContainerAccessPolicyResult

    struct SetBlobContainerMetadataResult
    {
      std::string RequestId;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
    }; // struct SetBlobContainerMetadataResult

    struct SetBlobExpiryResult
    {
      std::string RequestId;
    }; // struct SetBlobExpiryResult

    struct SetBlobHttpHeadersResult
    {
      std::string RequestId;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      Azure::Nullable<int64_t> SequenceNumber;
    }; // struct SetBlobHttpHeadersResult

    struct SetBlobMetadataResult
    {
      std::string RequestId;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      Azure::Nullable<int64_t> SequenceNumber;
    }; // struct SetBlobMetadataResult

    struct SetBlobTagsResult
    {
      std::string RequestId;
    }; // struct SetBlobTagsResult

    struct SetServicePropertiesResult
    {
      std::string RequestId;
    }; // struct SetServicePropertiesResult

    class SkuName {
    public:
      SkuName() = default;
      explicit SkuName(std::string value) : m_value(std::move(value)) {}
      bool operator==(const SkuName& other) const { return m_value == other.m_value; }
      bool operator!=(const SkuName& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      AZ_STORAGE_BLOBS_DLLEXPORT const static SkuName StandardLrs;
      AZ_STORAGE_BLOBS_DLLEXPORT const static SkuName StandardGrs;
      AZ_STORAGE_BLOBS_DLLEXPORT const static SkuName StandardRagrs;
      AZ_STORAGE_BLOBS_DLLEXPORT const static SkuName StandardZrs;
      AZ_STORAGE_BLOBS_DLLEXPORT const static SkuName PremiumLrs;
      AZ_STORAGE_BLOBS_DLLEXPORT const static SkuName PremiumZrs;
      AZ_STORAGE_BLOBS_DLLEXPORT const static SkuName StandardGzrs;
      AZ_STORAGE_BLOBS_DLLEXPORT const static SkuName StandardRagzrs;

    private:
      std::string m_value;
    }; // extensible enum SkuName

    namespace _detail {
      struct SubmitBlobBatchResult
      {
        std::string RequestId;
        std::string ContentType;
      }; // struct SubmitBlobBatchResult
    } // namespace _detail

    struct UndeleteBlobContainerResult
    {
      std::string RequestId;
    }; // struct UndeleteBlobContainerResult

    struct UndeleteBlobResult
    {
      std::string RequestId;
    }; // struct UndeleteBlobResult

    struct UserDelegationKey
    {
      std::string SignedObjectId;
      std::string SignedTenantId;
      Azure::DateTime SignedStartsOn;
      Azure::DateTime SignedExpiresOn;
      std::string SignedService;
      std::string SignedVersion;
      std::string Value;
    }; // struct UserDelegationKey

    struct AccountInfo
    {
      Models::SkuName SkuName;
      Models::AccountKind AccountKind;
      bool IsHierarchicalNamespaceEnabled = false;
    }; // struct AccountInfo

    struct BlobAnalyticsLogging
    {
      std::string Version;
      bool Delete = false;
      bool Read = false;
      bool Write = false;
      BlobRetentionPolicy RetentionPolicy;
    }; // struct BlobAnalyticsLogging

    struct BlobContainerAccessPolicy
    {
      PublicAccessType AccessType = PublicAccessType::None;
      std::vector<BlobSignedIdentifier> SignedIdentifiers;
    }; // struct BlobContainerAccessPolicy

    struct BlobContainerItemDetails
    {
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      Storage::Metadata Metadata;
      PublicAccessType AccessType = PublicAccessType::None;
      bool HasImmutabilityPolicy = false;
      bool HasLegalHold = false;
      Azure::Nullable<BlobLeaseDurationType> LeaseDuration;
      BlobLeaseState LeaseState = BlobLeaseState::Available;
      BlobLeaseStatus LeaseStatus = BlobLeaseStatus::Unlocked;
      std::string DefaultEncryptionScope;
      bool PreventEncryptionScopeOverride = false;
      Azure::Nullable<int32_t> RemainingRetentionDays;
      Azure::Nullable<Azure::DateTime> DeletedOn;
    }; // struct BlobContainerItemDetails

    struct BlobContainerProperties
    {
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      Storage::Metadata Metadata;
      PublicAccessType AccessType = PublicAccessType::None;
      bool HasImmutabilityPolicy = false;
      bool HasLegalHold = false;
      Azure::Nullable<BlobLeaseDurationType> LeaseDuration;
      BlobLeaseState LeaseState = BlobLeaseState::Available;
      BlobLeaseStatus LeaseStatus = BlobLeaseStatus::Unlocked;
      std::string DefaultEncryptionScope;
      bool PreventEncryptionScopeOverride = false;
    }; // struct BlobContainerProperties

    struct BlobGeoReplication
    {
      BlobGeoReplicationStatus Status;
      Azure::Nullable<Azure::DateTime> LastSyncedOn;
    }; // struct BlobGeoReplication

    struct BlobMetrics
    {
      std::string Version;
      bool IsEnabled = false;
      BlobRetentionPolicy RetentionPolicy;
      Azure::Nullable<bool> IncludeApis;
    }; // struct BlobMetrics

    struct FindBlobsByTagsSinglePageResult
    {
      std::string RequestId;
      std::string ServiceEndpoint;
      Azure::Nullable<std::string> ContinuationToken;
      std::vector<FilterBlobItem> Items;
    }; // struct FindBlobsByTagsSinglePageResult

    struct GetBlockListResult
    {
      std::string RequestId;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      std::string ContentType;
      int64_t BlobSize = 0;
      std::vector<BlobBlock> CommittedBlocks;
      std::vector<BlobBlock> UncommittedBlocks;
    }; // struct GetBlockListResult

    struct ObjectReplicationRule
    {
      std::string RuleId;
      ObjectReplicationStatus ReplicationStatus;
    }; // struct ObjectReplicationRule

    namespace _detail {
      struct StartCopyBlobFromUriResult
      {
        std::string RequestId;
        Azure::ETag ETag;
        Azure::DateTime LastModified;
        std::string CopyId;
        Models::CopyStatus CopyStatus;
        Azure::Nullable<std::string> VersionId;
      }; // struct StartCopyBlobFromUriResult
    } // namespace _detail

    namespace _detail {
      struct StartCopyPageBlobIncrementalResult
      {
        std::string RequestId;
        Azure::ETag ETag;
        Azure::DateTime LastModified;
        std::string CopyId;
        Models::CopyStatus CopyStatus;
        Azure::Nullable<std::string> VersionId;
      }; // struct StartCopyPageBlobIncrementalResult
    } // namespace _detail

    struct AppendBlockFromUriResult
    {
      std::string RequestId;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      Azure::Nullable<ContentHash> TransactionalContentHash;
      int64_t AppendOffset = 0;
      int64_t CommittedBlockCount = 0;
      bool IsServerEncrypted = false;
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      Azure::Nullable<std::string> EncryptionScope;
    }; // struct AppendBlockFromUriResult

    struct AppendBlockResult
    {
      std::string RequestId;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      Azure::Nullable<ContentHash> TransactionalContentHash;
      int64_t AppendOffset = 0;
      int64_t CommittedBlockCount = 0;
      bool IsServerEncrypted = false;
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      Azure::Nullable<std::string> EncryptionScope;
    }; // struct AppendBlockResult

    struct BlobContainerItem
    {
      std::string Name;
      bool IsDeleted = false;
      Azure::Nullable<std::string> VersionId;
      BlobContainerItemDetails Details;
    }; // struct BlobContainerItem

    struct BlobHttpHeaders
    {
      std::string ContentType;
      std::string ContentEncoding;
      std::string ContentLanguage;
      Storage::ContentHash ContentHash;
      std::string CacheControl;
      std::string ContentDisposition;
    }; // struct BlobHttpHeaders

    struct BlobServiceProperties
    {
      BlobAnalyticsLogging Logging;
      BlobMetrics HourMetrics;
      BlobMetrics MinuteMetrics;
      std::vector<BlobCorsRule> Cors;
      Azure::Nullable<std::string> DefaultServiceVersion;
      BlobRetentionPolicy DeleteRetentionPolicy;
      BlobStaticWebsite StaticWebsite;
    }; // struct BlobServiceProperties

    struct CommitBlockListResult
    {
      std::string RequestId;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      Azure::Nullable<std::string> VersionId;
      bool IsServerEncrypted = false;
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      Azure::Nullable<std::string> EncryptionScope;
      Azure::Nullable<ContentHash> TransactionalContentHash;
    }; // struct CommitBlockListResult

    struct ObjectReplicationPolicy
    {
      std::string PolicyId;
      std::vector<ObjectReplicationRule> Rules;
    }; // struct ObjectReplicationPolicy

    struct ServiceStatistics
    {
      BlobGeoReplication GeoReplication;
    }; // struct ServiceStatistics

    struct StageBlockFromUriResult
    {
      std::string RequestId;
      Azure::Nullable<ContentHash> TransactionalContentHash;
      bool IsServerEncrypted = false;
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      Azure::Nullable<std::string> EncryptionScope;
    }; // struct StageBlockFromUriResult

    struct StageBlockResult
    {
      std::string RequestId;
      Azure::Nullable<ContentHash> TransactionalContentHash;
      bool IsServerEncrypted = false;
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      Azure::Nullable<std::string> EncryptionScope;
    }; // struct StageBlockResult

    struct UploadBlockBlobResult
    {
      std::string RequestId;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      Azure::Nullable<std::string> VersionId;
      bool IsServerEncrypted = false;
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      Azure::Nullable<std::string> EncryptionScope;
      Azure::Nullable<ContentHash> TransactionalContentHash;
    }; // struct UploadBlockBlobResult

    struct UploadPageBlobPagesFromUriResult
    {
      std::string RequestId;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      Azure::Nullable<ContentHash> TransactionalContentHash;
      int64_t SequenceNumber = 0;
      bool IsServerEncrypted = false;
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      Azure::Nullable<std::string> EncryptionScope;
    }; // struct UploadPageBlobPagesFromUriResult

    struct UploadPageBlobPagesResult
    {
      std::string RequestId;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      Azure::Nullable<ContentHash> TransactionalContentHash;
      int64_t SequenceNumber = 0;
      bool IsServerEncrypted = false;
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      Azure::Nullable<std::string> EncryptionScope;
    }; // struct UploadPageBlobPagesResult

    struct BlobItemDetails
    {
      BlobHttpHeaders HttpHeaders;
      Storage::Metadata Metadata;
      Azure::DateTime CreatedOn;
      Azure::Nullable<Azure::DateTime> ExpiresOn;
      Azure::Nullable<Azure::DateTime> LastAccessedOn;
      Azure::DateTime LastModified;
      Azure::ETag ETag;
      Azure::Nullable<AccessTier> Tier;
      Azure::Nullable<bool> IsAccessTierInferred;
      BlobLeaseStatus LeaseStatus = BlobLeaseStatus::Unlocked;
      BlobLeaseState LeaseState = BlobLeaseState::Available;
      Azure::Nullable<BlobLeaseDurationType> LeaseDuration;
      bool IsServerEncrypted = false;
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      Azure::Nullable<std::string> EncryptionScope;
      Azure::Nullable<int64_t> SequenceNumber; // only for page blobd
      Azure::Nullable<bool> IsSealed; // only for append blob
      std::vector<ObjectReplicationPolicy>
          ObjectReplicationSourceProperties; // only valid for replication source blob
    }; // struct BlobItemDetails

    struct BlobProperties
    {
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      Azure::DateTime CreatedOn;
      Azure::Nullable<Azure::DateTime> ExpiresOn;
      Azure::Nullable<Azure::DateTime> LastAccessedOn;
      Storage::Metadata Metadata;
      Models::BlobType BlobType;
      Azure::Nullable<BlobLeaseDurationType> LeaseDuration;
      Azure::Nullable<BlobLeaseState> LeaseState;
      Azure::Nullable<BlobLeaseStatus> LeaseStatus;
      int64_t BlobSize = 0;
      BlobHttpHeaders HttpHeaders;
      Azure::Nullable<int64_t> SequenceNumber; // only for page blob
      Azure::Nullable<int32_t> CommittedBlockCount; // only for append blob
      Azure::Nullable<bool> IsSealed; // only for append blob
      bool IsServerEncrypted = false;
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      Azure::Nullable<std::string> EncryptionScope;
      Azure::Nullable<AccessTier> Tier;
      Azure::Nullable<bool> IsAccessTierInferred;
      Azure::Nullable<BlobArchiveStatus> ArchiveStatus;
      Azure::Nullable<Models::RehydratePriority> RehydratePriority;
      Azure::Nullable<Azure::DateTime> AccessTierChangedOn;
      Azure::Nullable<std::string> CopyId;
      Azure::Nullable<std::string> CopySource;
      Azure::Nullable<Models::CopyStatus> CopyStatus;
      Azure::Nullable<std::string> CopyStatusDescription;
      Azure::Nullable<bool> IsIncrementalCopy;
      Azure::Nullable<std::string> IncrementalCopyDestinationSnapshot;
      Azure::Nullable<std::string> CopyProgress;
      Azure::Nullable<Azure::DateTime> CopyCompletedOn;
      Azure::Nullable<std::string>
          ObjectReplicationDestinationPolicyId; // only valid for replication destination blob
      std::vector<ObjectReplicationPolicy>
          ObjectReplicationSourceProperties; // only valid for replication source blob
      Azure::Nullable<int32_t> TagCount;
      Azure::Nullable<std::string> VersionId;
      Azure::Nullable<bool> IsCurrentVersion;
    }; // struct BlobProperties

    struct DownloadBlobDetails
    {
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      Azure::DateTime CreatedOn;
      Azure::Nullable<Azure::DateTime> ExpiresOn;
      Azure::Nullable<Azure::DateTime> LastAccessedOn;
      BlobHttpHeaders HttpHeaders;
      Storage::Metadata Metadata;
      Azure::Nullable<int64_t> SequenceNumber; // only for page blob
      Azure::Nullable<int64_t> CommittedBlockCount; // only for append blob
      Azure::Nullable<bool> IsSealed; // only for append blob
      Azure::Nullable<BlobLeaseDurationType> LeaseDuration;
      Azure::Nullable<BlobLeaseState> LeaseState;
      Azure::Nullable<BlobLeaseStatus> LeaseStatus;
      bool IsServerEncrypted = false;
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      Azure::Nullable<std::string> EncryptionScope;
      Azure::Nullable<std::string>
          ObjectReplicationDestinationPolicyId; // only valid for replication destination blob
      std::vector<ObjectReplicationPolicy>
          ObjectReplicationSourceProperties; // only valid for replication source blob
      Azure::Nullable<int32_t> TagCount;
      Azure::Nullable<std::string> CopyId;
      Azure::Nullable<std::string> CopySource;
      Azure::Nullable<Models::CopyStatus> CopyStatus;
      Azure::Nullable<std::string> CopyStatusDescription;
      Azure::Nullable<std::string> CopyProgress;
      Azure::Nullable<Azure::DateTime> CopyCompletedOn;
      Azure::Nullable<std::string> VersionId;
      Azure::Nullable<bool> IsCurrentVersion;
    }; // struct DownloadBlobDetails

    struct ListBlobContainersSinglePageResult
    {
      std::string RequestId;
      std::string ServiceEndpoint;
      std::string Prefix;
      Azure::Nullable<std::string> ContinuationToken;
      std::vector<BlobContainerItem> Items;
    }; // struct ListBlobContainersSinglePageResult

    struct BlobItem
    {
      std::string Name;
      int64_t BlobSize = 0;
      Models::BlobType BlobType;
      bool IsDeleted = false;
      std::string Snapshot;
      Azure::Nullable<std::string> VersionId;
      Azure::Nullable<bool> IsCurrentVersion;
      BlobItemDetails Details;
    }; // struct BlobItem

    struct DownloadBlobResult
    {
      std::string RequestId;
      std::unique_ptr<Azure::Core::IO::BodyStream> BodyStream;
      Azure::Core::Http::HttpRange ContentRange;
      int64_t BlobSize = 0;
      Models::BlobType BlobType;
      Azure::Nullable<ContentHash> TransactionalContentHash; // hash for the downloaded range
      DownloadBlobDetails Details;
    }; // struct DownloadBlobResult

    struct ListBlobsByHierarchySinglePageResult
    {
      std::string RequestId;
      std::string ServiceEndpoint;
      std::string BlobContainerName;
      std::string Prefix;
      std::string Delimiter;
      Azure::Nullable<std::string> ContinuationToken;
      std::vector<BlobItem> Items;
      std::vector<std::string> BlobPrefixes;
    }; // struct ListBlobsByHierarchySinglePageResult

    struct ListBlobsSinglePageResult
    {
      std::string RequestId;
      std::string ServiceEndpoint;
      std::string BlobContainerName;
      std::string Prefix;
      Azure::Nullable<std::string> ContinuationToken;
      std::vector<BlobItem> Items;
    }; // struct ListBlobsSinglePageResult

  } // namespace Models

  namespace _detail {

    using namespace Models;

    inline std::string ListBlobContainersIncludeFlagsToString(
        const ListBlobContainersIncludeFlags& val)
    {
      ListBlobContainersIncludeFlags value_list[] = {
          ListBlobContainersIncludeFlags::Metadata,
          ListBlobContainersIncludeFlags::Deleted,
      };
      const char* string_list[] = {
          "metadata",
          "deleted",
      };
      std::string ret;
      for (std::size_t i = 0; i < sizeof(value_list) / sizeof(ListBlobContainersIncludeFlags); ++i)
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

    inline std::string ListBlobsIncludeFlagsToString(const ListBlobsIncludeFlags& val)
    {
      ListBlobsIncludeFlags value_list[] = {
          ListBlobsIncludeFlags::Copy,
          ListBlobsIncludeFlags::Deleted,
          ListBlobsIncludeFlags::Metadata,
          ListBlobsIncludeFlags::Snapshots,
          ListBlobsIncludeFlags::Versions,
          ListBlobsIncludeFlags::UncomittedBlobs,
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
      for (std::size_t i = 0; i < sizeof(value_list) / sizeof(ListBlobsIncludeFlags); ++i)
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

    class BlobRestClient {
    public:
      class Service {
      public:
        struct ListBlobContainersSinglePageOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::string> Prefix;
          Azure::Nullable<std::string> ContinuationToken;
          Azure::Nullable<int32_t> MaxResults;
          ListBlobContainersIncludeFlags Include = ListBlobContainersIncludeFlags::None;
        }; // struct ListBlobContainersSinglePageOptions

        static Azure::Response<ListBlobContainersSinglePageResult> ListBlobContainersSinglePage(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const ListBlobContainersSinglePageOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.GetUrl().AppendQueryParameter("comp", "list");
          if (options.Prefix.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "prefix", Storage::_detail::UrlEncodeQueryParameter(options.Prefix.GetValue()));
          }
          if (options.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "marker",
                Storage::_detail::UrlEncodeQueryParameter(options.ContinuationToken.GetValue()));
          }
          if (options.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "maxresults", std::to_string(options.MaxResults.GetValue()));
          }
          std::string list_blob_containers_include_flags
              = ListBlobContainersIncludeFlagsToString(options.Include);
          if (!list_blob_containers_include_flags.empty())
          {
            request.GetUrl().AppendQueryParameter(
                "include",
                Storage::_detail::UrlEncodeQueryParameter(list_blob_containers_include_flags));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          ListBlobContainersSinglePageResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            Storage::_detail::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = ListBlobContainersSinglePageResultFromXml(reader);
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          return Azure::Response<ListBlobContainersSinglePageResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct GetUserDelegationKeyOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::DateTime StartsOn;
          Azure::DateTime ExpiresOn;
        }; // struct GetUserDelegationKeyOptions

        static Azure::Response<UserDelegationKey> GetUserDelegationKey(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const GetUserDelegationKeyOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          std::string xml_body;
          {
            Storage::_detail::XmlWriter writer;
            GetUserDelegationKeyOptionsToXml(writer, options);
            xml_body = writer.GetDocument();
            writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::End});
          }
          Azure::Core::IO::MemoryBodyStream xml_body_stream(
              reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
          auto request = Azure::Core::Http::Request(
              Azure::Core::Http::HttpMethod::Post, url, &xml_body_stream);
          request.SetHeader("Content-Length", std::to_string(xml_body_stream.Length()));
          request.GetUrl().AppendQueryParameter("restype", "service");
          request.GetUrl().AppendQueryParameter("comp", "userdelegationkey");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          UserDelegationKey response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            Storage::_detail::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = UserDelegationKeyFromXml(reader);
          }
          return Azure::Response<UserDelegationKey>(std::move(response), std::move(pHttpResponse));
        }

        struct GetServicePropertiesOptions
        {
          Azure::Nullable<int32_t> Timeout;
        }; // struct GetServicePropertiesOptions

        static Azure::Response<BlobServiceProperties> GetProperties(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const GetServicePropertiesOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter("restype", "service");
          request.GetUrl().AppendQueryParameter("comp", "properties");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          BlobServiceProperties response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            Storage::_detail::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = BlobServicePropertiesFromXml(reader);
          }
          return Azure::Response<BlobServiceProperties>(
              std::move(response), std::move(pHttpResponse));
        }

        struct SetServicePropertiesOptions
        {
          Azure::Nullable<int32_t> Timeout;
          BlobServiceProperties Properties;
        }; // struct SetServicePropertiesOptions

        static Azure::Response<SetServicePropertiesResult> SetProperties(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const SetServicePropertiesOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          std::string xml_body;
          {
            Storage::_detail::XmlWriter writer;
            SetServicePropertiesOptionsToXml(writer, options);
            xml_body = writer.GetDocument();
            writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::End});
          }
          Azure::Core::IO::MemoryBodyStream xml_body_stream(
              reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
          auto request = Azure::Core::Http::Request(
              Azure::Core::Http::HttpMethod::Put, url, &xml_body_stream);
          request.SetHeader("Content-Length", std::to_string(xml_body_stream.Length()));
          request.GetUrl().AppendQueryParameter("restype", "service");
          request.GetUrl().AppendQueryParameter("comp", "properties");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          SetServicePropertiesResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 202))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          return Azure::Response<SetServicePropertiesResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct GetAccountInfoOptions
        {
          Azure::Nullable<int32_t> Timeout;
        }; // struct GetAccountInfoOptions

        static Azure::Response<AccountInfo> GetAccountInfo(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const GetAccountInfoOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, url);
          request.GetUrl().AppendQueryParameter("restype", "account");
          request.GetUrl().AppendQueryParameter("comp", "properties");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          AccountInfo response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.SkuName = SkuName(httpResponse.GetHeaders().at("x-ms-sku-name"));
          response.AccountKind = AccountKind(httpResponse.GetHeaders().at("x-ms-account-kind"));
          response.IsHierarchicalNamespaceEnabled
              = httpResponse.GetHeaders().at("x-ms-is-hns-enabled") == "true";
          return Azure::Response<AccountInfo>(std::move(response), std::move(pHttpResponse));
        }

        struct GetServiceStatisticsOptions
        {
          Azure::Nullable<int32_t> Timeout;
        }; // struct GetServiceStatisticsOptions

        static Azure::Response<ServiceStatistics> GetStatistics(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const GetServiceStatisticsOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter("restype", "service");
          request.GetUrl().AppendQueryParameter("comp", "stats");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          ServiceStatistics response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            Storage::_detail::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = ServiceStatisticsFromXml(reader);
          }
          return Azure::Response<ServiceStatistics>(std::move(response), std::move(pHttpResponse));
        }

        struct FindBlobsByTagsSinglePageOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string Where;
          Azure::Nullable<std::string> ContinuationToken;
          Azure::Nullable<int32_t> MaxResults;
        }; // struct FindBlobsByTagsSinglePageOptions

        static Azure::Response<FindBlobsByTagsSinglePageResult> FindBlobsByTagsSinglePage(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const FindBlobsByTagsSinglePageOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.GetUrl().AppendQueryParameter("comp", "blobs");
          request.GetUrl().AppendQueryParameter(
              "where", Storage::_detail::UrlEncodeQueryParameter(options.Where));
          if (options.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "marker",
                Storage::_detail::UrlEncodeQueryParameter(options.ContinuationToken.GetValue()));
          }
          if (options.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "maxresults", std::to_string(options.MaxResults.GetValue()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          FindBlobsByTagsSinglePageResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            Storage::_detail::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = FindBlobsByTagsSinglePageResultFromXml(reader);
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          return Azure::Response<FindBlobsByTagsSinglePageResult>(
              std::move(response), std::move(pHttpResponse));
        }

      private:
        static BlobServiceProperties BlobServicePropertiesFromXml(
            Storage::_detail::XmlReader& reader)
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
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::Text)
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

        static FindBlobsByTagsSinglePageResult FindBlobsByTagsSinglePageResultFromXml(
            Storage::_detail::XmlReader& reader)
        {
          FindBlobsByTagsSinglePageResult ret;
          enum class XmlTagName
          {
            k_EnumerationResults,
            k_NextMarker,
            k_Blobs,
            k_Blob,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
            {
              if (std::strcmp(node.Name, "EnumerationResults") == 0)
              {
                path.emplace_back(XmlTagName::k_EnumerationResults);
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
            else if (node.Type == Storage::_detail::XmlNodeType::Text)
            {
              if (path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                  && path[1] == XmlTagName::k_NextMarker)
              {
                ret.ContinuationToken = node.Value;
              }
            }
            else if (node.Type == Storage::_detail::XmlNodeType::Attribute)
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

        static ListBlobContainersSinglePageResult ListBlobContainersSinglePageResultFromXml(
            Storage::_detail::XmlReader& reader)
        {
          ListBlobContainersSinglePageResult ret;
          enum class XmlTagName
          {
            k_EnumerationResults,
            k_Prefix,
            k_NextMarker,
            k_Containers,
            k_Container,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
            {
              if (std::strcmp(node.Name, "EnumerationResults") == 0)
              {
                path.emplace_back(XmlTagName::k_EnumerationResults);
              }
              else if (std::strcmp(node.Name, "Prefix") == 0)
              {
                path.emplace_back(XmlTagName::k_Prefix);
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
            else if (node.Type == Storage::_detail::XmlNodeType::Text)
            {
              if (path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                  && path[1] == XmlTagName::k_Prefix)
              {
                ret.Prefix = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                  && path[1] == XmlTagName::k_NextMarker)
              {
                ret.ContinuationToken = node.Value;
              }
            }
            else if (node.Type == Storage::_detail::XmlNodeType::Attribute)
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

        static ServiceStatistics ServiceStatisticsFromXml(Storage::_detail::XmlReader& reader)
        {
          ServiceStatistics ret;
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
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::Text)
            {
            }
          }
          return ret;
        }

        static UserDelegationKey UserDelegationKeyFromXml(Storage::_detail::XmlReader& reader)
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
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::Text)
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
                ret.SignedStartsOn
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc3339);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_UserDelegationKey
                  && path[1] == XmlTagName::k_SignedExpiry)
              {
                ret.SignedExpiresOn
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc3339);
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

        static BlobAnalyticsLogging BlobAnalyticsLoggingFromXml(Storage::_detail::XmlReader& reader)
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
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::Text)
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

        static BlobContainerItem BlobContainerItemFromXml(Storage::_detail::XmlReader& reader)
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
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
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
                ret.Details.Metadata = MetadataFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == Storage::_detail::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_Name)
              {
                ret.Name = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_Etag)
              {
                ret.Details.ETag = Azure::ETag(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_LastModified)
              {
                ret.Details.LastModified
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_PublicAccess)
              {
                ret.Details.AccessType = PublicAccessType(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_HasImmutabilityPolicy)
              {
                ret.Details.HasImmutabilityPolicy = std::strcmp(node.Value, "true") == 0;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_HasLegalHold)
              {
                ret.Details.HasLegalHold = std::strcmp(node.Value, "true") == 0;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_LeaseStatus)
              {
                ret.Details.LeaseStatus = BlobLeaseStatus(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_LeaseState)
              {
                ret.Details.LeaseState = BlobLeaseState(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_LeaseDuration)
              {
                ret.Details.LeaseDuration = BlobLeaseDurationType(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_DefaultEncryptionScope)
              {
                ret.Details.DefaultEncryptionScope = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_DenyEncryptionScopeOverride)
              {
                ret.Details.PreventEncryptionScopeOverride = std::strcmp(node.Value, "true") == 0;
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
                ret.Details.DeletedOn
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_RemainingRetentionDays)
              {
                ret.Details.RemainingRetentionDays = std::stoi(node.Value);
              }
            }
          }
          return ret;
        }

        static BlobCorsRule BlobCorsRuleFromXml(Storage::_detail::XmlReader& reader)
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
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::Text)
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

        static BlobGeoReplication BlobGeoReplicationFromXml(Storage::_detail::XmlReader& reader)
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
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_Status)
              {
                ret.Status = BlobGeoReplicationStatus(node.Value);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_LastSyncTime)
              {
                ret.LastSyncedOn
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
              }
            }
          }
          return ret;
        }

        static BlobMetrics BlobMetricsFromXml(Storage::_detail::XmlReader& reader)
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
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_Version)
              {
                ret.Version = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_Enabled)
              {
                ret.IsEnabled = std::strcmp(node.Value, "true") == 0;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_IncludeAPIs)
              {
                ret.IncludeApis = std::strcmp(node.Value, "true") == 0;
              }
            }
          }
          return ret;
        }

        static BlobRetentionPolicy BlobRetentionPolicyFromXml(Storage::_detail::XmlReader& reader)
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
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_Enabled)
              {
                ret.IsEnabled = std::strcmp(node.Value, "true") == 0;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_Days)
              {
                ret.Days = std::stoi(node.Value);
              }
            }
          }
          return ret;
        }

        static BlobStaticWebsite BlobStaticWebsiteFromXml(Storage::_detail::XmlReader& reader)
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
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_Enabled)
              {
                ret.IsEnabled = std::strcmp(node.Value, "true") == 0;
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

        static FilterBlobItem FilterBlobItemFromXml(Storage::_detail::XmlReader& reader)
        {
          FilterBlobItem ret;
          enum class XmlTagName
          {
            k_Name,
            k_ContainerName,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
            {
              if (std::strcmp(node.Name, "Name") == 0)
              {
                path.emplace_back(XmlTagName::k_Name);
              }
              else if (std::strcmp(node.Name, "ContainerName") == 0)
              {
                path.emplace_back(XmlTagName::k_ContainerName);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
            }
            else if (node.Type == Storage::_detail::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_Name)
              {
                ret.BlobName = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_ContainerName)
              {
                ret.BlobContainerName = node.Value;
              }
            }
          }
          return ret;
        }

        static Metadata MetadataFromXml(Storage::_detail::XmlReader& reader)
        {
          Metadata ret;
          int depth = 0;
          std::string key;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
            {
              if (depth++ == 0)
              {
                key = node.Name;
              }
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
            {
              if (depth-- == 0)
              {
                break;
              }
            }
            else if (depth == 1 && node.Type == Storage::_detail::XmlNodeType::Text)
            {
              ret.emplace(std::move(key), std::string(node.Value));
            }
          }
          return ret;
        }

        static void GetUserDelegationKeyOptionsToXml(
            Storage::_detail::XmlWriter& writer,
            const GetUserDelegationKeyOptions& options)
        {
          writer.Write(
              Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "KeyInfo"});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "Start"});
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::Text,
              nullptr,
              options.StartsOn
                  .ToString(
                      Azure::DateTime::DateFormat::Rfc3339,
                      Azure::DateTime::TimeFractionFormat::Truncate)
                  .data()});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          writer.Write(
              Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "Expiry"});
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::Text,
              nullptr,
              options.ExpiresOn
                  .ToString(
                      Azure::DateTime::DateFormat::Rfc3339,
                      Azure::DateTime::TimeFractionFormat::Truncate)
                  .data()});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
        }

        static void SetServicePropertiesOptionsToXml(
            Storage::_detail::XmlWriter& writer,
            const SetServicePropertiesOptions& options)
        {
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::StartTag, "StorageServiceProperties"});
          BlobServicePropertiesToXml(writer, options.Properties);
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
        }

        static void BlobServicePropertiesToXml(
            Storage::_detail::XmlWriter& writer,
            const BlobServiceProperties& options)
        {
          writer.Write(
              Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "Logging"});
          BlobAnalyticsLoggingToXml(writer, options.Logging);
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          writer.Write(
              Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "HourMetrics"});
          BlobMetricsToXml(writer, options.HourMetrics);
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          writer.Write(
              Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "MinuteMetrics"});
          BlobMetricsToXml(writer, options.MinuteMetrics);
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "Cors"});
          for (const auto& i : options.Cors)
          {
            BlobCorsRuleToXml(writer, i);
          }
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          if (options.DefaultServiceVersion.HasValue())
          {
            writer.Write(Storage::_detail::XmlNode{
                Storage::_detail::XmlNodeType::StartTag, "DefaultServiceVersion"});
            writer.Write(Storage::_detail::XmlNode{
                Storage::_detail::XmlNodeType::Text,
                nullptr,
                options.DefaultServiceVersion.GetValue().data()});
            writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          }
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::StartTag, "DeleteRetentionPolicy"});
          BlobRetentionPolicyToXml(writer, options.DeleteRetentionPolicy);
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          writer.Write(
              Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "StaticWebsite"});
          BlobStaticWebsiteToXml(writer, options.StaticWebsite);
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
        }

        static void BlobAnalyticsLoggingToXml(
            Storage::_detail::XmlWriter& writer,
            const BlobAnalyticsLogging& options)
        {
          writer.Write(
              Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "Version"});
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::Text, nullptr, options.Version.data()});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          writer.Write(
              Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "Delete"});
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::Text, nullptr, options.Delete ? "true" : "false"});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "Read"});
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::Text, nullptr, options.Read ? "true" : "false"});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "Write"});
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::Text, nullptr, options.Write ? "true" : "false"});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::StartTag, "RetentionPolicy"});
          BlobRetentionPolicyToXml(writer, options.RetentionPolicy);
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
        }

        static void BlobCorsRuleToXml(
            Storage::_detail::XmlWriter& writer,
            const BlobCorsRule& options)
        {
          writer.Write(
              Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "CorsRule"});
          writer.Write(
              Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "AllowedOrigins"});
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::Text, nullptr, options.AllowedOrigins.data()});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          writer.Write(
              Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "AllowedMethods"});
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::Text, nullptr, options.AllowedMethods.data()});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          writer.Write(
              Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "AllowedHeaders"});
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::Text, nullptr, options.AllowedHeaders.data()});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          writer.Write(
              Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "ExposedHeaders"});
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::Text, nullptr, options.ExposedHeaders.data()});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::StartTag, "MaxAgeInSeconds"});
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::Text,
              nullptr,
              std::to_string(options.MaxAgeInSeconds).data()});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
        }

        static void BlobMetricsToXml(
            Storage::_detail::XmlWriter& writer,
            const BlobMetrics& options)
        {
          writer.Write(
              Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "Version"});
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::Text, nullptr, options.Version.data()});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          writer.Write(
              Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "Enabled"});
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::Text, nullptr, options.IsEnabled ? "true" : "false"});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          if (options.IncludeApis.HasValue())
          {
            writer.Write(
                Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "IncludeAPIs"});
            writer.Write(Storage::_detail::XmlNode{
                Storage::_detail::XmlNodeType::Text,
                nullptr,
                options.IncludeApis.GetValue() ? "true" : "false"});
            writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          }
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::StartTag, "RetentionPolicy"});
          BlobRetentionPolicyToXml(writer, options.RetentionPolicy);
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
        }

        static void BlobRetentionPolicyToXml(
            Storage::_detail::XmlWriter& writer,
            const BlobRetentionPolicy& options)
        {
          writer.Write(
              Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "Enabled"});
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::Text, nullptr, options.IsEnabled ? "true" : "false"});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          if (options.Days.HasValue())
          {
            writer.Write(
                Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "Days"});
            writer.Write(Storage::_detail::XmlNode{
                Storage::_detail::XmlNodeType::Text,
                nullptr,
                std::to_string(options.Days.GetValue()).data()});
            writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          }
        }

        static void BlobStaticWebsiteToXml(
            Storage::_detail::XmlWriter& writer,
            const BlobStaticWebsite& options)
        {
          writer.Write(
              Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "Enabled"});
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::Text, nullptr, options.IsEnabled ? "true" : "false"});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          if (options.IndexDocument.HasValue())
          {
            writer.Write(Storage::_detail::XmlNode{
                Storage::_detail::XmlNodeType::StartTag, "IndexDocument"});
            writer.Write(Storage::_detail::XmlNode{
                Storage::_detail::XmlNodeType::Text,
                nullptr,
                options.IndexDocument.GetValue().data()});
            writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          }
          if (options.DefaultIndexDocumentPath.HasValue())
          {
            writer.Write(Storage::_detail::XmlNode{
                Storage::_detail::XmlNodeType::StartTag, "DefaultIndexDocumentPath"});
            writer.Write(Storage::_detail::XmlNode{
                Storage::_detail::XmlNodeType::Text,
                nullptr,
                options.DefaultIndexDocumentPath.GetValue().data()});
            writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          }
          if (options.ErrorDocument404Path.HasValue())
          {
            writer.Write(Storage::_detail::XmlNode{
                Storage::_detail::XmlNodeType::StartTag, "ErrorDocument404Path"});
            writer.Write(Storage::_detail::XmlNode{
                Storage::_detail::XmlNodeType::Text,
                nullptr,
                options.ErrorDocument404Path.GetValue().data()});
            writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          }
        }

      }; // class Service

      class BlobContainer {
      public:
        struct CreateBlobContainerOptions
        {
          Azure::Nullable<int32_t> Timeout;
          PublicAccessType AccessType = PublicAccessType::None;
          Storage::Metadata Metadata;
          Azure::Nullable<std::string> DefaultEncryptionScope;
          Azure::Nullable<bool> PreventEncryptionScopeOverride;
        }; // struct CreateBlobContainerOptions

        static Azure::Response<CreateBlobContainerResult> Create(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const CreateBlobContainerOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          for (const auto& pair : options.Metadata)
          {
            request.SetHeader("x-ms-meta-" + pair.first, pair.second);
          }
          if (!options.AccessType.ToString().empty())
          {
            request.SetHeader("x-ms-blob-public-access", options.AccessType.ToString());
          }
          if (options.DefaultEncryptionScope.HasValue())
          {
            request.SetHeader(
                "x-ms-default-encryption-scope", options.DefaultEncryptionScope.GetValue());
          }
          if (options.PreventEncryptionScopeOverride.HasValue())
          {
            request.SetHeader(
                "x-ms-deny-encryption-scope-override",
                options.PreventEncryptionScopeOverride.GetValue() ? "true" : "false");
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          CreateBlobContainerResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 201))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          return Azure::Response<CreateBlobContainerResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct DeleteBlobContainerOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
        }; // struct DeleteBlobContainerOptions

        static Azure::Response<DeleteBlobContainerResult> Delete(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const DeleteBlobContainerOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Delete, url);
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          DeleteBlobContainerResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 202))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          return Azure::Response<DeleteBlobContainerResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct UndeleteBlobContainerOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string DeletedBlobContainerName;
          std::string DeletedBlobContainerVersion;
        }; // struct UndeleteBlobContainerOptions

        static Azure::Response<UndeleteBlobContainerResult> Undelete(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const UndeleteBlobContainerOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.GetUrl().AppendQueryParameter("comp", "undelete");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.SetHeader("x-ms-deleted-container-name", options.DeletedBlobContainerName);
          request.SetHeader("x-ms-deleted-container-version", options.DeletedBlobContainerVersion);
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          UndeleteBlobContainerResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 201))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          return Azure::Response<UndeleteBlobContainerResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct GetBlobContainerPropertiesOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::string> LeaseId;
        }; // struct GetBlobContainerPropertiesOptions

        static Azure::Response<BlobContainerProperties> GetProperties(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const GetBlobContainerPropertiesOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, url);
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          BlobContainerProperties response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          for (auto i = httpResponse.GetHeaders().lower_bound("x-ms-meta-");
               i != httpResponse.GetHeaders().end() && i->first.substr(0, 10) == "x-ms-meta-";
               ++i)
          {
            response.Metadata.emplace(i->first.substr(10), i->second);
          }
          auto x_ms_blob_public_access__iterator
              = httpResponse.GetHeaders().find("x-ms-blob-public-access");
          if (x_ms_blob_public_access__iterator != httpResponse.GetHeaders().end())
          {
            response.AccessType = PublicAccessType(x_ms_blob_public_access__iterator->second);
          }
          response.HasImmutabilityPolicy
              = httpResponse.GetHeaders().at("x-ms-has-immutability-policy") == "true";
          response.HasLegalHold = httpResponse.GetHeaders().at("x-ms-has-legal-hold") == "true";
          response.LeaseStatus = BlobLeaseStatus(httpResponse.GetHeaders().at("x-ms-lease-status"));
          response.LeaseState = BlobLeaseState(httpResponse.GetHeaders().at("x-ms-lease-state"));
          auto x_ms_lease_duration__iterator
              = httpResponse.GetHeaders().find("x-ms-lease-duration");
          if (x_ms_lease_duration__iterator != httpResponse.GetHeaders().end())
          {
            response.LeaseDuration = BlobLeaseDurationType(x_ms_lease_duration__iterator->second);
          }
          response.DefaultEncryptionScope
              = httpResponse.GetHeaders().at("x-ms-default-encryption-scope");
          response.PreventEncryptionScopeOverride
              = httpResponse.GetHeaders().at("x-ms-deny-encryption-scope-override") == "true";
          return Azure::Response<BlobContainerProperties>(
              std::move(response), std::move(pHttpResponse));
        }

        struct SetBlobContainerMetadataOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Storage::Metadata Metadata;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
        }; // struct SetBlobContainerMetadataOptions

        static Azure::Response<SetBlobContainerMetadataResult> SetMetadata(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const SetBlobContainerMetadataOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.GetUrl().AppendQueryParameter("comp", "metadata");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          for (const auto& pair : options.Metadata)
          {
            request.SetHeader("x-ms-meta-" + pair.first, pair.second);
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          SetBlobContainerMetadataResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          return Azure::Response<SetBlobContainerMetadataResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct ListBlobsSinglePageOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::string> Prefix;
          Azure::Nullable<std::string> ContinuationToken;
          Azure::Nullable<int32_t> MaxResults;
          ListBlobsIncludeFlags Include = ListBlobsIncludeFlags::None;
        }; // struct ListBlobsSinglePageOptions

        static Azure::Response<ListBlobsSinglePageResult> ListBlobsSinglePage(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const ListBlobsSinglePageOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.SetHeader("x-ms-version", "2020-02-10");
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
                "prefix", Storage::_detail::UrlEncodeQueryParameter(options.Prefix.GetValue()));
          }
          if (options.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "marker",
                Storage::_detail::UrlEncodeQueryParameter(options.ContinuationToken.GetValue()));
          }
          if (options.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "maxresults", std::to_string(options.MaxResults.GetValue()));
          }
          std::string list_blobs_include_flags = ListBlobsIncludeFlagsToString(options.Include);
          if (!list_blobs_include_flags.empty())
          {
            request.GetUrl().AppendQueryParameter(
                "include", Storage::_detail::UrlEncodeQueryParameter(list_blobs_include_flags));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          ListBlobsSinglePageResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            Storage::_detail::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = ListBlobsSinglePageResultFromXml(reader);
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          return Azure::Response<ListBlobsSinglePageResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct ListBlobsByHierarchySinglePageOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::string> Prefix;
          Azure::Nullable<std::string> Delimiter;
          Azure::Nullable<std::string> ContinuationToken;
          Azure::Nullable<int32_t> MaxResults;
          ListBlobsIncludeFlags Include = ListBlobsIncludeFlags::None;
        }; // struct ListBlobsByHierarchySinglePageOptions

        static Azure::Response<ListBlobsByHierarchySinglePageResult> ListBlobsByHierarchySinglePage(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const ListBlobsByHierarchySinglePageOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.SetHeader("x-ms-version", "2020-02-10");
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
                "prefix", Storage::_detail::UrlEncodeQueryParameter(options.Prefix.GetValue()));
          }
          if (options.Delimiter.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "delimiter",
                Storage::_detail::UrlEncodeQueryParameter(options.Delimiter.GetValue()));
          }
          if (options.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "marker",
                Storage::_detail::UrlEncodeQueryParameter(options.ContinuationToken.GetValue()));
          }
          if (options.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "maxresults", std::to_string(options.MaxResults.GetValue()));
          }
          std::string list_blobs_include_flags = ListBlobsIncludeFlagsToString(options.Include);
          if (!list_blobs_include_flags.empty())
          {
            request.GetUrl().AppendQueryParameter(
                "include", Storage::_detail::UrlEncodeQueryParameter(list_blobs_include_flags));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          ListBlobsByHierarchySinglePageResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            Storage::_detail::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = ListBlobsByHierarchySinglePageResultFromXml(reader);
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          return Azure::Response<ListBlobsByHierarchySinglePageResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct GetBlobContainerAccessPolicyOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::string> LeaseId;
        }; // struct GetBlobContainerAccessPolicyOptions

        static Azure::Response<BlobContainerAccessPolicy> GetAccessPolicy(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const GetBlobContainerAccessPolicyOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.GetUrl().AppendQueryParameter("comp", "acl");
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          BlobContainerAccessPolicy response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            Storage::_detail::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = BlobContainerAccessPolicyFromXml(reader);
          }
          auto x_ms_blob_public_access__iterator
              = httpResponse.GetHeaders().find("x-ms-blob-public-access");
          if (x_ms_blob_public_access__iterator != httpResponse.GetHeaders().end())
          {
            response.AccessType = PublicAccessType(x_ms_blob_public_access__iterator->second);
          }
          return Azure::Response<BlobContainerAccessPolicy>(
              std::move(response), std::move(pHttpResponse));
        }

        struct SetBlobContainerAccessPolicyOptions
        {
          Azure::Nullable<int32_t> Timeout;
          PublicAccessType AccessType = PublicAccessType::None;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          std::vector<BlobSignedIdentifier> SignedIdentifiers;
        }; // struct SetBlobContainerAccessPolicyOptions

        static Azure::Response<SetBlobContainerAccessPolicyResult> SetAccessPolicy(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const SetBlobContainerAccessPolicyOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          std::string xml_body;
          {
            Storage::_detail::XmlWriter writer;
            SetBlobContainerAccessPolicyOptionsToXml(writer, options);
            xml_body = writer.GetDocument();
            writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::End});
          }
          Azure::Core::IO::MemoryBodyStream xml_body_stream(
              reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
          auto request = Azure::Core::Http::Request(
              Azure::Core::Http::HttpMethod::Put, url, &xml_body_stream);
          request.SetHeader("Content-Length", std::to_string(xml_body_stream.Length()));
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.GetUrl().AppendQueryParameter("comp", "acl");
          if (!options.AccessType.ToString().empty())
          {
            request.SetHeader("x-ms-blob-public-access", options.AccessType.ToString());
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          SetBlobContainerAccessPolicyResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          return Azure::Response<SetBlobContainerAccessPolicyResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct AcquireBlobContainerLeaseOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::chrono::seconds LeaseDuration;
          Azure::Nullable<std::string> ProposedLeaseId;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
        }; // struct AcquireBlobContainerLeaseOptions

        static Azure::Response<Models::_detail::AcquireBlobContainerLeaseResult> AcquireLease(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const AcquireBlobContainerLeaseOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.GetUrl().AppendQueryParameter("comp", "lease");
          request.SetHeader("x-ms-lease-action", "acquire");
          request.SetHeader("x-ms-lease-duration", std::to_string(options.LeaseDuration.count()));
          if (options.ProposedLeaseId.HasValue())
          {
            request.SetHeader("x-ms-proposed-lease-id", options.ProposedLeaseId.GetValue());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::AcquireBlobContainerLeaseResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 201))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
          return Azure::Response<Models::_detail::AcquireBlobContainerLeaseResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct RenewBlobContainerLeaseOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string LeaseId;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
        }; // struct RenewBlobContainerLeaseOptions

        static Azure::Response<Models::_detail::RenewBlobContainerLeaseResult> RenewLease(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const RenewBlobContainerLeaseOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.GetUrl().AppendQueryParameter("comp", "lease");
          request.SetHeader("x-ms-lease-action", "renew");
          request.SetHeader("x-ms-lease-id", options.LeaseId);
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::RenewBlobContainerLeaseResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
          return Azure::Response<Models::_detail::RenewBlobContainerLeaseResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct ChangeBlobContainerLeaseOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string LeaseId;
          std::string ProposedLeaseId;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
        }; // struct ChangeBlobContainerLeaseOptions

        static Azure::Response<Models::_detail::ChangeBlobContainerLeaseResult> ChangeLease(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const ChangeBlobContainerLeaseOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.GetUrl().AppendQueryParameter("comp", "lease");
          request.SetHeader("x-ms-lease-action", "change");
          request.SetHeader("x-ms-lease-id", options.LeaseId);
          request.SetHeader("x-ms-proposed-lease-id", options.ProposedLeaseId);
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::ChangeBlobContainerLeaseResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
          return Azure::Response<Models::_detail::ChangeBlobContainerLeaseResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct ReleaseBlobContainerLeaseOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string LeaseId;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
        }; // struct ReleaseBlobContainerLeaseOptions

        static Azure::Response<Models::_detail::ReleaseBlobContainerLeaseResult> ReleaseLease(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const ReleaseBlobContainerLeaseOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.GetUrl().AppendQueryParameter("comp", "lease");
          request.SetHeader("x-ms-lease-action", "release");
          request.SetHeader("x-ms-lease-id", options.LeaseId);
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::ReleaseBlobContainerLeaseResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          return Azure::Response<Models::_detail::ReleaseBlobContainerLeaseResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct BreakBlobContainerLeaseOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::chrono::seconds> BreakPeriod;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
        }; // struct BreakBlobContainerLeaseOptions

        static Azure::Response<Models::_detail::BreakBlobContainerLeaseResult> BreakLease(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const BreakBlobContainerLeaseOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.GetUrl().AppendQueryParameter("comp", "lease");
          request.SetHeader("x-ms-lease-action", "break");
          if (options.BreakPeriod.HasValue())
          {
            request.SetHeader(
                "x-ms-lease-break-period", std::to_string(options.BreakPeriod.GetValue().count()));
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::BreakBlobContainerLeaseResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 202))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.LeaseTime = std::stoi(httpResponse.GetHeaders().at("x-ms-lease-time"));
          return Azure::Response<Models::_detail::BreakBlobContainerLeaseResult>(
              std::move(response), std::move(pHttpResponse));
        }

      private:
        static BlobContainerAccessPolicy BlobContainerAccessPolicyFromXml(
            Storage::_detail::XmlReader& reader)
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
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::Text)
            {
            }
          }
          return ret;
        }

        static ListBlobsByHierarchySinglePageResult ListBlobsByHierarchySinglePageResultFromXml(
            Storage::_detail::XmlReader& reader)
        {
          ListBlobsByHierarchySinglePageResult ret;
          enum class XmlTagName
          {
            k_EnumerationResults,
            k_Prefix,
            k_Delimiter,
            k_NextMarker,
            k_Blobs,
            k_Blob,
            k_BlobPrefix,
            k_Name,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
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
              else if (std::strcmp(node.Name, "Name") == 0)
              {
                path.emplace_back(XmlTagName::k_Name);
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
            else if (node.Type == Storage::_detail::XmlNodeType::Text)
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
                  && path[1] == XmlTagName::k_NextMarker)
              {
                ret.ContinuationToken = node.Value;
              }
              else if (
                  path.size() == 4 && path[0] == XmlTagName::k_EnumerationResults
                  && path[1] == XmlTagName::k_Blobs && path[2] == XmlTagName::k_BlobPrefix
                  && path[3] == XmlTagName::k_Name)
              {
                ret.BlobPrefixes.emplace_back(node.Value);
              }
            }
            else if (node.Type == Storage::_detail::XmlNodeType::Attribute)
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
                ret.BlobContainerName = node.Value;
              }
            }
          }
          return ret;
        }

        static ListBlobsSinglePageResult ListBlobsSinglePageResultFromXml(
            Storage::_detail::XmlReader& reader)
        {
          ListBlobsSinglePageResult ret;
          enum class XmlTagName
          {
            k_EnumerationResults,
            k_Prefix,
            k_NextMarker,
            k_Blobs,
            k_Blob,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
            {
              if (std::strcmp(node.Name, "EnumerationResults") == 0)
              {
                path.emplace_back(XmlTagName::k_EnumerationResults);
              }
              else if (std::strcmp(node.Name, "Prefix") == 0)
              {
                path.emplace_back(XmlTagName::k_Prefix);
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
            else if (node.Type == Storage::_detail::XmlNodeType::Text)
            {
              if (path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                  && path[1] == XmlTagName::k_Prefix)
              {
                ret.Prefix = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                  && path[1] == XmlTagName::k_NextMarker)
              {
                ret.ContinuationToken = node.Value;
              }
            }
            else if (node.Type == Storage::_detail::XmlNodeType::Attribute)
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
                ret.BlobContainerName = node.Value;
              }
            }
          }
          return ret;
        }

        static BlobItem BlobItemFromXml(Storage::_detail::XmlReader& reader)
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
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
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
                ret.Details.Metadata = MetadataFromXml(reader);
                path.pop_back();
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_OrMetadata)
              {
                ret.Details.ObjectReplicationSourceProperties
                    = ObjectReplicationSourcePropertiesFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == Storage::_detail::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_Name)
              {
                ret.Name = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_Deleted)
              {
                ret.IsDeleted = std::strcmp(node.Value, "true") == 0;
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
                ret.Details.HttpHeaders.ContentType = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_ContentEncoding)
              {
                ret.Details.HttpHeaders.ContentEncoding = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_ContentLanguage)
              {
                ret.Details.HttpHeaders.ContentLanguage = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_ContentMD5)
              {
                ret.Details.HttpHeaders.ContentHash.Value
                    = Azure::Core::Convert::Base64Decode(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_CacheControl)
              {
                ret.Details.HttpHeaders.CacheControl = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_ContentDisposition)
              {
                ret.Details.HttpHeaders.ContentDisposition = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_CreationTime)
              {
                ret.Details.CreatedOn
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_ExpiryTime)
              {
                ret.Details.ExpiresOn
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_LastAccessTime)
              {
                ret.Details.LastAccessedOn
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_LastModified)
              {
                ret.Details.LastModified
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_Etag)
              {
                ret.Details.ETag = Azure::ETag(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_ContentLength)
              {
                ret.BlobSize = std::stoll(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_BlobType)
              {
                ret.BlobType = BlobType(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_AccessTier)
              {
                ret.Details.Tier = AccessTier(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_AccessTierInferred)
              {
                ret.Details.IsAccessTierInferred = std::strcmp(node.Value, "true") == 0;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_LeaseStatus)
              {
                ret.Details.LeaseStatus = BlobLeaseStatus(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_LeaseState)
              {
                ret.Details.LeaseState = BlobLeaseState(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_LeaseDuration)
              {
                ret.Details.LeaseDuration = BlobLeaseDurationType(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_ServerEncrypted)
              {
                ret.Details.IsServerEncrypted = std::strcmp(node.Value, "true") == 0;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_EncryptionKeySHA256)
              {
                ret.Details.EncryptionKeySha256 = Azure::Core::Convert::Base64Decode(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_Sealed)
              {
                ret.Details.IsSealed = std::strcmp(node.Value, "true") == 0;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_xmsblobsequencenumber)
              {
                ret.Details.SequenceNumber = std::stoll(node.Value);
              }
            }
          }
          return ret;
        }

        static BlobSignedIdentifier BlobSignedIdentifierFromXml(Storage::_detail::XmlReader& reader)
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
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_Id)
              {
                ret.Id = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_AccessPolicy
                  && path[1] == XmlTagName::k_Start)
              {
                ret.StartsOn
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc3339);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_AccessPolicy
                  && path[1] == XmlTagName::k_Expiry)
              {
                ret.ExpiresOn
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc3339);
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

        static std::vector<ObjectReplicationPolicy> ObjectReplicationSourcePropertiesFromXml(
            Storage::_detail::XmlReader& reader)
        {
          int depth = 0;
          std::map<std::string, std::vector<ObjectReplicationRule>> orPropertiesMap;
          std::string policyId;
          std::string ruleId;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
            {
              if (depth-- == 0)
              {
                break;
              }
            }
            if (depth == 1 && node.Type == Storage::_detail::XmlNodeType::Text)
            {
              ObjectReplicationRule rule;
              rule.RuleId = std::move(ruleId);
              rule.ReplicationStatus = ObjectReplicationStatus(node.Value);
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

        static Metadata MetadataFromXml(Storage::_detail::XmlReader& reader)
        {
          Metadata ret;
          int depth = 0;
          std::string key;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
            {
              if (depth++ == 0)
              {
                key = node.Name;
              }
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
            {
              if (depth-- == 0)
              {
                break;
              }
            }
            else if (depth == 1 && node.Type == Storage::_detail::XmlNodeType::Text)
            {
              ret.emplace(std::move(key), std::string(node.Value));
            }
          }
          return ret;
        }

        static void SetBlobContainerAccessPolicyOptionsToXml(
            Storage::_detail::XmlWriter& writer,
            const SetBlobContainerAccessPolicyOptions& options)
        {
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::StartTag, "SignedIdentifiers"});
          for (const auto& i : options.SignedIdentifiers)
          {
            BlobSignedIdentifierToXml(writer, i);
          }
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
        }

        static void BlobSignedIdentifierToXml(
            Storage::_detail::XmlWriter& writer,
            const BlobSignedIdentifier& options)
        {
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::StartTag, "SignedIdentifier"});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "Id"});
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::Text, nullptr, options.Id.data()});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          writer.Write(
              Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "AccessPolicy"});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "Start"});
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::Text,
              nullptr,
              options.StartsOn
                  .ToString(
                      Azure::DateTime::DateFormat::Rfc3339,
                      Azure::DateTime::TimeFractionFormat::AllDigits)
                  .data()});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          writer.Write(
              Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "Expiry"});
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::Text,
              nullptr,
              options.ExpiresOn
                  .ToString(
                      Azure::DateTime::DateFormat::Rfc3339,
                      Azure::DateTime::TimeFractionFormat::AllDigits)
                  .data()});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          writer.Write(
              Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "Permission"});
          writer.Write(Storage::_detail::XmlNode{
              Storage::_detail::XmlNodeType::Text, nullptr, options.Permissions.data()});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
        }

      }; // class BlobContainer

      class Blob {
      public:
        struct DownloadBlobOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<Azure::Core::Http::HttpRange> Range;
          Azure::Nullable<HashAlgorithm> RangeHashAlgorithm;
          Azure::Nullable<std::string> EncryptionKey;
          Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
          Azure::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct DownloadBlobOptions

        static Azure::Response<DownloadBlobResult> Download(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const DownloadBlobOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url, true);
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          if (options.Range.HasValue())
          {
            std::string headerValue
                = "bytes=" + std::to_string(options.Range.GetValue().Offset) + "-";
            if (options.Range.GetValue().Length.HasValue())
            {
              headerValue += std::to_string(
                  options.Range.GetValue().Offset + options.Range.GetValue().Length.GetValue() - 1);
            }
            request.SetHeader("x-ms-range", std::move(headerValue));
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.GetValue()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue().ToString());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          if (options.RangeHashAlgorithm.HasValue())
          {
            if (options.RangeHashAlgorithm.GetValue() == HashAlgorithm::Md5)
            {
              request.SetHeader("x-ms-range-get-content-md5", "true");
            }
            else if (options.RangeHashAlgorithm.GetValue() == HashAlgorithm::Crc64)
            {
              request.SetHeader("x-ms-range-get-content-crc64", "true");
            }
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          DownloadBlobResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200 || http_status_code == 206))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.BodyStream = httpResponse.GetBodyStream();
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          {
            const auto& headers = httpResponse.GetHeaders();
            auto content_md5_iterator = headers.find("content-md5");
            if (content_md5_iterator != headers.end())
            {
              ContentHash hash;
              hash.Algorithm = HashAlgorithm::Md5;
              hash.Value = Azure::Core::Convert::Base64Decode(content_md5_iterator->second);
              response.TransactionalContentHash = std::move(hash);
            }
            auto x_ms_content_crc64_iterator = headers.find("x-ms-content-crc64");
            if (x_ms_content_crc64_iterator != headers.end())
            {
              ContentHash hash;
              hash.Algorithm = HashAlgorithm::Crc64;
              hash.Value = Azure::Core::Convert::Base64Decode(x_ms_content_crc64_iterator->second);
              response.TransactionalContentHash = std::move(hash);
            }
          }
          response.BlobType = BlobType(httpResponse.GetHeaders().at("x-ms-blob-type"));
          auto content_range_iterator = httpResponse.GetHeaders().find("content-range");
          if (content_range_iterator != httpResponse.GetHeaders().end())
          {
            const std::string& content_range = content_range_iterator->second;
            auto bytes_pos = content_range.find("bytes ");
            auto dash_pos = content_range.find("-", bytes_pos + 6);
            auto slash_pos = content_range.find("/", dash_pos + 1);
            int64_t range_start_offset = std::stoll(std::string(
                content_range.begin() + bytes_pos + 6, content_range.begin() + dash_pos));
            int64_t range_end_offset = std::stoll(std::string(
                content_range.begin() + dash_pos + 1, content_range.begin() + slash_pos));
            response.ContentRange = Azure::Core::Http::HttpRange{
                range_start_offset, range_end_offset - range_start_offset + 1};
          }
          else
          {
            response.ContentRange = Azure::Core::Http::HttpRange{
                0, std::stoll(httpResponse.GetHeaders().at("content-length"))};
          }
          if (content_range_iterator != httpResponse.GetHeaders().end())
          {
            const std::string& content_range = content_range_iterator->second;
            auto slash_pos = content_range.find("/");
            response.BlobSize = std::stoll(content_range.substr(slash_pos + 1));
          }
          else
          {
            response.BlobSize = std::stoll(httpResponse.GetHeaders().at("content-length"));
          }
          response.Details.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.Details.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          auto content_type__iterator = httpResponse.GetHeaders().find("content-type");
          if (content_type__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.HttpHeaders.ContentType = content_type__iterator->second;
          }
          auto content_encoding__iterator = httpResponse.GetHeaders().find("content-encoding");
          if (content_encoding__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.HttpHeaders.ContentEncoding = content_encoding__iterator->second;
          }
          auto content_language__iterator = httpResponse.GetHeaders().find("content-language");
          if (content_language__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.HttpHeaders.ContentLanguage = content_language__iterator->second;
          }
          auto cache_control__iterator = httpResponse.GetHeaders().find("cache-control");
          if (cache_control__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.HttpHeaders.CacheControl = cache_control__iterator->second;
          }
          auto content_md5__iterator = httpResponse.GetHeaders().find("content-md5");
          if (content_md5__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.HttpHeaders.ContentHash.Value
                = Azure::Core::Convert::Base64Decode(content_md5__iterator->second);
          }
          auto x_ms_blob_content_md5__iterator
              = httpResponse.GetHeaders().find("x-ms-blob-content-md5");
          if (x_ms_blob_content_md5__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.HttpHeaders.ContentHash.Value
                = Azure::Core::Convert::Base64Decode(x_ms_blob_content_md5__iterator->second);
          }
          auto content_disposition__iterator
              = httpResponse.GetHeaders().find("content-disposition");
          if (content_disposition__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.HttpHeaders.ContentDisposition = content_disposition__iterator->second;
          }
          for (auto i = httpResponse.GetHeaders().lower_bound("x-ms-meta-");
               i != httpResponse.GetHeaders().end() && i->first.substr(0, 10) == "x-ms-meta-";
               ++i)
          {
            response.Details.Metadata.emplace(i->first.substr(10), i->second);
          }
          response.Details.IsServerEncrypted
              = httpResponse.GetHeaders().at("x-ms-server-encrypted") == "true";
          auto x_ms_encryption_key_sha256__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
          if (x_ms_encryption_key_sha256__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.EncryptionKeySha256
                = Azure::Core::Convert::Base64Decode(x_ms_encryption_key_sha256__iterator->second);
          }
          auto x_ms_encryption_scope__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-scope");
          if (x_ms_encryption_scope__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.EncryptionScope = x_ms_encryption_scope__iterator->second;
          }
          auto x_ms_lease_status__iterator = httpResponse.GetHeaders().find("x-ms-lease-status");
          if (x_ms_lease_status__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.LeaseStatus = BlobLeaseStatus(x_ms_lease_status__iterator->second);
          }
          auto x_ms_lease_state__iterator = httpResponse.GetHeaders().find("x-ms-lease-state");
          if (x_ms_lease_state__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.LeaseState = BlobLeaseState(x_ms_lease_state__iterator->second);
          }
          auto x_ms_lease_duration__iterator
              = httpResponse.GetHeaders().find("x-ms-lease-duration");
          if (x_ms_lease_duration__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.LeaseDuration
                = BlobLeaseDurationType(x_ms_lease_duration__iterator->second);
          }
          response.Details.CreatedOn = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("x-ms-creation-time"),
              Azure::DateTime::DateFormat::Rfc1123);
          auto x_ms_expiry_time__iterator = httpResponse.GetHeaders().find("x-ms-expiry-time");
          if (x_ms_expiry_time__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.ExpiresOn = Azure::DateTime::Parse(
                x_ms_expiry_time__iterator->second, Azure::DateTime::DateFormat::Rfc1123);
          }
          auto x_ms_last_access_time__iterator
              = httpResponse.GetHeaders().find("x-ms-last-access-time");
          if (x_ms_last_access_time__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.LastAccessedOn = Azure::DateTime::Parse(
                x_ms_last_access_time__iterator->second, Azure::DateTime::DateFormat::Rfc1123);
          }
          auto x_ms_blob_sequence_number__iterator
              = httpResponse.GetHeaders().find("x-ms-blob-sequence-number");
          if (x_ms_blob_sequence_number__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.SequenceNumber
                = std::stoll(x_ms_blob_sequence_number__iterator->second);
          }
          auto x_ms_blob_committed_block_count__iterator
              = httpResponse.GetHeaders().find("x-ms-blob-committed-block-count");
          if (x_ms_blob_committed_block_count__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.CommittedBlockCount
                = std::stoll(x_ms_blob_committed_block_count__iterator->second);
          }
          auto x_ms_blob_sealed__iterator = httpResponse.GetHeaders().find("x-ms-blob-sealed");
          if (x_ms_blob_sealed__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.IsSealed = x_ms_blob_sealed__iterator->second == "true";
          }
          auto x_ms_or_policy_id__iterator = httpResponse.GetHeaders().find("x-ms-or-policy-id");
          if (x_ms_or_policy_id__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.ObjectReplicationDestinationPolicyId
                = x_ms_or_policy_id__iterator->second;
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
              std::string policyId
                  = std::string(header.begin() + 8, header.begin() + underscorePos);
              std::string ruleId = header.substr(underscorePos + 1);

              ObjectReplicationRule rule;
              rule.RuleId = std::move(ruleId);
              rule.ReplicationStatus = ObjectReplicationStatus(i->second);
              orPropertiesMap[policyId].emplace_back(std::move(rule));
            }
            for (auto& property : orPropertiesMap)
            {
              ObjectReplicationPolicy policy;
              policy.PolicyId = property.first;
              policy.Rules = std::move(property.second);
              response.Details.ObjectReplicationSourceProperties.emplace_back(std::move(policy));
            }
          }
          auto x_ms_tag_count__iterator = httpResponse.GetHeaders().find("x-ms-tag-count");
          if (x_ms_tag_count__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.TagCount = std::stoi(x_ms_tag_count__iterator->second);
          }
          auto x_ms_copy_id__iterator = httpResponse.GetHeaders().find("x-ms-copy-id");
          if (x_ms_copy_id__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.CopyId = x_ms_copy_id__iterator->second;
          }
          auto x_ms_copy_source__iterator = httpResponse.GetHeaders().find("x-ms-copy-source");
          if (x_ms_copy_source__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.CopySource = x_ms_copy_source__iterator->second;
          }
          auto x_ms_copy_status__iterator = httpResponse.GetHeaders().find("x-ms-copy-status");
          if (x_ms_copy_status__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.CopyStatus = CopyStatus(x_ms_copy_status__iterator->second);
          }
          auto x_ms_copy_status_description__iterator
              = httpResponse.GetHeaders().find("x-ms-copy-status-description");
          if (x_ms_copy_status_description__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.CopyStatusDescription = x_ms_copy_status_description__iterator->second;
          }
          auto x_ms_copy_progress__iterator = httpResponse.GetHeaders().find("x-ms-copy-progress");
          if (x_ms_copy_progress__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.CopyProgress = x_ms_copy_progress__iterator->second;
          }
          auto x_ms_copy_completion_time__iterator
              = httpResponse.GetHeaders().find("x-ms-copy-completion-time");
          if (x_ms_copy_completion_time__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.CopyCompletedOn = Azure::DateTime::Parse(
                x_ms_copy_completion_time__iterator->second, Azure::DateTime::DateFormat::Rfc1123);
          }
          auto x_ms_version_id__iterator = httpResponse.GetHeaders().find("x-ms-version-id");
          if (x_ms_version_id__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.VersionId = x_ms_version_id__iterator->second;
          }
          auto x_ms_is_current_version__iterator
              = httpResponse.GetHeaders().find("x-ms-is-current-version");
          if (x_ms_is_current_version__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.IsCurrentVersion = x_ms_is_current_version__iterator->second == "true";
          }
          return Azure::Response<DownloadBlobResult>(std::move(response), std::move(pHttpResponse));
        }

        struct DeleteBlobOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<DeleteSnapshotsOption> DeleteSnapshots;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct DeleteBlobOptions

        static Azure::Core::Http::Request DeleteCreateMessage(
            const Azure::Core::Url& url,
            const DeleteBlobOptions& options)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Delete, url);
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          if (options.DeleteSnapshots.HasValue())
          {
            request.SetHeader(
                "x-ms-delete-snapshots", options.DeleteSnapshots.GetValue().ToString());
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          return request;
        }

        static Azure::Response<DeleteBlobResult> DeleteCreateResponse(
            std::unique_ptr<Azure::Core::Http::RawResponse> pHttpResponse,
            const Azure::Core::Context& context)
        {
          (void)context;
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          DeleteBlobResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 202))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          return Azure::Response<DeleteBlobResult>(std::move(response), std::move(pHttpResponse));
        }

        static Azure::Response<DeleteBlobResult> Delete(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const DeleteBlobOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = DeleteCreateMessage(url, options);
          auto pHttpResponse = pipeline.Send(request, context);
          return DeleteCreateResponse(std::move(pHttpResponse), context);
        }

        struct SetBlobExpiryOptions
        {
          Azure::Nullable<int32_t> Timeout;
          ScheduleBlobExpiryOriginType ExpiryOrigin;
          Azure::Nullable<std::string> ExpiryTime;
        }; // struct SetBlobExpiryOptions

        static Azure::Response<SetBlobExpiryResult> ScheduleDeletion(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const SetBlobExpiryOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.GetUrl().AppendQueryParameter("comp", "expiry");
          request.SetHeader("x-ms-expiry-option", options.ExpiryOrigin.ToString());
          if (options.ExpiryTime.HasValue())
          {
            request.SetHeader("x-ms-expiry-time", options.ExpiryTime.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          SetBlobExpiryResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          return Azure::Response<SetBlobExpiryResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct UndeleteBlobOptions
        {
          Azure::Nullable<int32_t> Timeout;
        }; // struct UndeleteBlobOptions

        static Azure::Response<UndeleteBlobResult> Undelete(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const UndeleteBlobOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.GetUrl().AppendQueryParameter("comp", "undelete");
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          UndeleteBlobResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          return Azure::Response<UndeleteBlobResult>(std::move(response), std::move(pHttpResponse));
        }

        struct GetBlobPropertiesOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::string> EncryptionKey;
          Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
          Azure::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct GetBlobPropertiesOptions

        static Azure::Response<BlobProperties> GetProperties(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const GetBlobPropertiesOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, url);
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.GetValue()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue().ToString());
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          BlobProperties response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.CreatedOn = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("x-ms-creation-time"),
              Azure::DateTime::DateFormat::Rfc1123);
          auto x_ms_expiry_time__iterator = httpResponse.GetHeaders().find("x-ms-expiry-time");
          if (x_ms_expiry_time__iterator != httpResponse.GetHeaders().end())
          {
            response.ExpiresOn = Azure::DateTime::Parse(
                x_ms_expiry_time__iterator->second, Azure::DateTime::DateFormat::Rfc1123);
          }
          auto x_ms_last_access_time__iterator
              = httpResponse.GetHeaders().find("x-ms-last-access-time");
          if (x_ms_last_access_time__iterator != httpResponse.GetHeaders().end())
          {
            response.LastAccessedOn = Azure::DateTime::Parse(
                x_ms_last_access_time__iterator->second, Azure::DateTime::DateFormat::Rfc1123);
          }
          for (auto i = httpResponse.GetHeaders().lower_bound("x-ms-meta-");
               i != httpResponse.GetHeaders().end() && i->first.substr(0, 10) == "x-ms-meta-";
               ++i)
          {
            response.Metadata.emplace(i->first.substr(10), i->second);
          }
          response.BlobType = BlobType(httpResponse.GetHeaders().at("x-ms-blob-type"));
          auto x_ms_lease_status__iterator = httpResponse.GetHeaders().find("x-ms-lease-status");
          if (x_ms_lease_status__iterator != httpResponse.GetHeaders().end())
          {
            response.LeaseStatus = BlobLeaseStatus(x_ms_lease_status__iterator->second);
          }
          auto x_ms_lease_state__iterator = httpResponse.GetHeaders().find("x-ms-lease-state");
          if (x_ms_lease_state__iterator != httpResponse.GetHeaders().end())
          {
            response.LeaseState = BlobLeaseState(x_ms_lease_state__iterator->second);
          }
          auto x_ms_lease_duration__iterator
              = httpResponse.GetHeaders().find("x-ms-lease-duration");
          if (x_ms_lease_duration__iterator != httpResponse.GetHeaders().end())
          {
            response.LeaseDuration = BlobLeaseDurationType(x_ms_lease_duration__iterator->second);
          }
          response.BlobSize = std::stoll(httpResponse.GetHeaders().at("content-length"));
          auto content_type__iterator = httpResponse.GetHeaders().find("content-type");
          if (content_type__iterator != httpResponse.GetHeaders().end())
          {
            response.HttpHeaders.ContentType = content_type__iterator->second;
          }
          auto content_encoding__iterator = httpResponse.GetHeaders().find("content-encoding");
          if (content_encoding__iterator != httpResponse.GetHeaders().end())
          {
            response.HttpHeaders.ContentEncoding = content_encoding__iterator->second;
          }
          auto content_language__iterator = httpResponse.GetHeaders().find("content-language");
          if (content_language__iterator != httpResponse.GetHeaders().end())
          {
            response.HttpHeaders.ContentLanguage = content_language__iterator->second;
          }
          auto cache_control__iterator = httpResponse.GetHeaders().find("cache-control");
          if (cache_control__iterator != httpResponse.GetHeaders().end())
          {
            response.HttpHeaders.CacheControl = cache_control__iterator->second;
          }
          auto content_md5__iterator = httpResponse.GetHeaders().find("content-md5");
          if (content_md5__iterator != httpResponse.GetHeaders().end())
          {
            response.HttpHeaders.ContentHash.Value
                = Azure::Core::Convert::Base64Decode(content_md5__iterator->second);
          }
          auto x_ms_blob_content_md5__iterator
              = httpResponse.GetHeaders().find("x-ms-blob-content-md5");
          if (x_ms_blob_content_md5__iterator != httpResponse.GetHeaders().end())
          {
            response.HttpHeaders.ContentHash.Value
                = Azure::Core::Convert::Base64Decode(x_ms_blob_content_md5__iterator->second);
          }
          auto content_disposition__iterator
              = httpResponse.GetHeaders().find("content-disposition");
          if (content_disposition__iterator != httpResponse.GetHeaders().end())
          {
            response.HttpHeaders.ContentDisposition = content_disposition__iterator->second;
          }
          auto x_ms_blob_sequence_number__iterator
              = httpResponse.GetHeaders().find("x-ms-blob-sequence-number");
          if (x_ms_blob_sequence_number__iterator != httpResponse.GetHeaders().end())
          {
            response.SequenceNumber = std::stoll(x_ms_blob_sequence_number__iterator->second);
          }
          auto x_ms_blob_committed_block_count__iterator
              = httpResponse.GetHeaders().find("x-ms-blob-committed-block-count");
          if (x_ms_blob_committed_block_count__iterator != httpResponse.GetHeaders().end())
          {
            response.CommittedBlockCount
                = std::stoi(x_ms_blob_committed_block_count__iterator->second);
          }
          auto x_ms_blob_sealed__iterator = httpResponse.GetHeaders().find("x-ms-blob-sealed");
          if (x_ms_blob_sealed__iterator != httpResponse.GetHeaders().end())
          {
            response.IsSealed = x_ms_blob_sealed__iterator->second == "true";
          }
          response.IsServerEncrypted
              = httpResponse.GetHeaders().at("x-ms-server-encrypted") == "true";
          auto x_ms_encryption_key_sha256__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
          if (x_ms_encryption_key_sha256__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionKeySha256
                = Azure::Core::Convert::Base64Decode(x_ms_encryption_key_sha256__iterator->second);
          }
          auto x_ms_encryption_scope__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-scope");
          if (x_ms_encryption_scope__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionScope = x_ms_encryption_scope__iterator->second;
          }
          auto x_ms_access_tier__iterator = httpResponse.GetHeaders().find("x-ms-access-tier");
          if (x_ms_access_tier__iterator != httpResponse.GetHeaders().end())
          {
            response.Tier = AccessTier(x_ms_access_tier__iterator->second);
          }
          auto x_ms_access_tier_inferred__iterator
              = httpResponse.GetHeaders().find("x-ms-access-tier-inferred");
          if (x_ms_access_tier_inferred__iterator != httpResponse.GetHeaders().end())
          {
            response.IsAccessTierInferred = x_ms_access_tier_inferred__iterator->second == "true";
          }
          auto x_ms_archive_status__iterator
              = httpResponse.GetHeaders().find("x-ms-archive-status");
          if (x_ms_archive_status__iterator != httpResponse.GetHeaders().end())
          {
            response.ArchiveStatus = BlobArchiveStatus(x_ms_archive_status__iterator->second);
          }
          auto x_ms_rehydrate_priority__iterator
              = httpResponse.GetHeaders().find("x-ms-rehydrate-priority");
          if (x_ms_rehydrate_priority__iterator != httpResponse.GetHeaders().end())
          {
            response.RehydratePriority
                = RehydratePriority(x_ms_rehydrate_priority__iterator->second);
          }
          auto x_ms_access_tier_change_time__iterator
              = httpResponse.GetHeaders().find("x-ms-access-tier-change-time");
          if (x_ms_access_tier_change_time__iterator != httpResponse.GetHeaders().end())
          {
            response.AccessTierChangedOn = Azure::DateTime::Parse(
                x_ms_access_tier_change_time__iterator->second,
                Azure::DateTime::DateFormat::Rfc1123);
          }
          auto x_ms_copy_id__iterator = httpResponse.GetHeaders().find("x-ms-copy-id");
          if (x_ms_copy_id__iterator != httpResponse.GetHeaders().end())
          {
            response.CopyId = x_ms_copy_id__iterator->second;
          }
          auto x_ms_copy_source__iterator = httpResponse.GetHeaders().find("x-ms-copy-source");
          if (x_ms_copy_source__iterator != httpResponse.GetHeaders().end())
          {
            response.CopySource = x_ms_copy_source__iterator->second;
          }
          auto x_ms_copy_status__iterator = httpResponse.GetHeaders().find("x-ms-copy-status");
          if (x_ms_copy_status__iterator != httpResponse.GetHeaders().end())
          {
            response.CopyStatus = CopyStatus(x_ms_copy_status__iterator->second);
          }
          auto x_ms_copy_status_description__iterator
              = httpResponse.GetHeaders().find("x-ms-copy-status-description");
          if (x_ms_copy_status_description__iterator != httpResponse.GetHeaders().end())
          {
            response.CopyStatusDescription = x_ms_copy_status_description__iterator->second;
          }
          auto x_ms_incremental_copy__iterator
              = httpResponse.GetHeaders().find("x-ms-incremental-copy");
          if (x_ms_incremental_copy__iterator != httpResponse.GetHeaders().end())
          {
            response.IsIncrementalCopy = x_ms_incremental_copy__iterator->second == "true";
          }
          auto x_ms_copy_destination_snapshot__iterator
              = httpResponse.GetHeaders().find("x-ms-copy-destination-snapshot");
          if (x_ms_copy_destination_snapshot__iterator != httpResponse.GetHeaders().end())
          {
            response.IncrementalCopyDestinationSnapshot
                = x_ms_copy_destination_snapshot__iterator->second;
          }
          auto x_ms_copy_progress__iterator = httpResponse.GetHeaders().find("x-ms-copy-progress");
          if (x_ms_copy_progress__iterator != httpResponse.GetHeaders().end())
          {
            response.CopyProgress = x_ms_copy_progress__iterator->second;
          }
          auto x_ms_copy_completion_time__iterator
              = httpResponse.GetHeaders().find("x-ms-copy-completion-time");
          if (x_ms_copy_completion_time__iterator != httpResponse.GetHeaders().end())
          {
            response.CopyCompletedOn = Azure::DateTime::Parse(
                x_ms_copy_completion_time__iterator->second, Azure::DateTime::DateFormat::Rfc1123);
          }
          auto x_ms_or_policy_id__iterator = httpResponse.GetHeaders().find("x-ms-or-policy-id");
          if (x_ms_or_policy_id__iterator != httpResponse.GetHeaders().end())
          {
            response.ObjectReplicationDestinationPolicyId = x_ms_or_policy_id__iterator->second;
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
              std::string policyId
                  = std::string(header.begin() + 8, header.begin() + underscorePos);
              std::string ruleId = header.substr(underscorePos + 1);

              ObjectReplicationRule rule;
              rule.RuleId = std::move(ruleId);
              rule.ReplicationStatus = ObjectReplicationStatus(i->second);
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
          auto x_ms_tag_count__iterator = httpResponse.GetHeaders().find("x-ms-tag-count");
          if (x_ms_tag_count__iterator != httpResponse.GetHeaders().end())
          {
            response.TagCount = std::stoi(x_ms_tag_count__iterator->second);
          }
          auto x_ms_version_id__iterator = httpResponse.GetHeaders().find("x-ms-version-id");
          if (x_ms_version_id__iterator != httpResponse.GetHeaders().end())
          {
            response.VersionId = x_ms_version_id__iterator->second;
          }
          auto x_ms_is_current_version__iterator
              = httpResponse.GetHeaders().find("x-ms-is-current-version");
          if (x_ms_is_current_version__iterator != httpResponse.GetHeaders().end())
          {
            response.IsCurrentVersion = x_ms_is_current_version__iterator->second == "true";
          }
          return Azure::Response<BlobProperties>(std::move(response), std::move(pHttpResponse));
        }

        struct SetBlobHttpHeadersOptions
        {
          Azure::Nullable<int32_t> Timeout;
          BlobHttpHeaders HttpHeaders;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct SetBlobHttpHeadersOptions

        static Azure::Response<SetBlobHttpHeadersResult> SetHttpHeaders(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const SetBlobHttpHeadersOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "properties");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          if (!options.HttpHeaders.ContentType.empty())
          {
            request.SetHeader("x-ms-blob-content-type", options.HttpHeaders.ContentType);
          }
          if (!options.HttpHeaders.ContentEncoding.empty())
          {
            request.SetHeader("x-ms-blob-content-encoding", options.HttpHeaders.ContentEncoding);
          }
          if (!options.HttpHeaders.ContentLanguage.empty())
          {
            request.SetHeader("x-ms-blob-content-language", options.HttpHeaders.ContentLanguage);
          }
          if (!options.HttpHeaders.CacheControl.empty())
          {
            request.SetHeader("x-ms-blob-cache-control", options.HttpHeaders.CacheControl);
          }
          if (!Azure::Core::Convert::Base64Encode(options.HttpHeaders.ContentHash.Value).empty())
          {
            request.SetHeader(
                "x-ms-blob-content-md5",
                Azure::Core::Convert::Base64Encode(options.HttpHeaders.ContentHash.Value));
          }
          if (!options.HttpHeaders.ContentDisposition.empty())
          {
            request.SetHeader(
                "x-ms-blob-content-disposition", options.HttpHeaders.ContentDisposition);
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          SetBlobHttpHeadersResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          auto x_ms_blob_sequence_number__iterator
              = httpResponse.GetHeaders().find("x-ms-blob-sequence-number");
          if (x_ms_blob_sequence_number__iterator != httpResponse.GetHeaders().end())
          {
            response.SequenceNumber = std::stoll(x_ms_blob_sequence_number__iterator->second);
          }
          return Azure::Response<SetBlobHttpHeadersResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct SetBlobMetadataOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Storage::Metadata Metadata;
          Azure::Nullable<std::string> EncryptionKey;
          Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
          Azure::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
          Azure::Nullable<std::string> EncryptionScope;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct SetBlobMetadataOptions

        static Azure::Response<SetBlobMetadataResult> SetMetadata(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const SetBlobMetadataOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "metadata");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          for (const auto& pair : options.Metadata)
          {
            request.SetHeader("x-ms-meta-" + pair.first, pair.second);
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.GetValue()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          SetBlobMetadataResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          return Azure::Response<SetBlobMetadataResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct SetBlobAccessTierOptions
        {
          Azure::Nullable<int32_t> Timeout;
          AccessTier Tier;
          Azure::Nullable<Models::RehydratePriority> RehydratePriority;
          Azure::Nullable<std::string> IfTags;
        }; // struct SetBlobAccessTierOptions

        static Azure::Core::Http::Request SetAccessTierCreateMessage(
            const Azure::Core::Url& url,
            const SetBlobAccessTierOptions& options)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "tier");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.SetHeader("x-ms-access-tier", options.Tier.ToString());
          if (options.RehydratePriority.HasValue())
          {
            request.SetHeader(
                "x-ms-rehydrate-priority", options.RehydratePriority.GetValue().ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          return request;
        }

        static Azure::Response<SetBlobAccessTierResult> SetAccessTierCreateResponse(
            std::unique_ptr<Azure::Core::Http::RawResponse> pHttpResponse,
            const Azure::Core::Context& context)
        {
          (void)context;
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          SetBlobAccessTierResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200 || http_status_code == 202))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          return Azure::Response<SetBlobAccessTierResult>(
              std::move(response), std::move(pHttpResponse));
        }

        static Azure::Response<SetBlobAccessTierResult> SetAccessTier(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const SetBlobAccessTierOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = SetAccessTierCreateMessage(url, options);
          auto pHttpResponse = pipeline.Send(request, context);
          return SetAccessTierCreateResponse(std::move(pHttpResponse), context);
        }

        struct StartCopyBlobFromUriOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Storage::Metadata Metadata;
          std::string SourceUri;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<std::string> SourceLeaseId;
          Azure::Nullable<AccessTier> Tier;
          Azure::Nullable<Models::RehydratePriority> RehydratePriority;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
          Azure::Nullable<Azure::DateTime> SourceIfModifiedSince;
          Azure::Nullable<Azure::DateTime> SourceIfUnmodifiedSince;
          Azure::ETag SourceIfMatch;
          Azure::ETag SourceIfNoneMatch;
          Azure::Nullable<std::string> SourceIfTags;
          Azure::Nullable<bool> ShouldSealDestination;
        }; // struct StartCopyBlobFromUriOptions

        static Azure::Response<Models::_detail::StartCopyBlobFromUriResult> StartCopyFromUri(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const StartCopyBlobFromUriOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          for (const auto& pair : options.Metadata)
          {
            request.SetHeader("x-ms-meta-" + pair.first, pair.second);
          }
          request.SetHeader("x-ms-copy-source", options.SourceUri);
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          if (options.SourceLeaseId.HasValue())
          {
            request.SetHeader("x-ms-source-lease-id", options.SourceLeaseId.GetValue());
          }
          if (options.Tier.HasValue())
          {
            request.SetHeader("x-ms-access-tier", options.Tier.GetValue().ToString());
          }
          if (options.RehydratePriority.HasValue())
          {
            request.SetHeader(
                "x-ms-rehydrate-priority", options.RehydratePriority.GetValue().ToString());
          }
          if (options.ShouldSealDestination.HasValue())
          {
            request.SetHeader(
                "x-ms-seal-blob", options.ShouldSealDestination.GetValue() ? "true" : "false");
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          if (options.SourceIfModifiedSince.HasValue())
          {
            request.SetHeader(
                "x-ms-source-if-modified-since",
                options.SourceIfModifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.SourceIfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "x-ms-source-if-unmodified-since",
                options.SourceIfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.SourceIfMatch.HasValue() && !options.SourceIfMatch.ToString().empty())
          {
            request.SetHeader("x-ms-source-if-match", options.SourceIfMatch.ToString());
          }
          if (options.SourceIfNoneMatch.HasValue() && !options.SourceIfNoneMatch.ToString().empty())
          {
            request.SetHeader("x-ms-source-if-none-match", options.SourceIfNoneMatch.ToString());
          }
          if (options.SourceIfTags.HasValue())
          {
            request.SetHeader("x-ms-source-if-tags", options.SourceIfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::StartCopyBlobFromUriResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 202))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.CopyId = httpResponse.GetHeaders().at("x-ms-copy-id");
          response.CopyStatus = CopyStatus(httpResponse.GetHeaders().at("x-ms-copy-status"));
          auto x_ms_version_id__iterator = httpResponse.GetHeaders().find("x-ms-version-id");
          if (x_ms_version_id__iterator != httpResponse.GetHeaders().end())
          {
            response.VersionId = x_ms_version_id__iterator->second;
          }
          return Azure::Response<Models::_detail::StartCopyBlobFromUriResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct AbortCopyBlobFromUriOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string CopyId;
          Azure::Nullable<std::string> LeaseId;
        }; // struct AbortCopyBlobFromUriOptions

        static Azure::Response<AbortCopyBlobFromUriResult> AbortCopyFromUri(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const AbortCopyBlobFromUriOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.GetUrl().AppendQueryParameter("comp", "copy");
          request.GetUrl().AppendQueryParameter(
              "copyid", Storage::_detail::UrlEncodeQueryParameter(options.CopyId));
          request.SetHeader("x-ms-copy-action", "abort");
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          AbortCopyBlobFromUriResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 204))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          return Azure::Response<AbortCopyBlobFromUriResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct CreateBlobSnapshotOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Storage::Metadata Metadata;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<std::string> EncryptionKey;
          Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
          Azure::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
          Azure::Nullable<std::string> EncryptionScope;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct CreateBlobSnapshotOptions

        static Azure::Response<CreateBlobSnapshotResult> CreateSnapshot(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const CreateBlobSnapshotOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "snapshot");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.GetValue()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
          }
          for (const auto& pair : options.Metadata)
          {
            request.SetHeader("x-ms-meta-" + pair.first, pair.second);
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          CreateBlobSnapshotResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 201))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.IsServerEncrypted
              = httpResponse.GetHeaders().at("x-ms-request-server-encrypted") == "true";
          auto x_ms_encryption_key_sha256__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
          if (x_ms_encryption_key_sha256__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionKeySha256
                = Azure::Core::Convert::Base64Decode(x_ms_encryption_key_sha256__iterator->second);
          }
          auto x_ms_encryption_scope__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-scope");
          if (x_ms_encryption_scope__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionScope = x_ms_encryption_scope__iterator->second;
          }
          response.Snapshot = httpResponse.GetHeaders().at("x-ms-snapshot");
          auto x_ms_version_id__iterator = httpResponse.GetHeaders().find("x-ms-version-id");
          if (x_ms_version_id__iterator != httpResponse.GetHeaders().end())
          {
            response.VersionId = x_ms_version_id__iterator->second;
          }
          return Azure::Response<CreateBlobSnapshotResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct GetBlobTagsOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::string> IfTags;
        }; // struct GetBlobTagsOptions

        static Azure::Response<GetBlobTagsResult> GetTags(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const GetBlobTagsOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.GetUrl().AppendQueryParameter("comp", "tags");
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          GetBlobTagsResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            Storage::_detail::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = GetBlobTagsResultFromXml(reader);
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          return Azure::Response<GetBlobTagsResult>(std::move(response), std::move(pHttpResponse));
        }

        struct SetBlobTagsOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::map<std::string, std::string> Tags;
          Azure::Nullable<std::string> IfTags;
        }; // struct SetBlobTagsOptions

        static Azure::Response<SetBlobTagsResult> SetTags(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const SetBlobTagsOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          std::string xml_body;
          {
            Storage::_detail::XmlWriter writer;
            SetBlobTagsOptionsToXml(writer, options);
            xml_body = writer.GetDocument();
            writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::End});
          }
          Azure::Core::IO::MemoryBodyStream xml_body_stream(
              reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
          auto request = Azure::Core::Http::Request(
              Azure::Core::Http::HttpMethod::Put, url, &xml_body_stream);
          request.SetHeader("Content-Length", std::to_string(xml_body_stream.Length()));
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.GetUrl().AppendQueryParameter("comp", "tags");
          request.SetHeader("Content-Type", "application/xml; charset=UTF-8");
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          SetBlobTagsResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 204))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          return Azure::Response<SetBlobTagsResult>(std::move(response), std::move(pHttpResponse));
        }

        struct AcquireBlobLeaseOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::chrono::seconds LeaseDuration;
          Azure::Nullable<std::string> ProposedLeaseId;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct AcquireBlobLeaseOptions

        static Azure::Response<Models::_detail::AcquireBlobLeaseResult> AcquireLease(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const AcquireBlobLeaseOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.GetUrl().AppendQueryParameter("comp", "lease");
          request.SetHeader("x-ms-lease-action", "acquire");
          request.SetHeader("x-ms-lease-duration", std::to_string(options.LeaseDuration.count()));
          if (options.ProposedLeaseId.HasValue())
          {
            request.SetHeader("x-ms-proposed-lease-id", options.ProposedLeaseId.GetValue());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::AcquireBlobLeaseResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 201))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
          return Azure::Response<Models::_detail::AcquireBlobLeaseResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct RenewBlobLeaseOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string LeaseId;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct RenewBlobLeaseOptions

        static Azure::Response<Models::_detail::RenewBlobLeaseResult> RenewLease(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const RenewBlobLeaseOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.GetUrl().AppendQueryParameter("comp", "lease");
          request.SetHeader("x-ms-lease-action", "renew");
          request.SetHeader("x-ms-lease-id", options.LeaseId);
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::RenewBlobLeaseResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
          return Azure::Response<Models::_detail::RenewBlobLeaseResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct ChangeBlobLeaseOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string LeaseId;
          std::string ProposedLeaseId;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct ChangeBlobLeaseOptions

        static Azure::Response<Models::_detail::ChangeBlobLeaseResult> ChangeLease(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const ChangeBlobLeaseOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.GetUrl().AppendQueryParameter("comp", "lease");
          request.SetHeader("x-ms-lease-action", "change");
          request.SetHeader("x-ms-lease-id", options.LeaseId);
          request.SetHeader("x-ms-proposed-lease-id", options.ProposedLeaseId);
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::ChangeBlobLeaseResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
          return Azure::Response<Models::_detail::ChangeBlobLeaseResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct ReleaseBlobLeaseOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string LeaseId;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct ReleaseBlobLeaseOptions

        static Azure::Response<Models::_detail::ReleaseBlobLeaseResult> ReleaseLease(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const ReleaseBlobLeaseOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.GetUrl().AppendQueryParameter("comp", "lease");
          request.SetHeader("x-ms-lease-action", "release");
          request.SetHeader("x-ms-lease-id", options.LeaseId);
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::ReleaseBlobLeaseResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          auto x_ms_blob_sequence_number__iterator
              = httpResponse.GetHeaders().find("x-ms-blob-sequence-number");
          if (x_ms_blob_sequence_number__iterator != httpResponse.GetHeaders().end())
          {
            response.SequenceNumber = std::stoll(x_ms_blob_sequence_number__iterator->second);
          }
          return Azure::Response<Models::_detail::ReleaseBlobLeaseResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct BreakBlobLeaseOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::chrono::seconds> BreakPeriod;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct BreakBlobLeaseOptions

        static Azure::Response<Models::_detail::BreakBlobLeaseResult> BreakLease(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const BreakBlobLeaseOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.GetUrl().AppendQueryParameter("comp", "lease");
          request.SetHeader("x-ms-lease-action", "break");
          if (options.BreakPeriod.HasValue())
          {
            request.SetHeader(
                "x-ms-lease-break-period", std::to_string(options.BreakPeriod.GetValue().count()));
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::BreakBlobLeaseResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 202))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.LeaseTime = std::stoi(httpResponse.GetHeaders().at("x-ms-lease-time"));
          return Azure::Response<Models::_detail::BreakBlobLeaseResult>(
              std::move(response), std::move(pHttpResponse));
        }

      private:
        static GetBlobTagsResult GetBlobTagsResultFromXml(Storage::_detail::XmlReader& reader)
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
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::Text)
            {
            }
          }
          return ret;
        }

        static std::map<std::string, std::string> TagsFromXml(Storage::_detail::XmlReader& reader)
        {
          std::map<std::string, std::string> ret;
          int depth = 0;
          std::string key;
          bool is_key = false;
          bool is_value = false;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
            {
              if (depth-- == 0)
              {
                break;
              }
            }
            if (depth == 2 && node.Type == Storage::_detail::XmlNodeType::Text)
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

        static void SetBlobTagsOptionsToXml(
            Storage::_detail::XmlWriter& writer,
            const SetBlobTagsOptions& options)
        {
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "Tags"});
          writer.Write(
              Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "TagSet"});
          for (const auto& i : options.Tags)
          {
            writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "Tag"});
            writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "Key"});
            writer.Write(Storage::_detail::XmlNode{
                Storage::_detail::XmlNodeType::Text, nullptr, i.first.data()});
            writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
            writer.Write(
                Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "Value"});
            writer.Write(Storage::_detail::XmlNode{
                Storage::_detail::XmlNodeType::Text, nullptr, i.second.data()});
            writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
            writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          }
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
        }

      }; // class Blob

      class BlockBlob {
      public:
        struct UploadBlockBlobOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<ContentHash> TransactionalContentHash;
          BlobHttpHeaders HttpHeaders;
          Storage::Metadata Metadata;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<AccessTier> Tier;
          Azure::Nullable<std::string> EncryptionKey;
          Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
          Azure::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
          Azure::Nullable<std::string> EncryptionScope;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct UploadBlockBlobOptions

        static Azure::Response<UploadBlockBlobResult> Upload(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            Azure::Core::IO::BodyStream* requestBody,
            const UploadBlockBlobOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request
              = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, requestBody);
          request.SetHeader("Content-Length", std::to_string(requestBody->Length()));
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.GetValue()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
          }
          if (options.TransactionalContentHash.HasValue())
          {
            if (options.TransactionalContentHash.GetValue().Algorithm == HashAlgorithm::Md5)
            {
              request.SetHeader(
                  "Content-MD5",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.GetValue().Value));
            }
            else if (options.TransactionalContentHash.GetValue().Algorithm == HashAlgorithm::Crc64)
            {
              request.SetHeader(
                  "x-ms-content-crc64",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.GetValue().Value));
            }
          }
          if (!options.HttpHeaders.ContentType.empty())
          {
            request.SetHeader("x-ms-blob-content-type", options.HttpHeaders.ContentType);
          }
          if (!options.HttpHeaders.ContentEncoding.empty())
          {
            request.SetHeader("x-ms-blob-content-encoding", options.HttpHeaders.ContentEncoding);
          }
          if (!options.HttpHeaders.ContentLanguage.empty())
          {
            request.SetHeader("x-ms-blob-content-language", options.HttpHeaders.ContentLanguage);
          }
          if (!options.HttpHeaders.CacheControl.empty())
          {
            request.SetHeader("x-ms-blob-cache-control", options.HttpHeaders.CacheControl);
          }
          if (!Azure::Core::Convert::Base64Encode(options.HttpHeaders.ContentHash.Value).empty())
          {
            request.SetHeader(
                "x-ms-blob-content-md5",
                Azure::Core::Convert::Base64Encode(options.HttpHeaders.ContentHash.Value));
          }
          if (!options.HttpHeaders.ContentDisposition.empty())
          {
            request.SetHeader(
                "x-ms-blob-content-disposition", options.HttpHeaders.ContentDisposition);
          }
          for (const auto& pair : options.Metadata)
          {
            request.SetHeader("x-ms-meta-" + pair.first, pair.second);
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          request.SetHeader("x-ms-blob-type", "BlockBlob");
          if (options.Tier.HasValue())
          {
            request.SetHeader("x-ms-access-tier", options.Tier.GetValue().ToString());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          UploadBlockBlobResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 201))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          {
            const auto& headers = httpResponse.GetHeaders();
            auto content_md5_iterator = headers.find("content-md5");
            if (content_md5_iterator != headers.end())
            {
              ContentHash hash;
              hash.Algorithm = HashAlgorithm::Md5;
              hash.Value = Azure::Core::Convert::Base64Decode(content_md5_iterator->second);
              response.TransactionalContentHash = std::move(hash);
            }
            auto x_ms_content_crc64_iterator = headers.find("x-ms-content-crc64");
            if (x_ms_content_crc64_iterator != headers.end())
            {
              ContentHash hash;
              hash.Algorithm = HashAlgorithm::Crc64;
              hash.Value = Azure::Core::Convert::Base64Decode(x_ms_content_crc64_iterator->second);
              response.TransactionalContentHash = std::move(hash);
            }
          }
          auto x_ms_version_id__iterator = httpResponse.GetHeaders().find("x-ms-version-id");
          if (x_ms_version_id__iterator != httpResponse.GetHeaders().end())
          {
            response.VersionId = x_ms_version_id__iterator->second;
          }
          response.IsServerEncrypted
              = httpResponse.GetHeaders().at("x-ms-request-server-encrypted") == "true";
          auto x_ms_encryption_key_sha256__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
          if (x_ms_encryption_key_sha256__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionKeySha256
                = Azure::Core::Convert::Base64Decode(x_ms_encryption_key_sha256__iterator->second);
          }
          auto x_ms_encryption_scope__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-scope");
          if (x_ms_encryption_scope__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionScope = x_ms_encryption_scope__iterator->second;
          }
          return Azure::Response<UploadBlockBlobResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct StageBlockOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string BlockId;
          Azure::Nullable<ContentHash> TransactionalContentHash;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<std::string> EncryptionKey;
          Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
          Azure::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
          Azure::Nullable<std::string> EncryptionScope;
        }; // struct StageBlockOptions

        static Azure::Response<StageBlockResult> StageBlock(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            Azure::Core::IO::BodyStream* requestBody,
            const StageBlockOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request
              = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, requestBody);
          request.SetHeader("Content-Length", std::to_string(requestBody->Length()));
          request.GetUrl().AppendQueryParameter("comp", "block");
          request.GetUrl().AppendQueryParameter(
              "blockid", Storage::_detail::UrlEncodeQueryParameter(options.BlockId));
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          if (options.TransactionalContentHash.HasValue())
          {
            if (options.TransactionalContentHash.GetValue().Algorithm == HashAlgorithm::Md5)
            {
              request.SetHeader(
                  "Content-MD5",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.GetValue().Value));
            }
            else if (options.TransactionalContentHash.GetValue().Algorithm == HashAlgorithm::Crc64)
            {
              request.SetHeader(
                  "x-ms-content-crc64",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.GetValue().Value));
            }
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.GetValue()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          StageBlockResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 201))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          {
            const auto& headers = httpResponse.GetHeaders();
            auto content_md5_iterator = headers.find("content-md5");
            if (content_md5_iterator != headers.end())
            {
              ContentHash hash;
              hash.Algorithm = HashAlgorithm::Md5;
              hash.Value = Azure::Core::Convert::Base64Decode(content_md5_iterator->second);
              response.TransactionalContentHash = std::move(hash);
            }
            auto x_ms_content_crc64_iterator = headers.find("x-ms-content-crc64");
            if (x_ms_content_crc64_iterator != headers.end())
            {
              ContentHash hash;
              hash.Algorithm = HashAlgorithm::Crc64;
              hash.Value = Azure::Core::Convert::Base64Decode(x_ms_content_crc64_iterator->second);
              response.TransactionalContentHash = std::move(hash);
            }
          }
          response.IsServerEncrypted
              = httpResponse.GetHeaders().at("x-ms-request-server-encrypted") == "true";
          auto x_ms_encryption_key_sha256__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
          if (x_ms_encryption_key_sha256__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionKeySha256
                = Azure::Core::Convert::Base64Decode(x_ms_encryption_key_sha256__iterator->second);
          }
          auto x_ms_encryption_scope__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-scope");
          if (x_ms_encryption_scope__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionScope = x_ms_encryption_scope__iterator->second;
          }
          return Azure::Response<StageBlockResult>(std::move(response), std::move(pHttpResponse));
        }

        struct StageBlockFromUriOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string BlockId;
          std::string SourceUri;
          Azure::Nullable<Azure::Core::Http::HttpRange> SourceRange;
          Azure::Nullable<ContentHash> TransactionalContentHash;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<std::string> EncryptionKey;
          Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
          Azure::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
          Azure::Nullable<std::string> EncryptionScope;
          Azure::Nullable<Azure::DateTime> SourceIfModifiedSince;
          Azure::Nullable<Azure::DateTime> SourceIfUnmodifiedSince;
          Azure::ETag SourceIfMatch;
          Azure::ETag SourceIfNoneMatch;
        }; // struct StageBlockFromUriOptions

        static Azure::Response<StageBlockFromUriResult> StageBlockFromUri(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const StageBlockFromUriOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "block");
          request.GetUrl().AppendQueryParameter(
              "blockid", Storage::_detail::UrlEncodeQueryParameter(options.BlockId));
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.SetHeader("x-ms-copy-source", options.SourceUri);
          if (options.SourceRange.HasValue())
          {
            std::string headerValue
                = "bytes=" + std::to_string(options.SourceRange.GetValue().Offset) + "-";
            if (options.SourceRange.GetValue().Length.HasValue())
            {
              headerValue += std::to_string(
                  options.SourceRange.GetValue().Offset
                  + options.SourceRange.GetValue().Length.GetValue() - 1);
            }
            request.SetHeader("x-ms-source_range", std::move(headerValue));
          }
          if (options.TransactionalContentHash.HasValue())
          {
            if (options.TransactionalContentHash.GetValue().Algorithm == HashAlgorithm::Md5)
            {
              request.SetHeader(
                  "x-ms-source-content-md5",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.GetValue().Value));
            }
            else if (options.TransactionalContentHash.GetValue().Algorithm == HashAlgorithm::Crc64)
            {
              request.SetHeader(
                  "x-ms-source-content-crc64",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.GetValue().Value));
            }
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.GetValue()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
          }
          if (options.SourceIfModifiedSince.HasValue())
          {
            request.SetHeader(
                "x-ms-source-if-modified-since",
                options.SourceIfModifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.SourceIfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "x-ms-source-if-unmodified-since",
                options.SourceIfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.SourceIfMatch.HasValue() && !options.SourceIfMatch.ToString().empty())
          {
            request.SetHeader("x-ms-source-if-match", options.SourceIfMatch.ToString());
          }
          if (options.SourceIfNoneMatch.HasValue() && !options.SourceIfNoneMatch.ToString().empty())
          {
            request.SetHeader("x-ms-source-if-none-match", options.SourceIfNoneMatch.ToString());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          StageBlockFromUriResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 201))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          {
            const auto& headers = httpResponse.GetHeaders();
            auto content_md5_iterator = headers.find("content-md5");
            if (content_md5_iterator != headers.end())
            {
              ContentHash hash;
              hash.Algorithm = HashAlgorithm::Md5;
              hash.Value = Azure::Core::Convert::Base64Decode(content_md5_iterator->second);
              response.TransactionalContentHash = std::move(hash);
            }
            auto x_ms_content_crc64_iterator = headers.find("x-ms-content-crc64");
            if (x_ms_content_crc64_iterator != headers.end())
            {
              ContentHash hash;
              hash.Algorithm = HashAlgorithm::Crc64;
              hash.Value = Azure::Core::Convert::Base64Decode(x_ms_content_crc64_iterator->second);
              response.TransactionalContentHash = std::move(hash);
            }
          }
          response.IsServerEncrypted
              = httpResponse.GetHeaders().at("x-ms-request-server-encrypted") == "true";
          auto x_ms_encryption_key_sha256__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
          if (x_ms_encryption_key_sha256__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionKeySha256
                = Azure::Core::Convert::Base64Decode(x_ms_encryption_key_sha256__iterator->second);
          }
          auto x_ms_encryption_scope__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-scope");
          if (x_ms_encryption_scope__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionScope = x_ms_encryption_scope__iterator->second;
          }
          return Azure::Response<StageBlockFromUriResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct CommitBlockListOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::vector<std::pair<BlockType, std::string>> BlockList;
          BlobHttpHeaders HttpHeaders;
          Storage::Metadata Metadata;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<std::string> EncryptionKey;
          Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
          Azure::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
          Azure::Nullable<std::string> EncryptionScope;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
          Azure::Nullable<AccessTier> Tier;
        }; // struct CommitBlockListOptions

        static Azure::Response<CommitBlockListResult> CommitBlockList(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const CommitBlockListOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          std::string xml_body;
          {
            Storage::_detail::XmlWriter writer;
            CommitBlockListOptionsToXml(writer, options);
            xml_body = writer.GetDocument();
            writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::End});
          }
          Azure::Core::IO::MemoryBodyStream xml_body_stream(
              reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
          auto request = Azure::Core::Http::Request(
              Azure::Core::Http::HttpMethod::Put, url, &xml_body_stream);
          request.SetHeader("Content-Length", std::to_string(xml_body_stream.Length()));
          request.GetUrl().AppendQueryParameter("comp", "blocklist");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          if (!options.HttpHeaders.ContentType.empty())
          {
            request.SetHeader("x-ms-blob-content-type", options.HttpHeaders.ContentType);
          }
          if (!options.HttpHeaders.ContentEncoding.empty())
          {
            request.SetHeader("x-ms-blob-content-encoding", options.HttpHeaders.ContentEncoding);
          }
          if (!options.HttpHeaders.ContentLanguage.empty())
          {
            request.SetHeader("x-ms-blob-content-language", options.HttpHeaders.ContentLanguage);
          }
          if (!options.HttpHeaders.CacheControl.empty())
          {
            request.SetHeader("x-ms-blob-cache-control", options.HttpHeaders.CacheControl);
          }
          if (!Azure::Core::Convert::Base64Encode(options.HttpHeaders.ContentHash.Value).empty())
          {
            request.SetHeader(
                "x-ms-blob-content-md5",
                Azure::Core::Convert::Base64Encode(options.HttpHeaders.ContentHash.Value));
          }
          if (!options.HttpHeaders.ContentDisposition.empty())
          {
            request.SetHeader(
                "x-ms-blob-content-disposition", options.HttpHeaders.ContentDisposition);
          }
          for (const auto& pair : options.Metadata)
          {
            request.SetHeader("x-ms-meta-" + pair.first, pair.second);
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.GetValue()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
          }
          if (options.Tier.HasValue())
          {
            request.SetHeader("x-ms-access-tier", options.Tier.GetValue().ToString());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          CommitBlockListResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 201))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          auto x_ms_version_id__iterator = httpResponse.GetHeaders().find("x-ms-version-id");
          if (x_ms_version_id__iterator != httpResponse.GetHeaders().end())
          {
            response.VersionId = x_ms_version_id__iterator->second;
          }
          response.IsServerEncrypted
              = httpResponse.GetHeaders().at("x-ms-request-server-encrypted") == "true";
          auto x_ms_encryption_key_sha256__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
          if (x_ms_encryption_key_sha256__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionKeySha256
                = Azure::Core::Convert::Base64Decode(x_ms_encryption_key_sha256__iterator->second);
          }
          auto x_ms_encryption_scope__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-scope");
          if (x_ms_encryption_scope__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionScope = x_ms_encryption_scope__iterator->second;
          }
          return Azure::Response<CommitBlockListResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct GetBlockListOptions
        {
          Azure::Nullable<int32_t> Timeout;
          BlockListTypeOption ListType = BlockListTypeOption::Committed;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<std::string> IfTags;
        }; // struct GetBlockListOptions

        static Azure::Response<GetBlockListResult> GetBlockList(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const GetBlockListOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter("comp", "blocklist");
          request.GetUrl().AppendQueryParameter(
              "blocklisttype",
              Storage::_detail::UrlEncodeQueryParameter(options.ListType.ToString()));
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          GetBlockListResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            Storage::_detail::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = GetBlockListResultFromXml(reader);
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.ContentType = httpResponse.GetHeaders().at("content-type");
          response.BlobSize = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-content-length"));
          return Azure::Response<GetBlockListResult>(std::move(response), std::move(pHttpResponse));
        }

      private:
        static GetBlockListResult GetBlockListResultFromXml(Storage::_detail::XmlReader& reader)
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
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::Text)
            {
            }
          }
          return ret;
        }

        static BlobBlock BlobBlockFromXml(Storage::_detail::XmlReader& reader)
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
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::Text)
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
            Storage::_detail::XmlWriter& writer,
            const CommitBlockListOptions& options)
        {
          writer.Write(
              Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::StartTag, "BlockList"});
          for (const auto& i : options.BlockList)
          {
            writer.Write(Storage::_detail::XmlNode{
                Storage::_detail::XmlNodeType::StartTag,
                i.first.ToString().data(),
                i.second.data()});
          }
          writer.Write(Storage::_detail::XmlNode{Storage::_detail::XmlNodeType::EndTag});
        }

      }; // class BlockBlob

      class PageBlob {
      public:
        struct CreatePageBlobOptions
        {
          Azure::Nullable<int32_t> Timeout;
          int64_t BlobSize = -1;
          Azure::Nullable<int64_t> SequenceNumber;
          BlobHttpHeaders HttpHeaders;
          Storage::Metadata Metadata;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<AccessTier> Tier;
          Azure::Nullable<std::string> EncryptionKey;
          Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
          Azure::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
          Azure::Nullable<std::string> EncryptionScope;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct CreatePageBlobOptions

        static Azure::Response<CreatePageBlobResult> Create(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const CreatePageBlobOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          if (!options.HttpHeaders.ContentType.empty())
          {
            request.SetHeader("x-ms-blob-content-type", options.HttpHeaders.ContentType);
          }
          if (!options.HttpHeaders.ContentEncoding.empty())
          {
            request.SetHeader("x-ms-blob-content-encoding", options.HttpHeaders.ContentEncoding);
          }
          if (!options.HttpHeaders.ContentLanguage.empty())
          {
            request.SetHeader("x-ms-blob-content-language", options.HttpHeaders.ContentLanguage);
          }
          if (!options.HttpHeaders.CacheControl.empty())
          {
            request.SetHeader("x-ms-blob-cache-control", options.HttpHeaders.CacheControl);
          }
          if (!Azure::Core::Convert::Base64Encode(options.HttpHeaders.ContentHash.Value).empty())
          {
            request.SetHeader(
                "x-ms-blob-content-md5",
                Azure::Core::Convert::Base64Encode(options.HttpHeaders.ContentHash.Value));
          }
          if (!options.HttpHeaders.ContentDisposition.empty())
          {
            request.SetHeader(
                "x-ms-blob-content-disposition", options.HttpHeaders.ContentDisposition);
          }
          for (const auto& pair : options.Metadata)
          {
            request.SetHeader("x-ms-meta-" + pair.first, pair.second);
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          request.SetHeader("x-ms-blob-type", "PageBlob");
          request.SetHeader("x-ms-blob-content-length", std::to_string(options.BlobSize));
          if (options.SequenceNumber.HasValue())
          {
            request.SetHeader(
                "x-ms-blob-sequence-number", std::to_string(options.SequenceNumber.GetValue()));
          }
          if (options.Tier.HasValue())
          {
            request.SetHeader("x-ms-access-tier", options.Tier.GetValue().ToString());
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.GetValue()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          CreatePageBlobResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 201))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          auto x_ms_version_id__iterator = httpResponse.GetHeaders().find("x-ms-version-id");
          if (x_ms_version_id__iterator != httpResponse.GetHeaders().end())
          {
            response.VersionId = x_ms_version_id__iterator->second;
          }
          response.IsServerEncrypted
              = httpResponse.GetHeaders().at("x-ms-request-server-encrypted") == "true";
          auto x_ms_encryption_key_sha256__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
          if (x_ms_encryption_key_sha256__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionKeySha256
                = Azure::Core::Convert::Base64Decode(x_ms_encryption_key_sha256__iterator->second);
          }
          auto x_ms_encryption_scope__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-scope");
          if (x_ms_encryption_scope__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionScope = x_ms_encryption_scope__iterator->second;
          }
          return Azure::Response<CreatePageBlobResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct UploadPageBlobPagesOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Core::Http::HttpRange Range;
          Azure::Nullable<ContentHash> TransactionalContentHash;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<int64_t> IfSequenceNumberLessThanOrEqualTo;
          Azure::Nullable<int64_t> IfSequenceNumberLessThan;
          Azure::Nullable<int64_t> IfSequenceNumberEqualTo;
          Azure::Nullable<std::string> EncryptionKey;
          Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
          Azure::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
          Azure::Nullable<std::string> EncryptionScope;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct UploadPageBlobPagesOptions

        static Azure::Response<UploadPageBlobPagesResult> UploadPages(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            Azure::Core::IO::BodyStream* requestBody,
            const UploadPageBlobPagesOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request
              = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, requestBody);
          request.SetHeader("Content-Length", std::to_string(requestBody->Length()));
          request.GetUrl().AppendQueryParameter("comp", "page");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          {
            std::string headerValue = "bytes=" + std::to_string(options.Range.Offset) + "-";
            if (options.Range.Length.HasValue())
            {
              headerValue
                  += std::to_string(options.Range.Offset + options.Range.Length.GetValue() - 1);
            }
            request.SetHeader("x-ms-range", std::move(headerValue));
          }
          if (options.TransactionalContentHash.HasValue())
          {
            if (options.TransactionalContentHash.GetValue().Algorithm == HashAlgorithm::Md5)
            {
              request.SetHeader(
                  "Content-MD5",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.GetValue().Value));
            }
            else if (options.TransactionalContentHash.GetValue().Algorithm == HashAlgorithm::Crc64)
            {
              request.SetHeader(
                  "x-ms-content-crc64",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.GetValue().Value));
            }
          }
          request.SetHeader("x-ms-page-write", "update");
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          if (options.IfSequenceNumberLessThanOrEqualTo.HasValue())
          {
            request.SetHeader(
                "x-ms-if-sequence-number-le",
                std::to_string(options.IfSequenceNumberLessThanOrEqualTo.GetValue()));
          }
          if (options.IfSequenceNumberLessThan.HasValue())
          {
            request.SetHeader(
                "x-ms-if-sequence-number-lt",
                std::to_string(options.IfSequenceNumberLessThan.GetValue()));
          }
          if (options.IfSequenceNumberEqualTo.HasValue())
          {
            request.SetHeader(
                "x-ms-if-sequence-number-eq",
                std::to_string(options.IfSequenceNumberEqualTo.GetValue()));
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.GetValue()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          UploadPageBlobPagesResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 201))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          {
            const auto& headers = httpResponse.GetHeaders();
            auto content_md5_iterator = headers.find("content-md5");
            if (content_md5_iterator != headers.end())
            {
              ContentHash hash;
              hash.Algorithm = HashAlgorithm::Md5;
              hash.Value = Azure::Core::Convert::Base64Decode(content_md5_iterator->second);
              response.TransactionalContentHash = std::move(hash);
            }
            auto x_ms_content_crc64_iterator = headers.find("x-ms-content-crc64");
            if (x_ms_content_crc64_iterator != headers.end())
            {
              ContentHash hash;
              hash.Algorithm = HashAlgorithm::Crc64;
              hash.Value = Azure::Core::Convert::Base64Decode(x_ms_content_crc64_iterator->second);
              response.TransactionalContentHash = std::move(hash);
            }
          }
          response.SequenceNumber
              = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-sequence-number"));
          response.IsServerEncrypted
              = httpResponse.GetHeaders().at("x-ms-request-server-encrypted") == "true";
          auto x_ms_encryption_key_sha256__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
          if (x_ms_encryption_key_sha256__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionKeySha256
                = Azure::Core::Convert::Base64Decode(x_ms_encryption_key_sha256__iterator->second);
          }
          auto x_ms_encryption_scope__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-scope");
          if (x_ms_encryption_scope__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionScope = x_ms_encryption_scope__iterator->second;
          }
          return Azure::Response<UploadPageBlobPagesResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct UploadPageBlobPagesFromUriOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string SourceUri;
          Azure::Core::Http::HttpRange SourceRange;
          Azure::Core::Http::HttpRange Range;
          Azure::Nullable<ContentHash> TransactionalContentHash;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<int64_t> IfSequenceNumberLessThanOrEqualTo;
          Azure::Nullable<int64_t> IfSequenceNumberLessThan;
          Azure::Nullable<int64_t> IfSequenceNumberEqualTo;
          Azure::Nullable<std::string> EncryptionKey;
          Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
          Azure::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
          Azure::Nullable<std::string> EncryptionScope;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct UploadPageBlobPagesFromUriOptions

        static Azure::Response<UploadPageBlobPagesFromUriResult> UploadPagesFromUri(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const UploadPageBlobPagesFromUriOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "page");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          {
            std::string headerValue = "bytes=" + std::to_string(options.Range.Offset) + "-";
            if (options.Range.Length.HasValue())
            {
              headerValue
                  += std::to_string(options.Range.Offset + options.Range.Length.GetValue() - 1);
            }
            request.SetHeader("x-ms-range", std::move(headerValue));
          }
          request.SetHeader("x-ms-copy-source", options.SourceUri);
          {
            std::string headerValue = "bytes=" + std::to_string(options.SourceRange.Offset) + "-";
            if (options.SourceRange.Length.HasValue())
            {
              headerValue += std::to_string(
                  options.SourceRange.Offset + options.SourceRange.Length.GetValue() - 1);
            }
            request.SetHeader("x-ms-source-range", std::move(headerValue));
          }
          if (options.TransactionalContentHash.HasValue())
          {
            if (options.TransactionalContentHash.GetValue().Algorithm == HashAlgorithm::Md5)
            {
              request.SetHeader(
                  "x-ms-source-content-md5",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.GetValue().Value));
            }
            else if (options.TransactionalContentHash.GetValue().Algorithm == HashAlgorithm::Crc64)
            {
              request.SetHeader(
                  "x-ms-source-content-crc64",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.GetValue().Value));
            }
          }
          request.SetHeader("x-ms-page-write", "update");
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          if (options.IfSequenceNumberLessThanOrEqualTo.HasValue())
          {
            request.SetHeader(
                "x-ms-if-sequence-number-le",
                std::to_string(options.IfSequenceNumberLessThanOrEqualTo.GetValue()));
          }
          if (options.IfSequenceNumberLessThan.HasValue())
          {
            request.SetHeader(
                "x-ms-if-sequence-number-lt",
                std::to_string(options.IfSequenceNumberLessThan.GetValue()));
          }
          if (options.IfSequenceNumberEqualTo.HasValue())
          {
            request.SetHeader(
                "x-ms-if-sequence-number-eq",
                std::to_string(options.IfSequenceNumberEqualTo.GetValue()));
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.GetValue()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          UploadPageBlobPagesFromUriResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 201))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          {
            const auto& headers = httpResponse.GetHeaders();
            auto content_md5_iterator = headers.find("content-md5");
            if (content_md5_iterator != headers.end())
            {
              ContentHash hash;
              hash.Algorithm = HashAlgorithm::Md5;
              hash.Value = Azure::Core::Convert::Base64Decode(content_md5_iterator->second);
              response.TransactionalContentHash = std::move(hash);
            }
            auto x_ms_content_crc64_iterator = headers.find("x-ms-content-crc64");
            if (x_ms_content_crc64_iterator != headers.end())
            {
              ContentHash hash;
              hash.Algorithm = HashAlgorithm::Crc64;
              hash.Value = Azure::Core::Convert::Base64Decode(x_ms_content_crc64_iterator->second);
              response.TransactionalContentHash = std::move(hash);
            }
          }
          response.SequenceNumber
              = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-sequence-number"));
          response.IsServerEncrypted
              = httpResponse.GetHeaders().at("x-ms-request-server-encrypted") == "true";
          auto x_ms_encryption_key_sha256__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
          if (x_ms_encryption_key_sha256__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionKeySha256
                = Azure::Core::Convert::Base64Decode(x_ms_encryption_key_sha256__iterator->second);
          }
          auto x_ms_encryption_scope__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-scope");
          if (x_ms_encryption_scope__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionScope = x_ms_encryption_scope__iterator->second;
          }
          return Azure::Response<UploadPageBlobPagesFromUriResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct ClearPageBlobPagesOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Core::Http::HttpRange Range;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<int64_t> IfSequenceNumberLessThanOrEqualTo;
          Azure::Nullable<int64_t> IfSequenceNumberLessThan;
          Azure::Nullable<int64_t> IfSequenceNumberEqualTo;
          Azure::Nullable<std::string> EncryptionKey;
          Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
          Azure::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
          Azure::Nullable<std::string> EncryptionScope;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct ClearPageBlobPagesOptions

        static Azure::Response<ClearPageBlobPagesResult> ClearPages(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const ClearPageBlobPagesOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "page");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          {
            std::string headerValue = "bytes=" + std::to_string(options.Range.Offset) + "-";
            if (options.Range.Length.HasValue())
            {
              headerValue
                  += std::to_string(options.Range.Offset + options.Range.Length.GetValue() - 1);
            }
            request.SetHeader("x-ms-range", std::move(headerValue));
          }
          request.SetHeader("x-ms-page-write", "clear");
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          if (options.IfSequenceNumberLessThanOrEqualTo.HasValue())
          {
            request.SetHeader(
                "x-ms-if-sequence-number-le",
                std::to_string(options.IfSequenceNumberLessThanOrEqualTo.GetValue()));
          }
          if (options.IfSequenceNumberLessThan.HasValue())
          {
            request.SetHeader(
                "x-ms-if-sequence-number-lt",
                std::to_string(options.IfSequenceNumberLessThan.GetValue()));
          }
          if (options.IfSequenceNumberEqualTo.HasValue())
          {
            request.SetHeader(
                "x-ms-if-sequence-number-eq",
                std::to_string(options.IfSequenceNumberEqualTo.GetValue()));
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.GetValue()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          ClearPageBlobPagesResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 201))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.SequenceNumber
              = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-sequence-number"));
          return Azure::Response<ClearPageBlobPagesResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct ResizePageBlobOptions
        {
          Azure::Nullable<int32_t> Timeout;
          int64_t BlobSize = -1;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<int64_t> IfSequenceNumberLessThanOrEqualTo;
          Azure::Nullable<int64_t> IfSequenceNumberLessThan;
          Azure::Nullable<int64_t> IfSequenceNumberEqualTo;
          Azure::Nullable<std::string> EncryptionKey;
          Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
          Azure::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
          Azure::Nullable<std::string> EncryptionScope;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct ResizePageBlobOptions

        static Azure::Response<ResizePageBlobResult> Resize(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const ResizePageBlobOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "properties");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.SetHeader("x-ms-blob-content-length", std::to_string(options.BlobSize));
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          if (options.IfSequenceNumberLessThanOrEqualTo.HasValue())
          {
            request.SetHeader(
                "x-ms-if-sequence-number-le",
                std::to_string(options.IfSequenceNumberLessThanOrEqualTo.GetValue()));
          }
          if (options.IfSequenceNumberLessThan.HasValue())
          {
            request.SetHeader(
                "x-ms-if-sequence-number-lt",
                std::to_string(options.IfSequenceNumberLessThan.GetValue()));
          }
          if (options.IfSequenceNumberEqualTo.HasValue())
          {
            request.SetHeader(
                "x-ms-if-sequence-number-eq",
                std::to_string(options.IfSequenceNumberEqualTo.GetValue()));
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.GetValue()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          ResizePageBlobResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.SequenceNumber
              = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-sequence-number"));
          return Azure::Response<ResizePageBlobResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct GetPageBlobPageRangesOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::string> PreviousSnapshot;
          Azure::Nullable<std::string> PreviousSnapshotUrl;
          Azure::Nullable<Azure::Core::Http::HttpRange> Range;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct GetPageBlobPageRangesOptions

        static Azure::Response<GetPageBlobPageRangesResult> GetPageRanges(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const GetPageBlobPageRangesOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter("comp", "pagelist");
          if (options.PreviousSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "prevsnapshot",
                Storage::_detail::UrlEncodeQueryParameter(options.PreviousSnapshot.GetValue()));
          }
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          if (options.Range.HasValue())
          {
            std::string headerValue
                = "bytes=" + std::to_string(options.Range.GetValue().Offset) + "-";
            if (options.Range.GetValue().Length.HasValue())
            {
              headerValue += std::to_string(
                  options.Range.GetValue().Offset + options.Range.GetValue().Length.GetValue() - 1);
            }
            request.SetHeader("x-ms-range", std::move(headerValue));
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          if (options.PreviousSnapshotUrl.HasValue())
          {
            request.SetHeader("x-ms-previous-snapshot-url", options.PreviousSnapshotUrl.GetValue());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          GetPageBlobPageRangesResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            Storage::_detail::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = GetPageBlobPageRangesResultFromXml(reader);
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.BlobSize = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-content-length"));
          return Azure::Response<GetPageBlobPageRangesResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct StartCopyPageBlobIncrementalOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string CopySource;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct StartCopyPageBlobIncrementalOptions

        static Azure::Response<Models::_detail::StartCopyPageBlobIncrementalResult>
        StartCopyIncremental(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const StartCopyPageBlobIncrementalOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "incrementalcopy");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.SetHeader("x-ms-copy-source", options.CopySource);
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::StartCopyPageBlobIncrementalResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 202))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.CopyId = httpResponse.GetHeaders().at("x-ms-copy-id");
          response.CopyStatus = CopyStatus(httpResponse.GetHeaders().at("x-ms-copy-status"));
          auto x_ms_version_id__iterator = httpResponse.GetHeaders().find("x-ms-version-id");
          if (x_ms_version_id__iterator != httpResponse.GetHeaders().end())
          {
            response.VersionId = x_ms_version_id__iterator->second;
          }
          return Azure::Response<Models::_detail::StartCopyPageBlobIncrementalResult>(
              std::move(response), std::move(pHttpResponse));
        }

      private:
        static GetPageBlobPageRangesResult GetPageBlobPageRangesResultFromXml(
            Storage::_detail::XmlReader& reader)
        {
          GetPageBlobPageRangesResult ret;
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
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::_detail::XmlNodeType::Text)
            {
            }
          }
          return ret;
        }

        static Azure::Core::Http::HttpRange ClearRangesFromXml(Storage::_detail::XmlReader& reader)
        {
          int depth = 0;
          bool is_start = false;
          bool is_end = false;
          int64_t start = 0;
          int64_t end = 0;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (
                node.Type == Storage::_detail::XmlNodeType::StartTag
                && strcmp(node.Name, "Start") == 0)
            {
              ++depth;
              is_start = true;
            }
            else if (
                node.Type == Storage::_detail::XmlNodeType::StartTag
                && strcmp(node.Name, "End") == 0)
            {
              ++depth;
              is_end = true;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
            {
              is_start = false;
              is_end = false;
              if (depth-- == 0)
              {
                break;
              }
            }
            if (depth == 1 && node.Type == Storage::_detail::XmlNodeType::Text)
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
          Azure::Core::Http::HttpRange ret;
          ret.Offset = start;
          ret.Length = end - start + 1;
          return ret;
        }

        static Azure::Core::Http::HttpRange PageRangesFromXml(Storage::_detail::XmlReader& reader)
        {
          int depth = 0;
          bool is_start = false;
          bool is_end = false;
          int64_t start = 0;
          int64_t end = 0;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::_detail::XmlNodeType::End)
            {
              break;
            }
            else if (
                node.Type == Storage::_detail::XmlNodeType::StartTag
                && strcmp(node.Name, "Start") == 0)
            {
              ++depth;
              is_start = true;
            }
            else if (
                node.Type == Storage::_detail::XmlNodeType::StartTag
                && strcmp(node.Name, "End") == 0)
            {
              ++depth;
              is_end = true;
            }
            else if (node.Type == Storage::_detail::XmlNodeType::EndTag)
            {
              is_start = false;
              is_end = false;
              if (depth-- == 0)
              {
                break;
              }
            }
            if (depth == 1 && node.Type == Storage::_detail::XmlNodeType::Text)
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
          Azure::Core::Http::HttpRange ret;
          ret.Offset = start;
          ret.Length = end - start + 1;
          return ret;
        }

      }; // class PageBlob

      class AppendBlob {
      public:
        struct CreateAppendBlobOptions
        {
          Azure::Nullable<int32_t> Timeout;
          BlobHttpHeaders HttpHeaders;
          Storage::Metadata Metadata;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<std::string> EncryptionKey;
          Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
          Azure::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
          Azure::Nullable<std::string> EncryptionScope;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct CreateAppendBlobOptions

        static Azure::Response<CreateAppendBlobResult> Create(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const CreateAppendBlobOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          if (!options.HttpHeaders.ContentType.empty())
          {
            request.SetHeader("x-ms-blob-content-type", options.HttpHeaders.ContentType);
          }
          if (!options.HttpHeaders.ContentEncoding.empty())
          {
            request.SetHeader("x-ms-blob-content-encoding", options.HttpHeaders.ContentEncoding);
          }
          if (!options.HttpHeaders.ContentLanguage.empty())
          {
            request.SetHeader("x-ms-blob-content-language", options.HttpHeaders.ContentLanguage);
          }
          if (!options.HttpHeaders.CacheControl.empty())
          {
            request.SetHeader("x-ms-blob-cache-control", options.HttpHeaders.CacheControl);
          }
          if (!Azure::Core::Convert::Base64Encode(options.HttpHeaders.ContentHash.Value).empty())
          {
            request.SetHeader(
                "x-ms-blob-content-md5",
                Azure::Core::Convert::Base64Encode(options.HttpHeaders.ContentHash.Value));
          }
          if (!options.HttpHeaders.ContentDisposition.empty())
          {
            request.SetHeader(
                "x-ms-blob-content-disposition", options.HttpHeaders.ContentDisposition);
          }
          for (const auto& pair : options.Metadata)
          {
            request.SetHeader("x-ms-meta-" + pair.first, pair.second);
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          request.SetHeader("x-ms-blob-type", "AppendBlob");
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.GetValue()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          CreateAppendBlobResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 201))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          auto x_ms_version_id__iterator = httpResponse.GetHeaders().find("x-ms-version-id");
          if (x_ms_version_id__iterator != httpResponse.GetHeaders().end())
          {
            response.VersionId = x_ms_version_id__iterator->second;
          }
          response.IsServerEncrypted
              = httpResponse.GetHeaders().at("x-ms-request-server-encrypted") == "true";
          auto x_ms_encryption_key_sha256__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
          if (x_ms_encryption_key_sha256__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionKeySha256
                = Azure::Core::Convert::Base64Decode(x_ms_encryption_key_sha256__iterator->second);
          }
          auto x_ms_encryption_scope__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-scope");
          if (x_ms_encryption_scope__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionScope = x_ms_encryption_scope__iterator->second;
          }
          return Azure::Response<CreateAppendBlobResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct AppendBlockOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<ContentHash> TransactionalContentHash;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<int64_t> MaxSize;
          Azure::Nullable<int64_t> AppendPosition;
          Azure::Nullable<std::string> EncryptionKey;
          Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
          Azure::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
          Azure::Nullable<std::string> EncryptionScope;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct AppendBlockOptions

        static Azure::Response<AppendBlockResult> AppendBlock(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            Azure::Core::IO::BodyStream* requestBody,
            const AppendBlockOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request
              = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, requestBody);
          request.SetHeader("Content-Length", std::to_string(requestBody->Length()));
          request.GetUrl().AppendQueryParameter("comp", "appendblock");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          if (options.TransactionalContentHash.HasValue())
          {
            if (options.TransactionalContentHash.GetValue().Algorithm == HashAlgorithm::Md5)
            {
              request.SetHeader(
                  "Content-MD5",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.GetValue().Value));
            }
            else if (options.TransactionalContentHash.GetValue().Algorithm == HashAlgorithm::Crc64)
            {
              request.SetHeader(
                  "x-ms-content-crc64",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.GetValue().Value));
            }
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          if (options.MaxSize.HasValue())
          {
            request.SetHeader(
                "x-ms-blob-condition-maxsize", std::to_string(options.MaxSize.GetValue()));
          }
          if (options.AppendPosition.HasValue())
          {
            request.SetHeader(
                "x-ms-blob-condition-appendpos", std::to_string(options.AppendPosition.GetValue()));
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.GetValue()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          AppendBlockResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 201))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          {
            const auto& headers = httpResponse.GetHeaders();
            auto content_md5_iterator = headers.find("content-md5");
            if (content_md5_iterator != headers.end())
            {
              ContentHash hash;
              hash.Algorithm = HashAlgorithm::Md5;
              hash.Value = Azure::Core::Convert::Base64Decode(content_md5_iterator->second);
              response.TransactionalContentHash = std::move(hash);
            }
            auto x_ms_content_crc64_iterator = headers.find("x-ms-content-crc64");
            if (x_ms_content_crc64_iterator != headers.end())
            {
              ContentHash hash;
              hash.Algorithm = HashAlgorithm::Crc64;
              hash.Value = Azure::Core::Convert::Base64Decode(x_ms_content_crc64_iterator->second);
              response.TransactionalContentHash = std::move(hash);
            }
          }
          response.AppendOffset
              = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-append-offset"));
          response.CommittedBlockCount
              = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-committed-block-count"));
          response.IsServerEncrypted
              = httpResponse.GetHeaders().at("x-ms-request-server-encrypted") == "true";
          auto x_ms_encryption_key_sha256__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
          if (x_ms_encryption_key_sha256__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionKeySha256
                = Azure::Core::Convert::Base64Decode(x_ms_encryption_key_sha256__iterator->second);
          }
          auto x_ms_encryption_scope__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-scope");
          if (x_ms_encryption_scope__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionScope = x_ms_encryption_scope__iterator->second;
          }
          return Azure::Response<AppendBlockResult>(std::move(response), std::move(pHttpResponse));
        }

        struct AppendBlockFromUriOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string SourceUri;
          Azure::Nullable<Azure::Core::Http::HttpRange> SourceRange;
          Azure::Nullable<ContentHash> TransactionalContentHash;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<int64_t> MaxSize;
          Azure::Nullable<int64_t> AppendPosition;
          Azure::Nullable<std::string> EncryptionKey;
          Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
          Azure::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
          Azure::Nullable<std::string> EncryptionScope;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct AppendBlockFromUriOptions

        static Azure::Response<AppendBlockFromUriResult> AppendBlockFromUri(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const AppendBlockFromUriOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "appendblock");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.SetHeader("x-ms-copy-source", options.SourceUri);
          if (options.SourceRange.HasValue())
          {
            std::string headerValue
                = "bytes=" + std::to_string(options.SourceRange.GetValue().Offset) + "-";
            if (options.SourceRange.GetValue().Length.HasValue())
            {
              headerValue += std::to_string(
                  options.SourceRange.GetValue().Offset
                  + options.SourceRange.GetValue().Length.GetValue() - 1);
            }
            request.SetHeader("x-ms-source-range", std::move(headerValue));
          }
          if (options.TransactionalContentHash.HasValue())
          {
            if (options.TransactionalContentHash.GetValue().Algorithm == HashAlgorithm::Md5)
            {
              request.SetHeader(
                  "x-ms-source-content-md5",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.GetValue().Value));
            }
            else if (options.TransactionalContentHash.GetValue().Algorithm == HashAlgorithm::Crc64)
            {
              request.SetHeader(
                  "x-ms-source-content-crc64",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.GetValue().Value));
            }
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          if (options.MaxSize.HasValue())
          {
            request.SetHeader(
                "x-ms-blob-condition-maxsize", std::to_string(options.MaxSize.GetValue()));
          }
          if (options.AppendPosition.HasValue())
          {
            request.SetHeader(
                "x-ms-blob-condition-appendpos", std::to_string(options.AppendPosition.GetValue()));
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.GetValue());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.GetValue()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.GetValue().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.GetValue());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          AppendBlockFromUriResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 201))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          {
            const auto& headers = httpResponse.GetHeaders();
            auto content_md5_iterator = headers.find("content-md5");
            if (content_md5_iterator != headers.end())
            {
              ContentHash hash;
              hash.Algorithm = HashAlgorithm::Md5;
              hash.Value = Azure::Core::Convert::Base64Decode(content_md5_iterator->second);
              response.TransactionalContentHash = std::move(hash);
            }
            auto x_ms_content_crc64_iterator = headers.find("x-ms-content-crc64");
            if (x_ms_content_crc64_iterator != headers.end())
            {
              ContentHash hash;
              hash.Algorithm = HashAlgorithm::Crc64;
              hash.Value = Azure::Core::Convert::Base64Decode(x_ms_content_crc64_iterator->second);
              response.TransactionalContentHash = std::move(hash);
            }
          }
          response.AppendOffset
              = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-append-offset"));
          response.CommittedBlockCount
              = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-committed-block-count"));
          response.IsServerEncrypted
              = httpResponse.GetHeaders().at("x-ms-request-server-encrypted") == "true";
          auto x_ms_encryption_key_sha256__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-key-sha256");
          if (x_ms_encryption_key_sha256__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionKeySha256
                = Azure::Core::Convert::Base64Decode(x_ms_encryption_key_sha256__iterator->second);
          }
          auto x_ms_encryption_scope__iterator
              = httpResponse.GetHeaders().find("x-ms-encryption-scope");
          if (x_ms_encryption_scope__iterator != httpResponse.GetHeaders().end())
          {
            response.EncryptionScope = x_ms_encryption_scope__iterator->second;
          }
          return Azure::Response<AppendBlockFromUriResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct SealAppendBlobOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
          Azure::Nullable<int64_t> AppendPosition;
        }; // struct SealAppendBlobOptions

        static Azure::Response<SealAppendBlobResult> Seal(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const SealAppendBlobOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "seal");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.GetValue());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.GetValue().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.GetValue().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfMatch.HasValue() && !options.IfMatch.ToString().empty())
          {
            request.SetHeader("If-Match", options.IfMatch.ToString());
          }
          if (options.IfNoneMatch.HasValue() && !options.IfNoneMatch.ToString().empty())
          {
            request.SetHeader("If-None-Match", options.IfNoneMatch.ToString());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.GetValue());
          }
          if (options.AppendPosition.HasValue())
          {
            request.SetHeader(
                "x-ms-blob-condition-appendpos", std::to_string(options.AppendPosition.GetValue()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          SealAppendBlobResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 200))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          return Azure::Response<SealAppendBlobResult>(
              std::move(response), std::move(pHttpResponse));
        }

      private:
      }; // class AppendBlob

      class BlobBatch {
      public:
        struct SubmitBlobBatchOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string ContentType;
        }; // struct SubmitBlobBatchOptions

        static Azure::Response<Models::_detail::SubmitBlobBatchResult> SubmitBatch(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            Azure::Core::IO::BodyStream* requestBody,
            const SubmitBlobBatchOptions& options,
            const Azure::Core::Context& context)
        {
          (void)options;
          auto request
              = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Post, url, requestBody);
          request.SetHeader("Content-Length", std::to_string(requestBody->Length()));
          request.GetUrl().AppendQueryParameter("comp", "batch");
          request.SetHeader("x-ms-version", "2020-02-10");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.GetValue()));
          }
          request.SetHeader("Content-Type", options.ContentType);
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::SubmitBlobBatchResult response;
          auto http_status_code
              = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  httpResponse.GetStatusCode());
          if (!(http_status_code == 202))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.RequestId = httpResponse.GetHeaders().at("x-ms-request-id");
          response.ContentType = httpResponse.GetHeaders().at("content-type");
          return Azure::Response<Models::_detail::SubmitBlobBatchResult>(
              std::move(response), std::move(pHttpResponse));
        }

      private:
      }; // class BlobBatch

    }; // class BlobRestClient

  } // namespace _detail

}}} // namespace Azure::Storage::Blobs
