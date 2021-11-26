// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include <azure/core/context.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/etag.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/response.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/xml_wrapper.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_exception.hpp>

#include "azure/storage/blobs/dll_import_export.hpp"

/* cspell:ignore xmsblobsequencenumber */

namespace Azure { namespace Storage { namespace Blobs {
  namespace Models {

    /**
     * @brief Extensible enum that used to identify the account kind.
     */
    class AccountKind final {
    public:
      AccountKind() = default;
      explicit AccountKind(std::string value) : m_value(std::move(value)) {}
      bool operator==(const AccountKind& other) const { return m_value == other.m_value; }
      bool operator!=(const AccountKind& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      /**
       * General-purpose v1 account.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccountKind Storage;
      /**
       * Blob Storage account.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccountKind BlobStorage;
      /**
       * General-purpose v2 account.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccountKind StorageV2;
      /**
       * File Storage account.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccountKind FileStorage;
      /**
       * Block Blob Storage account.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccountKind BlockBlobStorage;

    private:
      std::string m_value;
    }; // extensible enum AccountKind

    /**
     * @brief Describes a single block in block blob.
     */
    struct BlobBlock final
    {
      /**
       * Base64 encoded block ID.
       */
      std::string Name;
      /**
       * Block size in bytes.
       */
      int64_t Size = 0;
    }; // struct BlobBlock

    /**
     * @brief Extensible enum used to identify copy status of a copy operation.
     */
    class CopyStatus final {
    public:
      CopyStatus() = default;
      explicit CopyStatus(std::string value) : m_value(std::move(value)) {}
      bool operator==(const CopyStatus& other) const { return m_value == other.m_value; }
      bool operator!=(const CopyStatus& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      /**
       * Successful.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static CopyStatus Success;
      /**
       * Pending.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static CopyStatus Pending;

    private:
      std::string m_value;
    }; // extensible enum CopyStatus

    /**
     * @brief Extensible enum used to identify the status of secondary storage endpoint.
     */
    class GeoReplicationStatus final {
    public:
      GeoReplicationStatus() = default;
      explicit GeoReplicationStatus(std::string value) : m_value(std::move(value)) {}
      bool operator==(const GeoReplicationStatus& other) const { return m_value == other.m_value; }
      bool operator!=(const GeoReplicationStatus& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      /**
       * The secondary location is active and operational.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static GeoReplicationStatus Live;
      /**
       * Initial synchronization from the primary location to the secondary location is in progress.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static GeoReplicationStatus Bootstrap;
      /**
       * The secondary location is temporarily unavailable.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static GeoReplicationStatus Unavailable;

    private:
      std::string m_value;
    }; // extensible enum GeoReplicationStatus

    /**
     * @brief Extensible enum used to identify the lease is of inifinite or fixed duration.
     */
    class LeaseDurationType final {
    public:
      LeaseDurationType() = default;
      explicit LeaseDurationType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const LeaseDurationType& other) const { return m_value == other.m_value; }
      bool operator!=(const LeaseDurationType& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      /**
       * Infinite duration.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static LeaseDurationType Infinite;
      /**
       * Fixed duration.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static LeaseDurationType Fixed;

    private:
      std::string m_value;
    }; // extensible enum LeaseDurationType

    /**
     * @brief Extensible enum used to identify the state of lease.
     */
    class LeaseState final {
    public:
      LeaseState() = default;
      explicit LeaseState(std::string value) : m_value(std::move(value)) {}
      bool operator==(const LeaseState& other) const { return m_value == other.m_value; }
      bool operator!=(const LeaseState& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      /**
       * The lease is unlocked and can be acquired.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static LeaseState Available;
      /**
       * The lease is locked.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static LeaseState Leased;
      /**
       * The lease duration has expired.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static LeaseState Expired;
      /**
       * The lease has been broken, but the lease will continue to be locked until the break period
       * has expired.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static LeaseState Breaking;
      /**
       * The lease has been broken, and the break period has expired.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static LeaseState Broken;

    private:
      std::string m_value;
    }; // extensible enum LeaseState

    /**
     * @brief Extensible enum used to identify the status of lease.
     */
    class LeaseStatus final {
    public:
      LeaseStatus() = default;
      explicit LeaseStatus(std::string value) : m_value(std::move(value)) {}
      bool operator==(const LeaseStatus& other) const { return m_value == other.m_value; }
      bool operator!=(const LeaseStatus& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      /**
       * The lease is locked.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static LeaseStatus Locked;
      /**
       * The lease is unlocked.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static LeaseStatus Unlocked;

    private:
      std::string m_value;
    }; // extensible enum LeaseStatus

    /**
     * @brief Extensible enum used to identify object replication status.
     */
    class ObjectReplicationStatus final {
    public:
      ObjectReplicationStatus() = default;
      explicit ObjectReplicationStatus(std::string value) : m_value(std::move(value)) {}
      bool operator==(const ObjectReplicationStatus& other) const
      {
        return m_value == other.m_value;
      }
      bool operator!=(const ObjectReplicationStatus& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      /**
       * Object replication to the destination container completed.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static ObjectReplicationStatus Complete;
      /**
       * Object replication to the destination container failed.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static ObjectReplicationStatus Failed;

    private:
      std::string m_value;
    }; // extensible enum ObjectReplicationStatus

    /**
     * @brief Extensible enum used to indicates whether data in the container may be accessed
     * publicly and the level of access.
     */
    class PublicAccessType final {
    public:
      PublicAccessType() = default;
      explicit PublicAccessType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const PublicAccessType& other) const { return m_value == other.m_value; }
      bool operator!=(const PublicAccessType& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      /**
       * Indicates full public read access for container and blob data. Clients can enumerate blobs
       * within the container via anonymous request, but cannot enumerate containers within the
       * storage account.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static PublicAccessType BlobContainer;
      /**
       * Indicates public read access for blobs. Blob data within this container can be read via
       * anonymous request, but container data is not available. Clients cannot enumerate blobs
       * within the container via anonymous request.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static PublicAccessType Blob;
      /**
       * Indicates the container is private to the account owner.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static PublicAccessType None;

    private:
      std::string m_value;
    }; // extensible enum PublicAccessType

    /**
     * @brief Determines how long the associated data should persist.
     */
    struct RetentionPolicy final
    {
      /**
       * Indicates whether this retention policy is enabled.
       */
      bool IsEnabled = false;
      /**
       * Indicates the number of days that metrics or logging or soft-deleted data should be
       * retained.
       */
      Azure::Nullable<int32_t> Days;
    }; // struct RetentionPolicy

    /**
     * @brief Describes how you reference an ACL in a blob container.
     */
    struct SignedIdentifier final
    {
      /**
       * A unique ID for this signed identifier.
       */
      std::string Id;
      /**
       * Date and time since when this policy is active.
       */
      Azure::Nullable<Azure::DateTime> StartsOn;
      /**
       * Date and time the policy expires.
       */
      Azure::Nullable<Azure::DateTime> ExpiresOn;
      /**
       * The permissions for this ACL policy.
       */
      std::string Permissions;
    }; // struct SignedIdentifier

    /**
     * @brief Extensible enum that used to identify the sku of a storage account.
     */
    class SkuName final {
    public:
      SkuName() = default;
      explicit SkuName(std::string value) : m_value(std::move(value)) {}
      bool operator==(const SkuName& other) const { return m_value == other.m_value; }
      bool operator!=(const SkuName& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      /**
       * Standard Locally Redundant Storage
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static SkuName StandardLrs;
      /**
       * Standard Geo Replicated Storage
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static SkuName StandardGrs;
      /**
       * Standard Read-access Geo Replicated Storage
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static SkuName StandardRagrs;
      /**
       * Standard Zone Redundant Storage
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static SkuName StandardZrs;
      /**
       * Provisioned IO Locally Redundant Storage
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static SkuName PremiumLrs;
      /**
       * Provisioned IO Zone Redundant Storage
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static SkuName PremiumZrs;
      /**
       * Standard Geo-zone-redundant Storage
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static SkuName StandardGzrs;
      /**
       * Standard Read-access Geo-zone-redundant Storage
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static SkuName StandardRagzrs;

    private:
      std::string m_value;
    }; // extensible enum SkuName

    /**
     * @brief Blob information from the result of
     * #Azure::Storage::Blobs::BlobServiceClient::FindBlobsByTags.
     */
    struct TaggedBlobItem final
    {
      /**
       * Blob name.
       */
      std::string BlobName;
      /**
       * Blob container name.
       */
      std::string BlobContainerName;
      /**
       * Matched tags
       */
      std::map<std::string, std::string> Tags;
    }; // struct TaggedBlobItem

    /**
     * @brief Azure analytics logging settings.
     */
    struct AnalyticsLogging final
    {
      /**
       * The version of storage analytics to configure.
       */
      std::string Version;
      /**
       * Whether all delete requests should be logged.
       */
      bool Delete = false;
      /**
       * Whether all read requests should be logged.
       */
      bool Read = false;
      /**
       * Whether all write requests should be logged.
       */
      bool Write = false;
      /**
       * Determines how long the data should be persist.
       */
      Models::RetentionPolicy RetentionPolicy;
    }; // struct AnalyticsLogging

    /**
     * @brief Detailed information of a blob container.
     */
    struct BlobContainerItemDetails final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
      /**
       * A set of name-value pairs associated with this blob as user-defined metadata.
       */
      Storage::Metadata Metadata;
      /**
       * Indicates whether data in the container may be accessed publicly and the level of access.
       */
      PublicAccessType AccessType = PublicAccessType::None;
      /**
       * Indicates whether the container has an immutability policy set on it.
       */
      bool HasImmutabilityPolicy = false;
      /**
       * Indicates whether the container has a legal hold.
       */
      bool HasLegalHold = false;
      /**
       * Indicates whether the lease is of infinite or fixed duration when the blob or container is
       * leased. This value is null if the blob or container is not leased.
       */
      Azure::Nullable<LeaseDurationType> LeaseDuration;
      /**
       * Lease state of the blob.
       */
      Models::LeaseState LeaseState = Models::LeaseState::Available;
      /**
       * The current lease status of the blob.
       */
      Models::LeaseStatus LeaseStatus = Models::LeaseStatus::Unlocked;
      /**
       * The default encryption scope for the container.
       */
      std::string DefaultEncryptionScope = "$account-encryption-key";
      /**
       * Indicates whether the container's default encryption scope can be overridden.
       */
      bool PreventEncryptionScopeOverride = false;
      /**
       * Remaining days before this container will be permanantely deleted. Only valid when this
       * container was deleted.
       */
      Azure::Nullable<int32_t> RemainingRetentionDays;
      /**
       * Data and time at which this container was deleted. Only valid when this container was
       * deleted.
       */
      Azure::Nullable<Azure::DateTime> DeletedOn;
    }; // struct BlobContainerItemDetails

    /**
     * @brief Settings for a CORS rule.
     */
    struct CorsRule final
    {
      /**
       * A comma-separated list of origin domains that are allowed via CORS, or "*" if all domains
       * are allowed.
       */
      std::string AllowedOrigins;
      /**
       * A comma-separated list of HTTP methods that are allowed to be executed by the origin. For
       * Azure Storage, permitted methods are DELETE, GET, HEAD, MERGE, POST, OPTIONS or PUT.
       */
      std::string AllowedMethods;
      /**
       * A comma-separated list of headers allowed to be part of the cross-origin request.
       */
      std::string AllowedHeaders;
      /**
       * A comma-separated list of response headers to expose to CORS clients.
       */
      std::string ExposedHeaders;
      /**
       * The number of seconds that the client/browser should cache a preflight response.
       */
      int32_t MaxAgeInSeconds = 0;
    }; // struct CorsRule

    /**
     * @brief Geo-replication information for the secondary storage endpoint.
     */
    struct GeoReplication final
    {
      /**
       * Status of the secondary storage endpoint.
       */
      GeoReplicationStatus Status;
      /**
       * All primary writes preceding this value are guaranteed to be available for read operations
       * at the secondary. Primary writes after this point in time may or may not be available for
       * reads. This value may be null if replication status is bootstrap or unavailable.
       */
      Azure::Nullable<Azure::DateTime> LastSyncedOn;
    }; // struct GeoReplication

    /**
     * @brief Summary of request statistics grouped by API in hour or minute aggregates for blobs.
     */
    struct Metrics final
    {
      /**
       * The version of storage analytics to configure.
       */
      std::string Version;
      /**
       * Indicates whether metrics are enabled for blob service.
       */
      bool IsEnabled = false;
      /**
       * Determines how long the metrics data should persist.
       */
      Models::RetentionPolicy RetentionPolicy;
      /**
       * Indicates whether metrics should generate summary statistics for called API operations.
       */
      Azure::Nullable<bool> IncludeApis;
    }; // struct Metrics

    /**
     * @brief Contains the object replication rule ID and replication status of a blob.
     */
    struct ObjectReplicationRule final
    {
      /**
       * Rule ID.
       */
      std::string RuleId;
      /**
       * Replication status.
       */
      ObjectReplicationStatus ReplicationStatus;
    }; // struct ObjectReplicationRule

    /**
     * @brief The properties that enable a storage account to host a static website.
     */
    struct StaticWebsite final
    {
      /**
       * Indicates whether this storage account is hosting a static website.
       */
      bool IsEnabled = false;
      /**
       * The default name of the index page under each directory.
       */
      Azure::Nullable<std::string> IndexDocument;
      /**
       * Absolute path of the default index page.
       */
      Azure::Nullable<std::string> DefaultIndexDocumentPath;
      /**
       * The absolute path of the custom 404 page.
       */
      Azure::Nullable<std::string> ErrorDocument404Path;
    }; // struct StaticWebsite

    /**
     * @brief Extensible enum used to identify access tier of a blob.
     */
    class AccessTier final {
    public:
      AccessTier() = default;
      explicit AccessTier(std::string value) : m_value(std::move(value)) {}
      bool operator==(const AccessTier& other) const { return m_value == other.m_value; }
      bool operator!=(const AccessTier& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      /**
       * P1 tier for page blob in a premium storage account.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P1;
      /**
       * P2 tier for page blob in a premium storage account.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P2;
      /**
       * P3 tier for page blob in a premium storage account.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P3;
      /**
       * P4 tier for page blob in a premium storage account.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P4;
      /**
       * P6 tier for page blob in a premium storage account.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P6;
      /**
       * P10 tier for page blob in a premium storage account.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P10;
      /**
       * P15 tier for page blob in a premium storage account.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P15;
      /**
       * P20 tier for page blob in a premium storage account.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P20;
      /**
       * P30 tier for page blob in a premium storage account.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P30;
      /**
       * P40 tier for page blob in a premium storage account.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P40;
      /**
       * P50 tier for page blob in a premium storage account.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P50;
      /**
       * P60 tier for page blob in a premium storage account.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P60;
      /**
       * P70 tier for page blob in a premium storage account.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P70;
      /**
       * P80 tier for page blob in a premium storage account.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier P80;
      /**
       * Optimized for storing data that is accessed frequently.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier Hot;
      /**
       * Optimized for storing data that is infrequently accessed and stored for at least 30 days.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier Cool;
      /**
       * Optimized for storing data that is rarely accessed and stored for at least 180 days with
       * flexible latency requirements, on the order of hours.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static AccessTier Archive;

    private:
      std::string m_value;
    }; // extensible enum AccessTier

    /**
     * @brief Extensible enum used to identify the destination tier when a blob is being rehydrated
     * and is not complete.
     */
    class ArchiveStatus final {
    public:
      ArchiveStatus() = default;
      explicit ArchiveStatus(std::string value) : m_value(std::move(value)) {}
      bool operator==(const ArchiveStatus& other) const { return m_value == other.m_value; }
      bool operator!=(const ArchiveStatus& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      /**
       * The blob is being rehydrated to hot tier.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static ArchiveStatus RehydratePendingToHot;
      /**
       * The blob is being rehydrated to cool tier.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static ArchiveStatus RehydratePendingToCool;

    private:
      std::string m_value;
    }; // extensible enum ArchiveStatus

    /**
     * @brief A container item from the result of
     * #Azure::Storage::Blobs::BlobServiceClient::ListBlobContainers.
     */
    struct BlobContainerItem final
    {
      /**
       * Blob container name.
       */
      std::string Name;
      /**
       * Indicates whether this container was deleted.
       */
      bool IsDeleted = false;
      /**
       * Version ID of a deleted container. This is null if a container is not deleted.
       */
      Azure::Nullable<std::string> VersionId;
      /**
       * Detailed information of this container item.
       */
      BlobContainerItemDetails Details;
    }; // struct BlobContainerItem

    /**
     * @brief Standard HTTP properties supported by containers and blobs.
     */
    struct BlobHttpHeaders final
    {
      /**
       * MIME content type of the blob.
       */
      std::string ContentType;
      /**
       * Specifies which content encodings have been applied to the blob.
       */
      std::string ContentEncoding;
      /**
       * Specifies the natural languages used by this blob.
       */
      std::string ContentLanguage;
      /**
       * A hash of the blob content.
       */
      Storage::ContentHash ContentHash;
      /**
       * Specifies directives for caching mechanisms.
       */
      std::string CacheControl;
      /**
       * Conveys additional information about how to process the resource payload, and also can be
       * used to attach additional metadata.
       */
      std::string ContentDisposition;
    }; // struct BlobHttpHeaders

    /**
     * @brief Extensible enum used to identify blob type.
     */
    class BlobType final {
    public:
      BlobType() = default;
      explicit BlobType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const BlobType& other) const { return m_value == other.m_value; }
      bool operator!=(const BlobType& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      /**
       * Block blob.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlobType BlockBlob;
      /**
       * Page blob.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlobType PageBlob;
      /**
       * Append blob.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlobType AppendBlob;

    private:
      std::string m_value;
    }; // extensible enum BlobType

    /**
     * @brief Contains object replication policy ID and the respective list of
     * #ObjectReplicationRule s. This is used when retrieving the object replication properties on
     * the source blob. The policy id for the destination blob is set in
     * ObjectReplicationDestinationPolicyId of the respective method responses.
     */
    struct ObjectReplicationPolicy final
    {
      /**
       * The policy ID.
       */
      std::string PolicyId;
      /**
       * The Rule IDs and respective replication status that are under the policy ID.
       */
      std::vector<ObjectReplicationRule> Rules;
    }; // struct ObjectReplicationPolicy

    /**
     * @brief Extensible enum used to identify rehydrate priority.
     */
    class RehydratePriority final {
    public:
      RehydratePriority() = default;
      explicit RehydratePriority(std::string value) : m_value(std::move(value)) {}
      bool operator==(const RehydratePriority& other) const { return m_value == other.m_value; }
      bool operator!=(const RehydratePriority& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      /**
       * High priority.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static RehydratePriority High;
      /**
       * Standard priority.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static RehydratePriority Standard;

    private:
      std::string m_value;
    }; // extensible enum RehydratePriority

    /**
     * @brief Detailed information of a blob.
     */
    struct BlobItemDetails final
    {
      /**
       * System properties of the blob.
       */
      BlobHttpHeaders HttpHeaders;
      /**
       * A set of name-value pairs associated with this blob as user-defined metadata.
       */
      Storage::Metadata Metadata;
      /**
       * The date and time at which the blob was created.
       */
      Azure::DateTime CreatedOn;
      /**
       * The time this blob will expire.
       */
      Azure::Nullable<Azure::DateTime> ExpiresOn;
      /**
       * The date and time at which the blob was last read or written to.
       */
      Azure::Nullable<Azure::DateTime> LastAccessedOn;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The tier of page blob or block blob.
       */
      Azure::Nullable<Models::AccessTier> AccessTier;
      /**
       * True if the access tier is not explicitly set on the blob.
       */
      Azure::Nullable<bool> IsAccessTierInferred;
      /**
       * The date and time the tier was changed on the object.
       */
      Azure::Nullable<Azure::DateTime> AccessTierChangedOn;
      /**
       * Indicates if the blob is being rehydrated.
       */
      Azure::Nullable<Models::ArchiveStatus> ArchiveStatus;
      /**
       * Priority of rehydrate if the blob is being rehydrated.
       */
      Azure::Nullable<Models::RehydratePriority> RehydratePriority;
      /**
       * The current lease status of the blob.
       */
      Models::LeaseStatus LeaseStatus = Models::LeaseStatus::Unlocked;
      /**
       * Lease state of the blob.
       */
      Models::LeaseState LeaseState = Models::LeaseState::Available;
      /**
       * Indicates whether the lease is of infinite or fixed duration when the blob or container is
       * leased. This value is null if the blob or container is not leased.
       */
      Azure::Nullable<LeaseDurationType> LeaseDuration;
      /**
       * True if the blob data and metadata are completely encrypted using the specified algorithm.
       * Otherwise, the value is set to false (when the blob is unencrypted, or if only parts of the
       * blob/application metadata are encrypted).
       */
      bool IsServerEncrypted = false;
      /**
       * The SHA-256 hash of the encryption key used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      /**
       * Name of the encryption scope used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::string> EncryptionScope;
      /**
       * The current sequence number for a page blob. This value is null for block blobs or append
       * blobs.
       */
      Azure::Nullable<int64_t> SequenceNumber;
      /**
       * If the blob has been sealed. This value is null for block blobs or page blobs.
       */
      Azure::Nullable<bool> IsSealed;
      /**
       * Only valid when Object Replication is enabled and current blob is the source.
       */
      std::vector<ObjectReplicationPolicy> ObjectReplicationSourceProperties;
      /**
       * String identifier for the last attempted Copy Blob operation where this blob was the
       * destination. This value is null if this blob has never been the destination of a copy
       * operation, or if this blob has been modified after a concluded copy operation.
       */
      Azure::Nullable<std::string> CopyId;
      /**
       * URL that specifies the source blob or file used in the last attempted copy operation where
       * this blob was the destination blob. This value is null if this blob has never been the
       * destination of a copy operation, or if this blob has been modified after a concluded copy
       * operation.
       */
      Azure::Nullable<std::string> CopySource;
      /**
       * State of the copy operation identified by the copy ID. Possible values include success,
       * pending, aborted, failed etc. This value is null if this blob has never been the
       * destination of a copy operation, or if this blob has been modified after a concluded copy
       * operation.
       */
      Azure::Nullable<Models::CopyStatus> CopyStatus;
      /**
       * Describes the cause of the last fatal or non-fatal copy operation failure. This is not null
       * only when copy status is failed or pending.
       */
      Azure::Nullable<std::string> CopyStatusDescription;
      /**
       * True if the copy operation is incremental copy.
       */
      Azure::Nullable<bool> IsIncrementalCopy;
      /**
       * Snapshot time of the last successful incremental copy snapshot for this blob.
       */
      Azure::Nullable<std::string> IncrementalCopyDestinationSnapshot;
      /**
       * Contains the number of bytes copied and the total bytes in the source in the last attempted
       * copy operation where this blob was the destination blob.
       */
      Azure::Nullable<std::string> CopyProgress;
      /**
       * Conclusion time of the last attempted copy operation where this blob was the destination
       * blob.
       */
      Azure::Nullable<Azure::DateTime> CopyCompletedOn;
      /**
       * User-defined tags for this blob.
       */
      std::map<std::string, std::string> Tags;
      /**
       * Data and time at which this blob was deleted. Only valid when this blob was deleted.
       */
      Azure::Nullable<Azure::DateTime> DeletedOn;
      /**
       * Remaining days before this blob will be permanantely deleted. Only valid when this blob was
       * deleted.
       */
      Azure::Nullable<int32_t> RemainingRetentionDays;
    }; // struct BlobItemDetails

    /**
     * @brief Detailed information of a downloaded blob.
     */
    struct DownloadBlobDetails final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
      /**
       * The date and time at which the blob was created.
       */
      Azure::DateTime CreatedOn;
      /**
       * The time this blob will expire.
       */
      Azure::Nullable<Azure::DateTime> ExpiresOn;
      /**
       * The date and time at which the blob was last read or written to.
       */
      Azure::Nullable<Azure::DateTime> LastAccessedOn;
      /**
       * System properties of the blob.
       */
      BlobHttpHeaders HttpHeaders;
      /**
       * A set of name-value pairs associated with this blob as user-defined metadata.
       */
      Storage::Metadata Metadata;
      /**
       * The current sequence number for a page blob. This value is null for block blobs or append
       * blobs.
       */
      Azure::Nullable<int64_t> SequenceNumber;
      /**
       * The number of committed blocks present in the blob. This value is null for block blobs or
       * page blobs.
       */
      Azure::Nullable<int32_t> CommittedBlockCount;
      /**
       * If the blob has been sealed. This value is null for block blobs or page blobs.
       */
      Azure::Nullable<bool> IsSealed;
      /**
       * Indicates whether the lease is of infinite or fixed duration when the blob or container is
       * leased. This value is null if the blob or container is not leased.
       */
      Azure::Nullable<LeaseDurationType> LeaseDuration;
      /**
       * Lease state of the blob.
       */
      Azure::Nullable<Models::LeaseState> LeaseState;
      /**
       * The current lease status of the blob.
       */
      Azure::Nullable<Models::LeaseStatus> LeaseStatus;
      /**
       * True if the blob data and metadata are completely encrypted using the specified algorithm.
       * Otherwise, the value is set to false (when the blob is unencrypted, or if only parts of the
       * blob/application metadata are encrypted).
       */
      bool IsServerEncrypted = false;
      /**
       * The SHA-256 hash of the encryption key used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      /**
       * Name of the encryption scope used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::string> EncryptionScope;
      /**
       * Only valid when Object Replication is enabled and current blob is the destination.
       */
      Azure::Nullable<std::string> ObjectReplicationDestinationPolicyId;
      /**
       * Only valid when Object Replication is enabled and current blob is the source.
       */
      std::vector<ObjectReplicationPolicy> ObjectReplicationSourceProperties;
      /**
       * the number of tags stored on the blob.
       */
      Azure::Nullable<int32_t> TagCount;
      /**
       * String identifier for the last attempted Copy Blob operation where this blob was the
       * destination. This value is null if this blob has never been the destination of a copy
       * operation, or if this blob has been modified after a concluded copy operation.
       */
      Azure::Nullable<std::string> CopyId;
      /**
       * URL that specifies the source blob or file used in the last attempted copy operation where
       * this blob was the destination blob. This value is null if this blob has never been the
       * destination of a copy operation, or if this blob has been modified after a concluded copy
       * operation.
       */
      Azure::Nullable<std::string> CopySource;
      /**
       * State of the copy operation identified by the copy ID. Possible values include success,
       * pending, aborted, failed etc. This value is null if this blob has never been the
       * destination of a copy operation, or if this blob has been modified after a concluded copy
       * operation.
       */
      Azure::Nullable<Models::CopyStatus> CopyStatus;
      /**
       * Describes the cause of the last fatal or non-fatal copy operation failure. This is not null
       * only when copy status is failed or pending.
       */
      Azure::Nullable<std::string> CopyStatusDescription;
      /**
       * Contains the number of bytes copied and the total bytes in the source in the last attempted
       * copy operation where this blob was the destination blob.
       */
      Azure::Nullable<std::string> CopyProgress;
      /**
       * Conclusion time of the last attempted copy operation where this blob was the destination
       * blob.
       */
      Azure::Nullable<Azure::DateTime> CopyCompletedOn;
      /**
       * A string value that uniquely identifies the blob. This value is null if Blob Versioning is
       * not enabled.
       */
      Azure::Nullable<std::string> VersionId;
      /**
       * Indicates if this is the current version of the blob. This value is null if Blob Versioning
       * is not enabled.
       */
      Azure::Nullable<bool> IsCurrentVersion;
    }; // struct DownloadBlobDetails

    /**
     * @brief A blob item from the result of #Azure::Storage::Blobs::BlobContainerClient::ListBlobs.
     */
    struct BlobItem final
    {
      std::string Name;
      /**
       * Size of the blob.
       */
      int64_t BlobSize = 0;
      /**
       * The blob's type.
       */
      Models::BlobType BlobType;
      /**
       * Indicates whether this blob was deleted.
       */
      bool IsDeleted = false;
      /**
       * A string value that uniquely identifies a blob snapshot.
       */
      std::string Snapshot;
      /**
       * A string value that uniquely identifies the blob. This value is null if Blob Versioning is
       * not enabled.
       */
      Azure::Nullable<std::string> VersionId;
      /**
       * Indicates if this is the current version of the blob. This value is null if Blob Versioning
       * is not enabled.
       */
      Azure::Nullable<bool> IsCurrentVersion;
      /**
       * Detailed information of the downloaded blob.
       */
      BlobItemDetails Details;
    }; // struct BlobItem

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlobClient::AbortCopy.
     */
    struct AbortBlobCopyFromUriResult final
    {
    }; // struct AbortBlobCopyFromUriResult

    /**
     * @brief Storage account information.
     */
    struct AccountInfo final
    {
      /**
       * The SKU name of the storage account. See
       * https://docs.microsoft.com/rest/api/storagerp/srp_sku_types for more information.
       */
      Models::SkuName SkuName;
      /**
       * The account kind of the storage account. See
       * https://docs.microsoft.com/rest/api/storagerp/srp_sku_types for more information.
       */
      Models::AccountKind AccountKind;
      /**
       * Indicates if the account has a hierarchical namespace enabled.
       */
      bool IsHierarchicalNamespaceEnabled = false;
    }; // struct AccountInfo

    /**
     * @brief Response type for #Azure::Storage::Blobs::AppendBlobClient::AppendBlockFromUri.
     */
    struct AppendBlockFromUriResult final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
      /**
       * The request may return a CRC64 or MD5 hash for the downloaded range of data.
       */
      Azure::Nullable<ContentHash> TransactionalContentHash;
      /**
       * The offset at which the block was committed.
       */
      int64_t AppendOffset = 0;
      /**
       * The number of committed blocks present in the blob. This value is null for block blobs or
       * page blobs.
       */
      int32_t CommittedBlockCount = 0;
      /**
       * True if the blob data and metadata are completely encrypted using the specified algorithm.
       * Otherwise, the value is set to false (when the blob is unencrypted, or if only parts of the
       * blob/application metadata are encrypted).
       */
      bool IsServerEncrypted = false;
      /**
       * The SHA-256 hash of the encryption key used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      /**
       * Name of the encryption scope used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::string> EncryptionScope;
    }; // struct AppendBlockFromUriResult

    /**
     * @brief Response type for #Azure::Storage::Blobs::AppendBlobClient::AppendBlock.
     */
    struct AppendBlockResult final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
      /**
       * The request may return a CRC64 or MD5 hash for the downloaded range of data.
       */
      Azure::Nullable<ContentHash> TransactionalContentHash;
      /**
       * The offset at which the block was committed.
       */
      int64_t AppendOffset = 0;
      /**
       * The number of committed blocks present in the blob. This value is null for block blobs or
       * page blobs.
       */
      int32_t CommittedBlockCount = 0;
      /**
       * True if the blob data and metadata are completely encrypted using the specified algorithm.
       * Otherwise, the value is set to false (when the blob is unencrypted, or if only parts of the
       * blob/application metadata are encrypted).
       */
      bool IsServerEncrypted = false;
      /**
       * The SHA-256 hash of the encryption key used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      /**
       * Name of the encryption scope used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::string> EncryptionScope;
    }; // struct AppendBlockResult

    /**
     * @brief Access policy for a blob container.
     */
    struct BlobContainerAccessPolicy final
    {
      /**
       * Indicates whether data in the container may be accessed publicly and the level of access.
       */
      PublicAccessType AccessType = PublicAccessType::None;
      /**
       * A collection of signed identifiers.
       */
      std::vector<SignedIdentifier> SignedIdentifiers;
    }; // struct BlobContainerAccessPolicy

    /**
     * @brief Properties of a blob container.
     */
    struct BlobContainerProperties final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
      /**
       * A set of name-value pairs associated with this blob as user-defined metadata.
       */
      Storage::Metadata Metadata;
      /**
       * Indicates whether data in the container may be accessed publicly and the level of access.
       */
      PublicAccessType AccessType = PublicAccessType::None;
      /**
       * Indicates whether the container has an immutability policy set on it.
       */
      bool HasImmutabilityPolicy = false;
      /**
       * Indicates whether the container has a legal hold.
       */
      bool HasLegalHold = false;
      /**
       * Indicates whether the lease is of infinite or fixed duration when the blob or container is
       * leased. This value is null if the blob or container is not leased.
       */
      Azure::Nullable<LeaseDurationType> LeaseDuration;
      /**
       * Lease state of the blob.
       */
      Models::LeaseState LeaseState = Models::LeaseState::Available;
      /**
       * The current lease status of the blob.
       */
      Models::LeaseStatus LeaseStatus = Models::LeaseStatus::Unlocked;
      /**
       * The default encryption scope for the container.
       */
      std::string DefaultEncryptionScope = "$account-encryption-key";
      /**
       * Indicates whether the container's default encryption scope can be overridden.
       */
      bool PreventEncryptionScopeOverride = false;
    }; // struct BlobContainerProperties

    /**
     * @brief Properties of a blob.
     */
    struct BlobProperties final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
      /**
       * The date and time at which the blob was created.
       */
      Azure::DateTime CreatedOn;
      /**
       * The time this blob will expire.
       */
      Azure::Nullable<Azure::DateTime> ExpiresOn;
      /**
       * The date and time at which the blob was last read or written to.
       */
      Azure::Nullable<Azure::DateTime> LastAccessedOn;
      /**
       * A set of name-value pairs associated with this blob as user-defined metadata.
       */
      Storage::Metadata Metadata;
      /**
       * The blob's type.
       */
      Models::BlobType BlobType;
      /**
       * Indicates whether the lease is of infinite or fixed duration when the blob or container is
       * leased. This value is null if the blob or container is not leased.
       */
      Azure::Nullable<LeaseDurationType> LeaseDuration;
      /**
       * Lease state of the blob.
       */
      Azure::Nullable<Models::LeaseState> LeaseState;
      /**
       * The current lease status of the blob.
       */
      Azure::Nullable<Models::LeaseStatus> LeaseStatus;
      /**
       * Size of the blob.
       */
      int64_t BlobSize = 0;
      /**
       * System properties of the blob.
       */
      BlobHttpHeaders HttpHeaders;
      /**
       * The current sequence number for a page blob. This value is null for block blobs or append
       * blobs.
       */
      Azure::Nullable<int64_t> SequenceNumber;
      /**
       * The number of committed blocks present in the blob. This value is null for block blobs or
       * page blobs.
       */
      Azure::Nullable<int32_t> CommittedBlockCount;
      /**
       * If the blob has been sealed. This value is null for block blobs or page blobs.
       */
      Azure::Nullable<bool> IsSealed;
      /**
       * True if the blob data and metadata are completely encrypted using the specified algorithm.
       * Otherwise, the value is set to false (when the blob is unencrypted, or if only parts of the
       * blob/application metadata are encrypted).
       */
      bool IsServerEncrypted = false;
      /**
       * The SHA-256 hash of the encryption key used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      /**
       * Name of the encryption scope used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::string> EncryptionScope;
      /**
       * The tier of page blob or block blob.
       */
      Azure::Nullable<Models::AccessTier> AccessTier;
      /**
       * True if the access tier is not explicitly set on the blob.
       */
      Azure::Nullable<bool> IsAccessTierInferred;
      /**
       * Indicates if the blob is being rehydrated.
       */
      Azure::Nullable<Models::ArchiveStatus> ArchiveStatus;
      /**
       * Priority of rehydrate if the blob is being rehydrated.
       */
      Azure::Nullable<Models::RehydratePriority> RehydratePriority;
      /**
       * The date and time the tier was changed on the object.
       */
      Azure::Nullable<Azure::DateTime> AccessTierChangedOn;
      /**
       * String identifier for the last attempted Copy Blob operation where this blob was the
       * destination. This value is null if this blob has never been the destination of a copy
       * operation, or if this blob has been modified after a concluded copy operation.
       */
      Azure::Nullable<std::string> CopyId;
      /**
       * URL that specifies the source blob or file used in the last attempted copy operation where
       * this blob was the destination blob. This value is null if this blob has never been the
       * destination of a copy operation, or if this blob has been modified after a concluded copy
       * operation.
       */
      Azure::Nullable<std::string> CopySource;
      /**
       * State of the copy operation identified by the copy ID. Possible values include success,
       * pending, aborted, failed etc. This value is null if this blob has never been the
       * destination of a copy operation, or if this blob has been modified after a concluded copy
       * operation.
       */
      Azure::Nullable<Models::CopyStatus> CopyStatus;
      /**
       * Describes the cause of the last fatal or non-fatal copy operation failure. This is not null
       * only when copy status is failed or pending.
       */
      Azure::Nullable<std::string> CopyStatusDescription;
      /**
       * True if the copy operation is incremental copy.
       */
      Azure::Nullable<bool> IsIncrementalCopy;
      /**
       * Snapshot time of the last successful incremental copy snapshot for this blob.
       */
      Azure::Nullable<std::string> IncrementalCopyDestinationSnapshot;
      /**
       * Contains the number of bytes copied and the total bytes in the source in the last attempted
       * copy operation where this blob was the destination blob.
       */
      Azure::Nullable<std::string> CopyProgress;
      /**
       * Conclusion time of the last attempted copy operation where this blob was the destination
       * blob.
       */
      Azure::Nullable<Azure::DateTime> CopyCompletedOn;
      /**
       * Only valid when Object Replication is enabled and current blob is the destination.
       */
      Azure::Nullable<std::string> ObjectReplicationDestinationPolicyId;
      /**
       * Only valid when Object Replication is enabled and current blob is the source.
       */
      std::vector<ObjectReplicationPolicy> ObjectReplicationSourceProperties;
      /**
       * the number of tags stored on the blob.
       */
      Azure::Nullable<int32_t> TagCount;
      /**
       * A string value that uniquely identifies the blob. This value is null if Blob Versioning is
       * not enabled.
       */
      Azure::Nullable<std::string> VersionId;
      /**
       * Indicates if this is the current version of the blob. This value is null if Blob Versioning
       * is not enabled.
       */
      Azure::Nullable<bool> IsCurrentVersion;
    }; // struct BlobProperties

    /**
     * @brief Properties of blob service.
     */
    struct BlobServiceProperties final
    {
      /**
       * Azure analytics logging settings.
       */
      AnalyticsLogging Logging;
      /**
       * Summary of request statistics grouped by API in hour aggregates for blobs.
       */
      Metrics HourMetrics;
      /**
       * Summary of request statistics grouped by API in minute aggregates for blobs.
       */
      Metrics MinuteMetrics;
      /**
       * CORS rules set.
       */
      std::vector<CorsRule> Cors;
      /**
       * The default API version used to handle blob service requests if API version is not
       * specified in the request header.
       */
      Azure::Nullable<std::string> DefaultServiceVersion;
      /**
       * Retention policy that determines how long the associated data should persist.
       */
      RetentionPolicy DeleteRetentionPolicy;
      /**
       * The properties that enable an storage account to host a static website.
       */
      Models::StaticWebsite StaticWebsite;
    }; // struct BlobServiceProperties

    /**
     * @brief Extensible enum used to specify blocks to list.
     */
    class BlockListType final {
    public:
      BlockListType() = default;
      explicit BlockListType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const BlockListType& other) const { return m_value == other.m_value; }
      bool operator!=(const BlockListType& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      /**
       * Only list committed blocks.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlockListType Committed;
      /**
       * Only list uncommitted blocks.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlockListType Uncommitted;
      /**
       * List both committed and uncommitted blocks.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlockListType All;

    private:
      std::string m_value;
    }; // extensible enum BlockListType

    /**
     * @brief Extensible enum used to specify how the service should look for a block ID.
     */
    class BlockType final {
    public:
      BlockType() = default;
      explicit BlockType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const BlockType& other) const { return m_value == other.m_value; }
      bool operator!=(const BlockType& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      /**
       * Look for a block ID in the committed block list.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlockType Committed;
      /**
       * Look for a block ID in the uncommitted block list.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlockType Uncommitted;
      /**
       * Look for a block ID in the uncommitted block list first and then in the committed block
       * list.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static BlockType Latest;

    private:
      std::string m_value;
    }; // extensible enum BlockType

    /**
     * @brief Response type for #Azure::Storage::Blobs::PageBlobClient::ClearPages.
     */
    struct ClearPagesResult final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
      /**
       * The current sequence number for a page blob. This value is null for block blobs or append
       * blobs.
       */
      int64_t SequenceNumber = 0;
    }; // struct ClearPagesResult

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlockBlobClient::CommitBlockList.
     */
    struct CommitBlockListResult final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
      /**
       * A string value that uniquely identifies the blob. This value is null if Blob Versioning is
       * not enabled.
       */
      Azure::Nullable<std::string> VersionId;
      /**
       * True if the blob data and metadata are completely encrypted using the specified algorithm.
       * Otherwise, the value is set to false (when the blob is unencrypted, or if only parts of the
       * blob/application metadata are encrypted).
       */
      bool IsServerEncrypted = false;
      /**
       * The SHA-256 hash of the encryption key used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      /**
       * Name of the encryption scope used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::string> EncryptionScope;
      /**
       * The request may return a CRC64 or MD5 hash for the downloaded range of data.
       */
      Azure::Nullable<ContentHash> TransactionalContentHash;
    }; // struct CommitBlockListResult

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlobClient::CopyFromUri.
     */
    struct CopyBlobFromUriResult final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
      /**
       * String identifier for the last attempted Copy Blob operation where this blob was the
       * destination. This value is null if this blob has never been the destination of a copy
       * operation, or if this blob has been modified after a concluded copy operation.
       */
      std::string CopyId;
      /**
       * State of the copy operation identified by the copy ID. Possible values include success,
       * pending, aborted, failed etc. This value is null if this blob has never been the
       * destination of a copy operation, or if this blob has been modified after a concluded copy
       * operation.
       */
      Models::CopyStatus CopyStatus;
      /**
       * A string value that uniquely identifies the blob. This value is null if Blob Versioning is
       * not enabled.
       */
      Azure::Nullable<std::string> VersionId;
    }; // struct CopyBlobFromUriResult

    /**
     * @brief Response type for #Azure::Storage::Blobs::AppendBlobClient::Create.
     */
    struct CreateAppendBlobResult final
    {
      /**
       * Indicates if the append blob was successfully created in this operation.
       */
      bool Created = true;
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
      /**
       * A string value that uniquely identifies the blob. This value is null if Blob Versioning is
       * not enabled.
       */
      Azure::Nullable<std::string> VersionId;
      /**
       * True if the blob data and metadata are completely encrypted using the specified algorithm.
       * Otherwise, the value is set to false (when the blob is unencrypted, or if only parts of the
       * blob/application metadata are encrypted).
       */
      bool IsServerEncrypted = false;
      /**
       * The SHA-256 hash of the encryption key used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      /**
       * Name of the encryption scope used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::string> EncryptionScope;
    }; // struct CreateAppendBlobResult

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlobContainerClient::Create.
     */
    struct CreateBlobContainerResult final
    {
      /**
       * Indicates if the container was successfully created in this operation.
       */
      bool Created = true;
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
    }; // struct CreateBlobContainerResult

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlobClient::CreateSnapshot.
     */
    struct CreateBlobSnapshotResult final
    {
      std::string Snapshot;
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
      /**
       * A string value that uniquely identifies the blob. This value is null if Blob Versioning is
       * not enabled.
       */
      Azure::Nullable<std::string> VersionId;
      /**
       * True if the blob data and metadata are completely encrypted using the specified algorithm.
       * Otherwise, the value is set to false (when the blob is unencrypted, or if only parts of the
       * blob/application metadata are encrypted).
       */
      bool IsServerEncrypted = false;
      /**
       * The SHA-256 hash of the encryption key used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      /**
       * Name of the encryption scope used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::string> EncryptionScope;
    }; // struct CreateBlobSnapshotResult

    /**
     * @brief Response type for #Azure::Storage::Blobs::PageBlobClient::Create.
     */
    struct CreatePageBlobResult final
    {
      /**
       * Indicates if the page blob was successfully created in this operation.
       */
      bool Created = true;
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
      /**
       * A string value that uniquely identifies the blob. This value is null if Blob Versioning is
       * not enabled.
       */
      Azure::Nullable<std::string> VersionId;
      /**
       * True if the blob data and metadata are completely encrypted using the specified algorithm.
       * Otherwise, the value is set to false (when the blob is unencrypted, or if only parts of the
       * blob/application metadata are encrypted).
       */
      bool IsServerEncrypted = false;
      /**
       * The SHA-256 hash of the encryption key used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      /**
       * Name of the encryption scope used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::string> EncryptionScope;
      /**
       * The current sequence number for a page blob. This value is null for block blobs or append
       * blobs.
       */
      Azure::Nullable<int64_t> SequenceNumber;
    }; // struct CreatePageBlobResult

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlobContainerClient::Delete.
     */
    struct DeleteBlobContainerResult final
    {
      /**
       * Indicates if the container was successfully deleted in this operation.
       */
      bool Deleted = true;
    }; // struct DeleteBlobContainerResult

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlobClient::Delete.
     */
    struct DeleteBlobResult final
    {
      /**
       * Indicates if the blob was successfully deleted in this operation.
       */
      bool Deleted = true;
    }; // struct DeleteBlobResult

    /**
     * @brief Extensible enum used to specify whether base blob should be deleted in a delete blob
     * operation.
     */
    class DeleteSnapshotsOption final {
    public:
      DeleteSnapshotsOption() = default;
      explicit DeleteSnapshotsOption(std::string value) : m_value(std::move(value)) {}
      bool operator==(const DeleteSnapshotsOption& other) const { return m_value == other.m_value; }
      bool operator!=(const DeleteSnapshotsOption& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      /**
       * Delete the base blob and all snapshots.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static DeleteSnapshotsOption IncludeSnapshots;
      /**
       * Delete only the blob's snapshots and not the blob itself.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static DeleteSnapshotsOption OnlySnapshots;

    private:
      std::string m_value;
    }; // extensible enum DeleteSnapshotsOption

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlobClient::Download.
     */
    struct DownloadBlobResult final
    {
      /**
       * Content of the blob or blob range.
       */
      std::unique_ptr<Azure::Core::IO::BodyStream> BodyStream;
      /**
       * Indicates the range of bytes returned.
       */
      Azure::Core::Http::HttpRange ContentRange;
      /**
       * Size of the blob.
       */
      int64_t BlobSize = 0;
      /**
       * The blob's type.
       */
      Models::BlobType BlobType;
      /**
       * The request may return a CRC64 or MD5 hash for the downloaded range of data.
       */
      Azure::Nullable<ContentHash> TransactionalContentHash;
      /**
       * Detailed information of the downloaded blob.
       */
      DownloadBlobDetails Details;
    }; // struct DownloadBlobResult

    /**
     * @brief Extensible enum used to identify encryption algorithm.
     */
    class EncryptionAlgorithmType final {
    public:
      EncryptionAlgorithmType() = default;
      explicit EncryptionAlgorithmType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const EncryptionAlgorithmType& other) const
      {
        return m_value == other.m_value;
      }
      bool operator!=(const EncryptionAlgorithmType& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      /**
       * AES-256
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static EncryptionAlgorithmType Aes256;

    private:
      std::string m_value;
    }; // extensible enum EncryptionAlgorithmType

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlockBlobClient::GetBlockList.
     */
    struct GetBlockListResult final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally. Empty if
       * the blob is not never committed.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob. Invalid if the blob is never committed.
       */
      Azure::DateTime LastModified;
      /**
       * Size of the blob. Invalid if the blob is never committed.
       */
      int64_t BlobSize = 0;
      /**
       * Collection of committed blocks.
       */
      std::vector<BlobBlock> CommittedBlocks;
      /**
       * Collection of uncommitted blocks.
       */
      std::vector<BlobBlock> UncommittedBlocks;
    }; // struct GetBlockListResult

    enum class ListBlobContainersIncludeFlags
    {
      /**
       * No extra data should be included.
       */
      None = 0,
      /**
       * Metadata should be included.
       */
      Metadata = 1,
      /**
       * Soft-deleted containers should be included in the response.
       */
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
      /**
       * No extra data should be included.
       */
      None = 0,
      /**
       * Metadata related to any current or previous copy operations should be included.
       */
      Copy = 1,
      /**
       * Soft-deleted blobs should be included.
       */
      Deleted = 2,
      /**
       * Metadata should be included.
       */
      Metadata = 4,
      /**
       * Snapshots should be included.
       */
      Snapshots = 8,
      /**
       * Versions of blobs should be included.
       */
      Versions = 16,
      /**
       * Uncommitted blobs should be included.
       */
      UncomittedBlobs = 32,
      /**
       * Tags should be included.
       */
      Tags = 64,
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

    /**
     * @brief Response type for Azure::Storage::Blobs::PageBlobClient::Resize.
     */
    struct ResizePageBlobResult final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
      /**
       * The current sequence number for a page blob. This value is null for block blobs or append
       * blobs.
       */
      int64_t SequenceNumber = 0;
    }; // struct ResizePageBlobResult

    /**
     * @brief Extensible enum used to specify when a file's expiration time should be relative to.
     */
    class ScheduleBlobExpiryOriginType final {
    public:
      ScheduleBlobExpiryOriginType() = default;
      explicit ScheduleBlobExpiryOriginType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const ScheduleBlobExpiryOriginType& other) const
      {
        return m_value == other.m_value;
      }
      bool operator!=(const ScheduleBlobExpiryOriginType& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      /**
       * Never expires.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static ScheduleBlobExpiryOriginType NeverExpire;
      /**
       * Relative to file's creation time.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static ScheduleBlobExpiryOriginType RelativeToCreation;
      /**
       * Relative to current time.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static ScheduleBlobExpiryOriginType RelativeToNow;
      /**
       * Absolute time.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static ScheduleBlobExpiryOriginType Absolute;

    private:
      std::string m_value;
    }; // extensible enum ScheduleBlobExpiryOriginType

    /**
     * @brief Response type for #Azure::Storage::Blobs::AppendBlobClient::Seal.
     */
    struct SealAppendBlobResult final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
      /**
       * If the blob has been sealed. This value is null for block blobs or page blobs.
       */
      bool IsSealed = true;
    }; // struct SealAppendBlobResult

    /**
     * @brief Indicates how the service should modify the blob's sequence number.
     */
    class SequenceNumberAction final {
    public:
      SequenceNumberAction() = default;
      explicit SequenceNumberAction(std::string value) : m_value(std::move(value)) {}
      bool operator==(const SequenceNumberAction& other) const { return m_value == other.m_value; }
      bool operator!=(const SequenceNumberAction& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      /**
       * Sets the sequence number to be the higher of the value included with the request and the
       * value currently stored for the blob.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static SequenceNumberAction Max;
      /**
       * Sets the sequence number to the value included with the request.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static SequenceNumberAction Update;
      /**
       * Increments the value of the sequence number by 1.
       */
      AZ_STORAGE_BLOBS_DLLEXPORT const static SequenceNumberAction Increment;

    private:
      std::string m_value;
    }; // extensible enum SequenceNumberAction

    /**
     * @brief Statistics for the storage service.
     */
    struct ServiceStatistics final
    {
      /**
       * Geo-replication information for the secondary storage endpoint.
       */
      Models::GeoReplication GeoReplication;
    }; // struct ServiceStatistics

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlobClient::SetAccessTier.
     */
    struct SetBlobAccessTierResult final
    {
    }; // struct SetBlobAccessTierResult

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlobContainerClient::SetAccessPolicy.
     */
    struct SetBlobContainerAccessPolicyResult final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
    }; // struct SetBlobContainerAccessPolicyResult

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlobContainerClient::SetMetadata.
     */
    struct SetBlobContainerMetadataResult final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
    }; // struct SetBlobContainerMetadataResult

    /**
     * @brief Response type for
     * #Azure::Storage::Files::DataLake::DataLakeFileClient::ScheduleDeletion.
     */
    struct SetBlobExpiryResult final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
    }; // struct SetBlobExpiryResult

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlobClient::SetHttpHeaders.
     */
    struct SetBlobHttpHeadersResult final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
      /**
       * The current sequence number for a page blob. This value is null for block blobs or append
       * blobs.
       */
      Azure::Nullable<int64_t> SequenceNumber;
    }; // struct SetBlobHttpHeadersResult

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlobClient::SetMetadata.
     */
    struct SetBlobMetadataResult final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
      /**
       * This value is always null, don't use it.
       */
      Azure::Nullable<int64_t> SequenceNumber;
      /**
       * True if the blob data and metadata are completely encrypted using the specified algorithm.
       * Otherwise, the value is set to false (when the blob is unencrypted, or if only parts of the
       * blob/application metadata are encrypted).
       */
      bool IsServerEncrypted = false;
      /**
       * The SHA-256 hash of the encryption key used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      /**
       * Name of the encryption scope used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::string> EncryptionScope;
    }; // struct SetBlobMetadataResult

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlobClient::SetTags.
     */
    struct SetBlobTagsResult final
    {
    }; // struct SetBlobTagsResult

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlobServiceClient::SetProperties.
     */
    struct SetServicePropertiesResult final
    {
    }; // struct SetServicePropertiesResult

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlockBlobClient::StageBlockFromUri.
     */
    struct StageBlockFromUriResult final
    {
      /**
       * The request may return a CRC64 or MD5 hash for the downloaded range of data.
       */
      Azure::Nullable<ContentHash> TransactionalContentHash;
      /**
       * True if the blob data and metadata are completely encrypted using the specified algorithm.
       * Otherwise, the value is set to false (when the blob is unencrypted, or if only parts of the
       * blob/application metadata are encrypted).
       */
      bool IsServerEncrypted = false;
      /**
       * The SHA-256 hash of the encryption key used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      /**
       * Name of the encryption scope used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::string> EncryptionScope;
    }; // struct StageBlockFromUriResult

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlockBlobClient::StageBlock.
     */
    struct StageBlockResult final
    {
      /**
       * The request may return a CRC64 or MD5 hash for the downloaded range of data.
       */
      Azure::Nullable<ContentHash> TransactionalContentHash;
      /**
       * True if the blob data and metadata are completely encrypted using the specified algorithm.
       * Otherwise, the value is set to false (when the blob is unencrypted, or if only parts of the
       * blob/application metadata are encrypted).
       */
      bool IsServerEncrypted = false;
      /**
       * The SHA-256 hash of the encryption key used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      /**
       * Name of the encryption scope used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::string> EncryptionScope;
    }; // struct StageBlockResult

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlobClient::Undelete.
     */
    struct UndeleteBlobResult final
    {
    }; // struct UndeleteBlobResult

    /**
     * @brief Response type for Azure::Storage::Blobs::PageBlobClient::UpdateSequenceNumber.
     */
    struct UpdateSequenceNumberResult final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
      /**
       * The current sequence number for a page blob. This value is null for block blobs or append
       * blobs.
       */
      int64_t SequenceNumber = 0;
    }; // struct UpdateSequenceNumberResult

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlockBlobClient::Upload.
     */
    struct UploadBlockBlobResult final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
      /**
       * A string value that uniquely identifies the blob. This value is null if Blob Versioning is
       * not enabled.
       */
      Azure::Nullable<std::string> VersionId;
      /**
       * True if the blob data and metadata are completely encrypted using the specified algorithm.
       * Otherwise, the value is set to false (when the blob is unencrypted, or if only parts of the
       * blob/application metadata are encrypted).
       */
      bool IsServerEncrypted = false;
      /**
       * The SHA-256 hash of the encryption key used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      /**
       * Name of the encryption scope used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::string> EncryptionScope;
      /**
       * The request may return a CRC64 or MD5 hash for the downloaded range of data.
       */
      Azure::Nullable<ContentHash> TransactionalContentHash;
    }; // struct UploadBlockBlobResult

    /**
     * @brief Response type for #Azure::Storage::Blobs::PageBlobClient::UploadPagesFromUri.
     */
    struct UploadPagesFromUriResult final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
      /**
       * The request may return a CRC64 or MD5 hash for the downloaded range of data.
       */
      Azure::Nullable<ContentHash> TransactionalContentHash;
      /**
       * The current sequence number for a page blob. This value is null for block blobs or append
       * blobs.
       */
      int64_t SequenceNumber = 0;
      /**
       * True if the blob data and metadata are completely encrypted using the specified algorithm.
       * Otherwise, the value is set to false (when the blob is unencrypted, or if only parts of the
       * blob/application metadata are encrypted).
       */
      bool IsServerEncrypted = false;
      /**
       * The SHA-256 hash of the encryption key used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      /**
       * Name of the encryption scope used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::string> EncryptionScope;
    }; // struct UploadPagesFromUriResult

    /**
     * @brief Response type for #Azure::Storage::Blobs::PageBlobClient::UploadPages.
     */
    struct UploadPagesResult final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;
      /**
       * The date and time the container was last modified. Any operation that modifies the blob,
       * including an update of the metadata or properties, changes the last-modified time of the
       * blob.
       */
      Azure::DateTime LastModified;
      /**
       * The request may return a CRC64 or MD5 hash for the downloaded range of data.
       */
      Azure::Nullable<ContentHash> TransactionalContentHash;
      /**
       * The current sequence number for a page blob. This value is null for block blobs or append
       * blobs.
       */
      int64_t SequenceNumber = 0;
      /**
       * True if the blob data and metadata are completely encrypted using the specified algorithm.
       * Otherwise, the value is set to false (when the blob is unencrypted, or if only parts of the
       * blob/application metadata are encrypted).
       */
      bool IsServerEncrypted = false;
      /**
       * The SHA-256 hash of the encryption key used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      /**
       * Name of the encryption scope used to encrypt the blob data and metadata.
       */
      Azure::Nullable<std::string> EncryptionScope;
    }; // struct UploadPagesResult

    /**
     * @brief A user delegation key that can be used to sign user delegation SAS.
     */
    struct UserDelegationKey final
    {
      /**
       * The immutable identifier for an object in the Microsoft identity system.
       */
      std::string SignedObjectId;
      /**
       * A GUID that represents the Azure AD tenant that the user is from.
       */
      std::string SignedTenantId;
      /**
       * The start time of the user delegation key.
       */
      Azure::DateTime SignedStartsOn;
      /**
       * The expiry time of the user delegation key.
       */
      Azure::DateTime SignedExpiresOn;
      /**
       * The service of the user delegation key can be used for.
       */
      std::string SignedService;
      /**
       * The rest api version used to get user delegation key.
       */
      std::string SignedVersion;
      /**
       * The signature of the user delegation key.
       */
      std::string Value;
    }; // struct UserDelegationKey

    namespace _detail {
      struct AcquireBlobContainerLeaseResult final
      {
        /**
         * The ETag contains a value that you can use to perform operations conditionally.
         */
        Azure::ETag ETag;
        /**
         * The date and time the container was last modified. Any operation that modifies the blob,
         * including an update of the metadata or properties, changes the last-modified time of the
         * blob.
         */
        Azure::DateTime LastModified;
        /**
         * Uniquely identifies a container's or blob's lease.
         */
        std::string LeaseId;
      }; // struct AcquireBlobContainerLeaseResult
    } // namespace _detail

    namespace _detail {
      struct AcquireBlobLeaseResult final
      {
        /**
         * The ETag contains a value that you can use to perform operations conditionally.
         */
        Azure::ETag ETag;
        /**
         * The date and time the container was last modified. Any operation that modifies the blob,
         * including an update of the metadata or properties, changes the last-modified time of the
         * blob.
         */
        Azure::DateTime LastModified;
        /**
         * Uniquely identifies a container's or blob's lease.
         */
        std::string LeaseId;
      }; // struct AcquireBlobLeaseResult
    } // namespace _detail

    namespace _detail {
      struct BreakBlobContainerLeaseResult final
      {
        /**
         * The ETag contains a value that you can use to perform operations conditionally.
         */
        Azure::ETag ETag;
        /**
         * The date and time the container was last modified. Any operation that modifies the blob,
         * including an update of the metadata or properties, changes the last-modified time of the
         * blob.
         */
        Azure::DateTime LastModified;
        /**
         * Approximate time remaining in the lease period in seconds.
         */
        int32_t LeaseTime = 0;
      }; // struct BreakBlobContainerLeaseResult
    } // namespace _detail

    namespace _detail {
      struct BreakBlobLeaseResult final
      {
        /**
         * The ETag contains a value that you can use to perform operations conditionally.
         */
        Azure::ETag ETag;
        /**
         * The date and time the container was last modified. Any operation that modifies the blob,
         * including an update of the metadata or properties, changes the last-modified time of the
         * blob.
         */
        Azure::DateTime LastModified;
        /**
         * Approximate time remaining in the lease period in seconds.
         */
        int32_t LeaseTime = 0;
      }; // struct BreakBlobLeaseResult
    } // namespace _detail

    namespace _detail {
      struct ChangeBlobContainerLeaseResult final
      {
        /**
         * The ETag contains a value that you can use to perform operations conditionally.
         */
        Azure::ETag ETag;
        /**
         * The date and time the container was last modified. Any operation that modifies the blob,
         * including an update of the metadata or properties, changes the last-modified time of the
         * blob.
         */
        Azure::DateTime LastModified;
        /**
         * Uniquely identifies a container's or blob's lease.
         */
        std::string LeaseId;
      }; // struct ChangeBlobContainerLeaseResult
    } // namespace _detail

    namespace _detail {
      struct ChangeBlobLeaseResult final
      {
        /**
         * The ETag contains a value that you can use to perform operations conditionally.
         */
        Azure::ETag ETag;
        /**
         * The date and time the container was last modified. Any operation that modifies the blob,
         * including an update of the metadata or properties, changes the last-modified time of the
         * blob.
         */
        Azure::DateTime LastModified;
        /**
         * Uniquely identifies a container's or blob's lease.
         */
        std::string LeaseId;
      }; // struct ChangeBlobLeaseResult
    } // namespace _detail

    namespace _detail {
      struct FindBlobsByTagsResult final
      {
        std::string ServiceEndpoint;
        Azure::Nullable<std::string> ContinuationToken;
        std::vector<TaggedBlobItem> Items;
      }; // struct FindBlobsByTagsResult
    } // namespace _detail

    namespace _detail {
      struct GetBlobTagsResult final
      {
        /**
         * User-defined tags for this blob.
         */
        std::map<std::string, std::string> Tags;
      }; // struct GetBlobTagsResult
    } // namespace _detail

    namespace _detail {
      struct GetPageRangesResult final
      {
        /**
         * The ETag contains a value that you can use to perform operations conditionally.
         */
        Azure::ETag ETag;
        /**
         * The date and time the container was last modified. Any operation that modifies the blob,
         * including an update of the metadata or properties, changes the last-modified time of the
         * blob.
         */
        Azure::DateTime LastModified;
        /**
         * Size of the blob.
         */
        int64_t BlobSize = 0;
        std::vector<Azure::Core::Http::HttpRange> PageRanges;
        std::vector<Azure::Core::Http::HttpRange> ClearRanges;
      }; // struct GetPageRangesResult
    } // namespace _detail

    namespace _detail {
      struct ListBlobContainersResult final
      {
        std::string ServiceEndpoint;
        std::string Prefix;
        Azure::Nullable<std::string> ContinuationToken;
        std::vector<BlobContainerItem> Items;
      }; // struct ListBlobContainersResult
    } // namespace _detail

    namespace _detail {
      struct ListBlobsByHierarchyResult final
      {
        std::string ServiceEndpoint;
        std::string BlobContainerName;
        std::string Prefix;
        std::string Delimiter;
        Azure::Nullable<std::string> ContinuationToken;
        std::vector<BlobItem> Items;
        std::vector<std::string> BlobPrefixes;
      }; // struct ListBlobsByHierarchyResult
    } // namespace _detail

    namespace _detail {
      struct ListBlobsResult final
      {
        std::string ServiceEndpoint;
        std::string BlobContainerName;
        std::string Prefix;
        Azure::Nullable<std::string> ContinuationToken;
        std::vector<BlobItem> Items;
      }; // struct ListBlobsResult
    } // namespace _detail

    namespace _detail {
      struct ReleaseBlobContainerLeaseResult final
      {
        /**
         * The ETag contains a value that you can use to perform operations conditionally.
         */
        Azure::ETag ETag;
        /**
         * The date and time the container was last modified. Any operation that modifies the blob,
         * including an update of the metadata or properties, changes the last-modified time of the
         * blob.
         */
        Azure::DateTime LastModified;
      }; // struct ReleaseBlobContainerLeaseResult
    } // namespace _detail

    namespace _detail {
      struct ReleaseBlobLeaseResult final
      {
        /**
         * The ETag contains a value that you can use to perform operations conditionally.
         */
        Azure::ETag ETag;
        /**
         * The date and time the container was last modified. Any operation that modifies the blob,
         * including an update of the metadata or properties, changes the last-modified time of the
         * blob.
         */
        Azure::DateTime LastModified;
      }; // struct ReleaseBlobLeaseResult
    } // namespace _detail

    namespace _detail {
      struct RenewBlobContainerLeaseResult final
      {
        /**
         * The ETag contains a value that you can use to perform operations conditionally.
         */
        Azure::ETag ETag;
        /**
         * The date and time the container was last modified. Any operation that modifies the blob,
         * including an update of the metadata or properties, changes the last-modified time of the
         * blob.
         */
        Azure::DateTime LastModified;
        /**
         * Uniquely identifies a container's or blob's lease.
         */
        std::string LeaseId;
      }; // struct RenewBlobContainerLeaseResult
    } // namespace _detail

    namespace _detail {
      struct RenewBlobLeaseResult final
      {
        /**
         * The ETag contains a value that you can use to perform operations conditionally.
         */
        Azure::ETag ETag;
        /**
         * The date and time the container was last modified. Any operation that modifies the blob,
         * including an update of the metadata or properties, changes the last-modified time of the
         * blob.
         */
        Azure::DateTime LastModified;
        /**
         * Uniquely identifies a container's or blob's lease.
         */
        std::string LeaseId;
      }; // struct RenewBlobLeaseResult
    } // namespace _detail

    namespace _detail {
      struct StartBlobCopyFromUriResult final
      {
        /**
         * The ETag contains a value that you can use to perform operations conditionally.
         */
        Azure::ETag ETag;
        /**
         * The date and time the container was last modified. Any operation that modifies the blob,
         * including an update of the metadata or properties, changes the last-modified time of the
         * blob.
         */
        Azure::DateTime LastModified;
        /**
         * String identifier for the last attempted Copy Blob operation where this blob was the
         * destination. This value is null if this blob has never been the destination of a copy
         * operation, or if this blob has been modified after a concluded copy operation.
         */
        std::string CopyId;
        /**
         * State of the copy operation identified by the copy ID. Possible values include success,
         * pending, aborted, failed etc. This value is null if this blob has never been the
         * destination of a copy operation, or if this blob has been modified after a concluded copy
         * operation.
         */
        Models::CopyStatus CopyStatus;
        /**
         * A string value that uniquely identifies the blob. This value is null if Blob Versioning
         * is not enabled.
         */
        Azure::Nullable<std::string> VersionId;
      }; // struct StartBlobCopyFromUriResult
    } // namespace _detail

    namespace _detail {
      struct StartBlobCopyIncrementalResult final
      {
        /**
         * The ETag contains a value that you can use to perform operations conditionally.
         */
        Azure::ETag ETag;
        /**
         * The date and time the container was last modified. Any operation that modifies the blob,
         * including an update of the metadata or properties, changes the last-modified time of the
         * blob.
         */
        Azure::DateTime LastModified;
        /**
         * String identifier for the last attempted Copy Blob operation where this blob was the
         * destination. This value is null if this blob has never been the destination of a copy
         * operation, or if this blob has been modified after a concluded copy operation.
         */
        std::string CopyId;
        /**
         * State of the copy operation identified by the copy ID. Possible values include success,
         * pending, aborted, failed etc. This value is null if this blob has never been the
         * destination of a copy operation, or if this blob has been modified after a concluded copy
         * operation.
         */
        Models::CopyStatus CopyStatus;
        /**
         * A string value that uniquely identifies the blob. This value is null if Blob Versioning
         * is not enabled.
         */
        Azure::Nullable<std::string> VersionId;
      }; // struct StartBlobCopyIncrementalResult
    } // namespace _detail

    namespace _detail {
      struct SubmitBlobBatchResult final
      {
        std::string ContentType;
      }; // struct SubmitBlobBatchResult
    } // namespace _detail

    namespace _detail {
      struct UndeleteBlobContainerResult final
      {
      }; // struct UndeleteBlobContainerResult
    } // namespace _detail

  } // namespace Models

  namespace _detail {
    constexpr static const char* ApiVersion = "2020-04-08";
  } // namespace _detail

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
      for (size_t i = 0; i < sizeof(value_list) / sizeof(ListBlobContainersIncludeFlags); ++i)
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
          ListBlobsIncludeFlags::Tags,
      };
      const char* string_list[] = {
          "copy",
          "deleted",
          "metadata",
          "snapshots",
          "versions",
          "uncommittedblobs",
          "tags",
      };
      std::string ret;
      for (size_t i = 0; i < sizeof(value_list) / sizeof(ListBlobsIncludeFlags); ++i)
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

    class BlobRestClient final {
    public:
      class Service final {
      public:
        struct ListBlobContainersOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::string> Prefix;
          Azure::Nullable<std::string> ContinuationToken;
          Azure::Nullable<int32_t> MaxResults;
          ListBlobContainersIncludeFlags Include = ListBlobContainersIncludeFlags::None;
        }; // struct ListBlobContainersOptions

        static Azure::Response<Models::_detail::ListBlobContainersResult> ListBlobContainers(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const ListBlobContainersOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("comp", "list");
          if (options.Prefix.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "prefix", _internal::UrlEncodeQueryParameter(options.Prefix.Value()));
          }
          if (options.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "marker", _internal::UrlEncodeQueryParameter(options.ContinuationToken.Value()));
          }
          if (options.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "maxresults", std::to_string(options.MaxResults.Value()));
          }
          std::string list_blob_containers_include_flags
              = ListBlobContainersIncludeFlagsToString(options.Include);
          if (!list_blob_containers_include_flags.empty())
          {
            request.GetUrl().AppendQueryParameter(
                "include", _internal::UrlEncodeQueryParameter(list_blob_containers_include_flags));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::ListBlobContainersResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            _internal::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = ListBlobContainersResultInternalFromXml(reader);
          }
          return Azure::Response<Models::_detail::ListBlobContainersResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct GetUserDelegationKeyOptions final
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
          std::string xml_body;
          {
            _internal::XmlWriter writer;
            GetUserDelegationKeyOptionsToXml(writer, options);
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::End});
            xml_body = writer.GetDocument();
          }
          Azure::Core::IO::MemoryBodyStream xml_body_stream(
              reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
          auto request = Azure::Core::Http::Request(
              Azure::Core::Http::HttpMethod::Post, url, &xml_body_stream);
          request.SetHeader("Content-Length", std::to_string(xml_body_stream.Length()));
          request.GetUrl().AppendQueryParameter("restype", "service");
          request.GetUrl().AppendQueryParameter("comp", "userdelegationkey");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          UserDelegationKey response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            _internal::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = UserDelegationKeyFromXml(reader);
          }
          return Azure::Response<UserDelegationKey>(std::move(response), std::move(pHttpResponse));
        }

        struct GetServicePropertiesOptions final
        {
          Azure::Nullable<int32_t> Timeout;
        }; // struct GetServicePropertiesOptions

        static Azure::Response<BlobServiceProperties> GetProperties(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const GetServicePropertiesOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter("restype", "service");
          request.GetUrl().AppendQueryParameter("comp", "properties");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          BlobServiceProperties response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            _internal::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = BlobServicePropertiesFromXml(reader);
          }
          return Azure::Response<BlobServiceProperties>(
              std::move(response), std::move(pHttpResponse));
        }

        struct SetServicePropertiesOptions final
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
          std::string xml_body;
          {
            _internal::XmlWriter writer;
            SetServicePropertiesOptionsToXml(writer, options);
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::End});
            xml_body = writer.GetDocument();
          }
          Azure::Core::IO::MemoryBodyStream xml_body_stream(
              reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
          auto request = Azure::Core::Http::Request(
              Azure::Core::Http::HttpMethod::Put, url, &xml_body_stream);
          request.SetHeader("Content-Length", std::to_string(xml_body_stream.Length()));
          request.GetUrl().AppendQueryParameter("restype", "service");
          request.GetUrl().AppendQueryParameter("comp", "properties");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          SetServicePropertiesResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Accepted)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          return Azure::Response<SetServicePropertiesResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct GetAccountInfoOptions final
        {
          Azure::Nullable<int32_t> Timeout;
        }; // struct GetAccountInfoOptions

        static Azure::Response<AccountInfo> GetAccountInfo(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const GetAccountInfoOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, url);
          request.GetUrl().AppendQueryParameter("restype", "account");
          request.GetUrl().AppendQueryParameter("comp", "properties");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          AccountInfo response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.SkuName = SkuName(httpResponse.GetHeaders().at("x-ms-sku-name"));
          response.AccountKind = AccountKind(httpResponse.GetHeaders().at("x-ms-account-kind"));
          response.IsHierarchicalNamespaceEnabled
              = httpResponse.GetHeaders().at("x-ms-is-hns-enabled") == "true";
          return Azure::Response<AccountInfo>(std::move(response), std::move(pHttpResponse));
        }

        struct GetServiceStatisticsOptions final
        {
          Azure::Nullable<int32_t> Timeout;
        }; // struct GetServiceStatisticsOptions

        static Azure::Response<ServiceStatistics> GetStatistics(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const GetServiceStatisticsOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter("restype", "service");
          request.GetUrl().AppendQueryParameter("comp", "stats");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          ServiceStatistics response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            _internal::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = ServiceStatisticsFromXml(reader);
          }
          return Azure::Response<ServiceStatistics>(std::move(response), std::move(pHttpResponse));
        }

        struct FindBlobsByTagsOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          std::string Where;
          Azure::Nullable<std::string> ContinuationToken;
          Azure::Nullable<int32_t> MaxResults;
        }; // struct FindBlobsByTagsOptions

        static Azure::Response<Models::_detail::FindBlobsByTagsResult> FindBlobsByTags(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const FindBlobsByTagsOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("comp", "blobs");
          request.GetUrl().AppendQueryParameter(
              "where", _internal::UrlEncodeQueryParameter(options.Where));
          if (options.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "marker", _internal::UrlEncodeQueryParameter(options.ContinuationToken.Value()));
          }
          if (options.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "maxresults", std::to_string(options.MaxResults.Value()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::FindBlobsByTagsResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            _internal::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = FindBlobsByTagsResultInternalFromXml(reader);
          }
          return Azure::Response<Models::_detail::FindBlobsByTagsResult>(
              std::move(response), std::move(pHttpResponse));
        }

      private:
        static BlobServiceProperties BlobServicePropertiesFromXml(_internal::XmlReader& reader)
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
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
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
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "StorageServiceProperties")
              {
                path.emplace_back(XmlTagName::k_StorageServiceProperties);
              }
              else if (node.Name == "Logging")
              {
                path.emplace_back(XmlTagName::k_Logging);
              }
              else if (node.Name == "HourMetrics")
              {
                path.emplace_back(XmlTagName::k_HourMetrics);
              }
              else if (node.Name == "MinuteMetrics")
              {
                path.emplace_back(XmlTagName::k_MinuteMetrics);
              }
              else if (node.Name == "Cors")
              {
                path.emplace_back(XmlTagName::k_Cors);
              }
              else if (node.Name == "CorsRule")
              {
                path.emplace_back(XmlTagName::k_CorsRule);
              }
              else if (node.Name == "DefaultServiceVersion")
              {
                path.emplace_back(XmlTagName::k_DefaultServiceVersion);
              }
              else if (node.Name == "DeleteRetentionPolicy")
              {
                path.emplace_back(XmlTagName::k_DeleteRetentionPolicy);
              }
              else if (node.Name == "StaticWebsite")
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
                ret.Logging = AnalyticsLoggingFromXml(reader);
                path.pop_back();
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_StorageServiceProperties
                  && path[1] == XmlTagName::k_HourMetrics)
              {
                ret.HourMetrics = MetricsFromXml(reader);
                path.pop_back();
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_StorageServiceProperties
                  && path[1] == XmlTagName::k_MinuteMetrics)
              {
                ret.MinuteMetrics = MetricsFromXml(reader);
                path.pop_back();
              }
              else if (
                  path.size() == 3 && path[0] == XmlTagName::k_StorageServiceProperties
                  && path[1] == XmlTagName::k_Cors && path[2] == XmlTagName::k_CorsRule)
              {
                ret.Cors.emplace_back(CorsRuleFromXml(reader));
                path.pop_back();
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_StorageServiceProperties
                  && path[1] == XmlTagName::k_DeleteRetentionPolicy)
              {
                ret.DeleteRetentionPolicy = RetentionPolicyFromXml(reader);
                path.pop_back();
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_StorageServiceProperties
                  && path[1] == XmlTagName::k_StaticWebsite)
              {
                ret.StaticWebsite = StaticWebsiteFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
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

        static Models::_detail::FindBlobsByTagsResult FindBlobsByTagsResultInternalFromXml(
            _internal::XmlReader& reader)
        {
          Models::_detail::FindBlobsByTagsResult ret;
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
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
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
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "EnumerationResults")
              {
                path.emplace_back(XmlTagName::k_EnumerationResults);
              }
              else if (node.Name == "NextMarker")
              {
                path.emplace_back(XmlTagName::k_NextMarker);
              }
              else if (node.Name == "Blobs")
              {
                path.emplace_back(XmlTagName::k_Blobs);
              }
              else if (node.Name == "Blob")
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
                ret.Items.emplace_back(TaggedBlobItemFromXml(reader));
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                  && path[1] == XmlTagName::k_NextMarker)
              {
                ret.ContinuationToken = node.Value;
              }
            }
            else if (node.Type == _internal::XmlNodeType::Attribute)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_EnumerationResults
                  && node.Name == "ServiceEndpoint")
              {
                ret.ServiceEndpoint = node.Value;
              }
            }
          }
          return ret;
        }

        static Models::_detail::ListBlobContainersResult ListBlobContainersResultInternalFromXml(
            _internal::XmlReader& reader)
        {
          Models::_detail::ListBlobContainersResult ret;
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
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
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
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "EnumerationResults")
              {
                path.emplace_back(XmlTagName::k_EnumerationResults);
              }
              else if (node.Name == "Prefix")
              {
                path.emplace_back(XmlTagName::k_Prefix);
              }
              else if (node.Name == "NextMarker")
              {
                path.emplace_back(XmlTagName::k_NextMarker);
              }
              else if (node.Name == "Containers")
              {
                path.emplace_back(XmlTagName::k_Containers);
              }
              else if (node.Name == "Container")
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
            else if (node.Type == _internal::XmlNodeType::Text)
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
            else if (node.Type == _internal::XmlNodeType::Attribute)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_EnumerationResults
                  && node.Name == "ServiceEndpoint")
              {
                ret.ServiceEndpoint = node.Value;
              }
            }
          }
          return ret;
        }

        static ServiceStatistics ServiceStatisticsFromXml(_internal::XmlReader& reader)
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
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
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
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "StorageServiceStats")
              {
                path.emplace_back(XmlTagName::k_StorageServiceStats);
              }
              else if (node.Name == "GeoReplication")
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
                ret.GeoReplication = GeoReplicationFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
            }
          }
          return ret;
        }

        static UserDelegationKey UserDelegationKeyFromXml(_internal::XmlReader& reader)
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
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
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
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "UserDelegationKey")
              {
                path.emplace_back(XmlTagName::k_UserDelegationKey);
              }
              else if (node.Name == "SignedOid")
              {
                path.emplace_back(XmlTagName::k_SignedOid);
              }
              else if (node.Name == "SignedTid")
              {
                path.emplace_back(XmlTagName::k_SignedTid);
              }
              else if (node.Name == "SignedStart")
              {
                path.emplace_back(XmlTagName::k_SignedStart);
              }
              else if (node.Name == "SignedExpiry")
              {
                path.emplace_back(XmlTagName::k_SignedExpiry);
              }
              else if (node.Name == "SignedService")
              {
                path.emplace_back(XmlTagName::k_SignedService);
              }
              else if (node.Name == "SignedVersion")
              {
                path.emplace_back(XmlTagName::k_SignedVersion);
              }
              else if (node.Name == "Value")
              {
                path.emplace_back(XmlTagName::k_Value);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
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

        static AnalyticsLogging AnalyticsLoggingFromXml(_internal::XmlReader& reader)
        {
          AnalyticsLogging ret;
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
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
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
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "Version")
              {
                path.emplace_back(XmlTagName::k_Version);
              }
              else if (node.Name == "Delete")
              {
                path.emplace_back(XmlTagName::k_Delete);
              }
              else if (node.Name == "Read")
              {
                path.emplace_back(XmlTagName::k_Read);
              }
              else if (node.Name == "Write")
              {
                path.emplace_back(XmlTagName::k_Write);
              }
              else if (node.Name == "RetentionPolicy")
              {
                path.emplace_back(XmlTagName::k_RetentionPolicy);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
              if (path.size() == 1 && path[0] == XmlTagName::k_RetentionPolicy)
              {
                ret.RetentionPolicy = RetentionPolicyFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_Version)
              {
                ret.Version = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_Delete)
              {
                ret.Delete = node.Value == "true";
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_Read)
              {
                ret.Read = node.Value == "true";
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_Write)
              {
                ret.Write = node.Value == "true";
              }
            }
          }
          return ret;
        }

        static BlobContainerItem BlobContainerItemFromXml(_internal::XmlReader& reader)
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
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
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
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "Name")
              {
                path.emplace_back(XmlTagName::k_Name);
              }
              else if (node.Name == "Properties")
              {
                path.emplace_back(XmlTagName::k_Properties);
              }
              else if (node.Name == "Etag")
              {
                path.emplace_back(XmlTagName::k_Etag);
              }
              else if (node.Name == "Last-Modified")
              {
                path.emplace_back(XmlTagName::k_LastModified);
              }
              else if (node.Name == "PublicAccess")
              {
                path.emplace_back(XmlTagName::k_PublicAccess);
              }
              else if (node.Name == "HasImmutabilityPolicy")
              {
                path.emplace_back(XmlTagName::k_HasImmutabilityPolicy);
              }
              else if (node.Name == "HasLegalHold")
              {
                path.emplace_back(XmlTagName::k_HasLegalHold);
              }
              else if (node.Name == "LeaseStatus")
              {
                path.emplace_back(XmlTagName::k_LeaseStatus);
              }
              else if (node.Name == "LeaseState")
              {
                path.emplace_back(XmlTagName::k_LeaseState);
              }
              else if (node.Name == "LeaseDuration")
              {
                path.emplace_back(XmlTagName::k_LeaseDuration);
              }
              else if (node.Name == "DefaultEncryptionScope")
              {
                path.emplace_back(XmlTagName::k_DefaultEncryptionScope);
              }
              else if (node.Name == "DenyEncryptionScopeOverride")
              {
                path.emplace_back(XmlTagName::k_DenyEncryptionScopeOverride);
              }
              else if (node.Name == "Metadata")
              {
                path.emplace_back(XmlTagName::k_Metadata);
              }
              else if (node.Name == "Deleted")
              {
                path.emplace_back(XmlTagName::k_Deleted);
              }
              else if (node.Name == "Version")
              {
                path.emplace_back(XmlTagName::k_Version);
              }
              else if (node.Name == "DeletedTime")
              {
                path.emplace_back(XmlTagName::k_DeletedTime);
              }
              else if (node.Name == "RemainingRetentionDays")
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
            else if (node.Type == _internal::XmlNodeType::Text)
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
                ret.Details.HasImmutabilityPolicy = node.Value == "true";
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_HasLegalHold)
              {
                ret.Details.HasLegalHold = node.Value == "true";
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_LeaseStatus)
              {
                ret.Details.LeaseStatus = LeaseStatus(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_LeaseState)
              {
                ret.Details.LeaseState = LeaseState(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_LeaseDuration)
              {
                ret.Details.LeaseDuration = LeaseDurationType(node.Value);
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
                ret.Details.PreventEncryptionScopeOverride = node.Value == "true";
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_Deleted)
              {
                ret.IsDeleted = node.Value == "true";
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

        static CorsRule CorsRuleFromXml(_internal::XmlReader& reader)
        {
          CorsRule ret;
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
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
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
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "AllowedOrigins")
              {
                path.emplace_back(XmlTagName::k_AllowedOrigins);
              }
              else if (node.Name == "AllowedMethods")
              {
                path.emplace_back(XmlTagName::k_AllowedMethods);
              }
              else if (node.Name == "MaxAgeInSeconds")
              {
                path.emplace_back(XmlTagName::k_MaxAgeInSeconds);
              }
              else if (node.Name == "ExposedHeaders")
              {
                path.emplace_back(XmlTagName::k_ExposedHeaders);
              }
              else if (node.Name == "AllowedHeaders")
              {
                path.emplace_back(XmlTagName::k_AllowedHeaders);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
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

        static GeoReplication GeoReplicationFromXml(_internal::XmlReader& reader)
        {
          GeoReplication ret;
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
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
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
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "Status")
              {
                path.emplace_back(XmlTagName::k_Status);
              }
              else if (node.Name == "LastSyncTime")
              {
                path.emplace_back(XmlTagName::k_LastSyncTime);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_Status)
              {
                ret.Status = GeoReplicationStatus(node.Value);
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

        static Metrics MetricsFromXml(_internal::XmlReader& reader)
        {
          Metrics ret;
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
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
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
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "Version")
              {
                path.emplace_back(XmlTagName::k_Version);
              }
              else if (node.Name == "Enabled")
              {
                path.emplace_back(XmlTagName::k_Enabled);
              }
              else if (node.Name == "IncludeAPIs")
              {
                path.emplace_back(XmlTagName::k_IncludeAPIs);
              }
              else if (node.Name == "RetentionPolicy")
              {
                path.emplace_back(XmlTagName::k_RetentionPolicy);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
              if (path.size() == 1 && path[0] == XmlTagName::k_RetentionPolicy)
              {
                ret.RetentionPolicy = RetentionPolicyFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_Version)
              {
                ret.Version = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_Enabled)
              {
                ret.IsEnabled = node.Value == "true";
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_IncludeAPIs)
              {
                ret.IncludeApis = node.Value == "true";
              }
            }
          }
          return ret;
        }

        static RetentionPolicy RetentionPolicyFromXml(_internal::XmlReader& reader)
        {
          RetentionPolicy ret;
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
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
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
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "Enabled")
              {
                path.emplace_back(XmlTagName::k_Enabled);
              }
              else if (node.Name == "Days")
              {
                path.emplace_back(XmlTagName::k_Days);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_Enabled)
              {
                ret.IsEnabled = node.Value == "true";
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_Days)
              {
                ret.Days = std::stoi(node.Value);
              }
            }
          }
          return ret;
        }

        static StaticWebsite StaticWebsiteFromXml(_internal::XmlReader& reader)
        {
          StaticWebsite ret;
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
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
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
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "Enabled")
              {
                path.emplace_back(XmlTagName::k_Enabled);
              }
              else if (node.Name == "IndexDocument")
              {
                path.emplace_back(XmlTagName::k_IndexDocument);
              }
              else if (node.Name == "DefaultIndexDocumentPath")
              {
                path.emplace_back(XmlTagName::k_DefaultIndexDocumentPath);
              }
              else if (node.Name == "ErrorDocument404Path")
              {
                path.emplace_back(XmlTagName::k_ErrorDocument404Path);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_Enabled)
              {
                ret.IsEnabled = node.Value == "true";
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

        static TaggedBlobItem TaggedBlobItemFromXml(_internal::XmlReader& reader)
        {
          TaggedBlobItem ret;
          enum class XmlTagName
          {
            k_Name,
            k_ContainerName,
            k_Tags,
            k_TagSet,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
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
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "Name")
              {
                path.emplace_back(XmlTagName::k_Name);
              }
              else if (node.Name == "ContainerName")
              {
                path.emplace_back(XmlTagName::k_ContainerName);
              }
              else if (node.Name == "Tags")
              {
                path.emplace_back(XmlTagName::k_Tags);
              }
              else if (node.Name == "TagSet")
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
            else if (node.Type == _internal::XmlNodeType::Text)
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

        static std::map<std::string, std::string> TagsFromXml(_internal::XmlReader& reader)
        {
          std::map<std::string, std::string> ret;
          int depth = 0;
          std::string key;
          bool is_key = false;
          bool is_value = false;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              ++depth;
              if (node.Name == "Key")
              {
                is_key = true;
              }
              else if (node.Name == "Value")
              {
                is_value = true;
              }
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (depth-- == 0)
              {
                break;
              }
            }
            if (depth == 2 && node.Type == _internal::XmlNodeType::Text)
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

        static Metadata MetadataFromXml(_internal::XmlReader& reader)
        {
          Metadata ret;
          int depth = 0;
          std::string key;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (depth++ == 0)
              {
                key = node.Name;
              }
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (depth-- == 0)
              {
                break;
              }
            }
            else if (depth == 1 && node.Type == _internal::XmlNodeType::Text)
            {
              ret.emplace(std::move(key), node.Value);
            }
          }
          return ret;
        }

        static void GetUserDelegationKeyOptionsToXml(
            _internal::XmlWriter& writer,
            const GetUserDelegationKeyOptions& options)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "KeyInfo"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Start"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text,
              std::string(),
              options.StartsOn.ToString(
                  Azure::DateTime::DateFormat::Rfc3339,
                  Azure::DateTime::TimeFractionFormat::Truncate)});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Expiry"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text,
              std::string(),
              options.ExpiresOn.ToString(
                  Azure::DateTime::DateFormat::Rfc3339,
                  Azure::DateTime::TimeFractionFormat::Truncate)});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

        static void SetServicePropertiesOptionsToXml(
            _internal::XmlWriter& writer,
            const SetServicePropertiesOptions& options)
        {
          writer.Write(
              _internal::XmlNode{_internal::XmlNodeType::StartTag, "StorageServiceProperties"});
          BlobServicePropertiesToXml(writer, options.Properties);
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

        static void BlobServicePropertiesToXml(
            _internal::XmlWriter& writer,
            const BlobServiceProperties& options)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Logging"});
          AnalyticsLoggingToXml(writer, options.Logging);
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "HourMetrics"});
          MetricsToXml(writer, options.HourMetrics);
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "MinuteMetrics"});
          MetricsToXml(writer, options.MinuteMetrics);
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Cors"});
          for (const auto& i : options.Cors)
          {
            CorsRuleToXml(writer, i);
          }
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          if (options.DefaultServiceVersion.HasValue())
          {
            writer.Write(
                _internal::XmlNode{_internal::XmlNodeType::StartTag, "DefaultServiceVersion"});
            writer.Write(_internal::XmlNode{
                _internal::XmlNodeType::Text,
                std::string(),
                options.DefaultServiceVersion.Value()});
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          }
          writer.Write(
              _internal::XmlNode{_internal::XmlNodeType::StartTag, "DeleteRetentionPolicy"});
          RetentionPolicyToXml(writer, options.DeleteRetentionPolicy);
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "StaticWebsite"});
          StaticWebsiteToXml(writer, options.StaticWebsite);
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

        static void AnalyticsLoggingToXml(
            _internal::XmlWriter& writer,
            const AnalyticsLogging& options)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Version"});
          writer.Write(
              _internal::XmlNode{_internal::XmlNodeType::Text, std::string(), options.Version});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Delete"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, std::string(), options.Delete ? "true" : "false"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Read"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, std::string(), options.Read ? "true" : "false"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Write"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, std::string(), options.Write ? "true" : "false"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "RetentionPolicy"});
          RetentionPolicyToXml(writer, options.RetentionPolicy);
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

        static void CorsRuleToXml(_internal::XmlWriter& writer, const CorsRule& options)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "CorsRule"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "AllowedOrigins"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, std::string(), options.AllowedOrigins});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "AllowedMethods"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, std::string(), options.AllowedMethods});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "AllowedHeaders"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, std::string(), options.AllowedHeaders});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "ExposedHeaders"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, std::string(), options.ExposedHeaders});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "MaxAgeInSeconds"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text,
              std::string(),
              std::to_string(options.MaxAgeInSeconds)});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

        static void MetricsToXml(_internal::XmlWriter& writer, const Metrics& options)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Version"});
          writer.Write(
              _internal::XmlNode{_internal::XmlNodeType::Text, std::string(), options.Version});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Enabled"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, std::string(), options.IsEnabled ? "true" : "false"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          if (options.IncludeApis.HasValue())
          {
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "IncludeAPIs"});
            writer.Write(_internal::XmlNode{
                _internal::XmlNodeType::Text,
                std::string(),
                options.IncludeApis.Value() ? "true" : "false"});
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          }
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "RetentionPolicy"});
          RetentionPolicyToXml(writer, options.RetentionPolicy);
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

        static void RetentionPolicyToXml(
            _internal::XmlWriter& writer,
            const RetentionPolicy& options)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Enabled"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, std::string(), options.IsEnabled ? "true" : "false"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          if (options.Days.HasValue())
          {
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Days"});
            writer.Write(_internal::XmlNode{
                _internal::XmlNodeType::Text, std::string(), std::to_string(options.Days.Value())});
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          }
        }

        static void StaticWebsiteToXml(_internal::XmlWriter& writer, const StaticWebsite& options)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Enabled"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, std::string(), options.IsEnabled ? "true" : "false"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          if (options.IndexDocument.HasValue())
          {
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "IndexDocument"});
            writer.Write(_internal::XmlNode{
                _internal::XmlNodeType::Text, std::string(), options.IndexDocument.Value()});
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          }
          if (options.DefaultIndexDocumentPath.HasValue())
          {
            writer.Write(
                _internal::XmlNode{_internal::XmlNodeType::StartTag, "DefaultIndexDocumentPath"});
            writer.Write(_internal::XmlNode{
                _internal::XmlNodeType::Text,
                std::string(),
                options.DefaultIndexDocumentPath.Value()});
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          }
          if (options.ErrorDocument404Path.HasValue())
          {
            writer.Write(
                _internal::XmlNode{_internal::XmlNodeType::StartTag, "ErrorDocument404Path"});
            writer.Write(_internal::XmlNode{
                _internal::XmlNodeType::Text, std::string(), options.ErrorDocument404Path.Value()});
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          }
        }

      }; // class Service

      class BlobContainer final {
      public:
        struct CreateBlobContainerOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
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
                "x-ms-default-encryption-scope", options.DefaultEncryptionScope.Value());
          }
          if (options.PreventEncryptionScopeOverride.HasValue())
          {
            request.SetHeader(
                "x-ms-deny-encryption-scope-override",
                options.PreventEncryptionScopeOverride.Value() ? "true" : "false");
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          CreateBlobContainerResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Created)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          return Azure::Response<CreateBlobContainerResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct DeleteBlobContainerOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Delete, url);
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          DeleteBlobContainerResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Accepted)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          return Azure::Response<DeleteBlobContainerResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct UndeleteBlobContainerOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          std::string DeletedBlobContainerName;
          std::string DeletedBlobContainerVersion;
        }; // struct UndeleteBlobContainerOptions

        static Azure::Response<Models::_detail::UndeleteBlobContainerResult> Undelete(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const UndeleteBlobContainerOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.GetUrl().AppendQueryParameter("comp", "undelete");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.SetHeader("x-ms-deleted-container-name", options.DeletedBlobContainerName);
          request.SetHeader("x-ms-deleted-container-version", options.DeletedBlobContainerVersion);
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::UndeleteBlobContainerResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Created)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          return Azure::Response<Models::_detail::UndeleteBlobContainerResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct GetBlobContainerPropertiesOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, url);
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          BlobContainerProperties response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
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
          response.LeaseStatus = LeaseStatus(httpResponse.GetHeaders().at("x-ms-lease-status"));
          response.LeaseState = LeaseState(httpResponse.GetHeaders().at("x-ms-lease-state"));
          auto x_ms_lease_duration__iterator
              = httpResponse.GetHeaders().find("x-ms-lease-duration");
          if (x_ms_lease_duration__iterator != httpResponse.GetHeaders().end())
          {
            response.LeaseDuration = LeaseDurationType(x_ms_lease_duration__iterator->second);
          }
          auto x_ms_default_encryption_scope__iterator
              = httpResponse.GetHeaders().find("x-ms-default-encryption-scope");
          if (x_ms_default_encryption_scope__iterator != httpResponse.GetHeaders().end())
          {
            response.DefaultEncryptionScope = x_ms_default_encryption_scope__iterator->second;
          }
          auto x_ms_deny_encryption_scope_override__iterator
              = httpResponse.GetHeaders().find("x-ms-deny-encryption-scope-override");
          if (x_ms_deny_encryption_scope_override__iterator != httpResponse.GetHeaders().end())
          {
            response.PreventEncryptionScopeOverride
                = x_ms_deny_encryption_scope_override__iterator->second == "true";
          }
          return Azure::Response<BlobContainerProperties>(
              std::move(response), std::move(pHttpResponse));
        }

        struct SetBlobContainerMetadataOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.GetUrl().AppendQueryParameter("comp", "metadata");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          for (const auto& pair : options.Metadata)
          {
            request.SetHeader("x-ms-meta-" + pair.first, pair.second);
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          SetBlobContainerMetadataResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          return Azure::Response<SetBlobContainerMetadataResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct ListBlobsOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::string> Prefix;
          Azure::Nullable<std::string> ContinuationToken;
          Azure::Nullable<int32_t> MaxResults;
          ListBlobsIncludeFlags Include = ListBlobsIncludeFlags::None;
        }; // struct ListBlobsOptions

        static Azure::Response<Models::_detail::ListBlobsResult> ListBlobs(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const ListBlobsOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.GetUrl().AppendQueryParameter("comp", "list");
          if (options.Prefix.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "prefix", _internal::UrlEncodeQueryParameter(options.Prefix.Value()));
          }
          if (options.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "marker", _internal::UrlEncodeQueryParameter(options.ContinuationToken.Value()));
          }
          if (options.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "maxresults", std::to_string(options.MaxResults.Value()));
          }
          std::string list_blobs_include_flags = ListBlobsIncludeFlagsToString(options.Include);
          if (!list_blobs_include_flags.empty())
          {
            request.GetUrl().AppendQueryParameter(
                "include", _internal::UrlEncodeQueryParameter(list_blobs_include_flags));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::ListBlobsResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            _internal::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = ListBlobsResultInternalFromXml(reader);
          }
          return Azure::Response<Models::_detail::ListBlobsResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct ListBlobsByHierarchyOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::string> Prefix;
          Azure::Nullable<std::string> Delimiter;
          Azure::Nullable<std::string> ContinuationToken;
          Azure::Nullable<int32_t> MaxResults;
          ListBlobsIncludeFlags Include = ListBlobsIncludeFlags::None;
        }; // struct ListBlobsByHierarchyOptions

        static Azure::Response<Models::_detail::ListBlobsByHierarchyResult> ListBlobsByHierarchy(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const ListBlobsByHierarchyOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.GetUrl().AppendQueryParameter("comp", "list");
          if (options.Prefix.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "prefix", _internal::UrlEncodeQueryParameter(options.Prefix.Value()));
          }
          if (options.Delimiter.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "delimiter", _internal::UrlEncodeQueryParameter(options.Delimiter.Value()));
          }
          if (options.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "marker", _internal::UrlEncodeQueryParameter(options.ContinuationToken.Value()));
          }
          if (options.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "maxresults", std::to_string(options.MaxResults.Value()));
          }
          std::string list_blobs_include_flags = ListBlobsIncludeFlagsToString(options.Include);
          if (!list_blobs_include_flags.empty())
          {
            request.GetUrl().AppendQueryParameter(
                "include", _internal::UrlEncodeQueryParameter(list_blobs_include_flags));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::ListBlobsByHierarchyResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            _internal::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = ListBlobsByHierarchyResultInternalFromXml(reader);
          }
          return Azure::Response<Models::_detail::ListBlobsByHierarchyResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct GetBlobContainerAccessPolicyOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.GetUrl().AppendQueryParameter("comp", "acl");
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          BlobContainerAccessPolicy response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            _internal::XmlReader reader(
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

        struct SetBlobContainerAccessPolicyOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          PublicAccessType AccessType = PublicAccessType::None;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          std::vector<SignedIdentifier> SignedIdentifiers;
        }; // struct SetBlobContainerAccessPolicyOptions

        static Azure::Response<SetBlobContainerAccessPolicyResult> SetAccessPolicy(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const SetBlobContainerAccessPolicyOptions& options,
            const Azure::Core::Context& context)
        {
          std::string xml_body;
          {
            _internal::XmlWriter writer;
            SetBlobContainerAccessPolicyOptionsToXml(writer, options);
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::End});
            xml_body = writer.GetDocument();
          }
          Azure::Core::IO::MemoryBodyStream xml_body_stream(
              reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
          auto request = Azure::Core::Http::Request(
              Azure::Core::Http::HttpMethod::Put, url, &xml_body_stream);
          request.SetHeader("Content-Length", std::to_string(xml_body_stream.Length()));
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.GetUrl().AppendQueryParameter("comp", "acl");
          if (!options.AccessType.ToString().empty())
          {
            request.SetHeader("x-ms-blob-public-access", options.AccessType.ToString());
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          SetBlobContainerAccessPolicyResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          return Azure::Response<SetBlobContainerAccessPolicyResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct AcquireBlobContainerLeaseOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.GetUrl().AppendQueryParameter("comp", "lease");
          request.SetHeader("x-ms-lease-action", "acquire");
          request.SetHeader("x-ms-lease-duration", std::to_string(options.LeaseDuration.count()));
          if (options.ProposedLeaseId.HasValue())
          {
            request.SetHeader("x-ms-proposed-lease-id", options.ProposedLeaseId.Value());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::AcquireBlobContainerLeaseResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Created)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
          return Azure::Response<Models::_detail::AcquireBlobContainerLeaseResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct RenewBlobContainerLeaseOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.GetUrl().AppendQueryParameter("comp", "lease");
          request.SetHeader("x-ms-lease-action", "renew");
          request.SetHeader("x-ms-lease-id", options.LeaseId);
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::RenewBlobContainerLeaseResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
          return Azure::Response<Models::_detail::RenewBlobContainerLeaseResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct ChangeBlobContainerLeaseOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
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
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::ChangeBlobContainerLeaseResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
          return Azure::Response<Models::_detail::ChangeBlobContainerLeaseResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct ReleaseBlobContainerLeaseOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.GetUrl().AppendQueryParameter("comp", "lease");
          request.SetHeader("x-ms-lease-action", "release");
          request.SetHeader("x-ms-lease-id", options.LeaseId);
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::ReleaseBlobContainerLeaseResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          return Azure::Response<Models::_detail::ReleaseBlobContainerLeaseResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct BreakBlobContainerLeaseOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("restype", "container");
          request.GetUrl().AppendQueryParameter("comp", "lease");
          request.SetHeader("x-ms-lease-action", "break");
          if (options.BreakPeriod.HasValue())
          {
            request.SetHeader(
                "x-ms-lease-break-period", std::to_string(options.BreakPeriod.Value().count()));
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::BreakBlobContainerLeaseResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Accepted)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.LeaseTime = std::stoi(httpResponse.GetHeaders().at("x-ms-lease-time"));
          return Azure::Response<Models::_detail::BreakBlobContainerLeaseResult>(
              std::move(response), std::move(pHttpResponse));
        }

      private:
        static BlobContainerAccessPolicy BlobContainerAccessPolicyFromXml(
            _internal::XmlReader& reader)
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
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
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
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "SignedIdentifiers")
              {
                path.emplace_back(XmlTagName::k_SignedIdentifiers);
              }
              else if (node.Name == "SignedIdentifier")
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
                ret.SignedIdentifiers.emplace_back(SignedIdentifierFromXml(reader));
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
            }
          }
          return ret;
        }

        static Models::_detail::ListBlobsByHierarchyResult
        ListBlobsByHierarchyResultInternalFromXml(_internal::XmlReader& reader)
        {
          Models::_detail::ListBlobsByHierarchyResult ret;
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
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
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
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "EnumerationResults")
              {
                path.emplace_back(XmlTagName::k_EnumerationResults);
              }
              else if (node.Name == "Prefix")
              {
                path.emplace_back(XmlTagName::k_Prefix);
              }
              else if (node.Name == "Delimiter")
              {
                path.emplace_back(XmlTagName::k_Delimiter);
              }
              else if (node.Name == "NextMarker")
              {
                path.emplace_back(XmlTagName::k_NextMarker);
              }
              else if (node.Name == "Blobs")
              {
                path.emplace_back(XmlTagName::k_Blobs);
              }
              else if (node.Name == "Blob")
              {
                path.emplace_back(XmlTagName::k_Blob);
              }
              else if (node.Name == "BlobPrefix")
              {
                path.emplace_back(XmlTagName::k_BlobPrefix);
              }
              else if (node.Name == "Name")
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
            else if (node.Type == _internal::XmlNodeType::Text)
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
            else if (node.Type == _internal::XmlNodeType::Attribute)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_EnumerationResults
                  && node.Name == "ServiceEndpoint")
              {
                ret.ServiceEndpoint = node.Value;
              }
              else if (
                  path.size() == 1 && path[0] == XmlTagName::k_EnumerationResults
                  && node.Name == "ContainerName")
              {
                ret.BlobContainerName = node.Value;
              }
            }
          }
          return ret;
        }

        static Models::_detail::ListBlobsResult ListBlobsResultInternalFromXml(
            _internal::XmlReader& reader)
        {
          Models::_detail::ListBlobsResult ret;
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
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
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
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "EnumerationResults")
              {
                path.emplace_back(XmlTagName::k_EnumerationResults);
              }
              else if (node.Name == "Prefix")
              {
                path.emplace_back(XmlTagName::k_Prefix);
              }
              else if (node.Name == "NextMarker")
              {
                path.emplace_back(XmlTagName::k_NextMarker);
              }
              else if (node.Name == "Blobs")
              {
                path.emplace_back(XmlTagName::k_Blobs);
              }
              else if (node.Name == "Blob")
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
            else if (node.Type == _internal::XmlNodeType::Text)
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
            else if (node.Type == _internal::XmlNodeType::Attribute)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_EnumerationResults
                  && node.Name == "ServiceEndpoint")
              {
                ret.ServiceEndpoint = node.Value;
              }
              else if (
                  path.size() == 1 && path[0] == XmlTagName::k_EnumerationResults
                  && node.Name == "ContainerName")
              {
                ret.BlobContainerName = node.Value;
              }
            }
          }
          return ret;
        }

        static BlobItem BlobItemFromXml(_internal::XmlReader& reader)
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
            k_AccessTierChangeTime,
            k_ArchiveStatus,
            k_RehydratePriority,
            k_LeaseStatus,
            k_LeaseState,
            k_LeaseDuration,
            k_ServerEncrypted,
            k_CustomerProvidedKeySha256,
            k_EncryptionScope,
            k_Sealed,
            k_xmsblobsequencenumber,
            k_CopyId,
            k_CopyStatus,
            k_CopySource,
            k_CopyProgress,
            k_CopyCompletionTime,
            k_CopyStatusDescription,
            k_IncrementalCopy,
            k_CopyDestinationSnapshot,
            k_DeletedTime,
            k_RemainingRetentionDays,
            k_Metadata,
            k_OrMetadata,
            k_Tags,
            k_TagSet,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
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
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "Name")
              {
                path.emplace_back(XmlTagName::k_Name);
              }
              else if (node.Name == "Deleted")
              {
                path.emplace_back(XmlTagName::k_Deleted);
              }
              else if (node.Name == "Snapshot")
              {
                path.emplace_back(XmlTagName::k_Snapshot);
              }
              else if (node.Name == "VersionId")
              {
                path.emplace_back(XmlTagName::k_VersionId);
              }
              else if (node.Name == "IsCurrentVersion")
              {
                path.emplace_back(XmlTagName::k_IsCurrentVersion);
              }
              else if (node.Name == "Properties")
              {
                path.emplace_back(XmlTagName::k_Properties);
              }
              else if (node.Name == "Content-Type")
              {
                path.emplace_back(XmlTagName::k_ContentType);
              }
              else if (node.Name == "Content-Encoding")
              {
                path.emplace_back(XmlTagName::k_ContentEncoding);
              }
              else if (node.Name == "Content-Language")
              {
                path.emplace_back(XmlTagName::k_ContentLanguage);
              }
              else if (node.Name == "Content-MD5")
              {
                path.emplace_back(XmlTagName::k_ContentMD5);
              }
              else if (node.Name == "Cache-Control")
              {
                path.emplace_back(XmlTagName::k_CacheControl);
              }
              else if (node.Name == "Content-Disposition")
              {
                path.emplace_back(XmlTagName::k_ContentDisposition);
              }
              else if (node.Name == "Creation-Time")
              {
                path.emplace_back(XmlTagName::k_CreationTime);
              }
              else if (node.Name == "Expiry-Time")
              {
                path.emplace_back(XmlTagName::k_ExpiryTime);
              }
              else if (node.Name == "LastAccessTime")
              {
                path.emplace_back(XmlTagName::k_LastAccessTime);
              }
              else if (node.Name == "Last-Modified")
              {
                path.emplace_back(XmlTagName::k_LastModified);
              }
              else if (node.Name == "Etag")
              {
                path.emplace_back(XmlTagName::k_Etag);
              }
              else if (node.Name == "Content-Length")
              {
                path.emplace_back(XmlTagName::k_ContentLength);
              }
              else if (node.Name == "BlobType")
              {
                path.emplace_back(XmlTagName::k_BlobType);
              }
              else if (node.Name == "AccessTier")
              {
                path.emplace_back(XmlTagName::k_AccessTier);
              }
              else if (node.Name == "AccessTierInferred")
              {
                path.emplace_back(XmlTagName::k_AccessTierInferred);
              }
              else if (node.Name == "AccessTierChangeTime")
              {
                path.emplace_back(XmlTagName::k_AccessTierChangeTime);
              }
              else if (node.Name == "ArchiveStatus")
              {
                path.emplace_back(XmlTagName::k_ArchiveStatus);
              }
              else if (node.Name == "RehydratePriority")
              {
                path.emplace_back(XmlTagName::k_RehydratePriority);
              }
              else if (node.Name == "LeaseStatus")
              {
                path.emplace_back(XmlTagName::k_LeaseStatus);
              }
              else if (node.Name == "LeaseState")
              {
                path.emplace_back(XmlTagName::k_LeaseState);
              }
              else if (node.Name == "LeaseDuration")
              {
                path.emplace_back(XmlTagName::k_LeaseDuration);
              }
              else if (node.Name == "ServerEncrypted")
              {
                path.emplace_back(XmlTagName::k_ServerEncrypted);
              }
              else if (node.Name == "CustomerProvidedKeySha256")
              {
                path.emplace_back(XmlTagName::k_CustomerProvidedKeySha256);
              }
              else if (node.Name == "EncryptionScope")
              {
                path.emplace_back(XmlTagName::k_EncryptionScope);
              }
              else if (node.Name == "Sealed")
              {
                path.emplace_back(XmlTagName::k_Sealed);
              }
              else if (node.Name == "x-ms-blob-sequence-number")
              {
                path.emplace_back(XmlTagName::k_xmsblobsequencenumber);
              }
              else if (node.Name == "CopyId")
              {
                path.emplace_back(XmlTagName::k_CopyId);
              }
              else if (node.Name == "CopyStatus")
              {
                path.emplace_back(XmlTagName::k_CopyStatus);
              }
              else if (node.Name == "CopySource")
              {
                path.emplace_back(XmlTagName::k_CopySource);
              }
              else if (node.Name == "CopyProgress")
              {
                path.emplace_back(XmlTagName::k_CopyProgress);
              }
              else if (node.Name == "CopyCompletionTime")
              {
                path.emplace_back(XmlTagName::k_CopyCompletionTime);
              }
              else if (node.Name == "CopyStatusDescription")
              {
                path.emplace_back(XmlTagName::k_CopyStatusDescription);
              }
              else if (node.Name == "IncrementalCopy")
              {
                path.emplace_back(XmlTagName::k_IncrementalCopy);
              }
              else if (node.Name == "CopyDestinationSnapshot")
              {
                path.emplace_back(XmlTagName::k_CopyDestinationSnapshot);
              }
              else if (node.Name == "DeletedTime")
              {
                path.emplace_back(XmlTagName::k_DeletedTime);
              }
              else if (node.Name == "RemainingRetentionDays")
              {
                path.emplace_back(XmlTagName::k_RemainingRetentionDays);
              }
              else if (node.Name == "Metadata")
              {
                path.emplace_back(XmlTagName::k_Metadata);
              }
              else if (node.Name == "OrMetadata")
              {
                path.emplace_back(XmlTagName::k_OrMetadata);
              }
              else if (node.Name == "Tags")
              {
                path.emplace_back(XmlTagName::k_Tags);
              }
              else if (node.Name == "TagSet")
              {
                path.emplace_back(XmlTagName::k_TagSet);
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
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Tags
                  && path[1] == XmlTagName::k_TagSet)
              {
                ret.Details.Tags = TagsFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_Name)
              {
                ret.Name = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_Deleted)
              {
                ret.IsDeleted = node.Value == "true";
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
                ret.IsCurrentVersion = node.Value == "true";
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
                ret.Details.AccessTier = AccessTier(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_AccessTierInferred)
              {
                ret.Details.IsAccessTierInferred = node.Value == "true";
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_AccessTierChangeTime)
              {
                ret.Details.AccessTierChangedOn
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_ArchiveStatus)
              {
                ret.Details.ArchiveStatus = ArchiveStatus(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_RehydratePriority)
              {
                ret.Details.RehydratePriority = RehydratePriority(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_LeaseStatus)
              {
                ret.Details.LeaseStatus = LeaseStatus(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_LeaseState)
              {
                ret.Details.LeaseState = LeaseState(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_LeaseDuration)
              {
                ret.Details.LeaseDuration = LeaseDurationType(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_ServerEncrypted)
              {
                ret.Details.IsServerEncrypted = node.Value == "true";
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_CustomerProvidedKeySha256)
              {
                ret.Details.EncryptionKeySha256 = Azure::Core::Convert::Base64Decode(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_EncryptionScope)
              {
                ret.Details.EncryptionScope = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_Sealed)
              {
                ret.Details.IsSealed = node.Value == "true";
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_xmsblobsequencenumber)
              {
                ret.Details.SequenceNumber = std::stoll(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_CopyId)
              {
                ret.Details.CopyId = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_CopyStatus)
              {
                ret.Details.CopyStatus = CopyStatus(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_CopySource)
              {
                ret.Details.CopySource = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_CopyProgress)
              {
                ret.Details.CopyProgress = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_CopyCompletionTime)
              {
                ret.Details.CopyCompletedOn
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_CopyStatusDescription)
              {
                ret.Details.CopyStatusDescription = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_IncrementalCopy)
              {
                ret.Details.IsIncrementalCopy = node.Value == "true";
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_Properties
                  && path[1] == XmlTagName::k_CopyDestinationSnapshot)
              {
                ret.Details.IncrementalCopyDestinationSnapshot = node.Value;
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

        static SignedIdentifier SignedIdentifierFromXml(_internal::XmlReader& reader)
        {
          SignedIdentifier ret;
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
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
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
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "Id")
              {
                path.emplace_back(XmlTagName::k_Id);
              }
              else if (node.Name == "AccessPolicy")
              {
                path.emplace_back(XmlTagName::k_AccessPolicy);
              }
              else if (node.Name == "Start")
              {
                path.emplace_back(XmlTagName::k_Start);
              }
              else if (node.Name == "Expiry")
              {
                path.emplace_back(XmlTagName::k_Expiry);
              }
              else if (node.Name == "Permission")
              {
                path.emplace_back(XmlTagName::k_Permission);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
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

        static std::map<std::string, std::string> TagsFromXml(_internal::XmlReader& reader)
        {
          std::map<std::string, std::string> ret;
          int depth = 0;
          std::string key;
          bool is_key = false;
          bool is_value = false;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              ++depth;
              if (node.Name == "Key")
              {
                is_key = true;
              }
              else if (node.Name == "Value")
              {
                is_value = true;
              }
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (depth-- == 0)
              {
                break;
              }
            }
            if (depth == 2 && node.Type == _internal::XmlNodeType::Text)
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

        static std::vector<ObjectReplicationPolicy> ObjectReplicationSourcePropertiesFromXml(
            _internal::XmlReader& reader)
        {
          int depth = 0;
          std::map<std::string, std::vector<ObjectReplicationRule>> orPropertiesMap;
          std::string policyId;
          std::string ruleId;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
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
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (depth-- == 0)
              {
                break;
              }
            }
            if (depth == 1 && node.Type == _internal::XmlNodeType::Text)
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

        static Metadata MetadataFromXml(_internal::XmlReader& reader)
        {
          Metadata ret;
          int depth = 0;
          std::string key;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (depth++ == 0)
              {
                key = node.Name;
              }
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (depth-- == 0)
              {
                break;
              }
            }
            else if (depth == 1 && node.Type == _internal::XmlNodeType::Text)
            {
              ret.emplace(std::move(key), node.Value);
            }
          }
          return ret;
        }

        static void SetBlobContainerAccessPolicyOptionsToXml(
            _internal::XmlWriter& writer,
            const SetBlobContainerAccessPolicyOptions& options)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "SignedIdentifiers"});
          for (const auto& i : options.SignedIdentifiers)
          {
            SignedIdentifierToXml(writer, i);
          }
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

        static void SignedIdentifierToXml(
            _internal::XmlWriter& writer,
            const SignedIdentifier& options)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "SignedIdentifier"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Id"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::Text, std::string(), options.Id});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "AccessPolicy"});
          if (options.StartsOn.HasValue())
          {
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Start"});
            writer.Write(_internal::XmlNode{
                _internal::XmlNodeType::Text,
                std::string(),
                options.StartsOn.Value().ToString(
                    Azure::DateTime::DateFormat::Rfc3339,
                    Azure::DateTime::TimeFractionFormat::AllDigits)});
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          }
          if (options.ExpiresOn.HasValue())
          {
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Expiry"});
            writer.Write(_internal::XmlNode{
                _internal::XmlNodeType::Text,
                std::string(),
                options.ExpiresOn.Value().ToString(
                    Azure::DateTime::DateFormat::Rfc3339,
                    Azure::DateTime::TimeFractionFormat::AllDigits)});
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          }
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Permission"});
          writer.Write(
              _internal::XmlNode{_internal::XmlNodeType::Text, std::string(), options.Permissions});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

      }; // class BlobContainer

      class Blob final {
      public:
        struct DownloadBlobOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url, false);
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          if (options.Range.HasValue())
          {
            std::string headerValue = "bytes=" + std::to_string(options.Range.Value().Offset) + "-";
            if (options.Range.Value().Length.HasValue())
            {
              headerValue += std::to_string(
                  options.Range.Value().Offset + options.Range.Value().Length.Value() - 1);
            }
            request.SetHeader("x-ms-range", std::move(headerValue));
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.Value());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.Value()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.Value().ToString());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.RangeHashAlgorithm.HasValue())
          {
            if (options.RangeHashAlgorithm.Value() == HashAlgorithm::Md5)
            {
              request.SetHeader("x-ms-range-get-content-md5", "true");
            }
            else if (options.RangeHashAlgorithm.Value() == HashAlgorithm::Crc64)
            {
              request.SetHeader("x-ms-range-get-content-crc64", "true");
            }
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          DownloadBlobResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (!(http_status_code == Azure::Core::Http::HttpStatusCode::Ok
                || http_status_code == Azure::Core::Http::HttpStatusCode::PartialContent))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.BodyStream = httpResponse.ExtractBodyStream();
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
          if (http_status_code == Azure::Core::Http::HttpStatusCode::Ok)
          {
            auto content_md5__iterator = httpResponse.GetHeaders().find("content-md5");
            if (content_md5__iterator != httpResponse.GetHeaders().end())
            {
              response.Details.HttpHeaders.ContentHash.Value
                  = Azure::Core::Convert::Base64Decode(content_md5__iterator->second);
            }
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
            response.Details.LeaseStatus = LeaseStatus(x_ms_lease_status__iterator->second);
          }
          auto x_ms_lease_state__iterator = httpResponse.GetHeaders().find("x-ms-lease-state");
          if (x_ms_lease_state__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.LeaseState = LeaseState(x_ms_lease_state__iterator->second);
          }
          auto x_ms_lease_duration__iterator
              = httpResponse.GetHeaders().find("x-ms-lease-duration");
          if (x_ms_lease_duration__iterator != httpResponse.GetHeaders().end())
          {
            response.Details.LeaseDuration
                = LeaseDurationType(x_ms_lease_duration__iterator->second);
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
                = std::stoi(x_ms_blob_committed_block_count__iterator->second);
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

        struct DeleteBlobOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Delete, url);
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          if (options.DeleteSnapshots.HasValue())
          {
            request.SetHeader("x-ms-delete-snapshots", options.DeleteSnapshots.Value().ToString());
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
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
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Accepted)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
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

        struct SetBlobExpiryOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("comp", "expiry");
          request.SetHeader("x-ms-expiry-option", options.ExpiryOrigin.ToString());
          if (options.ExpiryTime.HasValue())
          {
            request.SetHeader("x-ms-expiry-time", options.ExpiryTime.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          SetBlobExpiryResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          return Azure::Response<SetBlobExpiryResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct UndeleteBlobOptions final
        {
          Azure::Nullable<int32_t> Timeout;
        }; // struct UndeleteBlobOptions

        static Azure::Response<UndeleteBlobResult> Undelete(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const UndeleteBlobOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("comp", "undelete");
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          UndeleteBlobResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          return Azure::Response<UndeleteBlobResult>(std::move(response), std::move(pHttpResponse));
        }

        struct GetBlobPropertiesOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, url);
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.Value());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.Value()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.Value().ToString());
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          BlobProperties response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
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
            response.LeaseStatus = LeaseStatus(x_ms_lease_status__iterator->second);
          }
          auto x_ms_lease_state__iterator = httpResponse.GetHeaders().find("x-ms-lease-state");
          if (x_ms_lease_state__iterator != httpResponse.GetHeaders().end())
          {
            response.LeaseState = LeaseState(x_ms_lease_state__iterator->second);
          }
          auto x_ms_lease_duration__iterator
              = httpResponse.GetHeaders().find("x-ms-lease-duration");
          if (x_ms_lease_duration__iterator != httpResponse.GetHeaders().end())
          {
            response.LeaseDuration = LeaseDurationType(x_ms_lease_duration__iterator->second);
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
            response.AccessTier = AccessTier(x_ms_access_tier__iterator->second);
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
            response.ArchiveStatus = ArchiveStatus(x_ms_archive_status__iterator->second);
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

        struct SetBlobHttpHeadersOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "properties");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
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
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          SetBlobHttpHeadersResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
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

        struct SetBlobMetadataOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "metadata");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          for (const auto& pair : options.Metadata)
          {
            request.SetHeader("x-ms-meta-" + pair.first, pair.second);
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.Value());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.Value()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.Value().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.Value());
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          SetBlobMetadataResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          auto x_ms_blob_sequence_number__iterator
              = httpResponse.GetHeaders().find("x-ms-blob-sequence-number");
          if (x_ms_blob_sequence_number__iterator != httpResponse.GetHeaders().end())
          {
            response.SequenceNumber = std::stoll(x_ms_blob_sequence_number__iterator->second);
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
          return Azure::Response<SetBlobMetadataResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct SetBlobAccessTierOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          Models::AccessTier AccessTier;
          Azure::Nullable<Models::RehydratePriority> RehydratePriority;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<std::string> IfTags;
        }; // struct SetBlobAccessTierOptions

        static Azure::Core::Http::Request SetAccessTierCreateMessage(
            const Azure::Core::Url& url,
            const SetBlobAccessTierOptions& options)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "tier");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.SetHeader("x-ms-access-tier", options.AccessTier.ToString());
          if (options.RehydratePriority.HasValue())
          {
            request.SetHeader(
                "x-ms-rehydrate-priority", options.RehydratePriority.Value().ToString());
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
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
          auto http_status_code = httpResponse.GetStatusCode();
          if (!(http_status_code == Azure::Core::Http::HttpStatusCode::Ok
                || http_status_code == Azure::Core::Http::HttpStatusCode::Accepted))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
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

        struct CopyBlobFromUriOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          Storage::Metadata Metadata;
          std::map<std::string, std::string> Tags;
          std::string SourceUri;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<Models::AccessTier> AccessTier;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
          Azure::Nullable<Azure::DateTime> SourceIfModifiedSince;
          Azure::Nullable<Azure::DateTime> SourceIfUnmodifiedSince;
          Azure::ETag SourceIfMatch;
          Azure::ETag SourceIfNoneMatch;
          Azure::Nullable<ContentHash> TransactionalContentHash;
        }; // struct CopyBlobFromUriOptions

        static Azure::Response<CopyBlobFromUriResult> CopyFromUri(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const CopyBlobFromUriOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-requires-sync", "true");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          for (const auto& pair : options.Metadata)
          {
            request.SetHeader("x-ms-meta-" + pair.first, pair.second);
          }
          if (!options.Tags.empty())
          {
            std::string blobTagsValue;
            for (const auto& tag : options.Tags)
            {
              if (!blobTagsValue.empty())
              {
                blobTagsValue += "&";
              }
              blobTagsValue += _internal::UrlEncodeQueryParameter(tag.first) + "="
                  + _internal::UrlEncodeQueryParameter(tag.second);
            }
            request.SetHeader("x-ms-tags", std::move(blobTagsValue));
          }
          request.SetHeader("x-ms-copy-source", options.SourceUri);
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.AccessTier.HasValue())
          {
            request.SetHeader("x-ms-access-tier", options.AccessTier.Value().ToString());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          if (options.SourceIfModifiedSince.HasValue())
          {
            request.SetHeader(
                "x-ms-source-if-modified-since",
                options.SourceIfModifiedSince.Value().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.SourceIfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "x-ms-source-if-unmodified-since",
                options.SourceIfUnmodifiedSince.Value().ToString(
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
          if (options.TransactionalContentHash.HasValue())
          {
            if (options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Md5)
            {
              request.SetHeader(
                  "x-ms-source-content-md5",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.Value().Value));
            }
            else if (options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Crc64)
            {
              request.SetHeader(
                  "x-ms-source-content-crc64",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.Value().Value));
            }
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          CopyBlobFromUriResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Accepted)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
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
          return Azure::Response<CopyBlobFromUriResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct StartBlobCopyFromUriOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          Storage::Metadata Metadata;
          std::map<std::string, std::string> Tags;
          std::string SourceUri;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<Models::AccessTier> AccessTier;
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
          Azure::Nullable<std::string> SourceLeaseId;
          Azure::Nullable<bool> ShouldSealDestination;
        }; // struct StartBlobCopyFromUriOptions

        static Azure::Response<Models::_detail::StartBlobCopyFromUriResult> StartCopyFromUri(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const StartBlobCopyFromUriOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          for (const auto& pair : options.Metadata)
          {
            request.SetHeader("x-ms-meta-" + pair.first, pair.second);
          }
          if (!options.Tags.empty())
          {
            std::string blobTagsValue;
            for (const auto& tag : options.Tags)
            {
              if (!blobTagsValue.empty())
              {
                blobTagsValue += "&";
              }
              blobTagsValue += _internal::UrlEncodeQueryParameter(tag.first) + "="
                  + _internal::UrlEncodeQueryParameter(tag.second);
            }
            request.SetHeader("x-ms-tags", std::move(blobTagsValue));
          }
          request.SetHeader("x-ms-copy-source", options.SourceUri);
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.AccessTier.HasValue())
          {
            request.SetHeader("x-ms-access-tier", options.AccessTier.Value().ToString());
          }
          if (options.RehydratePriority.HasValue())
          {
            request.SetHeader(
                "x-ms-rehydrate-priority", options.RehydratePriority.Value().ToString());
          }
          if (options.ShouldSealDestination.HasValue())
          {
            request.SetHeader(
                "x-ms-seal-blob", options.ShouldSealDestination.Value() ? "true" : "false");
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          if (options.SourceIfModifiedSince.HasValue())
          {
            request.SetHeader(
                "x-ms-source-if-modified-since",
                options.SourceIfModifiedSince.Value().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.SourceIfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "x-ms-source-if-unmodified-since",
                options.SourceIfUnmodifiedSince.Value().ToString(
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
            request.SetHeader("x-ms-source-if-tags", options.SourceIfTags.Value());
          }
          if (options.SourceLeaseId.HasValue())
          {
            request.SetHeader("x-ms-source-lease-id", options.SourceLeaseId.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::StartBlobCopyFromUriResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Accepted)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
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
          return Azure::Response<Models::_detail::StartBlobCopyFromUriResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct AbortBlobCopyFromUriOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          std::string CopyId;
          Azure::Nullable<std::string> LeaseId;
        }; // struct AbortBlobCopyFromUriOptions

        static Azure::Response<AbortBlobCopyFromUriResult> AbortCopyFromUri(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const AbortBlobCopyFromUriOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("comp", "copy");
          request.GetUrl().AppendQueryParameter(
              "copyid", _internal::UrlEncodeQueryParameter(options.CopyId));
          request.SetHeader("x-ms-copy-action", "abort");
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          AbortBlobCopyFromUriResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::NoContent)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          return Azure::Response<AbortBlobCopyFromUriResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct CreateBlobSnapshotOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "snapshot");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.Value());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.Value()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.Value().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.Value());
          }
          for (const auto& pair : options.Metadata)
          {
            request.SetHeader("x-ms-meta-" + pair.first, pair.second);
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          CreateBlobSnapshotResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Created)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
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

        struct GetBlobTagsOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::string> IfTags;
          Azure::Nullable<std::string> LeaseId;
        }; // struct GetBlobTagsOptions

        static Azure::Response<Models::_detail::GetBlobTagsResult> GetTags(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const GetBlobTagsOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("comp", "tags");
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::GetBlobTagsResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            _internal::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = GetBlobTagsResultInternalFromXml(reader);
          }
          return Azure::Response<Models::_detail::GetBlobTagsResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct SetBlobTagsOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          std::map<std::string, std::string> Tags;
          Azure::Nullable<std::string> IfTags;
          Azure::Nullable<std::string> LeaseId;
        }; // struct SetBlobTagsOptions

        static Azure::Response<SetBlobTagsResult> SetTags(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const SetBlobTagsOptions& options,
            const Azure::Core::Context& context)
        {
          std::string xml_body;
          {
            _internal::XmlWriter writer;
            SetBlobTagsOptionsToXml(writer, options);
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::End});
            xml_body = writer.GetDocument();
          }
          Azure::Core::IO::MemoryBodyStream xml_body_stream(
              reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
          auto request = Azure::Core::Http::Request(
              Azure::Core::Http::HttpMethod::Put, url, &xml_body_stream);
          request.SetHeader("Content-Length", std::to_string(xml_body_stream.Length()));
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("comp", "tags");
          request.SetHeader("Content-Type", "application/xml; charset=UTF-8");
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          SetBlobTagsResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::NoContent)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          return Azure::Response<SetBlobTagsResult>(std::move(response), std::move(pHttpResponse));
        }

        struct AcquireBlobLeaseOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("comp", "lease");
          request.SetHeader("x-ms-lease-action", "acquire");
          request.SetHeader("x-ms-lease-duration", std::to_string(options.LeaseDuration.count()));
          if (options.ProposedLeaseId.HasValue())
          {
            request.SetHeader("x-ms-proposed-lease-id", options.ProposedLeaseId.Value());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::AcquireBlobLeaseResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Created)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
          return Azure::Response<Models::_detail::AcquireBlobLeaseResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct RenewBlobLeaseOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("comp", "lease");
          request.SetHeader("x-ms-lease-action", "renew");
          request.SetHeader("x-ms-lease-id", options.LeaseId);
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::RenewBlobLeaseResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
          return Azure::Response<Models::_detail::RenewBlobLeaseResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct ChangeBlobLeaseOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("comp", "lease");
          request.SetHeader("x-ms-lease-action", "change");
          request.SetHeader("x-ms-lease-id", options.LeaseId);
          request.SetHeader("x-ms-proposed-lease-id", options.ProposedLeaseId);
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::ChangeBlobLeaseResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.LeaseId = httpResponse.GetHeaders().at("x-ms-lease-id");
          return Azure::Response<Models::_detail::ChangeBlobLeaseResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct ReleaseBlobLeaseOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("comp", "lease");
          request.SetHeader("x-ms-lease-action", "release");
          request.SetHeader("x-ms-lease-id", options.LeaseId);
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::ReleaseBlobLeaseResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          return Azure::Response<Models::_detail::ReleaseBlobLeaseResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct BreakBlobLeaseOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("comp", "lease");
          request.SetHeader("x-ms-lease-action", "break");
          if (options.BreakPeriod.HasValue())
          {
            request.SetHeader(
                "x-ms-lease-break-period", std::to_string(options.BreakPeriod.Value().count()));
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::BreakBlobLeaseResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Accepted)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.LeaseTime = std::stoi(httpResponse.GetHeaders().at("x-ms-lease-time"));
          return Azure::Response<Models::_detail::BreakBlobLeaseResult>(
              std::move(response), std::move(pHttpResponse));
        }

      private:
        static Models::_detail::GetBlobTagsResult GetBlobTagsResultInternalFromXml(
            _internal::XmlReader& reader)
        {
          Models::_detail::GetBlobTagsResult ret;
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
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
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
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "Tags")
              {
                path.emplace_back(XmlTagName::k_Tags);
              }
              else if (node.Name == "TagSet")
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
            else if (node.Type == _internal::XmlNodeType::Text)
            {
            }
          }
          return ret;
        }

        static std::map<std::string, std::string> TagsFromXml(_internal::XmlReader& reader)
        {
          std::map<std::string, std::string> ret;
          int depth = 0;
          std::string key;
          bool is_key = false;
          bool is_value = false;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              ++depth;
              if (node.Name == "Key")
              {
                is_key = true;
              }
              else if (node.Name == "Value")
              {
                is_value = true;
              }
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (depth-- == 0)
              {
                break;
              }
            }
            if (depth == 2 && node.Type == _internal::XmlNodeType::Text)
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
            _internal::XmlWriter& writer,
            const SetBlobTagsOptions& options)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Tags"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "TagSet"});
          for (const auto& i : options.Tags)
          {
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Tag"});
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Key"});
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::Text, std::string(), i.first});
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Value"});
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::Text, std::string(), i.second});
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          }
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

      }; // class Blob

      class BlockBlob final {
      public:
        struct UploadBlockBlobOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<ContentHash> TransactionalContentHash;
          BlobHttpHeaders HttpHeaders;
          Storage::Metadata Metadata;
          std::map<std::string, std::string> Tags;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<Models::AccessTier> AccessTier;
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
            Azure::Core::IO::BodyStream& requestBody,
            const UploadBlockBlobOptions& options,
            const Azure::Core::Context& context)
        {
          auto request
              = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, &requestBody);
          request.SetHeader("Content-Length", std::to_string(requestBody.Length()));
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.Value());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.Value()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.Value().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.Value());
          }
          if (options.TransactionalContentHash.HasValue())
          {
            if (options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Md5)
            {
              request.SetHeader(
                  "Content-MD5",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.Value().Value));
            }
            else if (options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Crc64)
            {
              request.SetHeader(
                  "x-ms-content-crc64",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.Value().Value));
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
          if (!options.Tags.empty())
          {
            std::string blobTagsValue;
            for (const auto& tag : options.Tags)
            {
              if (!blobTagsValue.empty())
              {
                blobTagsValue += "&";
              }
              blobTagsValue += _internal::UrlEncodeQueryParameter(tag.first) + "="
                  + _internal::UrlEncodeQueryParameter(tag.second);
            }
            request.SetHeader("x-ms-tags", std::move(blobTagsValue));
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          request.SetHeader("x-ms-blob-type", "BlockBlob");
          if (options.AccessTier.HasValue())
          {
            request.SetHeader("x-ms-access-tier", options.AccessTier.Value().ToString());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          UploadBlockBlobResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Created)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
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

        struct StageBlockOptions final
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
            Azure::Core::IO::BodyStream& requestBody,
            const StageBlockOptions& options,
            const Azure::Core::Context& context)
        {
          auto request
              = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, &requestBody);
          request.SetHeader("Content-Length", std::to_string(requestBody.Length()));
          request.GetUrl().AppendQueryParameter("comp", "block");
          request.GetUrl().AppendQueryParameter(
              "blockid", _internal::UrlEncodeQueryParameter(options.BlockId));
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          if (options.TransactionalContentHash.HasValue())
          {
            if (options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Md5)
            {
              request.SetHeader(
                  "Content-MD5",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.Value().Value));
            }
            else if (options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Crc64)
            {
              request.SetHeader(
                  "x-ms-content-crc64",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.Value().Value));
            }
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.Value());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.Value()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.Value().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          StageBlockResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Created)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
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

        struct StageBlockFromUriOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "block");
          request.GetUrl().AppendQueryParameter(
              "blockid", _internal::UrlEncodeQueryParameter(options.BlockId));
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.SetHeader("x-ms-copy-source", options.SourceUri);
          if (options.SourceRange.HasValue())
          {
            std::string headerValue
                = "bytes=" + std::to_string(options.SourceRange.Value().Offset) + "-";
            if (options.SourceRange.Value().Length.HasValue())
            {
              headerValue += std::to_string(
                  options.SourceRange.Value().Offset + options.SourceRange.Value().Length.Value()
                  - 1);
            }
            request.SetHeader("x-ms-source_range", std::move(headerValue));
          }
          if (options.TransactionalContentHash.HasValue())
          {
            if (options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Md5)
            {
              request.SetHeader(
                  "x-ms-source-content-md5",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.Value().Value));
            }
            else if (options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Crc64)
            {
              request.SetHeader(
                  "x-ms-source-content-crc64",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.Value().Value));
            }
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.Value());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.Value()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.Value().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.Value());
          }
          if (options.SourceIfModifiedSince.HasValue())
          {
            request.SetHeader(
                "x-ms-source-if-modified-since",
                options.SourceIfModifiedSince.Value().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.SourceIfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "x-ms-source-if-unmodified-since",
                options.SourceIfUnmodifiedSince.Value().ToString(
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
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Created)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
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

        struct CommitBlockListOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          std::vector<std::pair<BlockType, std::string>> BlockList;
          BlobHttpHeaders HttpHeaders;
          Storage::Metadata Metadata;
          std::map<std::string, std::string> Tags;
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
          Azure::Nullable<Models::AccessTier> AccessTier;
        }; // struct CommitBlockListOptions

        static Azure::Response<CommitBlockListResult> CommitBlockList(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const CommitBlockListOptions& options,
            const Azure::Core::Context& context)
        {
          std::string xml_body;
          {
            _internal::XmlWriter writer;
            CommitBlockListOptionsToXml(writer, options);
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::End});
            xml_body = writer.GetDocument();
          }
          Azure::Core::IO::MemoryBodyStream xml_body_stream(
              reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
          auto request = Azure::Core::Http::Request(
              Azure::Core::Http::HttpMethod::Put, url, &xml_body_stream);
          request.SetHeader("Content-Length", std::to_string(xml_body_stream.Length()));
          request.GetUrl().AppendQueryParameter("comp", "blocklist");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
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
          if (!options.Tags.empty())
          {
            std::string blobTagsValue;
            for (const auto& tag : options.Tags)
            {
              if (!blobTagsValue.empty())
              {
                blobTagsValue += "&";
              }
              blobTagsValue += _internal::UrlEncodeQueryParameter(tag.first) + "="
                  + _internal::UrlEncodeQueryParameter(tag.second);
            }
            request.SetHeader("x-ms-tags", std::move(blobTagsValue));
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.Value());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.Value()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.Value().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.Value());
          }
          if (options.AccessTier.HasValue())
          {
            request.SetHeader("x-ms-access-tier", options.AccessTier.Value().ToString());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          CommitBlockListResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Created)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
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

        struct GetBlockListOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          BlockListType ListType = BlockListType::Committed;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<std::string> IfTags;
        }; // struct GetBlockListOptions

        static Azure::Response<GetBlockListResult> GetBlockList(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const GetBlockListOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter("comp", "blocklist");
          request.GetUrl().AppendQueryParameter(
              "blocklisttype", _internal::UrlEncodeQueryParameter(options.ListType.ToString()));
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.IfTags.HasValue())
          {
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          GetBlockListResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            _internal::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = GetBlockListResultFromXml(reader);
          }
          auto etag__iterator = httpResponse.GetHeaders().find("etag");
          if (etag__iterator != httpResponse.GetHeaders().end())
          {
            response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          }
          auto last_modified__iterator = httpResponse.GetHeaders().find("last-modified");
          if (last_modified__iterator != httpResponse.GetHeaders().end())
          {
            response.LastModified = Azure::DateTime::Parse(
                last_modified__iterator->second, Azure::DateTime::DateFormat::Rfc1123);
          }
          auto x_ms_blob_content_length__iterator
              = httpResponse.GetHeaders().find("x-ms-blob-content-length");
          if (x_ms_blob_content_length__iterator != httpResponse.GetHeaders().end())
          {
            response.BlobSize = std::stoll(x_ms_blob_content_length__iterator->second);
          }
          return Azure::Response<GetBlockListResult>(std::move(response), std::move(pHttpResponse));
        }

      private:
        static GetBlockListResult GetBlockListResultFromXml(_internal::XmlReader& reader)
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
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
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
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "BlockList")
              {
                path.emplace_back(XmlTagName::k_BlockList);
              }
              else if (node.Name == "CommittedBlocks")
              {
                path.emplace_back(XmlTagName::k_CommittedBlocks);
              }
              else if (node.Name == "Block")
              {
                path.emplace_back(XmlTagName::k_Block);
              }
              else if (node.Name == "UncommittedBlocks")
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
            else if (node.Type == _internal::XmlNodeType::Text)
            {
            }
          }
          return ret;
        }

        static BlobBlock BlobBlockFromXml(_internal::XmlReader& reader)
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
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
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
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "Name")
              {
                path.emplace_back(XmlTagName::k_Name);
              }
              else if (node.Name == "Size")
              {
                path.emplace_back(XmlTagName::k_Size);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
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
            _internal::XmlWriter& writer,
            const CommitBlockListOptions& options)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "BlockList"});
          for (const auto& i : options.BlockList)
          {
            writer.Write(
                _internal::XmlNode{_internal::XmlNodeType::StartTag, i.first.ToString(), i.second});
          }
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

      }; // class BlockBlob

      class PageBlob final {
      public:
        struct CreatePageBlobOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          int64_t BlobSize = -1;
          Azure::Nullable<int64_t> SequenceNumber;
          BlobHttpHeaders HttpHeaders;
          Storage::Metadata Metadata;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<Models::AccessTier> AccessTier;
          Azure::Nullable<std::string> EncryptionKey;
          Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
          Azure::Nullable<EncryptionAlgorithmType> EncryptionAlgorithm;
          Azure::Nullable<std::string> EncryptionScope;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
          std::map<std::string, std::string> Tags;
        }; // struct CreatePageBlobOptions

        static Azure::Response<CreatePageBlobResult> Create(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const CreatePageBlobOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
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
          if (!options.Tags.empty())
          {
            std::string blobTagsValue;
            for (const auto& tag : options.Tags)
            {
              if (!blobTagsValue.empty())
              {
                blobTagsValue += "&";
              }
              blobTagsValue += _internal::UrlEncodeQueryParameter(tag.first) + "="
                  + _internal::UrlEncodeQueryParameter(tag.second);
            }
            request.SetHeader("x-ms-tags", std::move(blobTagsValue));
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          request.SetHeader("x-ms-blob-type", "PageBlob");
          request.SetHeader("x-ms-blob-content-length", std::to_string(options.BlobSize));
          if (options.SequenceNumber.HasValue())
          {
            request.SetHeader(
                "x-ms-blob-sequence-number", std::to_string(options.SequenceNumber.Value()));
          }
          if (options.AccessTier.HasValue())
          {
            request.SetHeader("x-ms-access-tier", options.AccessTier.Value().ToString());
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.Value());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.Value()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.Value().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.Value());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          CreatePageBlobResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Created)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
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

        struct UploadPageBlobPagesOptions final
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

        static Azure::Response<UploadPagesResult> UploadPages(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            Azure::Core::IO::BodyStream& requestBody,
            const UploadPageBlobPagesOptions& options,
            const Azure::Core::Context& context)
        {
          auto request
              = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, &requestBody);
          request.SetHeader("Content-Length", std::to_string(requestBody.Length()));
          request.GetUrl().AppendQueryParameter("comp", "page");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          {
            std::string headerValue = "bytes=" + std::to_string(options.Range.Offset) + "-";
            if (options.Range.Length.HasValue())
            {
              headerValue
                  += std::to_string(options.Range.Offset + options.Range.Length.Value() - 1);
            }
            request.SetHeader("x-ms-range", std::move(headerValue));
          }
          if (options.TransactionalContentHash.HasValue())
          {
            if (options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Md5)
            {
              request.SetHeader(
                  "Content-MD5",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.Value().Value));
            }
            else if (options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Crc64)
            {
              request.SetHeader(
                  "x-ms-content-crc64",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.Value().Value));
            }
          }
          request.SetHeader("x-ms-page-write", "update");
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.IfSequenceNumberLessThanOrEqualTo.HasValue())
          {
            request.SetHeader(
                "x-ms-if-sequence-number-le",
                std::to_string(options.IfSequenceNumberLessThanOrEqualTo.Value()));
          }
          if (options.IfSequenceNumberLessThan.HasValue())
          {
            request.SetHeader(
                "x-ms-if-sequence-number-lt",
                std::to_string(options.IfSequenceNumberLessThan.Value()));
          }
          if (options.IfSequenceNumberEqualTo.HasValue())
          {
            request.SetHeader(
                "x-ms-if-sequence-number-eq",
                std::to_string(options.IfSequenceNumberEqualTo.Value()));
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.Value());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.Value()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.Value().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.Value());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          UploadPagesResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Created)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
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
          return Azure::Response<UploadPagesResult>(std::move(response), std::move(pHttpResponse));
        }

        struct UploadPageBlobPagesFromUriOptions final
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
          Azure::Nullable<Azure::DateTime> SourceIfModifiedSince;
          Azure::Nullable<Azure::DateTime> SourceIfUnmodifiedSince;
          Azure::ETag SourceIfMatch;
          Azure::ETag SourceIfNoneMatch;
        }; // struct UploadPageBlobPagesFromUriOptions

        static Azure::Response<UploadPagesFromUriResult> UploadPagesFromUri(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const UploadPageBlobPagesFromUriOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "page");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          {
            std::string headerValue = "bytes=" + std::to_string(options.Range.Offset) + "-";
            if (options.Range.Length.HasValue())
            {
              headerValue
                  += std::to_string(options.Range.Offset + options.Range.Length.Value() - 1);
            }
            request.SetHeader("x-ms-range", std::move(headerValue));
          }
          request.SetHeader("x-ms-copy-source", options.SourceUri);
          {
            std::string headerValue = "bytes=" + std::to_string(options.SourceRange.Offset) + "-";
            if (options.SourceRange.Length.HasValue())
            {
              headerValue += std::to_string(
                  options.SourceRange.Offset + options.SourceRange.Length.Value() - 1);
            }
            request.SetHeader("x-ms-source-range", std::move(headerValue));
          }
          if (options.TransactionalContentHash.HasValue())
          {
            if (options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Md5)
            {
              request.SetHeader(
                  "x-ms-source-content-md5",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.Value().Value));
            }
            else if (options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Crc64)
            {
              request.SetHeader(
                  "x-ms-source-content-crc64",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.Value().Value));
            }
          }
          request.SetHeader("x-ms-page-write", "update");
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.IfSequenceNumberLessThanOrEqualTo.HasValue())
          {
            request.SetHeader(
                "x-ms-if-sequence-number-le",
                std::to_string(options.IfSequenceNumberLessThanOrEqualTo.Value()));
          }
          if (options.IfSequenceNumberLessThan.HasValue())
          {
            request.SetHeader(
                "x-ms-if-sequence-number-lt",
                std::to_string(options.IfSequenceNumberLessThan.Value()));
          }
          if (options.IfSequenceNumberEqualTo.HasValue())
          {
            request.SetHeader(
                "x-ms-if-sequence-number-eq",
                std::to_string(options.IfSequenceNumberEqualTo.Value()));
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.Value());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.Value()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.Value().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.Value());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          if (options.SourceIfModifiedSince.HasValue())
          {
            request.SetHeader(
                "x-ms-source-if-modified-since",
                options.SourceIfModifiedSince.Value().ToString(
                    Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.SourceIfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "x-ms-source-if-unmodified-since",
                options.SourceIfUnmodifiedSince.Value().ToString(
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
          UploadPagesFromUriResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Created)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
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
          return Azure::Response<UploadPagesFromUriResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct ClearPageBlobPagesOptions final
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

        static Azure::Response<ClearPagesResult> ClearPages(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const ClearPageBlobPagesOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "page");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          {
            std::string headerValue = "bytes=" + std::to_string(options.Range.Offset) + "-";
            if (options.Range.Length.HasValue())
            {
              headerValue
                  += std::to_string(options.Range.Offset + options.Range.Length.Value() - 1);
            }
            request.SetHeader("x-ms-range", std::move(headerValue));
          }
          request.SetHeader("x-ms-page-write", "clear");
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.IfSequenceNumberLessThanOrEqualTo.HasValue())
          {
            request.SetHeader(
                "x-ms-if-sequence-number-le",
                std::to_string(options.IfSequenceNumberLessThanOrEqualTo.Value()));
          }
          if (options.IfSequenceNumberLessThan.HasValue())
          {
            request.SetHeader(
                "x-ms-if-sequence-number-lt",
                std::to_string(options.IfSequenceNumberLessThan.Value()));
          }
          if (options.IfSequenceNumberEqualTo.HasValue())
          {
            request.SetHeader(
                "x-ms-if-sequence-number-eq",
                std::to_string(options.IfSequenceNumberEqualTo.Value()));
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.Value());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.Value()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.Value().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.Value());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          ClearPagesResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Created)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.SequenceNumber
              = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-sequence-number"));
          return Azure::Response<ClearPagesResult>(std::move(response), std::move(pHttpResponse));
        }

        struct ResizePageBlobOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          int64_t BlobSize = -1;
          Azure::Nullable<std::string> LeaseId;
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "properties");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.SetHeader("x-ms-blob-content-length", std::to_string(options.BlobSize));
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          ResizePageBlobResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.SequenceNumber
              = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-sequence-number"));
          return Azure::Response<ResizePageBlobResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct UpdatePageBlobSequenceNumberOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          SequenceNumberAction Action;
          Azure::Nullable<int64_t> SequenceNumber;
          Azure::Nullable<std::string> LeaseId;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct UpdatePageBlobSequenceNumberOptions

        static Azure::Response<UpdateSequenceNumberResult> UpdateSequenceNumber(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const UpdatePageBlobSequenceNumberOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "properties");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          request.SetHeader("x-ms-sequence-number-action", options.Action.ToString());
          if (options.SequenceNumber.HasValue())
          {
            request.SetHeader(
                "x-ms-blob-sequence-number", std::to_string(options.SequenceNumber.Value()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          UpdateSequenceNumberResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.SequenceNumber
              = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-sequence-number"));
          return Azure::Response<UpdateSequenceNumberResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct GetPageBlobPageRangesOptions final
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

        static Azure::Response<Models::_detail::GetPageRangesResult> GetPageRanges(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const GetPageBlobPageRangesOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter("comp", "pagelist");
          if (options.PreviousSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "prevsnapshot",
                _internal::UrlEncodeQueryParameter(options.PreviousSnapshot.Value()));
          }
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          if (options.Range.HasValue())
          {
            std::string headerValue = "bytes=" + std::to_string(options.Range.Value().Offset) + "-";
            if (options.Range.Value().Length.HasValue())
            {
              headerValue += std::to_string(
                  options.Range.Value().Offset + options.Range.Value().Length.Value() - 1);
            }
            request.SetHeader("x-ms-range", std::move(headerValue));
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.PreviousSnapshotUrl.HasValue())
          {
            request.SetHeader("x-ms-previous-snapshot-url", options.PreviousSnapshotUrl.Value());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::GetPageRangesResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            _internal::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = GetPageRangesResultInternalFromXml(reader);
          }
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          response.BlobSize = std::stoll(httpResponse.GetHeaders().at("x-ms-blob-content-length"));
          return Azure::Response<Models::_detail::GetPageRangesResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct StartBlobCopyIncrementalOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          std::string CopySource;
          Azure::Nullable<Azure::DateTime> IfModifiedSince;
          Azure::Nullable<Azure::DateTime> IfUnmodifiedSince;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<std::string> IfTags;
        }; // struct StartBlobCopyIncrementalOptions

        static Azure::Response<Models::_detail::StartBlobCopyIncrementalResult>
        StartCopyIncremental(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const StartBlobCopyIncrementalOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "incrementalcopy");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.SetHeader("x-ms-copy-source", options.CopySource);
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::StartBlobCopyIncrementalResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Accepted)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
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
          return Azure::Response<Models::_detail::StartBlobCopyIncrementalResult>(
              std::move(response), std::move(pHttpResponse));
        }

      private:
        static Models::_detail::GetPageRangesResult GetPageRangesResultInternalFromXml(
            _internal::XmlReader& reader)
        {
          Models::_detail::GetPageRangesResult ret;
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
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
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
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "PageList")
              {
                path.emplace_back(XmlTagName::k_PageList);
              }
              else if (node.Name == "PageRange")
              {
                path.emplace_back(XmlTagName::k_PageRange);
              }
              else if (node.Name == "ClearRange")
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
            else if (node.Type == _internal::XmlNodeType::Text)
            {
            }
          }
          return ret;
        }

        static Azure::Core::Http::HttpRange ClearRangesFromXml(_internal::XmlReader& reader)
        {
          int depth = 0;
          bool is_start = false;
          bool is_end = false;
          int64_t start = 0;
          int64_t end = 0;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::StartTag && node.Name == "Start")
            {
              ++depth;
              is_start = true;
            }
            else if (node.Type == _internal::XmlNodeType::StartTag && node.Name == "End")
            {
              ++depth;
              is_end = true;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              is_start = false;
              is_end = false;
              if (depth-- == 0)
              {
                break;
              }
            }
            if (depth == 1 && node.Type == _internal::XmlNodeType::Text)
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

        static Azure::Core::Http::HttpRange PageRangesFromXml(_internal::XmlReader& reader)
        {
          int depth = 0;
          bool is_start = false;
          bool is_end = false;
          int64_t start = 0;
          int64_t end = 0;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::StartTag && node.Name == "Start")
            {
              ++depth;
              is_start = true;
            }
            else if (node.Type == _internal::XmlNodeType::StartTag && node.Name == "End")
            {
              ++depth;
              is_end = true;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              is_start = false;
              is_end = false;
              if (depth-- == 0)
              {
                break;
              }
            }
            if (depth == 1 && node.Type == _internal::XmlNodeType::Text)
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

      class AppendBlob final {
      public:
        struct CreateAppendBlobOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          BlobHttpHeaders HttpHeaders;
          Storage::Metadata Metadata;
          std::map<std::string, std::string> Tags;
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
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
          if (!options.Tags.empty())
          {
            std::string blobTagsValue;
            for (const auto& tag : options.Tags)
            {
              if (!blobTagsValue.empty())
              {
                blobTagsValue += "&";
              }
              blobTagsValue += _internal::UrlEncodeQueryParameter(tag.first) + "="
                  + _internal::UrlEncodeQueryParameter(tag.second);
            }
            request.SetHeader("x-ms-tags", std::move(blobTagsValue));
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          request.SetHeader("x-ms-blob-type", "AppendBlob");
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.Value());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.Value()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.Value().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.Value());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          CreateAppendBlobResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Created)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
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

        struct AppendBlockOptions final
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
            Azure::Core::IO::BodyStream& requestBody,
            const AppendBlockOptions& options,
            const Azure::Core::Context& context)
        {
          auto request
              = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, &requestBody);
          request.SetHeader("Content-Length", std::to_string(requestBody.Length()));
          request.GetUrl().AppendQueryParameter("comp", "appendblock");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          if (options.TransactionalContentHash.HasValue())
          {
            if (options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Md5)
            {
              request.SetHeader(
                  "Content-MD5",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.Value().Value));
            }
            else if (options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Crc64)
            {
              request.SetHeader(
                  "x-ms-content-crc64",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.Value().Value));
            }
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.MaxSize.HasValue())
          {
            request.SetHeader(
                "x-ms-blob-condition-maxsize", std::to_string(options.MaxSize.Value()));
          }
          if (options.AppendPosition.HasValue())
          {
            request.SetHeader(
                "x-ms-blob-condition-appendpos", std::to_string(options.AppendPosition.Value()));
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.Value());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.Value()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.Value().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.Value());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          AppendBlockResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Created)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
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
              = std::stoi(httpResponse.GetHeaders().at("x-ms-blob-committed-block-count"));
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

        struct AppendBlockFromUriOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "appendblock");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.SetHeader("x-ms-copy-source", options.SourceUri);
          if (options.SourceRange.HasValue())
          {
            std::string headerValue
                = "bytes=" + std::to_string(options.SourceRange.Value().Offset) + "-";
            if (options.SourceRange.Value().Length.HasValue())
            {
              headerValue += std::to_string(
                  options.SourceRange.Value().Offset + options.SourceRange.Value().Length.Value()
                  - 1);
            }
            request.SetHeader("x-ms-source-range", std::move(headerValue));
          }
          if (options.TransactionalContentHash.HasValue())
          {
            if (options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Md5)
            {
              request.SetHeader(
                  "x-ms-source-content-md5",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.Value().Value));
            }
            else if (options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Crc64)
            {
              request.SetHeader(
                  "x-ms-source-content-crc64",
                  Azure::Core::Convert::Base64Encode(
                      options.TransactionalContentHash.Value().Value));
            }
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.MaxSize.HasValue())
          {
            request.SetHeader(
                "x-ms-blob-condition-maxsize", std::to_string(options.MaxSize.Value()));
          }
          if (options.AppendPosition.HasValue())
          {
            request.SetHeader(
                "x-ms-blob-condition-appendpos", std::to_string(options.AppendPosition.Value()));
          }
          if (options.EncryptionKey.HasValue())
          {
            request.SetHeader("x-ms-encryption-key", options.EncryptionKey.Value());
          }
          if (options.EncryptionKeySha256.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-key-sha256",
                Azure::Core::Convert::Base64Encode(options.EncryptionKeySha256.Value()));
          }
          if (options.EncryptionAlgorithm.HasValue())
          {
            request.SetHeader(
                "x-ms-encryption-algorithm", options.EncryptionAlgorithm.Value().ToString());
          }
          if (options.EncryptionScope.HasValue())
          {
            request.SetHeader("x-ms-encryption-scope", options.EncryptionScope.Value());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          AppendBlockFromUriResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Created)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
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
              = std::stoi(httpResponse.GetHeaders().at("x-ms-blob-committed-block-count"));
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

        struct SealAppendBlobOptions final
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
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "seal");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          if (options.LeaseId.HasValue())
          {
            request.SetHeader("x-ms-lease-id", options.LeaseId.Value());
          }
          if (options.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Modified-Since",
                options.IfModifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
          }
          if (options.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                "If-Unmodified-Since",
                options.IfUnmodifiedSince.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
            request.SetHeader("x-ms-if-tags", options.IfTags.Value());
          }
          if (options.AppendPosition.HasValue())
          {
            request.SetHeader(
                "x-ms-blob-condition-appendpos", std::to_string(options.AppendPosition.Value()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          SealAppendBlobResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.ETag = Azure::ETag(httpResponse.GetHeaders().at("etag"));
          response.LastModified = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("last-modified"), Azure::DateTime::DateFormat::Rfc1123);
          return Azure::Response<SealAppendBlobResult>(
              std::move(response), std::move(pHttpResponse));
        }

      private:
      }; // class AppendBlob

      class BlobBatch final {
      public:
        struct SubmitBlobBatchOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          std::string ContentType;
        }; // struct SubmitBlobBatchOptions

        static Azure::Response<Models::_detail::SubmitBlobBatchResult> SubmitBatch(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            Azure::Core::IO::BodyStream& requestBody,
            const SubmitBlobBatchOptions& options,
            const Azure::Core::Context& context)
        {
          auto request
              = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Post, url, &requestBody);
          request.SetHeader("Content-Length", std::to_string(requestBody.Length()));
          request.GetUrl().AppendQueryParameter("comp", "batch");
          request.SetHeader("x-ms-version", "2020-04-08");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.SetHeader("Content-Type", options.ContentType);
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::SubmitBlobBatchResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Accepted)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.ContentType = httpResponse.GetHeaders().at("content-type");
          return Azure::Response<Models::_detail::SubmitBlobBatchResult>(
              std::move(response), std::move(pHttpResponse));
        }

      private:
      }; // class BlobBatch

    }; // class BlobRestClient

  } // namespace _detail

}}} // namespace Azure::Storage::Blobs
