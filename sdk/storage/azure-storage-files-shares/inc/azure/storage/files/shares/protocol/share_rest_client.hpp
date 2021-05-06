
// Copyright (c) Microsoft Corporation. All rights reserved.
// An SPDX-License-Identifier: MIT

#pragma once

#include "azure/storage/files/shares/dll_import_export.hpp"
#include "azure/storage/files/shares/share_file_attributes.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <azure/core/datetime.hpp>
#include <azure/core/etag.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/response.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_exception.hpp>
#include <azure/storage/common/xml_wrapper.hpp>

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  namespace Models {
    struct FileHttpHeaders
    {
      std::string CacheControl;
      std::string ContentDisposition;
      std::string ContentEncoding;
      std::string ContentLanguage;
      std::string ContentType;
      Storage::ContentHash ContentHash;
    };

    struct FileSmbProperties
    {
      /**
       * @brief Permission key of the directory or file.
       */
      Azure::Nullable<std::string> PermissionKey;

      /**
       * @brief If specified, the provided file attributes shall be set. Default value:
       * FileAttribute::Archive for file and FileAttribute::Directory for directory.
       * FileAttribute::None can also be specified as default.
       */
      FileAttributes Attributes;

      /**
       * @brief Creation time for the file/directory.
       */
      Azure::Nullable<DateTime> CreatedOn;

      /**
       * @brief Last write time for the file/directory.
       */
      Azure::Nullable<DateTime> LastWrittenOn;

      /**
       * @brief Changed time for the file/directory.
       */
      Azure::Nullable<DateTime> ChangedOn;

      /**
       * @brief The fileId of the file.
       */
      std::string FileId;

      /**
       * @brief The parentId of the file
       */
      std::string ParentFileId;
    };
    // Specifies the access tier of the share.
    class AccessTier {
    public:
      AccessTier() = default;
      explicit AccessTier(std::string value) : m_value(std::move(value)) {}
      bool operator==(const AccessTier& other) const { return m_value == other.m_value; }
      bool operator!=(const AccessTier& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static AccessTier TransactionOptimized;
      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static AccessTier Hot;
      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static AccessTier Cool;
      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static AccessTier Premium;

    private:
      std::string m_value;
    }; // extensible enum AccessTier

    // Specifies the option to copy file security descriptor from source file or to set it using the
    // value which is defined by the header value of x-ms-file-permission or
    // x-ms-file-permission-key.
    class PermissionCopyMode {
    public:
      PermissionCopyMode() = default;
      explicit PermissionCopyMode(std::string value) : m_value(std::move(value)) {}
      bool operator==(const PermissionCopyMode& other) const { return m_value == other.m_value; }
      bool operator!=(const PermissionCopyMode& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static PermissionCopyMode Source;
      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static PermissionCopyMode Override;

    private:
      std::string m_value;
    }; // extensible enum PermissionCopyMode

    // Specifies the option include to delete the base share and all of its snapshots.
    class DeleteSnapshotsOption {
    public:
      DeleteSnapshotsOption() = default;
      explicit DeleteSnapshotsOption(std::string value) : m_value(std::move(value)) {}
      bool operator==(const DeleteSnapshotsOption& other) const { return m_value == other.m_value; }
      bool operator!=(const DeleteSnapshotsOption& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static DeleteSnapshotsOption Include;

    private:
      std::string m_value;
    }; // extensible enum DeleteSnapshotsOption

    // An Access policy.
    struct AccessPolicy
    {
      DateTime StartsOn; // The date-time the policy is active.
      DateTime ExpiresOn; // The date-time the policy expires.
      std::string Permission; // The permissions for the ACL policy.
    };

    // CORS is an HTTP feature that enables a web application running under one domain to access
    // resources in another domain. Web browsers implement a security restriction known as
    // same-origin policy that prevents a web page from calling APIs in a different domain; CORS
    // provides a secure way to allow one domain (the origin domain) to call APIs in another domain.
    struct CorsRule
    {
      std::string AllowedOrigins; // The origin domains that are permitted to make a request against
                                  // the storage service via CORS. The origin domain is the domain
                                  // from which the request originates. Note that the origin must be
                                  // an exact case-sensitive match with the origin that the user age
                                  // sends to the service. You can also use the wildcard character
                                  // '*' to allow all origin domains to make requests via CORS.
      std::string AllowedMethods; // The methods (HTTP request verbs) that the origin domain may use
                                  // for a CORS request. (comma separated)
      std::string AllowedHeaders; // The request headers that the origin domain may specify on the
                                  // CORS request.
      std::string ExposedHeaders; // The response headers that may be sent in the response to the
                                  // CORS request and exposed by the browser to the request issuer.
      int32_t MaxAgeInSeconds = int32_t(); // The maximum amount time that a browser should cache
                                           // the preflight OPTIONS request.
    };

    // A listed directory item.
    struct DirectoryItem
    {
      std::string Name;
    };

    // File properties.
    struct FileItemDetails
    {
      int64_t ContentLength
          = int64_t(); // Content length of the file. This value may not be up-to-date since an SMB
                       // client may have modified the file locally. The value of Content-Length may
                       // not reflect that fact until the handle is closed or the op-lock is broken.
                       // To retrieve current property values, call Get File Properties.
    };

    // A listed file item.
    struct FileItem
    {
      std::string Name;
      FileItemDetails Details;
    };

    // A listed Azure Storage handle item.
    struct HandleItem
    {
      std::string HandleId; // XSMB service handle ID
      std::string Path; // File or directory name including full path starting from share root
      std::string FileId; // FileId uniquely identifies the file or directory.
      std::string ParentId; // ParentId uniquely identifies the parent directory of the object.
      std::string SessionId; // SMB session ID in context of which the file handle was opened
      std::string ClientIp; // Client IP that opened the handle
      DateTime OpenedOn; // Time when the session that previously opened the handle has last been
                         // reconnected. (UTC)
      DateTime LastReconnectedOn; // Time handle was last connected to (UTC)
    };

    // When a file or share is leased, specifies whether the lease is of infinite or fixed duration.
    class LeaseDuration {
    public:
      LeaseDuration() = default;
      explicit LeaseDuration(std::string value) : m_value(std::move(value)) {}
      bool operator==(const LeaseDuration& other) const { return m_value == other.m_value; }
      bool operator!=(const LeaseDuration& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static LeaseDuration Infinite;
      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static LeaseDuration Fixed;

    private:
      std::string m_value;
    }; // extensible enum LeaseDuration

    // Lease state of the file or share.
    class LeaseState {
    public:
      LeaseState() = default;
      explicit LeaseState(std::string value) : m_value(std::move(value)) {}
      bool operator==(const LeaseState& other) const { return m_value == other.m_value; }
      bool operator!=(const LeaseState& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static LeaseState Available;
      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static LeaseState Leased;
      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static LeaseState Expired;
      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static LeaseState Breaking;
      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static LeaseState Broken;

    private:
      std::string m_value;
    }; // extensible enum LeaseState

    // The current lease status of the file or share.
    class LeaseStatus {
    public:
      LeaseStatus() = default;
      explicit LeaseStatus(std::string value) : m_value(std::move(value)) {}
      bool operator==(const LeaseStatus& other) const { return m_value == other.m_value; }
      bool operator!=(const LeaseStatus& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static LeaseStatus Locked;
      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static LeaseStatus Unlocked;

    private:
      std::string m_value;
    }; // extensible enum LeaseStatus

    // Properties of a share.
    struct ShareItemDetails
    {
      DateTime LastModified;
      Azure::ETag Etag;
      int64_t Quota = int64_t();
      Azure::Nullable<int32_t> ProvisionedIops;
      Azure::Nullable<int32_t> ProvisionedIngressMBps;
      Azure::Nullable<int32_t> ProvisionedEgressMBps;
      Azure::Nullable<DateTime> NextAllowedQuotaDowngradeTime;
      Azure::Nullable<DateTime> DeletedOn;
      int32_t RemainingRetentionDays = int32_t();
      Azure::Nullable<Models::AccessTier> AccessTier; // The access tier of the share.
      Azure::Nullable<DateTime> AccessTierChangedOn;
      Azure::Nullable<std::string> AccessTierTransitionState;
      Models::LeaseStatus LeaseStatus;
      Models::LeaseState LeaseState;
      Models::LeaseDuration LeaseDuration;
    };

    // A listed Azure Storage share item.
    struct ShareItem
    {
      std::string Name;
      std::string Snapshot;
      bool Deleted = bool();
      std::string Version;
      ShareItemDetails Details;
      Storage::Metadata ShareMetadata;
    };

    // The retention policy.
    struct RetentionPolicy
    {
      bool Enabled
          = bool(); // Indicates whether a retention policy is enabled for the File service. If
                    // false, metrics data is retained, and the user is responsible for deleting it.
      Azure::Nullable<int32_t>
          Days; // Indicates the number of days that metrics data should be retained. All data older
                // than this value will be deleted. Metrics data is deleted on a best-effort basis
                // after the retention period expires.
    };

    // Storage Analytics metrics for file service.
    struct Metrics
    {
      std::string Version; // The version of Storage Analytics to configure.
      bool Enabled = bool(); // Indicates whether metrics are enabled for the File service.
      Azure::Nullable<bool> IncludeApis; // Indicates whether metrics should generate summary
                                         // statistics for called API operations.
      Models::RetentionPolicy RetentionPolicy;
    };

    // Settings for SMB multichannel
    struct SmbMultichannel
    {
      bool Enabled = bool(); // If SMB multichannel is enabled.
    };

    // Settings for SMB protocol.
    struct SmbSettings
    {
      SmbMultichannel Multichannel; // Settings for SMB Multichannel.
    };

    // Protocol settings
    struct ProtocolSettings
    {
      SmbSettings Settings; // Settings for SMB protocol.
    };

    // Signed identifier.
    struct SignedIdentifier
    {
      std::string Id; // A unique id.
      AccessPolicy Policy; // The access policy.
    };

    // Storage service properties.
    struct ShareServiceProperties
    {
      Metrics HourMetrics; // A summary of request statistics grouped by API in hourly aggregates
                           // for files.
      Metrics MinuteMetrics; // A summary of request statistics grouped by API in minute aggregates
                             // for files.
      std::vector<CorsRule> Cors; // The set of CORS rules.
      Azure::Nullable<ProtocolSettings> Protocol; // Protocol settings
    };

    // State of the copy operation identified by 'x-ms-copy-id'.
    class CopyStatus {
    public:
      CopyStatus() = default;
      explicit CopyStatus(std::string value) : m_value(std::move(value)) {}
      bool operator==(const CopyStatus& other) const { return m_value == other.m_value; }
      bool operator!=(const CopyStatus& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static CopyStatus Pending;
      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static CopyStatus Success;
      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static CopyStatus Aborted;
      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static CopyStatus Failed;

    private:
      std::string m_value;
    }; // extensible enum CopyStatus

    enum class ListSharesIncludeFlags
    {
      None = 0,
      Snapshots = 1,
      Metadata = 2,
      Deleted = 4,
    };

    inline ListSharesIncludeFlags operator|(ListSharesIncludeFlags lhs, ListSharesIncludeFlags rhs)
    {
      using type = std::underlying_type_t<ListSharesIncludeFlags>;
      return static_cast<ListSharesIncludeFlags>(static_cast<type>(lhs) | static_cast<type>(rhs));
    }

    inline ListSharesIncludeFlags& operator|=(
        ListSharesIncludeFlags& lhs,
        ListSharesIncludeFlags rhs)
    {
      lhs = lhs | rhs;
      return lhs;
    }

    inline ListSharesIncludeFlags operator&(ListSharesIncludeFlags lhs, ListSharesIncludeFlags rhs)
    {
      using type = std::underlying_type_t<ListSharesIncludeFlags>;
      return static_cast<ListSharesIncludeFlags>(static_cast<type>(lhs) & static_cast<type>(rhs));
    }

    inline ListSharesIncludeFlags& operator&=(
        ListSharesIncludeFlags& lhs,
        ListSharesIncludeFlags rhs)
    {
      lhs = lhs & rhs;
      return lhs;
    }
    inline std::string ListSharesIncludeFlagsToString(const ListSharesIncludeFlags& val)
    {
      ListSharesIncludeFlags value_list[] = {
          ListSharesIncludeFlags::Snapshots,
          ListSharesIncludeFlags::Metadata,
          ListSharesIncludeFlags::Deleted,
      };
      const char* string_list[] = {
          "snapshots",
          "metadata",
          "deleted",
      };
      std::string result;
      for (std::size_t i = 0; i < sizeof(value_list) / sizeof(ListSharesIncludeFlags); ++i)
      {
        if ((val & value_list[i]) == value_list[i])
        {
          if (!result.empty())
          {
            result += ",";
          }
          result += string_list[i];
        }
      }
      return result;
    }

  } // namespace Models
  namespace _detail {
    using namespace Models;
    constexpr static const char* DefaultServiceApiVersion = "2020-02-10";
    constexpr static const char* QueryCopyId = "copyid";
    constexpr static const char* QueryIncludeFlags = "include";
    constexpr static const char* QueryContinuationToken = "marker";
    constexpr static const char* QueryPageSizeHint = "maxresults";
    constexpr static const char* QueryPrefix = "prefix";
    constexpr static const char* QueryPrevShareSnapshot = "prevsharesnapshot";
    constexpr static const char* QueryShareSnapshot = "sharesnapshot";
    constexpr static const char* QueryTimeout = "timeout";
    constexpr static const char* QueryRestype = "restype";
    constexpr static const char* QueryComp = "comp";
    constexpr static const char* HeaderVersion = "x-ms-version";
    constexpr static const char* HeaderAccessTier = "x-ms-access-tier";
    constexpr static const char* HeaderContentLength = "content-length";
    constexpr static const char* HeaderContentHashMd5 = "content-md5";
    constexpr static const char* HeaderCopyActionAbortConstant = "x-ms-copy-action";
    constexpr static const char* HeaderCopySource = "x-ms-copy-source";
    constexpr static const char* HeaderFilePermissionCopyMode = "x-ms-file-permission-copy-mode";
    constexpr static const char* HeaderIgnoreReadOnly = "x-ms-file-copy-ignore-read-only";
    constexpr static const char* HeaderFileAttributes = "x-ms-file-attributes";
    constexpr static const char* HeaderFileCreatedOn = "x-ms-file-creation-time";
    constexpr static const char* HeaderFileLastWrittenOn = "x-ms-file-last-write-time";
    constexpr static const char* HeaderSetArchiveAttribute = "x-ms-file-copy-set-archive";
    constexpr static const char* HeaderDeletedShareName = "x-ms-deleted-share-name";
    constexpr static const char* HeaderDeletedShareVersion = "x-ms-deleted-share-version";
    constexpr static const char* HeaderDeleteSnapshots = "x-ms-delete-snapshots";
    constexpr static const char* HeaderFileCacheControl = "x-ms-cache-control";
    constexpr static const char* HeaderFileContentDisposition = "x-ms-content-disposition";
    constexpr static const char* HeaderFileContentEncoding = "x-ms-content-encoding";
    constexpr static const char* HeaderFileContentLanguage = "x-ms-content-language";
    constexpr static const char* HeaderFileContentType = "x-ms-content-type";
    constexpr static const char* HeaderFilePermission = "x-ms-file-permission";
    constexpr static const char* HeaderFilePermissionKey = "x-ms-file-permission-key";
    constexpr static const char* HeaderFileRangeWriteFromUrl = "x-ms-write";
    constexpr static const char* HeaderFileRangeWriteFromUrlTypeDefault = "update";
    constexpr static const char* HeaderFileTypeConstant = "x-ms-type";
    constexpr static const char* HeaderRangeGetContentMd5 = "x-ms-range-get-content-md5";
    constexpr static const char* HeaderHandleId = "x-ms-handle-id";
    constexpr static const char* HeaderBreakPeriod = "x-ms-lease-break-period";
    constexpr static const char* HeaderDuration = "x-ms-lease-duration";
    constexpr static const char* HeaderLeaseId = "x-ms-lease-id";
    constexpr static const char* HeaderMetadata = "x-ms-meta";
    constexpr static const char* HeaderProposedLeaseId = "x-ms-proposed-lease-id";
    constexpr static const char* HeaderRange = "x-ms-range";
    constexpr static const char* HeaderRecursive = "x-ms-recursive";
    constexpr static const char* HeaderQuota = "x-ms-share-quota";
    constexpr static const char* HeaderSourceContentHashCrc64 = "x-ms-source-content-crc64";
    constexpr static const char* HeaderSourceIfMatchHashCrc64 = "x-ms-source-if-match-crc64";
    constexpr static const char* HeaderSourceIfNoneMatchHashCrc64
        = "x-ms-source-if-none-match-crc64";
    constexpr static const char* HeaderSourceRange = "x-ms-source-range";
    constexpr static const char* HeaderRequestId = "x-ms-request-id";
    constexpr static const char* HeaderErrorCode = "x-ms-error-code";
    constexpr static const char* HeaderETag = "etag";
    constexpr static const char* HeaderLastModified = "last-modified";
    constexpr static const char* HeaderDate = "date";
    constexpr static const char* HeaderProvisionedIops = "x-ms-share-provisioned-iops";
    constexpr static const char* HeaderProvisionedIngressMBps
        = "x-ms-share-provisioned-ingress-mbps";
    constexpr static const char* HeaderProvisionedEgressMBps = "x-ms-share-provisioned-egress-mbps";
    constexpr static const char* HeaderNextAllowedQuotaDowngradeTime
        = "x-ms-share-next-allowed-quota-downgrade-time";
    constexpr static const char* HeaderLeaseDuration = "x-ms-lease-duration";
    constexpr static const char* HeaderLeaseState = "x-ms-lease-state";
    constexpr static const char* HeaderLeaseStatus = "x-ms-lease-status";
    constexpr static const char* HeaderAccessTierChangedOn = "x-ms-access-tier-change-time";
    constexpr static const char* HeaderAccessTierTransitionState
        = "x-ms-access-tier-transition-state";
    constexpr static const char* HeaderClientRequestId = "x-ms-client-request-id";
    constexpr static const char* HeaderAction = "x-ms-lease-action";
    constexpr static const char* HeaderSnapshot = "x-ms-snapshot";
    constexpr static const char* HeaderRequestIsServerEncrypted = "x-ms-request-server-encrypted";
    constexpr static const char* HeaderAttributes = "x-ms-file-attributes";
    constexpr static const char* HeaderCreatedOn = "x-ms-file-creation-time";
    constexpr static const char* HeaderLastWrittenOn = "x-ms-file-last-write-time";
    constexpr static const char* HeaderChangedOn = "x-ms-file-change-time";
    constexpr static const char* HeaderFileId = "x-ms-file-id";
    constexpr static const char* HeaderParentFileId = "x-ms-file-parent-id";
    constexpr static const char* HeaderIsServerEncrypted = "x-ms-server-encrypted";
    constexpr static const char* HeaderContentType = "content-type";
    constexpr static const char* HeaderContinuationToken = "x-ms-marker";
    constexpr static const char* HeaderNumberOfHandlesClosed = "x-ms-number-of-handles-closed";
    constexpr static const char* HeaderNumberOfHandlesFailedToClose
        = "x-ms-number-of-handles-failed";
    constexpr static const char* HeaderXMsContentLength = "x-ms-content-length";
    constexpr static const char* HeaderContentRange = "content-range";
    constexpr static const char* HeaderTransactionalContentHashMd5 = "content-md5";
    constexpr static const char* HeaderContentEncoding = "content-encoding";
    constexpr static const char* HeaderCacheControl = "cache-control";
    constexpr static const char* HeaderContentDisposition = "content-disposition";
    constexpr static const char* HeaderContentLanguage = "content-language";
    constexpr static const char* HeaderAcceptRanges = "accept-ranges";
    constexpr static const char* HeaderCopyCompletedOn = "x-ms-copy-completion-time";
    constexpr static const char* HeaderCopyStatusDescription = "x-ms-copy-status-description";
    constexpr static const char* HeaderCopyId = "x-ms-copy-id";
    constexpr static const char* HeaderCopyProgress = "x-ms-copy-progress";
    constexpr static const char* HeaderCopyStatus = "x-ms-copy-status";
    constexpr static const char* HeaderXMsRange = "x-ms-range";
    constexpr static const char* HeaderFileRangeWriteType = "x-ms-write";
    constexpr static const char* HeaderFileRangeWriteTypeDefault = "update";
    constexpr static const char* HeaderTransactionalContentHashCrc64 = "x-ms-content-crc64";

    // Only update is supported: - Update: Writes the bytes downloaded from the source url into the
    // specified range.
    class FileRangeWriteFromUrlType {
    public:
      FileRangeWriteFromUrlType() = default;
      explicit FileRangeWriteFromUrlType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const FileRangeWriteFromUrlType& other) const
      {
        return m_value == other.m_value;
      }
      bool operator!=(const FileRangeWriteFromUrlType& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static FileRangeWriteFromUrlType Update;

    private:
      std::string m_value;
    }; // extensible enum FileRangeWriteFromUrlType

    // Abstract for entries that can be listed from Directory.
    struct FilesAndDirectoriesListSinglePage
    {
      std::vector<DirectoryItem> DirectoryItems;
      std::vector<FileItem> FileItems;
    };

    // An enumeration of directories and files.
    struct ListFilesAndDirectoriesSinglePageResponse
    {
      std::string ServiceEndpoint;
      std::string ShareName;
      std::string ShareSnapshot;
      std::string DirectoryPath;
      std::string Prefix;
      int32_t PageSizeHint = int32_t();
      FilesAndDirectoriesListSinglePage SinglePage;
      Azure::Nullable<std::string> ContinuationToken;
    };

    // An enumeration of handles.
    struct ListHandlesResponse
    {
      std::vector<HandleItem> HandleList;
      Azure::Nullable<std::string> ContinuationToken;
    };

    // An enumeration of shares.
    struct ListSharesResponse
    {
      std::string ServiceEndpoint;
      std::string Prefix;
      int32_t PageSizeHint = int32_t();
      std::vector<ShareItem> Items;
      Azure::Nullable<std::string> ContinuationToken;
    };

    // An Azure Storage file range.
    struct FileRange
    {
      int64_t Start = int64_t(); // Start of the range.
      int64_t End = int64_t(); // End of the range.
    };

    // An Azure Storage file clear range.
    struct ClearRange
    {
      int64_t Start = int64_t(); // Start of the range.
      int64_t End = int64_t(); // End of the range.
    };

    // The list of file ranges
    struct RangeList
    {
      std::vector<Core::Http::HttpRange> Ranges;
      std::vector<Core::Http::HttpRange> ClearRanges;
    };

    // Stats for the share.
    struct ShareStats
    {
      int64_t ShareUsageInBytes
          = int64_t(); // The approximate size of the data stored in bytes. Note that this value may
                       // not include all recently created or recently resized files.
    };

    // A permission (a security descriptor) at the share level.
    struct SharePermission
    {
      std::string
          FilePermission; // The permission in the Security Descriptor Definition Language (SDDL).
    };

    // Describes what lease action to take.
    class LeaseAction {
    public:
      LeaseAction() = default;
      explicit LeaseAction(std::string value) : m_value(std::move(value)) {}
      bool operator==(const LeaseAction& other) const { return m_value == other.m_value; }
      bool operator!=(const LeaseAction& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static LeaseAction Acquire;
      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static LeaseAction Release;
      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static LeaseAction Change;
      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static LeaseAction Renew;
      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static LeaseAction Break;

    private:
      std::string m_value;
    }; // extensible enum LeaseAction

    // Specify one of the following options: - Update: Writes the bytes specified by the request
    // body into the specified range. The Range and Content-Length headers must match to perform the
    // update. - Clear: Clears the specified range and releases the space used in storage for that
    // range. To clear a range, set the Content-Length header to zero, and set the Range header to a
    // value that indicates the range to clear, up to maximum file size.
    class FileRangeWriteType {
    public:
      FileRangeWriteType() = default;
      explicit FileRangeWriteType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const FileRangeWriteType& other) const { return m_value == other.m_value; }
      bool operator!=(const FileRangeWriteType& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static FileRangeWriteType Update;
      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static FileRangeWriteType Clear;

    private:
      std::string m_value;
    }; // extensible enum FileRangeWriteType

    struct ServiceSetPropertiesResult
    {
    };

    struct ServiceGetPropertiesResult
    {
      Metrics HourMetrics;
      Metrics MinuteMetrics;
      std::vector<CorsRule> Cors;
      Azure::Nullable<ProtocolSettings> Protocol;
    };

    struct ServiceListSharesSinglePageResult
    {
      std::string ServiceEndpoint;
      std::string Prefix;
      int32_t PageSizeHint = int32_t();
      std::vector<ShareItem> Items;
      Azure::Nullable<std::string> ContinuationToken;
    };

    struct ShareCreateResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
    };

    struct ShareGetPropertiesResult
    {
      Storage::Metadata Metadata;
      Azure::ETag ETag;
      DateTime LastModified;
      int64_t Quota = int64_t();
      Azure::Nullable<int32_t> ProvisionedIops;
      Azure::Nullable<int32_t> ProvisionedIngressMBps;
      Azure::Nullable<int32_t> ProvisionedEgressMBps;
      Azure::Nullable<DateTime> NextAllowedQuotaDowngradeTime;
      Azure::Nullable<Models::LeaseDuration> LeaseDuration;
      Azure::Nullable<Models::LeaseState> LeaseState;
      Azure::Nullable<Models::LeaseStatus> LeaseStatus;
      Azure::Nullable<Models::AccessTier> AccessTier;
      Azure::Nullable<DateTime> AccessTierChangedOn;
      Azure::Nullable<std::string> AccessTierTransitionState;
    };

    struct ShareDeleteResult
    {
    };

    struct ShareAcquireLeaseResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
      std::string LeaseId;
    };

    struct ShareReleaseLeaseResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
    };

    struct ShareChangeLeaseResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
      std::string LeaseId;
    };

    struct ShareRenewLeaseResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
      std::string LeaseId;
    };

    struct ShareBreakLeaseResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
    };

    struct ShareCreateSnapshotResult
    {
      std::string Snapshot;
      Azure::ETag ETag;
      DateTime LastModified;
    };

    struct ShareCreatePermissionResult
    {
      std::string FilePermissionKey;
    };

    struct ShareGetPermissionResult
    {
      std::string FilePermission;
    };

    struct ShareSetPropertiesResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
    };

    struct ShareSetMetadataResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
    };

    struct ShareGetAccessPolicyResult
    {
      std::vector<SignedIdentifier> SignedIdentifiers;
    };

    struct ShareSetAccessPolicyResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
    };

    struct ShareGetStatisticsResult
    {
      int64_t ShareUsageInBytes = int64_t();
      Azure::ETag ETag;
      DateTime LastModified;
    };

    struct ShareRestoreResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
    };

    struct DirectoryCreateResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
      bool IsServerEncrypted = bool();
      FileSmbProperties SmbProperties;
    };

    struct DirectoryGetPropertiesResult
    {
      Storage::Metadata Metadata;
      Azure::ETag ETag;
      DateTime LastModified;
      bool IsServerEncrypted = bool();
      FileSmbProperties SmbProperties;
    };

    struct DirectoryDeleteResult
    {
    };

    struct DirectorySetPropertiesResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
      bool IsServerEncrypted = bool();
      FileSmbProperties SmbProperties;
    };

    struct DirectorySetMetadataResult
    {
      Azure::ETag ETag;
      bool IsServerEncrypted = bool();
    };

    struct DirectoryListFilesAndDirectoriesSinglePageResult
    {
      std::string ServiceEndpoint;
      std::string ShareName;
      std::string ShareSnapshot;
      std::string DirectoryPath;
      std::string Prefix;
      int32_t PageSizeHint = int32_t();
      FilesAndDirectoriesListSinglePage SinglePage;
      Azure::Nullable<std::string> ContinuationToken;
      FileHttpHeaders HttpHeaders;
    };

    struct DirectoryListHandlesResult
    {
      std::vector<HandleItem> HandleList;
      Azure::Nullable<std::string> ContinuationToken;
      FileHttpHeaders HttpHeaders;
    };

    struct DirectoryForceCloseHandlesResult
    {
      Azure::Nullable<std::string> ContinuationToken;
      int32_t NumberOfHandlesClosed = int32_t();
      int32_t NumberOfHandlesFailedToClose = int32_t();
    };

    struct FileCreateResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
      bool IsServerEncrypted = bool();
      FileSmbProperties SmbProperties;
    };

    struct FileDownloadResult
    {
      std::unique_ptr<Azure::Core::IO::BodyStream> BodyStream;
      DateTime LastModified;
      Storage::Metadata Metadata;
      FileHttpHeaders HttpHeaders;
      Azure::Core::Http::HttpRange ContentRange;
      int64_t FileSize = int64_t();
      Azure::ETag ETag;
      Azure::Nullable<Storage::ContentHash> TransactionalContentHash;
      std::string AcceptRanges;
      Azure::Nullable<DateTime> CopyCompletedOn;
      Azure::Nullable<std::string> CopyStatusDescription;
      Azure::Nullable<std::string> CopyId;
      Azure::Nullable<std::string> CopyProgress;
      Azure::Nullable<std::string> CopySource;
      Azure::Nullable<Models::CopyStatus> CopyStatus;
      bool IsServerEncrypted = bool();
      FileSmbProperties SmbProperties;
      Azure::Nullable<Models::LeaseDuration> LeaseDuration;
      Azure::Nullable<Models::LeaseState> LeaseState;
      Azure::Nullable<Models::LeaseStatus> LeaseStatus;
    };

    struct FileGetPropertiesResult
    {
      DateTime LastModified;
      Storage::Metadata Metadata;
      int64_t FileSize = int64_t();
      FileHttpHeaders HttpHeaders;
      Azure::ETag ETag;
      Azure::Nullable<DateTime> CopyCompletedOn;
      Azure::Nullable<std::string> CopyStatusDescription;
      Azure::Nullable<std::string> CopyId;
      Azure::Nullable<std::string> CopyProgress;
      Azure::Nullable<std::string> CopySource;
      Azure::Nullable<Models::CopyStatus> CopyStatus;
      bool IsServerEncrypted = bool();
      FileSmbProperties SmbProperties;
      Azure::Nullable<Models::LeaseDuration> LeaseDuration;
      Azure::Nullable<Models::LeaseState> LeaseState;
      Azure::Nullable<Models::LeaseStatus> LeaseStatus;
    };

    struct FileDeleteResult
    {
    };

    struct FileSetHttpHeadersResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
      bool IsServerEncrypted = bool();
      FileSmbProperties SmbProperties;
    };

    struct FileSetMetadataResult
    {
      Azure::ETag ETag;
      bool IsServerEncrypted = bool();
    };

    struct FileAcquireLeaseResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
      std::string LeaseId;
    };

    struct FileReleaseLeaseResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
    };

    struct FileChangeLeaseResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
      std::string LeaseId;
    };

    struct FileBreakLeaseResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
      Azure::Nullable<std::string> LeaseId;
    };

    struct FileUploadRangeResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
      Storage::ContentHash TransactionalContentHash;
      bool IsServerEncrypted = bool();
    };

    struct FileUploadRangeFromUrlResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
      Storage::ContentHash TransactionalContentHash;
      bool IsServerEncrypted = bool();
    };

    struct FileGetRangeListResult
    {
      std::vector<Core::Http::HttpRange> Ranges;
      std::vector<Core::Http::HttpRange> ClearRanges;
      DateTime LastModified;
      Azure::ETag ETag;
      int64_t FileSize = int64_t();
    };

    struct FileStartCopyResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
      std::string CopyId;
      Models::CopyStatus CopyStatus;
    };

    struct FileAbortCopyResult
    {
    };

    struct FileListHandlesResult
    {
      std::vector<HandleItem> HandleList;
      Azure::Nullable<std::string> ContinuationToken;
      FileHttpHeaders HttpHeaders;
    };

    struct FileForceCloseHandlesResult
    {
      Azure::Nullable<std::string> ContinuationToken;
      int32_t NumberOfHandlesClosed = int32_t();
      int32_t NumberOfHandlesFailedToClose = int32_t();
    };

    class ShareRestClient {
    private:
      static Azure::Core::Http::HttpRange HttpRangeFromXml(_internal::XmlReader& reader)
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
          else if (node.Type == _internal::XmlNodeType::StartTag && strcmp(node.Name, "Start") == 0)
          {
            ++depth;
            is_start = true;
          }
          else if (node.Type == _internal::XmlNodeType::StartTag && strcmp(node.Name, "End") == 0)
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

    public:
      class Service {
      public:
        struct SetPropertiesOptions
        {
          ShareServiceProperties ServiceProperties;
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
        };

        static Azure::Response<ServiceSetPropertiesResult> SetProperties(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetPropertiesOptions& setPropertiesOptions)
        {

          std::string xml_body;
          {
            _internal::XmlWriter writer;
            ShareServicePropertiesToXml(writer, setPropertiesOptions.ServiceProperties);
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::End});
            xml_body = writer.GetDocument();
          }
          auto body = Azure::Core::IO::MemoryBodyStream(
              reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, &body);
          request.SetHeader("Content-Length", std::to_string(body.Length()));
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "service");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "properties");
          if (setPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(setPropertiesOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, setPropertiesOptions.ApiVersionParameter);
          return SetPropertiesParseResult(context, pipeline.Send(request, context));
        }

        struct GetPropertiesOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
        };

        static Azure::Response<ServiceGetPropertiesResult> GetProperties(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetPropertiesOptions& getPropertiesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "service");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "properties");
          if (getPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(getPropertiesOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, getPropertiesOptions.ApiVersionParameter);
          return GetPropertiesParseResult(context, pipeline.Send(request, context));
        }

        struct ListSharesSinglePageOptions
        {
          Azure::Nullable<std::string> Prefix;
          Azure::Nullable<std::string> ContinuationToken;
          Azure::Nullable<int32_t> MaxResults;
          Azure::Nullable<ListSharesIncludeFlags> IncludeFlags;
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
        };

        static Azure::Response<ServiceListSharesSinglePageResult> ListSharesSinglePage(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ListSharesSinglePageOptions& listSharesSinglePageOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "list");
          if (listSharesSinglePageOptions.Prefix.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryPrefix,
                _internal::UrlEncodeQueryParameter(listSharesSinglePageOptions.Prefix.Value()));
          }
          if (listSharesSinglePageOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryContinuationToken,
                _internal::UrlEncodeQueryParameter(
                    listSharesSinglePageOptions.ContinuationToken.Value()));
          }
          if (listSharesSinglePageOptions.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryPageSizeHint,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(listSharesSinglePageOptions.MaxResults.Value())));
          }
          if (listSharesSinglePageOptions.IncludeFlags.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryIncludeFlags,
                _internal::UrlEncodeQueryParameter(ListSharesIncludeFlagsToString(
                    listSharesSinglePageOptions.IncludeFlags.Value())));
          }
          if (listSharesSinglePageOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(listSharesSinglePageOptions.Timeout.Value())));
          }
          request.SetHeader(
              _detail::HeaderVersion, listSharesSinglePageOptions.ApiVersionParameter);
          return ListSharesSinglePageParseResult(context, pipeline.Send(request, context));
        }

      private:
        static Azure::Response<ServiceSetPropertiesResult> SetPropertiesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
          {
            // Success (Accepted)
            ServiceSetPropertiesResult result;
            return Azure::Response<ServiceSetPropertiesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static void RetentionPolicyToXml(
            _internal::XmlWriter& writer,
            const RetentionPolicy& object)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Enabled"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, nullptr, object.Enabled ? "true" : "false"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          if (object.Days.HasValue())
          {
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Days"});
            writer.Write(_internal::XmlNode{
                _internal::XmlNodeType::Text, nullptr, std::to_string(object.Days.Value()).data()});
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          }
        }

        static void MetricsToXml(_internal::XmlWriter& writer, const Metrics& object)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Version"});
          writer.Write(
              _internal::XmlNode{_internal::XmlNodeType::Text, nullptr, object.Version.data()});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Enabled"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, nullptr, object.Enabled ? "true" : "false"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          if (object.IncludeApis.HasValue())
          {
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "IncludeAPIs"});
            writer.Write(_internal::XmlNode{
                _internal::XmlNodeType::Text,
                nullptr,
                object.IncludeApis.Value() ? "true" : "false"});
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          }
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "RetentionPolicy"});
          RetentionPolicyToXml(writer, object.RetentionPolicy);
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

        static void CorsRuleToXml(_internal::XmlWriter& writer, const CorsRule& object)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "CorsRule"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "AllowedOrigins"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, nullptr, object.AllowedOrigins.data()});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "AllowedMethods"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, nullptr, object.AllowedMethods.data()});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "AllowedHeaders"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, nullptr, object.AllowedHeaders.data()});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "ExposedHeaders"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, nullptr, object.ExposedHeaders.data()});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "MaxAgeInSeconds"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text,
              nullptr,
              std::to_string(object.MaxAgeInSeconds).data()});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

        static void SmbMultichannelToXml(
            _internal::XmlWriter& writer,
            const SmbMultichannel& object)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Multichannel"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Enabled"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, nullptr, object.Enabled ? "true" : "false"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

        static void SmbSettingsToXml(_internal::XmlWriter& writer, const SmbSettings& object)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "SMB"});
          SmbMultichannelToXml(writer, object.Multichannel);
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

        static void ProtocolSettingsToXml(
            _internal::XmlWriter& writer,
            const ProtocolSettings& object)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "ProtocolSettings"});
          SmbSettingsToXml(writer, object.Settings);
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

        static void ShareServicePropertiesToXml(
            _internal::XmlWriter& writer,
            const ShareServiceProperties& object)
        {
          writer.Write(
              _internal::XmlNode{_internal::XmlNodeType::StartTag, "StorageServiceProperties"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "HourMetrics"});
          MetricsToXml(writer, object.HourMetrics);
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "MinuteMetrics"});
          MetricsToXml(writer, object.MinuteMetrics);
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          if (object.Cors.size() > 0)
          {
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Cors"});
            for (const auto& item : object.Cors)
            {
              CorsRuleToXml(writer, item);
            }
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          }
          if (object.Protocol.HasValue())
          {
            ProtocolSettingsToXml(writer, object.Protocol.Value());
          }
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }
        static Azure::Response<ServiceGetPropertiesResult> GetPropertiesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success.
            const auto& bodyBuffer = response.GetBody();
            auto reader = _internal::XmlReader(
                reinterpret_cast<const char*>(bodyBuffer.data()), bodyBuffer.size());
            ServiceGetPropertiesResult result = bodyBuffer.empty()
                ? ServiceGetPropertiesResult()
                : ServiceGetPropertiesResultFromShareServiceProperties(
                    ShareServicePropertiesFromXml(reader));
            return Azure::Response<ServiceGetPropertiesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static RetentionPolicy RetentionPolicyFromXml(_internal::XmlReader& reader)
        {
          auto result = RetentionPolicy();
          enum class XmlTagName
          {
            Days,
            Enabled,
            Unknown,
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

              if (std::strcmp(node.Name, "Days") == 0)
              {
                path.emplace_back(XmlTagName::Days);
              }
              else if (std::strcmp(node.Name, "Enabled") == 0)
              {
                path.emplace_back(XmlTagName::Enabled);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::Days)
              {
                result.Days = std::stoi(node.Value);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::Enabled)
              {
                result.Enabled = (std::strcmp(node.Value, "true") == 0);
              }
            }
          }
          return result;
        }

        static Metrics MetricsFromXml(_internal::XmlReader& reader)
        {
          auto result = Metrics();
          enum class XmlTagName
          {
            Enabled,
            IncludeAPIs,
            RetentionPolicy,
            Unknown,
            Version,
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

              if (std::strcmp(node.Name, "Enabled") == 0)
              {
                path.emplace_back(XmlTagName::Enabled);
              }
              else if (std::strcmp(node.Name, "IncludeAPIs") == 0)
              {
                path.emplace_back(XmlTagName::IncludeAPIs);
              }
              else if (std::strcmp(node.Name, "RetentionPolicy") == 0)
              {
                path.emplace_back(XmlTagName::RetentionPolicy);
              }
              else if (std::strcmp(node.Name, "Version") == 0)
              {
                path.emplace_back(XmlTagName::Version);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }

              if (path.size() == 1 && path[0] == XmlTagName::RetentionPolicy)
              {
                result.RetentionPolicy = RetentionPolicyFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::Enabled)
              {
                result.Enabled = (std::strcmp(node.Value, "true") == 0);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::IncludeAPIs)
              {
                result.IncludeApis = (std::strcmp(node.Value, "true") == 0);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::Version)
              {
                result.Version = node.Value;
              }
            }
          }
          return result;
        }

        static CorsRule CorsRuleFromXml(_internal::XmlReader& reader)
        {
          auto result = CorsRule();
          enum class XmlTagName
          {
            AllowedHeaders,
            AllowedMethods,
            AllowedOrigins,
            ExposedHeaders,
            MaxAgeInSeconds,
            Unknown,
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

              if (std::strcmp(node.Name, "AllowedHeaders") == 0)
              {
                path.emplace_back(XmlTagName::AllowedHeaders);
              }
              else if (std::strcmp(node.Name, "AllowedMethods") == 0)
              {
                path.emplace_back(XmlTagName::AllowedMethods);
              }
              else if (std::strcmp(node.Name, "AllowedOrigins") == 0)
              {
                path.emplace_back(XmlTagName::AllowedOrigins);
              }
              else if (std::strcmp(node.Name, "ExposedHeaders") == 0)
              {
                path.emplace_back(XmlTagName::ExposedHeaders);
              }
              else if (std::strcmp(node.Name, "MaxAgeInSeconds") == 0)
              {
                path.emplace_back(XmlTagName::MaxAgeInSeconds);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::AllowedHeaders)
              {
                result.AllowedHeaders = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::AllowedMethods)
              {
                result.AllowedMethods = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::AllowedOrigins)
              {
                result.AllowedOrigins = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::ExposedHeaders)
              {
                result.ExposedHeaders = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::MaxAgeInSeconds)
              {
                result.MaxAgeInSeconds = std::stoi(node.Value);
              }
            }
          }
          return result;
        }

        static SmbMultichannel SmbMultichannelFromXml(_internal::XmlReader& reader)
        {
          auto result = SmbMultichannel();
          enum class XmlTagName
          {
            Enabled,
            Unknown,
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

              if (std::strcmp(node.Name, "Enabled") == 0)
              {
                path.emplace_back(XmlTagName::Enabled);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::Enabled)
              {
                result.Enabled = (std::strcmp(node.Value, "true") == 0);
              }
            }
          }
          return result;
        }

        static SmbSettings SmbSettingsFromXml(_internal::XmlReader& reader)
        {
          auto result = SmbSettings();
          enum class XmlTagName
          {
            Multichannel,
            Unknown,
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

              if (std::strcmp(node.Name, "Multichannel") == 0)
              {
                path.emplace_back(XmlTagName::Multichannel);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }

              if (path.size() == 1 && path[0] == XmlTagName::Multichannel)
              {
                result.Multichannel = SmbMultichannelFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
            }
          }
          return result;
        }

        static ProtocolSettings ProtocolSettingsFromXml(_internal::XmlReader& reader)
        {
          auto result = ProtocolSettings();
          enum class XmlTagName
          {
            SMB,
            Unknown,
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

              if (std::strcmp(node.Name, "SMB") == 0)
              {
                path.emplace_back(XmlTagName::SMB);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }

              if (path.size() == 1 && path[0] == XmlTagName::SMB)
              {
                result.Settings = SmbSettingsFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
            }
          }
          return result;
        }

        static ShareServiceProperties ShareServicePropertiesFromXml(_internal::XmlReader& reader)
        {
          auto result = ShareServiceProperties();
          enum class XmlTagName
          {
            Cors,
            CorsRule,
            HourMetrics,
            MinuteMetrics,
            ProtocolSettings,
            StorageServiceProperties,
            Unknown,
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

              if (std::strcmp(node.Name, "Cors") == 0)
              {
                path.emplace_back(XmlTagName::Cors);
              }
              else if (std::strcmp(node.Name, "CorsRule") == 0)
              {
                path.emplace_back(XmlTagName::CorsRule);
              }
              else if (std::strcmp(node.Name, "HourMetrics") == 0)
              {
                path.emplace_back(XmlTagName::HourMetrics);
              }
              else if (std::strcmp(node.Name, "MinuteMetrics") == 0)
              {
                path.emplace_back(XmlTagName::MinuteMetrics);
              }
              else if (std::strcmp(node.Name, "ProtocolSettings") == 0)
              {
                path.emplace_back(XmlTagName::ProtocolSettings);
              }
              else if (std::strcmp(node.Name, "StorageServiceProperties") == 0)
              {
                path.emplace_back(XmlTagName::StorageServiceProperties);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }

              if (path.size() == 2 && path[0] == XmlTagName::StorageServiceProperties
                  && path[1] == XmlTagName::HourMetrics)
              {
                result.HourMetrics = MetricsFromXml(reader);
                path.pop_back();
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::StorageServiceProperties
                  && path[1] == XmlTagName::MinuteMetrics)
              {
                result.MinuteMetrics = MetricsFromXml(reader);
                path.pop_back();
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::StorageServiceProperties
                  && path[1] == XmlTagName::ProtocolSettings)
              {
                result.Protocol = ProtocolSettingsFromXml(reader);
                path.pop_back();
              }
              else if (
                  path.size() == 3 && path[0] == XmlTagName::StorageServiceProperties
                  && path[1] == XmlTagName::Cors && path[2] == XmlTagName::CorsRule)
              {
                result.Cors.emplace_back(CorsRuleFromXml(reader));
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
            }
          }
          return result;
        }

        static ServiceGetPropertiesResult ServiceGetPropertiesResultFromShareServiceProperties(
            ShareServiceProperties object)
        {
          ServiceGetPropertiesResult result;
          result.HourMetrics = std::move(object.HourMetrics);
          result.MinuteMetrics = std::move(object.MinuteMetrics);
          result.Cors = std::move(object.Cors);
          result.Protocol = std::move(object.Protocol);

          return result;
        }
        static Azure::Response<ServiceListSharesSinglePageResult> ListSharesSinglePageParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success.
            const auto& bodyBuffer = response.GetBody();
            auto reader = _internal::XmlReader(
                reinterpret_cast<const char*>(bodyBuffer.data()), bodyBuffer.size());
            ServiceListSharesSinglePageResult result = bodyBuffer.empty()
                ? ServiceListSharesSinglePageResult()
                : ServiceListSharesSinglePageResultFromListSharesResponse(
                    ListSharesResponseFromXml(reader));
            return Azure::Response<ServiceListSharesSinglePageResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static LeaseStatus LeaseStatusFromXml(_internal::XmlReader& reader)
        {
          LeaseStatus result;
          enum class XmlTagName
          {
            LeaseStatus,
            Unknown,
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

              if (std::strcmp(node.Name, "LeaseStatus") == 0)
              {
                path.emplace_back(XmlTagName::LeaseStatus);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::LeaseStatus)
              {
                result = LeaseStatus(node.Value);
              }
            }
          }
          return result;
        }

        static LeaseState LeaseStateFromXml(_internal::XmlReader& reader)
        {
          LeaseState result;
          enum class XmlTagName
          {
            LeaseState,
            Unknown,
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

              if (std::strcmp(node.Name, "LeaseState") == 0)
              {
                path.emplace_back(XmlTagName::LeaseState);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::LeaseState)
              {
                result = LeaseState(node.Value);
              }
            }
          }
          return result;
        }

        static LeaseDuration LeaseDurationFromXml(_internal::XmlReader& reader)
        {
          LeaseDuration result;
          enum class XmlTagName
          {
            LeaseDuration,
            Unknown,
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

              if (std::strcmp(node.Name, "LeaseDuration") == 0)
              {
                path.emplace_back(XmlTagName::LeaseDuration);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::LeaseDuration)
              {
                result = LeaseDuration(node.Value);
              }
            }
          }
          return result;
        }

        static ShareItemDetails ShareItemDetailsFromXml(_internal::XmlReader& reader)
        {
          auto result = ShareItemDetails();
          enum class XmlTagName
          {
            AccessTier,
            AccessTierChangeTime,
            AccessTierTransitionState,
            DeletedTime,
            Etag,
            LastModified,
            LeaseDuration,
            LeaseState,
            LeaseStatus,
            NextAllowedQuotaDowngradeTime,
            ProvisionedEgressMBps,
            ProvisionedIngressMBps,
            ProvisionedIops,
            Quota,
            RemainingRetentionDays,
            Unknown,
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

              if (std::strcmp(node.Name, "AccessTier") == 0)
              {
                path.emplace_back(XmlTagName::AccessTier);
              }
              else if (std::strcmp(node.Name, "AccessTierChangeTime") == 0)
              {
                path.emplace_back(XmlTagName::AccessTierChangeTime);
              }
              else if (std::strcmp(node.Name, "AccessTierTransitionState") == 0)
              {
                path.emplace_back(XmlTagName::AccessTierTransitionState);
              }
              else if (std::strcmp(node.Name, "DeletedTime") == 0)
              {
                path.emplace_back(XmlTagName::DeletedTime);
              }
              else if (std::strcmp(node.Name, "Etag") == 0)
              {
                path.emplace_back(XmlTagName::Etag);
              }
              else if (std::strcmp(node.Name, "Last-Modified") == 0)
              {
                path.emplace_back(XmlTagName::LastModified);
              }
              else if (std::strcmp(node.Name, "LeaseDuration") == 0)
              {
                path.emplace_back(XmlTagName::LeaseDuration);
              }
              else if (std::strcmp(node.Name, "LeaseState") == 0)
              {
                path.emplace_back(XmlTagName::LeaseState);
              }
              else if (std::strcmp(node.Name, "LeaseStatus") == 0)
              {
                path.emplace_back(XmlTagName::LeaseStatus);
              }
              else if (std::strcmp(node.Name, "NextAllowedQuotaDowngradeTime") == 0)
              {
                path.emplace_back(XmlTagName::NextAllowedQuotaDowngradeTime);
              }
              else if (std::strcmp(node.Name, "ProvisionedEgressMBps") == 0)
              {
                path.emplace_back(XmlTagName::ProvisionedEgressMBps);
              }
              else if (std::strcmp(node.Name, "ProvisionedIngressMBps") == 0)
              {
                path.emplace_back(XmlTagName::ProvisionedIngressMBps);
              }
              else if (std::strcmp(node.Name, "ProvisionedIops") == 0)
              {
                path.emplace_back(XmlTagName::ProvisionedIops);
              }
              else if (std::strcmp(node.Name, "Quota") == 0)
              {
                path.emplace_back(XmlTagName::Quota);
              }
              else if (std::strcmp(node.Name, "RemainingRetentionDays") == 0)
              {
                path.emplace_back(XmlTagName::RemainingRetentionDays);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }

              if (path.size() == 1 && path[0] == XmlTagName::LeaseStatus)
              {
                result.LeaseStatus = LeaseStatusFromXml(reader);
                path.pop_back();
              }
              else if (path.size() == 1 && path[0] == XmlTagName::LeaseState)
              {
                result.LeaseState = LeaseStateFromXml(reader);
                path.pop_back();
              }
              else if (path.size() == 1 && path[0] == XmlTagName::LeaseDuration)
              {
                result.LeaseDuration = LeaseDurationFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::AccessTier)
              {
                result.AccessTier = AccessTier(node.Value);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::AccessTierChangeTime)
              {
                result.AccessTierChangedOn
                    = DateTime::Parse(node.Value, DateTime::DateFormat::Rfc1123);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::AccessTierTransitionState)
              {
                result.AccessTierTransitionState = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::DeletedTime)
              {
                result.DeletedOn = DateTime::Parse(node.Value, DateTime::DateFormat::Rfc1123);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::Etag)
              {
                result.Etag = Azure::ETag(node.Value);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::LastModified)
              {
                result.LastModified = DateTime::Parse(node.Value, DateTime::DateFormat::Rfc1123);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::NextAllowedQuotaDowngradeTime)
              {
                result.NextAllowedQuotaDowngradeTime
                    = DateTime::Parse(node.Value, DateTime::DateFormat::Rfc1123);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::ProvisionedEgressMBps)
              {
                result.ProvisionedEgressMBps = std::stoi(node.Value);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::ProvisionedIngressMBps)
              {
                result.ProvisionedIngressMBps = std::stoi(node.Value);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::ProvisionedIops)
              {
                result.ProvisionedIops = std::stoi(node.Value);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::Quota)
              {
                result.Quota = std::stoll(node.Value);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::RemainingRetentionDays)
              {
                result.RemainingRetentionDays = std::stoi(node.Value);
              }
            }
          }
          return result;
        }

        static Metadata MetadataFromXml(_internal::XmlReader& reader)
        {
          Metadata result;
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
              result.emplace(std::move(key), std::string(node.Value));
            }
          }
          return result;
        }

        static ShareItem ShareItemFromXml(_internal::XmlReader& reader)
        {
          auto result = ShareItem();
          enum class XmlTagName
          {
            Deleted,
            Metadata,
            Name,
            Properties,
            Snapshot,
            Unknown,
            Version,
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

              if (std::strcmp(node.Name, "Deleted") == 0)
              {
                path.emplace_back(XmlTagName::Deleted);
              }
              else if (std::strcmp(node.Name, "Metadata") == 0)
              {
                path.emplace_back(XmlTagName::Metadata);
              }
              else if (std::strcmp(node.Name, "Name") == 0)
              {
                path.emplace_back(XmlTagName::Name);
              }
              else if (std::strcmp(node.Name, "Properties") == 0)
              {
                path.emplace_back(XmlTagName::Properties);
              }
              else if (std::strcmp(node.Name, "Snapshot") == 0)
              {
                path.emplace_back(XmlTagName::Snapshot);
              }
              else if (std::strcmp(node.Name, "Version") == 0)
              {
                path.emplace_back(XmlTagName::Version);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }

              if (path.size() == 1 && path[0] == XmlTagName::Properties)
              {
                result.Details = ShareItemDetailsFromXml(reader);
                path.pop_back();
              }
              else if (path.size() == 1 && path[0] == XmlTagName::Metadata)
              {
                result.ShareMetadata = MetadataFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::Deleted)
              {
                result.Deleted = (std::strcmp(node.Value, "true") == 0);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::Name)
              {
                result.Name = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::Snapshot)
              {
                result.Snapshot = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::Version)
              {
                result.Version = node.Value;
              }
            }
          }
          return result;
        }

        static ListSharesResponse ListSharesResponseFromXml(_internal::XmlReader& reader)
        {
          auto result = ListSharesResponse();
          enum class XmlTagName
          {
            EnumerationResults,
            MaxResults,
            NextMarker,
            Prefix,
            Share,
            Shares,
            Unknown,
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

              if (std::strcmp(node.Name, "EnumerationResults") == 0)
              {
                path.emplace_back(XmlTagName::EnumerationResults);
              }
              else if (std::strcmp(node.Name, "MaxResults") == 0)
              {
                path.emplace_back(XmlTagName::MaxResults);
              }
              else if (std::strcmp(node.Name, "NextMarker") == 0)
              {
                path.emplace_back(XmlTagName::NextMarker);
              }
              else if (std::strcmp(node.Name, "Prefix") == 0)
              {
                path.emplace_back(XmlTagName::Prefix);
              }
              else if (std::strcmp(node.Name, "Share") == 0)
              {
                path.emplace_back(XmlTagName::Share);
              }
              else if (std::strcmp(node.Name, "Shares") == 0)
              {
                path.emplace_back(XmlTagName::Shares);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }
              if (path.size() == 3 && path[0] == XmlTagName::EnumerationResults
                  && path[1] == XmlTagName::Shares && path[2] == XmlTagName::Share)
              {
                result.Items.emplace_back(ShareItemFromXml(reader));
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 2 && path[0] == XmlTagName::EnumerationResults
                  && path[1] == XmlTagName::NextMarker)
              {
                result.ContinuationToken = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::EnumerationResults
                  && path[1] == XmlTagName::MaxResults)
              {
                result.PageSizeHint = std::stoi(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::EnumerationResults
                  && path[1] == XmlTagName::Prefix)
              {
                result.Prefix = node.Value;
              }
            }
            else if (node.Type == _internal::XmlNodeType::Attribute)
            {
              if (path.size() == 1 && path[0] == XmlTagName::EnumerationResults
                  && (std::strcmp(node.Name, "ServiceEndpoint") == 0))
              {
                result.ServiceEndpoint = node.Value;
              }
            }
          }
          return result;
        }

        static ServiceListSharesSinglePageResult
        ServiceListSharesSinglePageResultFromListSharesResponse(ListSharesResponse object)
        {
          ServiceListSharesSinglePageResult result;
          result.ServiceEndpoint = std::move(object.ServiceEndpoint);
          result.Prefix = std::move(object.Prefix);
          result.PageSizeHint = object.PageSizeHint;
          result.Items = std::move(object.Items);
          result.ContinuationToken = std::move(object.ContinuationToken);

          return result;
        }
      };

      class Share {
      public:
        struct CreateOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Storage::Metadata Metadata;
          Azure::Nullable<int64_t> ShareQuota;
          Azure::Nullable<AccessTier> XMsAccessTier;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
        };

        static Azure::Response<ShareCreateResult> Create(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const CreateOptions& createOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "share");
          if (createOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(std::to_string(createOptions.Timeout.Value())));
          }
          for (const auto& pair : createOptions.Metadata)
          {
            request.SetHeader(_detail::HeaderMetadata + ("-" + pair.first), pair.second);
          }
          if (createOptions.ShareQuota.HasValue())
          {
            request.SetHeader(
                _detail::HeaderQuota, std::to_string(createOptions.ShareQuota.Value()));
          }
          if (createOptions.XMsAccessTier.HasValue())
          {
            request.SetHeader(
                _detail::HeaderAccessTier, createOptions.XMsAccessTier.Value().ToString());
          }
          request.SetHeader(_detail::HeaderVersion, createOptions.ApiVersionParameter);
          return CreateParseResult(context, pipeline.Send(request, context));
        }

        struct GetPropertiesOptions
        {
          Azure::Nullable<std::string> ShareSnapshot;
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<std::string> LeaseIdOptional;
        };

        static Azure::Response<ShareGetPropertiesResult> GetProperties(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetPropertiesOptions& getPropertiesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "share");
          if (getPropertiesOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryShareSnapshot,
                _internal::UrlEncodeQueryParameter(getPropertiesOptions.ShareSnapshot.Value()));
          }
          if (getPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(getPropertiesOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, getPropertiesOptions.ApiVersionParameter);
          if (getPropertiesOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(_detail::HeaderLeaseId, getPropertiesOptions.LeaseIdOptional.Value());
          }
          return GetPropertiesParseResult(context, pipeline.Send(request, context));
        }

        struct DeleteOptions
        {
          Azure::Nullable<std::string> ShareSnapshot;
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<DeleteSnapshotsOption> XMsDeleteSnapshots;
          Azure::Nullable<std::string> LeaseIdOptional;
        };

        static Azure::Response<ShareDeleteResult> Delete(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const DeleteOptions& deleteOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Delete, url);
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "share");
          if (deleteOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryShareSnapshot,
                _internal::UrlEncodeQueryParameter(deleteOptions.ShareSnapshot.Value()));
          }
          if (deleteOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(std::to_string(deleteOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, deleteOptions.ApiVersionParameter);
          if (deleteOptions.XMsDeleteSnapshots.HasValue())
          {
            request.SetHeader(
                _detail::HeaderDeleteSnapshots,
                deleteOptions.XMsDeleteSnapshots.Value().ToString());
          }
          if (deleteOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(_detail::HeaderLeaseId, deleteOptions.LeaseIdOptional.Value());
          }
          return DeleteParseResult(context, pipeline.Send(request, context));
        }

        struct AcquireLeaseOptions
        {
          Azure::Nullable<int32_t> Timeout;
          int32_t LeaseDuration = int32_t();
          Azure::Nullable<std::string> ProposedLeaseIdOptional;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<std::string> ShareSnapshot;
        };

        static Azure::Response<ShareAcquireLeaseResult> AcquireLease(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const AcquireLeaseOptions& acquireLeaseOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "lease");
          request.SetHeader(_detail::HeaderAction, "acquire");
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "share");
          if (acquireLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(acquireLeaseOptions.Timeout.Value())));
          }
          request.SetHeader(
              _detail::HeaderDuration, std::to_string(acquireLeaseOptions.LeaseDuration));
          if (acquireLeaseOptions.ProposedLeaseIdOptional.HasValue())
          {
            request.SetHeader(
                _detail::HeaderProposedLeaseId,
                acquireLeaseOptions.ProposedLeaseIdOptional.Value());
          }
          request.SetHeader(_detail::HeaderVersion, acquireLeaseOptions.ApiVersionParameter);
          if (acquireLeaseOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryShareSnapshot,
                _internal::UrlEncodeQueryParameter(acquireLeaseOptions.ShareSnapshot.Value()));
          }
          return AcquireLeaseParseResult(context, pipeline.Send(request, context));
        }

        struct ReleaseLeaseOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string LeaseIdRequired;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<std::string> ShareSnapshot;
        };

        static Azure::Response<ShareReleaseLeaseResult> ReleaseLease(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ReleaseLeaseOptions& releaseLeaseOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "lease");
          request.SetHeader(_detail::HeaderAction, "release");
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "share");
          if (releaseLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(releaseLeaseOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderLeaseId, releaseLeaseOptions.LeaseIdRequired);
          request.SetHeader(_detail::HeaderVersion, releaseLeaseOptions.ApiVersionParameter);
          if (releaseLeaseOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryShareSnapshot,
                _internal::UrlEncodeQueryParameter(releaseLeaseOptions.ShareSnapshot.Value()));
          }
          return ReleaseLeaseParseResult(context, pipeline.Send(request, context));
        }

        struct ChangeLeaseOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string LeaseIdRequired;
          Azure::Nullable<std::string> ProposedLeaseIdOptional;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<std::string> ShareSnapshot;
        };

        static Azure::Response<ShareChangeLeaseResult> ChangeLease(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ChangeLeaseOptions& changeLeaseOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "lease");
          request.SetHeader(_detail::HeaderAction, "change");
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "share");
          if (changeLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(changeLeaseOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderLeaseId, changeLeaseOptions.LeaseIdRequired);
          if (changeLeaseOptions.ProposedLeaseIdOptional.HasValue())
          {
            request.SetHeader(
                _detail::HeaderProposedLeaseId, changeLeaseOptions.ProposedLeaseIdOptional.Value());
          }
          request.SetHeader(_detail::HeaderVersion, changeLeaseOptions.ApiVersionParameter);
          if (changeLeaseOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryShareSnapshot,
                _internal::UrlEncodeQueryParameter(changeLeaseOptions.ShareSnapshot.Value()));
          }
          return ChangeLeaseParseResult(context, pipeline.Send(request, context));
        }

        struct RenewLeaseOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string LeaseIdRequired;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<std::string> ShareSnapshot;
        };

        static Azure::Response<ShareRenewLeaseResult> RenewLease(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const RenewLeaseOptions& renewLeaseOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "lease");
          request.SetHeader(_detail::HeaderAction, "renew");
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "share");
          if (renewLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(renewLeaseOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderLeaseId, renewLeaseOptions.LeaseIdRequired);
          request.SetHeader(_detail::HeaderVersion, renewLeaseOptions.ApiVersionParameter);
          if (renewLeaseOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryShareSnapshot,
                _internal::UrlEncodeQueryParameter(renewLeaseOptions.ShareSnapshot.Value()));
          }
          return RenewLeaseParseResult(context, pipeline.Send(request, context));
        }

        struct BreakLeaseOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<int32_t> LeaseBreakPeriod;
          Azure::Nullable<std::string> LeaseIdOptional;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<std::string> ShareSnapshot;
        };

        static Azure::Response<ShareBreakLeaseResult> BreakLease(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const BreakLeaseOptions& breakLeaseOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "lease");
          request.SetHeader(_detail::HeaderAction, "break");
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "share");
          if (breakLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(breakLeaseOptions.Timeout.Value())));
          }
          if (breakLeaseOptions.LeaseBreakPeriod.HasValue())
          {
            request.SetHeader(
                _detail::HeaderBreakPeriod,
                std::to_string(breakLeaseOptions.LeaseBreakPeriod.Value()));
          }
          if (breakLeaseOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(_detail::HeaderLeaseId, breakLeaseOptions.LeaseIdOptional.Value());
          }
          request.SetHeader(_detail::HeaderVersion, breakLeaseOptions.ApiVersionParameter);
          if (breakLeaseOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryShareSnapshot,
                _internal::UrlEncodeQueryParameter(breakLeaseOptions.ShareSnapshot.Value()));
          }
          return BreakLeaseParseResult(context, pipeline.Send(request, context));
        }

        struct CreateSnapshotOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Storage::Metadata Metadata;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
        };

        static Azure::Response<ShareCreateSnapshotResult> CreateSnapshot(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const CreateSnapshotOptions& createSnapshotOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "snapshot");
          if (createSnapshotOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(createSnapshotOptions.Timeout.Value())));
          }
          for (const auto& pair : createSnapshotOptions.Metadata)
          {
            request.SetHeader(_detail::HeaderMetadata + ("-" + pair.first), pair.second);
          }
          request.SetHeader(_detail::HeaderVersion, createSnapshotOptions.ApiVersionParameter);
          return CreateSnapshotParseResult(context, pipeline.Send(request, context));
        }

        struct CreatePermissionOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          SharePermission Permission;
        };

        static Azure::Response<ShareCreatePermissionResult> CreatePermission(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const CreatePermissionOptions& createPermissionOptions)
        {

          std::string json_body;
          {
            Azure::Core::Json::_internal::json json;
            SharePermissionToJson(json, createPermissionOptions.Permission);
            json_body = json.dump();
          }
          auto body = Azure::Core::IO::MemoryBodyStream(
              reinterpret_cast<const uint8_t*>(json_body.data()), json_body.length());
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, &body);
          request.SetHeader("Content-Length", std::to_string(body.Length()));
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "filepermission");
          if (createPermissionOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(createPermissionOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, createPermissionOptions.ApiVersionParameter);
          return CreatePermissionParseResult(context, pipeline.Send(request, context));
        }

        struct GetPermissionOptions
        {
          std::string FilePermissionKeyRequired;
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
        };

        static Azure::Response<ShareGetPermissionResult> GetPermission(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetPermissionOptions& getPermissionOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "filepermission");
          request.SetHeader(
              _detail::HeaderFilePermissionKey, getPermissionOptions.FilePermissionKeyRequired);
          if (getPermissionOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(getPermissionOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, getPermissionOptions.ApiVersionParameter);
          return GetPermissionParseResult(context, pipeline.Send(request, context));
        }

        struct SetPropertiesOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<int64_t> ShareQuota;
          Azure::Nullable<AccessTier> XMsAccessTier;
          Azure::Nullable<std::string> LeaseIdOptional;
        };

        static Azure::Response<ShareSetPropertiesResult> SetProperties(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetPropertiesOptions& setPropertiesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "properties");
          if (setPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(setPropertiesOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, setPropertiesOptions.ApiVersionParameter);
          if (setPropertiesOptions.ShareQuota.HasValue())
          {
            request.SetHeader(
                _detail::HeaderQuota, std::to_string(setPropertiesOptions.ShareQuota.Value()));
          }
          if (setPropertiesOptions.XMsAccessTier.HasValue())
          {
            request.SetHeader(
                _detail::HeaderAccessTier, setPropertiesOptions.XMsAccessTier.Value().ToString());
          }
          if (setPropertiesOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(_detail::HeaderLeaseId, setPropertiesOptions.LeaseIdOptional.Value());
          }
          return SetPropertiesParseResult(context, pipeline.Send(request, context));
        }

        struct SetMetadataOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Storage::Metadata Metadata;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<std::string> LeaseIdOptional;
        };

        static Azure::Response<ShareSetMetadataResult> SetMetadata(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetMetadataOptions& setMetadataOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "metadata");
          if (setMetadataOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(setMetadataOptions.Timeout.Value())));
          }
          for (const auto& pair : setMetadataOptions.Metadata)
          {
            request.SetHeader(_detail::HeaderMetadata + ("-" + pair.first), pair.second);
          }
          request.SetHeader(_detail::HeaderVersion, setMetadataOptions.ApiVersionParameter);
          if (setMetadataOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(_detail::HeaderLeaseId, setMetadataOptions.LeaseIdOptional.Value());
          }
          return SetMetadataParseResult(context, pipeline.Send(request, context));
        }

        struct GetAccessPolicyOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<std::string> LeaseIdOptional;
        };

        static Azure::Response<ShareGetAccessPolicyResult> GetAccessPolicy(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetAccessPolicyOptions& getAccessPolicyOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "acl");
          if (getAccessPolicyOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(getAccessPolicyOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, getAccessPolicyOptions.ApiVersionParameter);
          if (getAccessPolicyOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(
                _detail::HeaderLeaseId, getAccessPolicyOptions.LeaseIdOptional.Value());
          }
          return GetAccessPolicyParseResult(context, pipeline.Send(request, context));
        }

        struct SetAccessPolicyOptions
        {
          std::vector<SignedIdentifier> ShareAcl;
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<std::string> LeaseIdOptional;
        };

        static Azure::Response<ShareSetAccessPolicyResult> SetAccessPolicy(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetAccessPolicyOptions& setAccessPolicyOptions)
        {

          std::string xml_body;
          {
            _internal::XmlWriter writer;
            SignedIdentifiersToXml(writer, setAccessPolicyOptions.ShareAcl);
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::End});
            xml_body = writer.GetDocument();
          }
          auto body = Azure::Core::IO::MemoryBodyStream(
              reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, &body);
          request.SetHeader("Content-Length", std::to_string(body.Length()));
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "acl");
          if (setAccessPolicyOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(setAccessPolicyOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, setAccessPolicyOptions.ApiVersionParameter);
          if (setAccessPolicyOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(
                _detail::HeaderLeaseId, setAccessPolicyOptions.LeaseIdOptional.Value());
          }
          return SetAccessPolicyParseResult(context, pipeline.Send(request, context));
        }

        struct GetStatisticsOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<std::string> LeaseIdOptional;
        };

        static Azure::Response<ShareGetStatisticsResult> GetStatistics(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetStatisticsOptions& getStatisticsOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "stats");
          if (getStatisticsOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(getStatisticsOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, getStatisticsOptions.ApiVersionParameter);
          if (getStatisticsOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(_detail::HeaderLeaseId, getStatisticsOptions.LeaseIdOptional.Value());
          }
          return GetStatisticsParseResult(context, pipeline.Send(request, context));
        }

        struct RestoreOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<std::string> DeletedShareName;
          Azure::Nullable<std::string> DeletedShareVersion;
        };

        static Azure::Response<ShareRestoreResult> Restore(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const RestoreOptions& restoreOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "undelete");
          if (restoreOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(std::to_string(restoreOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, restoreOptions.ApiVersionParameter);
          if (restoreOptions.DeletedShareName.HasValue())
          {
            request.SetHeader(
                _detail::HeaderDeletedShareName, restoreOptions.DeletedShareName.Value());
          }
          if (restoreOptions.DeletedShareVersion.HasValue())
          {
            request.SetHeader(
                _detail::HeaderDeletedShareVersion, restoreOptions.DeletedShareVersion.Value());
          }
          return RestoreParseResult(context, pipeline.Send(request, context));
        }

      private:
        static Azure::Response<ShareCreateResult> CreateParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // Success, Share created.
            ShareCreateResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            return Azure::Response<ShareCreateResult>(std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<ShareGetPropertiesResult> GetPropertiesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success
            ShareGetPropertiesResult result;

            for (auto i = response.GetHeaders().lower_bound(_detail::HeaderMetadata);
                 i != response.GetHeaders().end()
                 && i->first.substr(0, 9) == _detail::HeaderMetadata;
                 ++i)
            {
              result.Metadata.emplace(i->first.substr(10), i->second);
            }
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            result.Quota = std::stoll(response.GetHeaders().at(_detail::HeaderQuota));
            if (response.GetHeaders().find(_detail::HeaderProvisionedIops)
                != response.GetHeaders().end())
            {
              result.ProvisionedIops
                  = std::stoi(response.GetHeaders().at(_detail::HeaderProvisionedIops));
            }
            if (response.GetHeaders().find(_detail::HeaderProvisionedIngressMBps)
                != response.GetHeaders().end())
            {
              result.ProvisionedIngressMBps
                  = std::stoi(response.GetHeaders().at(_detail::HeaderProvisionedIngressMBps));
            }
            if (response.GetHeaders().find(_detail::HeaderProvisionedEgressMBps)
                != response.GetHeaders().end())
            {
              result.ProvisionedEgressMBps
                  = std::stoi(response.GetHeaders().at(_detail::HeaderProvisionedEgressMBps));
            }
            if (response.GetHeaders().find(_detail::HeaderNextAllowedQuotaDowngradeTime)
                != response.GetHeaders().end())
            {
              result.NextAllowedQuotaDowngradeTime = DateTime::Parse(
                  response.GetHeaders().at(_detail::HeaderNextAllowedQuotaDowngradeTime),
                  DateTime::DateFormat::Rfc1123);
            }
            if (response.GetHeaders().find(_detail::HeaderLeaseDuration)
                != response.GetHeaders().end())
            {
              result.LeaseDuration
                  = LeaseDuration(response.GetHeaders().at(_detail::HeaderLeaseDuration));
            }
            if (response.GetHeaders().find(_detail::HeaderLeaseState)
                != response.GetHeaders().end())
            {
              result.LeaseState = LeaseState(response.GetHeaders().at(_detail::HeaderLeaseState));
            }
            if (response.GetHeaders().find(_detail::HeaderLeaseStatus)
                != response.GetHeaders().end())
            {
              result.LeaseStatus
                  = LeaseStatus(response.GetHeaders().at(_detail::HeaderLeaseStatus));
            }
            if (response.GetHeaders().find("x-ms-access-tier") != response.GetHeaders().end())
            {
              result.AccessTier = AccessTier(response.GetHeaders().at("x-ms-access-tier"));
            }
            if (response.GetHeaders().find(_detail::HeaderAccessTierChangedOn)
                != response.GetHeaders().end())
            {
              result.AccessTierChangedOn = DateTime::Parse(
                  response.GetHeaders().at(_detail::HeaderAccessTierChangedOn),
                  DateTime::DateFormat::Rfc1123);
            }
            if (response.GetHeaders().find(_detail::HeaderAccessTierTransitionState)
                != response.GetHeaders().end())
            {
              result.AccessTierTransitionState
                  = response.GetHeaders().at(_detail::HeaderAccessTierTransitionState);
            }
            return Azure::Response<ShareGetPropertiesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<ShareDeleteResult> DeleteParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
          {
            // Accepted
            ShareDeleteResult result;
            return Azure::Response<ShareDeleteResult>(std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<ShareAcquireLeaseResult> AcquireLeaseParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // The Acquire operation completed successfully.
            ShareAcquireLeaseResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            result.LeaseId = response.GetHeaders().at(_detail::HeaderLeaseId);
            return Azure::Response<ShareAcquireLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<ShareReleaseLeaseResult> ReleaseLeaseParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // The Release operation completed successfully.
            ShareReleaseLeaseResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            return Azure::Response<ShareReleaseLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<ShareChangeLeaseResult> ChangeLeaseParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // The Change operation completed successfully.
            ShareChangeLeaseResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            result.LeaseId = response.GetHeaders().at(_detail::HeaderLeaseId);
            return Azure::Response<ShareChangeLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<ShareRenewLeaseResult> RenewLeaseParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // The Renew operation completed successfully.
            ShareRenewLeaseResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            result.LeaseId = response.GetHeaders().at(_detail::HeaderLeaseId);
            return Azure::Response<ShareRenewLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<ShareBreakLeaseResult> BreakLeaseParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
          {
            // The Break operation completed successfully.
            ShareBreakLeaseResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            return Azure::Response<ShareBreakLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<ShareCreateSnapshotResult> CreateSnapshotParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // Success, Share snapshot created.
            ShareCreateSnapshotResult result;
            result.Snapshot = response.GetHeaders().at(_detail::HeaderSnapshot);
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            return Azure::Response<ShareCreateSnapshotResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<ShareCreatePermissionResult> CreatePermissionParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // Success, Share level permission created.
            ShareCreatePermissionResult result;
            result.FilePermissionKey = response.GetHeaders().at(_detail::HeaderFilePermissionKey);
            return Azure::Response<ShareCreatePermissionResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static void SharePermissionToJson(
            Azure::Core::Json::_internal::json& node,
            const SharePermission& object)
        {
          node["permission"] = object.FilePermission;
        }

        static Azure::Response<ShareGetPermissionResult> GetPermissionParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success
            const auto& bodyBuffer = response.GetBody();
            ShareGetPermissionResult result = bodyBuffer.empty()
                ? ShareGetPermissionResult()
                : ShareGetPermissionResultFromSharePermission(
                    SharePermissionFromJson(Azure::Core::Json::_internal::json::parse(bodyBuffer)));
            return Azure::Response<ShareGetPermissionResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static SharePermission SharePermissionFromJson(
            const Azure::Core::Json::_internal::json& node)
        {
          SharePermission result;
          result.FilePermission = node["permission"].get<std::string>();
          return result;
        }

        static ShareGetPermissionResult ShareGetPermissionResultFromSharePermission(
            SharePermission object)
        {
          ShareGetPermissionResult result;
          result.FilePermission = std::move(object.FilePermission);

          return result;
        }
        static Azure::Response<ShareSetPropertiesResult> SetPropertiesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success
            ShareSetPropertiesResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            return Azure::Response<ShareSetPropertiesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<ShareSetMetadataResult> SetMetadataParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success
            ShareSetMetadataResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            return Azure::Response<ShareSetMetadataResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<ShareGetAccessPolicyResult> GetAccessPolicyParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success
            const auto& bodyBuffer = response.GetBody();
            auto reader = _internal::XmlReader(
                reinterpret_cast<const char*>(bodyBuffer.data()), bodyBuffer.size());
            ShareGetAccessPolicyResult result = bodyBuffer.empty()
                ? ShareGetAccessPolicyResult()
                : ShareGetAccessPolicyResultFromSignedIdentifiers(SignedIdentifiersFromXml(reader));
            return Azure::Response<ShareGetAccessPolicyResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static AccessPolicy AccessPolicyFromXml(_internal::XmlReader& reader)
        {
          auto result = AccessPolicy();
          enum class XmlTagName
          {
            Expiry,
            Permission,
            Start,
            Unknown,
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

              if (std::strcmp(node.Name, "Expiry") == 0)
              {
                path.emplace_back(XmlTagName::Expiry);
              }
              else if (std::strcmp(node.Name, "Permission") == 0)
              {
                path.emplace_back(XmlTagName::Permission);
              }
              else if (std::strcmp(node.Name, "Start") == 0)
              {
                path.emplace_back(XmlTagName::Start);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::Expiry)
              {
                result.ExpiresOn = DateTime::Parse(node.Value, DateTime::DateFormat::Rfc3339);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::Permission)
              {
                result.Permission = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::Start)
              {
                result.StartsOn = DateTime::Parse(node.Value, DateTime::DateFormat::Rfc3339);
              }
            }
          }
          return result;
        }

        static SignedIdentifier SignedIdentifierFromXml(_internal::XmlReader& reader)
        {
          auto result = SignedIdentifier();
          enum class XmlTagName
          {
            AccessPolicy,
            Id,
            Unknown,
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

              if (std::strcmp(node.Name, "AccessPolicy") == 0)
              {
                path.emplace_back(XmlTagName::AccessPolicy);
              }
              else if (std::strcmp(node.Name, "Id") == 0)
              {
                path.emplace_back(XmlTagName::Id);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }

              if (path.size() == 1 && path[0] == XmlTagName::AccessPolicy)
              {
                result.Policy = AccessPolicyFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::Id)
              {
                result.Id = node.Value;
              }
            }
          }
          return result;
        }

        static std::vector<SignedIdentifier> SignedIdentifiersFromXml(_internal::XmlReader& reader)
        {
          auto result = std::vector<SignedIdentifier>();
          enum class XmlTagName
          {
            SignedIdentifier,
            SignedIdentifiers,
            Unknown,
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

              if (std::strcmp(node.Name, "SignedIdentifier") == 0)
              {
                path.emplace_back(XmlTagName::SignedIdentifier);
              }
              else if (std::strcmp(node.Name, "SignedIdentifiers") == 0)
              {
                path.emplace_back(XmlTagName::SignedIdentifiers);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }

              if (path.size() == 2 && path[0] == XmlTagName::SignedIdentifiers
                  && path[1] == XmlTagName::SignedIdentifier)
              {
                result.emplace_back(SignedIdentifierFromXml(reader));
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
            }
          }
          return result;
        }

        static ShareGetAccessPolicyResult ShareGetAccessPolicyResultFromSignedIdentifiers(
            std::vector<SignedIdentifier> object)
        {
          ShareGetAccessPolicyResult result;
          result.SignedIdentifiers = std::move(object);

          return result;
        }
        static Azure::Response<ShareSetAccessPolicyResult> SetAccessPolicyParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success.
            ShareSetAccessPolicyResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            return Azure::Response<ShareSetAccessPolicyResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static void AccessPolicyToXml(_internal::XmlWriter& writer, const AccessPolicy& object)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "AccessPolicy"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Start"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text,
              nullptr,
              object.StartsOn
                  .ToString(
                      Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits)
                  .data()});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Expiry"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text,
              nullptr,
              object.ExpiresOn
                  .ToString(
                      Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits)
                  .data()});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Permission"});
          writer.Write(
              _internal::XmlNode{_internal::XmlNodeType::Text, nullptr, object.Permission.data()});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

        static void SignedIdentifierToXml(
            _internal::XmlWriter& writer,
            const SignedIdentifier& object)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "SignedIdentifier"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Id"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::Text, nullptr, object.Id.data()});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          AccessPolicyToXml(writer, object.Policy);
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

        static void SignedIdentifiersToXml(
            _internal::XmlWriter& writer,
            const std::vector<SignedIdentifier>& object)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "SignedIdentifiers"});
          for (const auto& item : object)
          {
            SignedIdentifierToXml(writer, item);
          }
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }
        static Azure::Response<ShareGetStatisticsResult> GetStatisticsParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success
            const auto& bodyBuffer = response.GetBody();
            auto reader = _internal::XmlReader(
                reinterpret_cast<const char*>(bodyBuffer.data()), bodyBuffer.size());
            ShareGetStatisticsResult result = bodyBuffer.empty()
                ? ShareGetStatisticsResult()
                : ShareGetStatisticsResultFromShareStats(ShareStatsFromXml(reader));
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            return Azure::Response<ShareGetStatisticsResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static ShareStats ShareStatsFromXml(_internal::XmlReader& reader)
        {
          auto result = ShareStats();
          enum class XmlTagName
          {
            ShareStats,
            ShareUsageBytes,
            Unknown,
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

              if (std::strcmp(node.Name, "ShareStats") == 0)
              {
                path.emplace_back(XmlTagName::ShareStats);
              }
              else if (std::strcmp(node.Name, "ShareUsageBytes") == 0)
              {
                path.emplace_back(XmlTagName::ShareUsageBytes);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 2 && path[0] == XmlTagName::ShareStats
                  && path[1] == XmlTagName::ShareUsageBytes)
              {
                result.ShareUsageInBytes = std::stoll(node.Value);
              }
            }
          }
          return result;
        }

        static ShareGetStatisticsResult ShareGetStatisticsResultFromShareStats(ShareStats object)
        {
          ShareGetStatisticsResult result;
          result.ShareUsageInBytes = object.ShareUsageInBytes;

          return result;
        }
        static Azure::Response<ShareRestoreResult> RestoreParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // Created
            ShareRestoreResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            return Azure::Response<ShareRestoreResult>(std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }
      };

      class Directory {
      public:
        struct CreateOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Storage::Metadata Metadata;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<std::string> FilePermission;
          Azure::Nullable<std::string> FilePermissionKey;
          std::string FileAttributes;
          std::string FileCreationTime;
          std::string FileLastWriteTime;
        };

        static Azure::Response<DirectoryCreateResult> Create(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const CreateOptions& createOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "directory");
          if (createOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(std::to_string(createOptions.Timeout.Value())));
          }
          for (const auto& pair : createOptions.Metadata)
          {
            request.SetHeader(_detail::HeaderMetadata + ("-" + pair.first), pair.second);
          }
          request.SetHeader(_detail::HeaderVersion, createOptions.ApiVersionParameter);
          if (createOptions.FilePermission.HasValue())
          {
            request.SetHeader(_detail::HeaderFilePermission, createOptions.FilePermission.Value());
          }
          if (createOptions.FilePermissionKey.HasValue())
          {
            request.SetHeader(
                _detail::HeaderFilePermissionKey, createOptions.FilePermissionKey.Value());
          }
          request.SetHeader(_detail::HeaderFileAttributes, createOptions.FileAttributes);
          request.SetHeader(_detail::HeaderFileCreatedOn, createOptions.FileCreationTime);
          request.SetHeader(_detail::HeaderFileLastWrittenOn, createOptions.FileLastWriteTime);
          return CreateParseResult(context, pipeline.Send(request, context));
        }

        struct GetPropertiesOptions
        {
          Azure::Nullable<std::string> ShareSnapshot;
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
        };

        static Azure::Response<DirectoryGetPropertiesResult> GetProperties(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetPropertiesOptions& getPropertiesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "directory");
          if (getPropertiesOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryShareSnapshot,
                _internal::UrlEncodeQueryParameter(getPropertiesOptions.ShareSnapshot.Value()));
          }
          if (getPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(getPropertiesOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, getPropertiesOptions.ApiVersionParameter);
          return GetPropertiesParseResult(context, pipeline.Send(request, context));
        }

        struct DeleteOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
        };

        static Azure::Response<DirectoryDeleteResult> Delete(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const DeleteOptions& deleteOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Delete, url);
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "directory");
          if (deleteOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(std::to_string(deleteOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, deleteOptions.ApiVersionParameter);
          return DeleteParseResult(context, pipeline.Send(request, context));
        }

        struct SetPropertiesOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<std::string> FilePermission;
          Azure::Nullable<std::string> FilePermissionKey;
          std::string FileAttributes;
          std::string FileCreationTime;
          std::string FileLastWriteTime;
        };

        static Azure::Response<DirectorySetPropertiesResult> SetProperties(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetPropertiesOptions& setPropertiesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "directory");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "properties");
          if (setPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(setPropertiesOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, setPropertiesOptions.ApiVersionParameter);
          if (setPropertiesOptions.FilePermission.HasValue())
          {
            request.SetHeader(
                _detail::HeaderFilePermission, setPropertiesOptions.FilePermission.Value());
          }
          if (setPropertiesOptions.FilePermissionKey.HasValue())
          {
            request.SetHeader(
                _detail::HeaderFilePermissionKey, setPropertiesOptions.FilePermissionKey.Value());
          }
          request.SetHeader(_detail::HeaderFileAttributes, setPropertiesOptions.FileAttributes);
          request.SetHeader(_detail::HeaderFileCreatedOn, setPropertiesOptions.FileCreationTime);
          request.SetHeader(
              _detail::HeaderFileLastWrittenOn, setPropertiesOptions.FileLastWriteTime);
          return SetPropertiesParseResult(context, pipeline.Send(request, context));
        }

        struct SetMetadataOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Storage::Metadata Metadata;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
        };

        static Azure::Response<DirectorySetMetadataResult> SetMetadata(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetMetadataOptions& setMetadataOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "directory");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "metadata");
          if (setMetadataOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(setMetadataOptions.Timeout.Value())));
          }
          for (const auto& pair : setMetadataOptions.Metadata)
          {
            request.SetHeader(_detail::HeaderMetadata + ("-" + pair.first), pair.second);
          }
          request.SetHeader(_detail::HeaderVersion, setMetadataOptions.ApiVersionParameter);
          return SetMetadataParseResult(context, pipeline.Send(request, context));
        }

        struct ListFilesAndDirectoriesSinglePageOptions
        {
          Azure::Nullable<std::string> Prefix;
          Azure::Nullable<std::string> ShareSnapshot;
          Azure::Nullable<std::string> ContinuationToken;
          Azure::Nullable<int32_t> MaxResults;
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
        };

        static Azure::Response<DirectoryListFilesAndDirectoriesSinglePageResult>
        ListFilesAndDirectoriesSinglePage(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ListFilesAndDirectoriesSinglePageOptions&
                listFilesAndDirectoriesSinglePageOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(_detail::QueryRestype, "directory");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "list");
          if (listFilesAndDirectoriesSinglePageOptions.Prefix.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryPrefix,
                _internal::UrlEncodeQueryParameter(
                    listFilesAndDirectoriesSinglePageOptions.Prefix.Value()));
          }
          if (listFilesAndDirectoriesSinglePageOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryShareSnapshot,
                _internal::UrlEncodeQueryParameter(
                    listFilesAndDirectoriesSinglePageOptions.ShareSnapshot.Value()));
          }
          if (listFilesAndDirectoriesSinglePageOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryContinuationToken,
                _internal::UrlEncodeQueryParameter(
                    listFilesAndDirectoriesSinglePageOptions.ContinuationToken.Value()));
          }
          if (listFilesAndDirectoriesSinglePageOptions.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryPageSizeHint,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(listFilesAndDirectoriesSinglePageOptions.MaxResults.Value())));
          }
          if (listFilesAndDirectoriesSinglePageOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(listFilesAndDirectoriesSinglePageOptions.Timeout.Value())));
          }
          request.SetHeader(
              _detail::HeaderVersion, listFilesAndDirectoriesSinglePageOptions.ApiVersionParameter);
          return ListFilesAndDirectoriesSinglePageParseResult(
              context, pipeline.Send(request, context));
        }

        struct ListHandlesOptions
        {
          Azure::Nullable<std::string> ContinuationToken;
          Azure::Nullable<int32_t> MaxResults;
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::string> ShareSnapshot;
          Azure::Nullable<bool> Recursive;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
        };

        static Azure::Response<DirectoryListHandlesResult> ListHandles(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ListHandlesOptions& listHandlesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "listhandles");
          if (listHandlesOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryContinuationToken,
                _internal::UrlEncodeQueryParameter(listHandlesOptions.ContinuationToken.Value()));
          }
          if (listHandlesOptions.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryPageSizeHint,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(listHandlesOptions.MaxResults.Value())));
          }
          if (listHandlesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(listHandlesOptions.Timeout.Value())));
          }
          if (listHandlesOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryShareSnapshot,
                _internal::UrlEncodeQueryParameter(listHandlesOptions.ShareSnapshot.Value()));
          }
          if (listHandlesOptions.Recursive.HasValue())
          {
            request.SetHeader(
                _detail::HeaderRecursive,
                (listHandlesOptions.Recursive.Value() ? "true" : "false"));
          }
          request.SetHeader(_detail::HeaderVersion, listHandlesOptions.ApiVersionParameter);
          return ListHandlesParseResult(context, pipeline.Send(request, context));
        }

        struct ForceCloseHandlesOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::string> ContinuationToken;
          Azure::Nullable<std::string> ShareSnapshot;
          std::string HandleId;
          Azure::Nullable<bool> Recursive;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
        };

        static Azure::Response<DirectoryForceCloseHandlesResult> ForceCloseHandles(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ForceCloseHandlesOptions& forceCloseHandlesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "forceclosehandles");
          if (forceCloseHandlesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(forceCloseHandlesOptions.Timeout.Value())));
          }
          if (forceCloseHandlesOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryContinuationToken,
                _internal::UrlEncodeQueryParameter(
                    forceCloseHandlesOptions.ContinuationToken.Value()));
          }
          if (forceCloseHandlesOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryShareSnapshot,
                _internal::UrlEncodeQueryParameter(forceCloseHandlesOptions.ShareSnapshot.Value()));
          }
          request.SetHeader(_detail::HeaderHandleId, forceCloseHandlesOptions.HandleId);
          if (forceCloseHandlesOptions.Recursive.HasValue())
          {
            request.SetHeader(
                _detail::HeaderRecursive,
                (forceCloseHandlesOptions.Recursive.Value() ? "true" : "false"));
          }
          request.SetHeader(_detail::HeaderVersion, forceCloseHandlesOptions.ApiVersionParameter);
          return ForceCloseHandlesParseResult(context, pipeline.Send(request, context));
        }

      private:
        static Azure::Response<DirectoryCreateResult> CreateParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // Success, Directory created.
            DirectoryCreateResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            result.IsServerEncrypted
                = response.GetHeaders().at(_detail::HeaderRequestIsServerEncrypted) == "true";
            result.SmbProperties.PermissionKey
                = response.GetHeaders().at(_detail::HeaderFilePermissionKey);
            result.SmbProperties.Attributes
                = FileAttributes(response.GetHeaders().at(_detail::HeaderAttributes));
            result.SmbProperties.CreatedOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderCreatedOn), DateTime::DateFormat::Rfc3339);
            result.SmbProperties.LastWrittenOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastWrittenOn),
                DateTime::DateFormat::Rfc3339);
            result.SmbProperties.ChangedOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderChangedOn), DateTime::DateFormat::Rfc3339);
            result.SmbProperties.FileId = response.GetHeaders().at(_detail::HeaderFileId);
            result.SmbProperties.ParentFileId
                = response.GetHeaders().at(_detail::HeaderParentFileId);
            return Azure::Response<DirectoryCreateResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<DirectoryGetPropertiesResult> GetPropertiesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success.
            DirectoryGetPropertiesResult result;

            for (auto i = response.GetHeaders().lower_bound(_detail::HeaderMetadata);
                 i != response.GetHeaders().end()
                 && i->first.substr(0, 9) == _detail::HeaderMetadata;
                 ++i)
            {
              result.Metadata.emplace(i->first.substr(10), i->second);
            }
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            result.IsServerEncrypted
                = response.GetHeaders().at(_detail::HeaderIsServerEncrypted) == "true";
            result.SmbProperties.Attributes
                = FileAttributes(response.GetHeaders().at(_detail::HeaderAttributes));
            result.SmbProperties.CreatedOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderCreatedOn), DateTime::DateFormat::Rfc3339);
            result.SmbProperties.LastWrittenOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastWrittenOn),
                DateTime::DateFormat::Rfc3339);
            result.SmbProperties.ChangedOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderChangedOn), DateTime::DateFormat::Rfc3339);
            result.SmbProperties.PermissionKey
                = response.GetHeaders().at(_detail::HeaderFilePermissionKey);
            result.SmbProperties.FileId = response.GetHeaders().at(_detail::HeaderFileId);
            result.SmbProperties.ParentFileId
                = response.GetHeaders().at(_detail::HeaderParentFileId);
            return Azure::Response<DirectoryGetPropertiesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<DirectoryDeleteResult> DeleteParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
          {
            // Success (Accepted).
            DirectoryDeleteResult result;
            return Azure::Response<DirectoryDeleteResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<DirectorySetPropertiesResult> SetPropertiesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success
            DirectorySetPropertiesResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            result.IsServerEncrypted
                = response.GetHeaders().at(_detail::HeaderRequestIsServerEncrypted) == "true";
            result.SmbProperties.PermissionKey
                = response.GetHeaders().at(_detail::HeaderFilePermissionKey);
            result.SmbProperties.Attributes
                = FileAttributes(response.GetHeaders().at(_detail::HeaderAttributes));
            result.SmbProperties.CreatedOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderCreatedOn), DateTime::DateFormat::Rfc3339);
            result.SmbProperties.LastWrittenOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastWrittenOn),
                DateTime::DateFormat::Rfc3339);
            result.SmbProperties.ChangedOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderChangedOn), DateTime::DateFormat::Rfc3339);
            result.SmbProperties.FileId = response.GetHeaders().at(_detail::HeaderFileId);
            result.SmbProperties.ParentFileId
                = response.GetHeaders().at(_detail::HeaderParentFileId);
            return Azure::Response<DirectorySetPropertiesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<DirectorySetMetadataResult> SetMetadataParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success (OK).
            DirectorySetMetadataResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.IsServerEncrypted
                = response.GetHeaders().at(_detail::HeaderRequestIsServerEncrypted) == "true";
            return Azure::Response<DirectorySetMetadataResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<DirectoryListFilesAndDirectoriesSinglePageResult>
        ListFilesAndDirectoriesSinglePageParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success.
            const auto& bodyBuffer = response.GetBody();
            auto reader = _internal::XmlReader(
                reinterpret_cast<const char*>(bodyBuffer.data()), bodyBuffer.size());
            DirectoryListFilesAndDirectoriesSinglePageResult result = bodyBuffer.empty()
                ? DirectoryListFilesAndDirectoriesSinglePageResult()
                : DirectoryListFilesAndDirectoriesSinglePageResultFromListFilesAndDirectoriesSinglePageResponse(
                    ListFilesAndDirectoriesSinglePageResponseFromXml(reader));
            result.HttpHeaders.ContentType = response.GetHeaders().at(_detail::HeaderContentType);
            return Azure::Response<DirectoryListFilesAndDirectoriesSinglePageResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static DirectoryItem DirectoryItemFromXml(_internal::XmlReader& reader)
        {
          auto result = DirectoryItem();
          enum class XmlTagName
          {
            Name,
            Unknown,
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

              if (std::strcmp(node.Name, "Name") == 0)
              {
                path.emplace_back(XmlTagName::Name);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::Name)
              {
                result.Name = node.Value;
              }
            }
          }
          return result;
        }

        static FileItemDetails FileItemDetailsFromXml(_internal::XmlReader& reader)
        {
          auto result = FileItemDetails();
          enum class XmlTagName
          {
            ContentLength,
            Unknown,
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

              if (std::strcmp(node.Name, "Content-Length") == 0)
              {
                path.emplace_back(XmlTagName::ContentLength);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::ContentLength)
              {
                result.ContentLength = std::stoll(node.Value);
              }
            }
          }
          return result;
        }

        static FileItem FileItemFromXml(_internal::XmlReader& reader)
        {
          auto result = FileItem();
          enum class XmlTagName
          {
            Name,
            Properties,
            Unknown,
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

              if (std::strcmp(node.Name, "Name") == 0)
              {
                path.emplace_back(XmlTagName::Name);
              }
              else if (std::strcmp(node.Name, "Properties") == 0)
              {
                path.emplace_back(XmlTagName::Properties);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }

              if (path.size() == 1 && path[0] == XmlTagName::Properties)
              {
                result.Details = FileItemDetailsFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::Name)
              {
                result.Name = node.Value;
              }
            }
          }
          return result;
        }

        static FilesAndDirectoriesListSinglePage FilesAndDirectoriesListSinglePageFromXml(
            _internal::XmlReader& reader)
        {
          auto result = FilesAndDirectoriesListSinglePage();
          enum class XmlTagName
          {
            Directory,
            File,
            Unknown,
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

              if (std::strcmp(node.Name, "Directory") == 0)
              {
                path.emplace_back(XmlTagName::Directory);
              }
              else if (std::strcmp(node.Name, "File") == 0)
              {
                path.emplace_back(XmlTagName::File);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }
              if (path.size() == 1 && path[0] == XmlTagName::Directory)
              {
                result.DirectoryItems.emplace_back(DirectoryItemFromXml(reader));
                path.pop_back();
              }
              else if (path.size() == 1 && path[0] == XmlTagName::File)
              {
                result.FileItems.emplace_back(FileItemFromXml(reader));
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
            }
          }
          return result;
        }

        static ListFilesAndDirectoriesSinglePageResponse
        ListFilesAndDirectoriesSinglePageResponseFromXml(_internal::XmlReader& reader)
        {
          auto result = ListFilesAndDirectoriesSinglePageResponse();
          enum class XmlTagName
          {
            Entries,
            EnumerationResults,
            MaxResults,
            NextMarker,
            Prefix,
            Unknown,
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

              if (std::strcmp(node.Name, "Entries") == 0)
              {
                path.emplace_back(XmlTagName::Entries);
              }
              else if (std::strcmp(node.Name, "EnumerationResults") == 0)
              {
                path.emplace_back(XmlTagName::EnumerationResults);
              }
              else if (std::strcmp(node.Name, "MaxResults") == 0)
              {
                path.emplace_back(XmlTagName::MaxResults);
              }
              else if (std::strcmp(node.Name, "NextMarker") == 0)
              {
                path.emplace_back(XmlTagName::NextMarker);
              }
              else if (std::strcmp(node.Name, "Prefix") == 0)
              {
                path.emplace_back(XmlTagName::Prefix);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }

              if (path.size() == 2 && path[0] == XmlTagName::EnumerationResults
                  && path[1] == XmlTagName::Entries)
              {
                result.SinglePage = FilesAndDirectoriesListSinglePageFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 2 && path[0] == XmlTagName::EnumerationResults
                  && path[1] == XmlTagName::NextMarker)
              {
                result.ContinuationToken = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::EnumerationResults
                  && path[1] == XmlTagName::MaxResults)
              {
                result.PageSizeHint = std::stoi(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::EnumerationResults
                  && path[1] == XmlTagName::Prefix)
              {
                result.Prefix = node.Value;
              }
            }
            else if (node.Type == _internal::XmlNodeType::Attribute)
            {
              if (path.size() == 1 && path[0] == XmlTagName::EnumerationResults
                  && (std::strcmp(node.Name, "DirectoryPath") == 0))
              {
                result.DirectoryPath = node.Value;
              }
              else if (
                  path.size() == 1 && path[0] == XmlTagName::EnumerationResults
                  && (std::strcmp(node.Name, "ServiceEndpoint") == 0))
              {
                result.ServiceEndpoint = node.Value;
              }
              else if (
                  path.size() == 1 && path[0] == XmlTagName::EnumerationResults
                  && (std::strcmp(node.Name, "ShareName") == 0))
              {
                result.ShareName = node.Value;
              }
              else if (
                  path.size() == 1 && path[0] == XmlTagName::EnumerationResults
                  && (std::strcmp(node.Name, "ShareSnapshot") == 0))
              {
                result.ShareSnapshot = node.Value;
              }
            }
          }
          return result;
        }

        static DirectoryListFilesAndDirectoriesSinglePageResult
        DirectoryListFilesAndDirectoriesSinglePageResultFromListFilesAndDirectoriesSinglePageResponse(
            ListFilesAndDirectoriesSinglePageResponse object)
        {
          DirectoryListFilesAndDirectoriesSinglePageResult result;
          result.ServiceEndpoint = std::move(object.ServiceEndpoint);
          result.ShareName = std::move(object.ShareName);
          result.ShareSnapshot = std::move(object.ShareSnapshot);
          result.DirectoryPath = std::move(object.DirectoryPath);
          result.Prefix = std::move(object.Prefix);
          result.PageSizeHint = object.PageSizeHint;
          result.SinglePage = std::move(object.SinglePage);
          result.ContinuationToken = std::move(object.ContinuationToken);

          return result;
        }
        static Azure::Response<DirectoryListHandlesResult> ListHandlesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success.
            const auto& bodyBuffer = response.GetBody();
            auto reader = _internal::XmlReader(
                reinterpret_cast<const char*>(bodyBuffer.data()), bodyBuffer.size());
            DirectoryListHandlesResult result = bodyBuffer.empty()
                ? DirectoryListHandlesResult()
                : DirectoryListHandlesResultFromListHandlesResponse(
                    ListHandlesResponseFromXml(reader));
            result.HttpHeaders.ContentType = response.GetHeaders().at(_detail::HeaderContentType);
            return Azure::Response<DirectoryListHandlesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static HandleItem HandleItemFromXml(_internal::XmlReader& reader)
        {
          auto result = HandleItem();
          enum class XmlTagName
          {
            ClientIp,
            FileId,
            HandleId,
            LastReconnectTime,
            OpenTime,
            ParentId,
            Path,
            SessionId,
            Unknown,
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

              if (std::strcmp(node.Name, "ClientIp") == 0)
              {
                path.emplace_back(XmlTagName::ClientIp);
              }
              else if (std::strcmp(node.Name, "FileId") == 0)
              {
                path.emplace_back(XmlTagName::FileId);
              }
              else if (std::strcmp(node.Name, "HandleId") == 0)
              {
                path.emplace_back(XmlTagName::HandleId);
              }
              else if (std::strcmp(node.Name, "LastReconnectTime") == 0)
              {
                path.emplace_back(XmlTagName::LastReconnectTime);
              }
              else if (std::strcmp(node.Name, "OpenTime") == 0)
              {
                path.emplace_back(XmlTagName::OpenTime);
              }
              else if (std::strcmp(node.Name, "ParentId") == 0)
              {
                path.emplace_back(XmlTagName::ParentId);
              }
              else if (std::strcmp(node.Name, "Path") == 0)
              {
                path.emplace_back(XmlTagName::Path);
              }
              else if (std::strcmp(node.Name, "SessionId") == 0)
              {
                path.emplace_back(XmlTagName::SessionId);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::ClientIp)
              {
                result.ClientIp = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::FileId)
              {
                result.FileId = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::HandleId)
              {
                result.HandleId = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::LastReconnectTime)
              {
                result.LastReconnectedOn
                    = DateTime::Parse(node.Value, DateTime::DateFormat::Rfc1123);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::OpenTime)
              {
                result.OpenedOn = DateTime::Parse(node.Value, DateTime::DateFormat::Rfc1123);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::ParentId)
              {
                result.ParentId = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::Path)
              {
                result.Path = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::SessionId)
              {
                result.SessionId = node.Value;
              }
            }
          }
          return result;
        }

        static ListHandlesResponse ListHandlesResponseFromXml(_internal::XmlReader& reader)
        {
          auto result = ListHandlesResponse();
          enum class XmlTagName
          {
            Entries,
            EnumerationResults,
            Handle,
            NextMarker,
            Unknown,
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

              if (std::strcmp(node.Name, "Entries") == 0)
              {
                path.emplace_back(XmlTagName::Entries);
              }
              else if (std::strcmp(node.Name, "EnumerationResults") == 0)
              {
                path.emplace_back(XmlTagName::EnumerationResults);
              }
              else if (std::strcmp(node.Name, "Handle") == 0)
              {
                path.emplace_back(XmlTagName::Handle);
              }
              else if (std::strcmp(node.Name, "NextMarker") == 0)
              {
                path.emplace_back(XmlTagName::NextMarker);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }
              if (path.size() == 3 && path[0] == XmlTagName::EnumerationResults
                  && path[1] == XmlTagName::Entries && path[2] == XmlTagName::Handle)
              {
                result.HandleList.emplace_back(HandleItemFromXml(reader));
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 2 && path[0] == XmlTagName::EnumerationResults
                  && path[1] == XmlTagName::NextMarker)
              {
                result.ContinuationToken = node.Value;
              }
            }
          }
          return result;
        }

        static DirectoryListHandlesResult DirectoryListHandlesResultFromListHandlesResponse(
            ListHandlesResponse object)
        {
          DirectoryListHandlesResult result;
          result.HandleList = std::move(object.HandleList);
          result.ContinuationToken = std::move(object.ContinuationToken);

          return result;
        }
        static Azure::Response<DirectoryForceCloseHandlesResult> ForceCloseHandlesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success.
            DirectoryForceCloseHandlesResult result;
            if (response.GetHeaders().find(_detail::HeaderContinuationToken)
                != response.GetHeaders().end())
            {
              result.ContinuationToken = response.GetHeaders().at(_detail::HeaderContinuationToken);
            }
            result.NumberOfHandlesClosed
                = std::stoi(response.GetHeaders().at(_detail::HeaderNumberOfHandlesClosed));
            result.NumberOfHandlesFailedToClose
                = std::stoi(response.GetHeaders().at(_detail::HeaderNumberOfHandlesFailedToClose));
            return Azure::Response<DirectoryForceCloseHandlesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }
      };

      class File {
      public:
        struct CreateOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          int64_t XMsContentLength = int64_t();
          Azure::Nullable<std::string> FileContentType;
          Azure::Nullable<std::string> FileContentEncoding;
          Azure::Nullable<std::string> FileContentLanguage;
          Azure::Nullable<std::string> FileCacheControl;
          Azure::Nullable<Storage::ContentHash> ContentMd5;
          Azure::Nullable<std::string> FileContentDisposition;
          Storage::Metadata Metadata;
          Azure::Nullable<std::string> FilePermission;
          Azure::Nullable<std::string> FilePermissionKey;
          std::string FileAttributes;
          std::string FileCreationTime;
          std::string FileLastWriteTime;
          Azure::Nullable<std::string> LeaseIdOptional;
        };

        static Azure::Response<FileCreateResult> Create(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const CreateOptions& createOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          if (createOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(std::to_string(createOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, createOptions.ApiVersionParameter);
          request.SetHeader(
              _detail::HeaderXMsContentLength, std::to_string(createOptions.XMsContentLength));
          request.SetHeader(_detail::HeaderFileTypeConstant, "file");
          if (createOptions.FileContentType.HasValue())
          {
            request.SetHeader(
                _detail::HeaderFileContentType, createOptions.FileContentType.Value());
          }
          if (createOptions.FileContentEncoding.HasValue())
          {
            request.SetHeader(
                _detail::HeaderFileContentEncoding, createOptions.FileContentEncoding.Value());
          }
          if (createOptions.FileContentLanguage.HasValue())
          {
            request.SetHeader(
                _detail::HeaderFileContentLanguage, createOptions.FileContentLanguage.Value());
          }
          if (createOptions.FileCacheControl.HasValue())
          {
            request.SetHeader(
                _detail::HeaderFileCacheControl, createOptions.FileCacheControl.Value());
          }
          if (createOptions.ContentMd5.HasValue())
          {
            request.SetHeader(
                _detail::HeaderContentHashMd5,
                _internal::ToBase64String(createOptions.ContentMd5.Value()));
          }
          if (createOptions.FileContentDisposition.HasValue())
          {
            request.SetHeader(
                _detail::HeaderFileContentDisposition,
                createOptions.FileContentDisposition.Value());
          }
          for (const auto& pair : createOptions.Metadata)
          {
            request.SetHeader(_detail::HeaderMetadata + ("-" + pair.first), pair.second);
          }
          if (createOptions.FilePermission.HasValue())
          {
            request.SetHeader(_detail::HeaderFilePermission, createOptions.FilePermission.Value());
          }
          if (createOptions.FilePermissionKey.HasValue())
          {
            request.SetHeader(
                _detail::HeaderFilePermissionKey, createOptions.FilePermissionKey.Value());
          }
          request.SetHeader(_detail::HeaderFileAttributes, createOptions.FileAttributes);
          request.SetHeader(_detail::HeaderFileCreatedOn, createOptions.FileCreationTime);
          request.SetHeader(_detail::HeaderFileLastWrittenOn, createOptions.FileLastWriteTime);
          if (createOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(_detail::HeaderLeaseId, createOptions.LeaseIdOptional.Value());
          }
          return CreateParseResult(context, pipeline.Send(request, context));
        }

        struct DownloadOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<std::string> Range;
          Azure::Nullable<bool> GetRangeContentMd5;
          Azure::Nullable<std::string> LeaseIdOptional;
        };

        static Azure::Response<FileDownloadResult> Download(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const DownloadOptions& downloadOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url, false);
          if (downloadOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(downloadOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, downloadOptions.ApiVersionParameter);
          if (downloadOptions.Range.HasValue())
          {
            request.SetHeader(_detail::HeaderRange, downloadOptions.Range.Value());
          }
          if (downloadOptions.GetRangeContentMd5.HasValue())
          {
            request.SetHeader(
                _detail::HeaderRangeGetContentMd5,
                (downloadOptions.GetRangeContentMd5.Value() ? "true" : "false"));
          }
          if (downloadOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(_detail::HeaderLeaseId, downloadOptions.LeaseIdOptional.Value());
          }
          return DownloadParseResult(context, pipeline.Send(request, context));
        }

        struct GetPropertiesOptions
        {
          Azure::Nullable<std::string> ShareSnapshot;
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<std::string> LeaseIdOptional;
        };

        static Azure::Response<FileGetPropertiesResult> GetProperties(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetPropertiesOptions& getPropertiesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Head, url);
          if (getPropertiesOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryShareSnapshot,
                _internal::UrlEncodeQueryParameter(getPropertiesOptions.ShareSnapshot.Value()));
          }
          if (getPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(getPropertiesOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, getPropertiesOptions.ApiVersionParameter);
          if (getPropertiesOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(_detail::HeaderLeaseId, getPropertiesOptions.LeaseIdOptional.Value());
          }
          return GetPropertiesParseResult(context, pipeline.Send(request, context));
        }

        struct DeleteOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<std::string> LeaseIdOptional;
        };

        static Azure::Response<FileDeleteResult> Delete(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const DeleteOptions& deleteOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Delete, url);
          if (deleteOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(std::to_string(deleteOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, deleteOptions.ApiVersionParameter);
          if (deleteOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(_detail::HeaderLeaseId, deleteOptions.LeaseIdOptional.Value());
          }
          return DeleteParseResult(context, pipeline.Send(request, context));
        }

        struct SetHttpHeadersOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<int64_t> XMsContentLength;
          Azure::Nullable<std::string> FileContentType;
          Azure::Nullable<std::string> FileContentEncoding;
          Azure::Nullable<std::string> FileContentLanguage;
          Azure::Nullable<std::string> FileCacheControl;
          Azure::Nullable<Storage::ContentHash> ContentMd5;
          Azure::Nullable<std::string> FileContentDisposition;
          Azure::Nullable<std::string> FilePermission;
          Azure::Nullable<std::string> FilePermissionKey;
          std::string FileAttributes;
          std::string FileCreationTime;
          std::string FileLastWriteTime;
          Azure::Nullable<std::string> LeaseIdOptional;
        };

        static Azure::Response<FileSetHttpHeadersResult> SetHttpHeaders(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetHttpHeadersOptions& setHttpHeadersOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "properties");
          if (setHttpHeadersOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(setHttpHeadersOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, setHttpHeadersOptions.ApiVersionParameter);
          if (setHttpHeadersOptions.XMsContentLength.HasValue())
          {
            request.SetHeader(
                _detail::HeaderXMsContentLength,
                std::to_string(setHttpHeadersOptions.XMsContentLength.Value()));
          }
          if (setHttpHeadersOptions.FileContentType.HasValue())
          {
            request.SetHeader(
                _detail::HeaderFileContentType, setHttpHeadersOptions.FileContentType.Value());
          }
          if (setHttpHeadersOptions.FileContentEncoding.HasValue())
          {
            request.SetHeader(
                _detail::HeaderFileContentEncoding,
                setHttpHeadersOptions.FileContentEncoding.Value());
          }
          if (setHttpHeadersOptions.FileContentLanguage.HasValue())
          {
            request.SetHeader(
                _detail::HeaderFileContentLanguage,
                setHttpHeadersOptions.FileContentLanguage.Value());
          }
          if (setHttpHeadersOptions.FileCacheControl.HasValue())
          {
            request.SetHeader(
                _detail::HeaderFileCacheControl, setHttpHeadersOptions.FileCacheControl.Value());
          }
          if (setHttpHeadersOptions.ContentMd5.HasValue())
          {
            request.SetHeader(
                _detail::HeaderContentHashMd5,
                _internal::ToBase64String(setHttpHeadersOptions.ContentMd5.Value()));
          }
          if (setHttpHeadersOptions.FileContentDisposition.HasValue())
          {
            request.SetHeader(
                _detail::HeaderFileContentDisposition,
                setHttpHeadersOptions.FileContentDisposition.Value());
          }
          if (setHttpHeadersOptions.FilePermission.HasValue())
          {
            request.SetHeader(
                _detail::HeaderFilePermission, setHttpHeadersOptions.FilePermission.Value());
          }
          if (setHttpHeadersOptions.FilePermissionKey.HasValue())
          {
            request.SetHeader(
                _detail::HeaderFilePermissionKey, setHttpHeadersOptions.FilePermissionKey.Value());
          }
          request.SetHeader(_detail::HeaderFileAttributes, setHttpHeadersOptions.FileAttributes);
          request.SetHeader(_detail::HeaderFileCreatedOn, setHttpHeadersOptions.FileCreationTime);
          request.SetHeader(
              _detail::HeaderFileLastWrittenOn, setHttpHeadersOptions.FileLastWriteTime);
          if (setHttpHeadersOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(
                _detail::HeaderLeaseId, setHttpHeadersOptions.LeaseIdOptional.Value());
          }
          return SetHttpHeadersParseResult(context, pipeline.Send(request, context));
        }

        struct SetMetadataOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Storage::Metadata Metadata;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<std::string> LeaseIdOptional;
        };

        static Azure::Response<FileSetMetadataResult> SetMetadata(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetMetadataOptions& setMetadataOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "metadata");
          if (setMetadataOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(setMetadataOptions.Timeout.Value())));
          }
          for (const auto& pair : setMetadataOptions.Metadata)
          {
            request.SetHeader(_detail::HeaderMetadata + ("-" + pair.first), pair.second);
          }
          request.SetHeader(_detail::HeaderVersion, setMetadataOptions.ApiVersionParameter);
          if (setMetadataOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(_detail::HeaderLeaseId, setMetadataOptions.LeaseIdOptional.Value());
          }
          return SetMetadataParseResult(context, pipeline.Send(request, context));
        }

        struct AcquireLeaseOptions
        {
          Azure::Nullable<int32_t> Timeout;
          int32_t LeaseDuration = int32_t();
          Azure::Nullable<std::string> ProposedLeaseIdOptional;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
        };

        static Azure::Response<FileAcquireLeaseResult> AcquireLease(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const AcquireLeaseOptions& acquireLeaseOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "lease");
          request.SetHeader(_detail::HeaderAction, "acquire");
          if (acquireLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(acquireLeaseOptions.Timeout.Value())));
          }
          request.SetHeader(
              _detail::HeaderDuration, std::to_string(acquireLeaseOptions.LeaseDuration));
          if (acquireLeaseOptions.ProposedLeaseIdOptional.HasValue())
          {
            request.SetHeader(
                _detail::HeaderProposedLeaseId,
                acquireLeaseOptions.ProposedLeaseIdOptional.Value());
          }
          request.SetHeader(_detail::HeaderVersion, acquireLeaseOptions.ApiVersionParameter);
          return AcquireLeaseParseResult(context, pipeline.Send(request, context));
        }

        struct ReleaseLeaseOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string LeaseIdRequired;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
        };

        static Azure::Response<FileReleaseLeaseResult> ReleaseLease(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ReleaseLeaseOptions& releaseLeaseOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "lease");
          request.SetHeader(_detail::HeaderAction, "release");
          if (releaseLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(releaseLeaseOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderLeaseId, releaseLeaseOptions.LeaseIdRequired);
          request.SetHeader(_detail::HeaderVersion, releaseLeaseOptions.ApiVersionParameter);
          return ReleaseLeaseParseResult(context, pipeline.Send(request, context));
        }

        struct ChangeLeaseOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string LeaseIdRequired;
          Azure::Nullable<std::string> ProposedLeaseIdOptional;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
        };

        static Azure::Response<FileChangeLeaseResult> ChangeLease(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ChangeLeaseOptions& changeLeaseOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "lease");
          request.SetHeader(_detail::HeaderAction, "change");
          if (changeLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(changeLeaseOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderLeaseId, changeLeaseOptions.LeaseIdRequired);
          if (changeLeaseOptions.ProposedLeaseIdOptional.HasValue())
          {
            request.SetHeader(
                _detail::HeaderProposedLeaseId, changeLeaseOptions.ProposedLeaseIdOptional.Value());
          }
          request.SetHeader(_detail::HeaderVersion, changeLeaseOptions.ApiVersionParameter);
          return ChangeLeaseParseResult(context, pipeline.Send(request, context));
        }

        struct BreakLeaseOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::string> LeaseIdOptional;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
        };

        static Azure::Response<FileBreakLeaseResult> BreakLease(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const BreakLeaseOptions& breakLeaseOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "lease");
          request.SetHeader(_detail::HeaderAction, "break");
          if (breakLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(breakLeaseOptions.Timeout.Value())));
          }
          if (breakLeaseOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(_detail::HeaderLeaseId, breakLeaseOptions.LeaseIdOptional.Value());
          }
          request.SetHeader(_detail::HeaderVersion, breakLeaseOptions.ApiVersionParameter);
          return BreakLeaseParseResult(context, pipeline.Send(request, context));
        }

        struct UploadRangeOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string XMsRange;
          FileRangeWriteType XMsWrite;
          int64_t ContentLength = int64_t();
          Azure::Nullable<Storage::ContentHash> ContentMd5;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<std::string> LeaseIdOptional;
        };

        static Azure::Response<FileUploadRangeResult> UploadRange(
            const Azure::Core::Url& url,
            Azure::Core::IO::BodyStream& bodyStream,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const UploadRangeOptions& uploadRangeOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url, &bodyStream);
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "range");
          if (uploadRangeOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(uploadRangeOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderXMsRange, uploadRangeOptions.XMsRange);
          request.SetHeader(
              _detail::HeaderFileRangeWriteType, uploadRangeOptions.XMsWrite.ToString());
          request.SetHeader(
              _detail::HeaderContentLength, std::to_string(uploadRangeOptions.ContentLength));
          if (uploadRangeOptions.ContentMd5.HasValue())
          {
            request.SetHeader(
                _detail::HeaderContentHashMd5,
                _internal::ToBase64String(uploadRangeOptions.ContentMd5.Value()));
          }
          request.SetHeader(_detail::HeaderVersion, uploadRangeOptions.ApiVersionParameter);
          if (uploadRangeOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(_detail::HeaderLeaseId, uploadRangeOptions.LeaseIdOptional.Value());
          }
          return UploadRangeParseResult(context, pipeline.Send(request, context));
        }

        struct UploadRangeFromUrlOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string TargetRange;
          std::string CopySource;
          Azure::Nullable<std::string> SourceRange;
          FileRangeWriteFromUrlType XMsWrite;
          int64_t ContentLength = int64_t();
          Azure::Nullable<Storage::ContentHash> SourceContentCrc64;
          Azure::Nullable<Storage::ContentHash> SourceIfMatchCrc64;
          Azure::Nullable<Storage::ContentHash> SourceIfNoneMatchCrc64;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<std::string> LeaseIdOptional;
        };

        static Azure::Response<FileUploadRangeFromUrlResult> UploadRangeFromUrl(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const UploadRangeFromUrlOptions& uploadRangeFromUrlOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "range");
          if (uploadRangeFromUrlOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(uploadRangeFromUrlOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderRange, uploadRangeFromUrlOptions.TargetRange);
          request.SetHeader(_detail::HeaderCopySource, uploadRangeFromUrlOptions.CopySource);
          if (uploadRangeFromUrlOptions.SourceRange.HasValue())
          {
            request.SetHeader(
                _detail::HeaderSourceRange, uploadRangeFromUrlOptions.SourceRange.Value());
          }
          request.SetHeader(
              _detail::HeaderFileRangeWriteFromUrl, uploadRangeFromUrlOptions.XMsWrite.ToString());
          request.SetHeader(
              _detail::HeaderContentLength,
              std::to_string(uploadRangeFromUrlOptions.ContentLength));
          if (uploadRangeFromUrlOptions.SourceContentCrc64.HasValue())
          {
            request.SetHeader(
                _detail::HeaderSourceContentHashCrc64,
                _internal::ToBase64String(uploadRangeFromUrlOptions.SourceContentCrc64.Value()));
          }
          if (uploadRangeFromUrlOptions.SourceIfMatchCrc64.HasValue())
          {
            request.SetHeader(
                _detail::HeaderSourceIfMatchHashCrc64,
                _internal::ToBase64String(uploadRangeFromUrlOptions.SourceIfMatchCrc64.Value()));
          }
          if (uploadRangeFromUrlOptions.SourceIfNoneMatchCrc64.HasValue())
          {
            request.SetHeader(
                _detail::HeaderSourceIfNoneMatchHashCrc64,
                _internal::ToBase64String(
                    uploadRangeFromUrlOptions.SourceIfNoneMatchCrc64.Value()));
          }
          request.SetHeader(_detail::HeaderVersion, uploadRangeFromUrlOptions.ApiVersionParameter);
          if (uploadRangeFromUrlOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(
                _detail::HeaderLeaseId, uploadRangeFromUrlOptions.LeaseIdOptional.Value());
          }
          return UploadRangeFromUrlParseResult(context, pipeline.Send(request, context));
        }

        struct GetRangeListOptions
        {
          Azure::Nullable<std::string> ShareSnapshot;
          Azure::Nullable<std::string> PrevShareSnapshot;
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<std::string> XMsRange;
          Azure::Nullable<std::string> LeaseIdOptional;
        };

        static Azure::Response<FileGetRangeListResult> GetRangeList(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetRangeListOptions& getRangeListOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "rangelist");
          if (getRangeListOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryShareSnapshot,
                _internal::UrlEncodeQueryParameter(getRangeListOptions.ShareSnapshot.Value()));
          }
          if (getRangeListOptions.PrevShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryPrevShareSnapshot,
                _internal::UrlEncodeQueryParameter(getRangeListOptions.PrevShareSnapshot.Value()));
          }
          if (getRangeListOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(getRangeListOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, getRangeListOptions.ApiVersionParameter);
          if (getRangeListOptions.XMsRange.HasValue())
          {
            request.SetHeader(_detail::HeaderXMsRange, getRangeListOptions.XMsRange.Value());
          }
          if (getRangeListOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(_detail::HeaderLeaseId, getRangeListOptions.LeaseIdOptional.Value());
          }
          return GetRangeListParseResult(context, pipeline.Send(request, context));
        }

        struct StartCopyOptions
        {
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Storage::Metadata Metadata;
          std::string CopySource;
          Azure::Nullable<std::string> FilePermission;
          Azure::Nullable<std::string> FilePermissionKey;
          Azure::Nullable<PermissionCopyMode> XMsFilePermissionCopyMode;
          Azure::Nullable<bool> FileCopyIgnoreReadOnly;
          Azure::Nullable<std::string> FileCopyFileAttributes;
          Azure::Nullable<std::string> FileCopyFileCreationTime;
          Azure::Nullable<std::string> FileCopyFileLastWriteTime;
          Azure::Nullable<bool> FileCopySetArchiveAttribute;
          Azure::Nullable<std::string> LeaseIdOptional;
        };

        static Azure::Response<FileStartCopyResult> StartCopy(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const StartCopyOptions& startCopyOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          if (startCopyOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(startCopyOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, startCopyOptions.ApiVersionParameter);
          for (const auto& pair : startCopyOptions.Metadata)
          {
            request.SetHeader(_detail::HeaderMetadata + ("-" + pair.first), pair.second);
          }
          request.SetHeader(_detail::HeaderCopySource, startCopyOptions.CopySource);
          if (startCopyOptions.FilePermission.HasValue())
          {
            request.SetHeader(
                _detail::HeaderFilePermission, startCopyOptions.FilePermission.Value());
          }
          if (startCopyOptions.FilePermissionKey.HasValue())
          {
            request.SetHeader(
                _detail::HeaderFilePermissionKey, startCopyOptions.FilePermissionKey.Value());
          }
          if (startCopyOptions.XMsFilePermissionCopyMode.HasValue())
          {
            request.SetHeader(
                _detail::HeaderFilePermissionCopyMode,
                startCopyOptions.XMsFilePermissionCopyMode.Value().ToString());
          }
          if (startCopyOptions.FileCopyIgnoreReadOnly.HasValue())
          {
            request.SetHeader(
                _detail::HeaderIgnoreReadOnly,
                (startCopyOptions.FileCopyIgnoreReadOnly.Value() ? "true" : "false"));
          }
          if (startCopyOptions.FileCopyFileAttributes.HasValue())
          {
            request.SetHeader(
                _detail::HeaderFileAttributes, startCopyOptions.FileCopyFileAttributes.Value());
          }
          if (startCopyOptions.FileCopyFileCreationTime.HasValue())
          {
            request.SetHeader(
                _detail::HeaderFileCreatedOn, startCopyOptions.FileCopyFileCreationTime.Value());
          }
          if (startCopyOptions.FileCopyFileLastWriteTime.HasValue())
          {
            request.SetHeader(
                _detail::HeaderFileLastWrittenOn,
                startCopyOptions.FileCopyFileLastWriteTime.Value());
          }
          if (startCopyOptions.FileCopySetArchiveAttribute.HasValue())
          {
            request.SetHeader(
                _detail::HeaderSetArchiveAttribute,
                (startCopyOptions.FileCopySetArchiveAttribute.Value() ? "true" : "false"));
          }
          if (startCopyOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(_detail::HeaderLeaseId, startCopyOptions.LeaseIdOptional.Value());
          }
          return StartCopyParseResult(context, pipeline.Send(request, context));
        }

        struct AbortCopyOptions
        {
          std::string CopyId;
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<std::string> LeaseIdOptional;
        };

        static Azure::Response<FileAbortCopyResult> AbortCopy(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const AbortCopyOptions& abortCopyOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "copy");
          request.GetUrl().AppendQueryParameter(
              _detail::QueryCopyId, _internal::UrlEncodeQueryParameter(abortCopyOptions.CopyId));
          if (abortCopyOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(abortCopyOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderCopyActionAbortConstant, "abort");
          request.SetHeader(_detail::HeaderVersion, abortCopyOptions.ApiVersionParameter);
          if (abortCopyOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(_detail::HeaderLeaseId, abortCopyOptions.LeaseIdOptional.Value());
          }
          return AbortCopyParseResult(context, pipeline.Send(request, context));
        }

        struct ListHandlesOptions
        {
          Azure::Nullable<std::string> ContinuationToken;
          Azure::Nullable<int32_t> MaxResults;
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::string> ShareSnapshot;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
        };

        static Azure::Response<FileListHandlesResult> ListHandles(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ListHandlesOptions& listHandlesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "listhandles");
          if (listHandlesOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryContinuationToken,
                _internal::UrlEncodeQueryParameter(listHandlesOptions.ContinuationToken.Value()));
          }
          if (listHandlesOptions.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryPageSizeHint,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(listHandlesOptions.MaxResults.Value())));
          }
          if (listHandlesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(listHandlesOptions.Timeout.Value())));
          }
          if (listHandlesOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryShareSnapshot,
                _internal::UrlEncodeQueryParameter(listHandlesOptions.ShareSnapshot.Value()));
          }
          request.SetHeader(_detail::HeaderVersion, listHandlesOptions.ApiVersionParameter);
          return ListHandlesParseResult(context, pipeline.Send(request, context));
        }

        struct ForceCloseHandlesOptions
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::string> ContinuationToken;
          Azure::Nullable<std::string> ShareSnapshot;
          std::string HandleId;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
        };

        static Azure::Response<FileForceCloseHandlesResult> ForceCloseHandles(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ForceCloseHandlesOptions& forceCloseHandlesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(_detail::QueryComp, "forceclosehandles");
          if (forceCloseHandlesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(forceCloseHandlesOptions.Timeout.Value())));
          }
          if (forceCloseHandlesOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryContinuationToken,
                _internal::UrlEncodeQueryParameter(
                    forceCloseHandlesOptions.ContinuationToken.Value()));
          }
          if (forceCloseHandlesOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryShareSnapshot,
                _internal::UrlEncodeQueryParameter(forceCloseHandlesOptions.ShareSnapshot.Value()));
          }
          request.SetHeader(_detail::HeaderHandleId, forceCloseHandlesOptions.HandleId);
          request.SetHeader(_detail::HeaderVersion, forceCloseHandlesOptions.ApiVersionParameter);
          return ForceCloseHandlesParseResult(context, pipeline.Send(request, context));
        }

      private:
        static Azure::Response<FileCreateResult> CreateParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // Success, File created.
            FileCreateResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            result.IsServerEncrypted
                = response.GetHeaders().at(_detail::HeaderRequestIsServerEncrypted) == "true";
            result.SmbProperties.PermissionKey
                = response.GetHeaders().at(_detail::HeaderFilePermissionKey);
            result.SmbProperties.Attributes
                = FileAttributes(response.GetHeaders().at(_detail::HeaderAttributes));
            result.SmbProperties.CreatedOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderCreatedOn), DateTime::DateFormat::Rfc3339);
            result.SmbProperties.LastWrittenOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastWrittenOn),
                DateTime::DateFormat::Rfc3339);
            result.SmbProperties.ChangedOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderChangedOn), DateTime::DateFormat::Rfc3339);
            result.SmbProperties.FileId = response.GetHeaders().at(_detail::HeaderFileId);
            result.SmbProperties.ParentFileId
                = response.GetHeaders().at(_detail::HeaderParentFileId);
            return Azure::Response<FileCreateResult>(std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<FileDownloadResult> DownloadParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Succeeded to read the entire file.
            FileDownloadResult result;
            result.BodyStream = response.ExtractBodyStream();
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);

            for (auto i = response.GetHeaders().lower_bound(_detail::HeaderMetadata);
                 i != response.GetHeaders().end()
                 && i->first.substr(0, 9) == _detail::HeaderMetadata;
                 ++i)
            {
              result.Metadata.emplace(i->first.substr(10), i->second);
            }
            result.HttpHeaders.ContentType = response.GetHeaders().at(_detail::HeaderContentType);

            auto content_range_iterator = response.GetHeaders().find(_detail::HeaderContentRange);
            if (content_range_iterator != response.GetHeaders().end())
            {
              const std::string& content_range = content_range_iterator->second;
              auto bytes_pos = content_range.find("bytes ");
              auto dash_pos = content_range.find("-", bytes_pos + 6);
              auto slash_pos = content_range.find("/", dash_pos + 1);
              int64_t range_start_offset = std::stoll(std::string(
                  content_range.begin() + bytes_pos + 6, content_range.begin() + dash_pos));
              int64_t range_end_offset = std::stoll(std::string(
                  content_range.begin() + dash_pos + 1, content_range.begin() + slash_pos));
              result.ContentRange = Azure::Core::Http::HttpRange{
                  range_start_offset, range_end_offset - range_start_offset + 1};
            }
            else
            {
              result.ContentRange = Azure::Core::Http::HttpRange{
                  0, std::stoll(response.GetHeaders().at(_detail::HeaderContentLength))};
            }
            if (content_range_iterator != response.GetHeaders().end())
            {
              const std::string& content_range = content_range_iterator->second;
              auto slash_pos = content_range.find("/");
              result.FileSize = std::stoll(content_range.substr(slash_pos + 1));
            }
            else
            {
              result.FileSize = std::stoll(response.GetHeaders().at(_detail::HeaderContentLength));
            }
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            if (response.GetHeaders().find(_detail::HeaderTransactionalContentHashMd5)
                != response.GetHeaders().end())
            {
              result.TransactionalContentHash = _internal::FromBase64String(
                  response.GetHeaders().at(_detail::HeaderTransactionalContentHashMd5),
                  HashAlgorithm::Md5);
            }
            if (response.GetHeaders().find(_detail::HeaderContentEncoding)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentEncoding
                  = response.GetHeaders().at(_detail::HeaderContentEncoding);
            }
            if (response.GetHeaders().find(_detail::HeaderCacheControl)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.CacheControl
                  = response.GetHeaders().at(_detail::HeaderCacheControl);
            }
            if (response.GetHeaders().find(_detail::HeaderContentDisposition)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentDisposition
                  = response.GetHeaders().at(_detail::HeaderContentDisposition);
            }
            if (response.GetHeaders().find(_detail::HeaderContentLanguage)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentLanguage
                  = response.GetHeaders().at(_detail::HeaderContentLanguage);
            }
            result.AcceptRanges = response.GetHeaders().at(_detail::HeaderAcceptRanges);
            if (response.GetHeaders().find(_detail::HeaderCopyCompletedOn)
                != response.GetHeaders().end())
            {
              result.CopyCompletedOn = DateTime::Parse(
                  response.GetHeaders().at(_detail::HeaderCopyCompletedOn),
                  DateTime::DateFormat::Rfc1123);
            }
            if (response.GetHeaders().find(_detail::HeaderCopyStatusDescription)
                != response.GetHeaders().end())
            {
              result.CopyStatusDescription
                  = response.GetHeaders().at(_detail::HeaderCopyStatusDescription);
            }
            if (response.GetHeaders().find(_detail::HeaderCopyId) != response.GetHeaders().end())
            {
              result.CopyId = response.GetHeaders().at(_detail::HeaderCopyId);
            }
            if (response.GetHeaders().find(_detail::HeaderCopyProgress)
                != response.GetHeaders().end())
            {
              result.CopyProgress = response.GetHeaders().at(_detail::HeaderCopyProgress);
            }
            if (response.GetHeaders().find(_detail::HeaderCopySource)
                != response.GetHeaders().end())
            {
              result.CopySource = response.GetHeaders().at(_detail::HeaderCopySource);
            }
            if (response.GetHeaders().find(_detail::HeaderCopyStatus)
                != response.GetHeaders().end())
            {
              result.CopyStatus = CopyStatus(response.GetHeaders().at(_detail::HeaderCopyStatus));
            }
            if (response.GetHeaders().find(_detail::HeaderContentHashMd5)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentHash = _internal::FromBase64String(
                  response.GetHeaders().at(_detail::HeaderContentHashMd5), HashAlgorithm::Md5);
            }
            if (response.GetHeaders().find(_detail::HeaderIsServerEncrypted)
                != response.GetHeaders().end())
            {
              result.IsServerEncrypted
                  = response.GetHeaders().at(_detail::HeaderIsServerEncrypted) == "true";
            }
            result.SmbProperties.Attributes
                = FileAttributes(response.GetHeaders().at(_detail::HeaderAttributes));
            result.SmbProperties.CreatedOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderCreatedOn), DateTime::DateFormat::Rfc3339);
            result.SmbProperties.LastWrittenOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastWrittenOn),
                DateTime::DateFormat::Rfc3339);
            result.SmbProperties.ChangedOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderChangedOn), DateTime::DateFormat::Rfc3339);
            result.SmbProperties.PermissionKey
                = response.GetHeaders().at(_detail::HeaderFilePermissionKey);
            result.SmbProperties.FileId = response.GetHeaders().at(_detail::HeaderFileId);
            result.SmbProperties.ParentFileId
                = response.GetHeaders().at(_detail::HeaderParentFileId);
            if (response.GetHeaders().find(_detail::HeaderLeaseDuration)
                != response.GetHeaders().end())
            {
              result.LeaseDuration
                  = LeaseDuration(response.GetHeaders().at(_detail::HeaderLeaseDuration));
            }
            if (response.GetHeaders().find(_detail::HeaderLeaseState)
                != response.GetHeaders().end())
            {
              result.LeaseState = LeaseState(response.GetHeaders().at(_detail::HeaderLeaseState));
            }
            if (response.GetHeaders().find(_detail::HeaderLeaseStatus)
                != response.GetHeaders().end())
            {
              result.LeaseStatus
                  = LeaseStatus(response.GetHeaders().at(_detail::HeaderLeaseStatus));
            }
            return Azure::Response<FileDownloadResult>(std::move(result), std::move(responsePtr));
          }
          else if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::PartialContent)
          {
            // Succeeded to read a specified range of the file.
            FileDownloadResult result;
            result.BodyStream = response.ExtractBodyStream();
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);

            for (auto i = response.GetHeaders().lower_bound(_detail::HeaderMetadata);
                 i != response.GetHeaders().end()
                 && i->first.substr(0, 9) == _detail::HeaderMetadata;
                 ++i)
            {
              result.Metadata.emplace(i->first.substr(10), i->second);
            }
            result.HttpHeaders.ContentType = response.GetHeaders().at(_detail::HeaderContentType);

            auto content_range_iterator = response.GetHeaders().find(_detail::HeaderContentRange);
            if (content_range_iterator != response.GetHeaders().end())
            {
              const std::string& content_range = content_range_iterator->second;
              auto bytes_pos = content_range.find("bytes ");
              auto dash_pos = content_range.find("-", bytes_pos + 6);
              auto slash_pos = content_range.find("/", dash_pos + 1);
              int64_t range_start_offset = std::stoll(std::string(
                  content_range.begin() + bytes_pos + 6, content_range.begin() + dash_pos));
              int64_t range_end_offset = std::stoll(std::string(
                  content_range.begin() + dash_pos + 1, content_range.begin() + slash_pos));
              result.ContentRange = Azure::Core::Http::HttpRange{
                  range_start_offset, range_end_offset - range_start_offset + 1};
            }
            else
            {
              result.ContentRange = Azure::Core::Http::HttpRange{
                  0, std::stoll(response.GetHeaders().at(_detail::HeaderContentLength))};
            }
            if (content_range_iterator != response.GetHeaders().end())
            {
              const std::string& content_range = content_range_iterator->second;
              auto slash_pos = content_range.find("/");
              result.FileSize = std::stoll(content_range.substr(slash_pos + 1));
            }
            else
            {
              result.FileSize = std::stoll(response.GetHeaders().at(_detail::HeaderContentLength));
            }
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            if (response.GetHeaders().find(_detail::HeaderTransactionalContentHashMd5)
                != response.GetHeaders().end())
            {
              result.TransactionalContentHash = _internal::FromBase64String(
                  response.GetHeaders().at(_detail::HeaderTransactionalContentHashMd5),
                  HashAlgorithm::Md5);
            }
            if (response.GetHeaders().find(_detail::HeaderContentEncoding)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentEncoding
                  = response.GetHeaders().at(_detail::HeaderContentEncoding);
            }
            if (response.GetHeaders().find(_detail::HeaderCacheControl)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.CacheControl
                  = response.GetHeaders().at(_detail::HeaderCacheControl);
            }
            if (response.GetHeaders().find(_detail::HeaderContentDisposition)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentDisposition
                  = response.GetHeaders().at(_detail::HeaderContentDisposition);
            }
            if (response.GetHeaders().find(_detail::HeaderContentLanguage)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentLanguage
                  = response.GetHeaders().at(_detail::HeaderContentLanguage);
            }
            result.AcceptRanges = response.GetHeaders().at(_detail::HeaderAcceptRanges);
            if (response.GetHeaders().find(_detail::HeaderCopyCompletedOn)
                != response.GetHeaders().end())
            {
              result.CopyCompletedOn = DateTime::Parse(
                  response.GetHeaders().at(_detail::HeaderCopyCompletedOn),
                  DateTime::DateFormat::Rfc1123);
            }
            if (response.GetHeaders().find(_detail::HeaderCopyStatusDescription)
                != response.GetHeaders().end())
            {
              result.CopyStatusDescription
                  = response.GetHeaders().at(_detail::HeaderCopyStatusDescription);
            }
            if (response.GetHeaders().find(_detail::HeaderCopyId) != response.GetHeaders().end())
            {
              result.CopyId = response.GetHeaders().at(_detail::HeaderCopyId);
            }
            if (response.GetHeaders().find(_detail::HeaderCopyProgress)
                != response.GetHeaders().end())
            {
              result.CopyProgress = response.GetHeaders().at(_detail::HeaderCopyProgress);
            }
            if (response.GetHeaders().find(_detail::HeaderCopySource)
                != response.GetHeaders().end())
            {
              result.CopySource = response.GetHeaders().at(_detail::HeaderCopySource);
            }
            if (response.GetHeaders().find(_detail::HeaderCopyStatus)
                != response.GetHeaders().end())
            {
              result.CopyStatus = CopyStatus(response.GetHeaders().at(_detail::HeaderCopyStatus));
            }
            if (response.GetHeaders().find(_detail::HeaderContentHashMd5)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentHash = _internal::FromBase64String(
                  response.GetHeaders().at(_detail::HeaderContentHashMd5), HashAlgorithm::Md5);
            }
            if (response.GetHeaders().find(_detail::HeaderIsServerEncrypted)
                != response.GetHeaders().end())
            {
              result.IsServerEncrypted
                  = response.GetHeaders().at(_detail::HeaderIsServerEncrypted) == "true";
            }
            result.SmbProperties.Attributes
                = FileAttributes(response.GetHeaders().at(_detail::HeaderAttributes));
            result.SmbProperties.CreatedOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderCreatedOn), DateTime::DateFormat::Rfc3339);
            result.SmbProperties.LastWrittenOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastWrittenOn),
                DateTime::DateFormat::Rfc3339);
            result.SmbProperties.ChangedOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderChangedOn), DateTime::DateFormat::Rfc3339);
            result.SmbProperties.PermissionKey
                = response.GetHeaders().at(_detail::HeaderFilePermissionKey);
            result.SmbProperties.FileId = response.GetHeaders().at(_detail::HeaderFileId);
            result.SmbProperties.ParentFileId
                = response.GetHeaders().at(_detail::HeaderParentFileId);
            if (response.GetHeaders().find(_detail::HeaderLeaseDuration)
                != response.GetHeaders().end())
            {
              result.LeaseDuration
                  = LeaseDuration(response.GetHeaders().at(_detail::HeaderLeaseDuration));
            }
            if (response.GetHeaders().find(_detail::HeaderLeaseState)
                != response.GetHeaders().end())
            {
              result.LeaseState = LeaseState(response.GetHeaders().at(_detail::HeaderLeaseState));
            }
            if (response.GetHeaders().find(_detail::HeaderLeaseStatus)
                != response.GetHeaders().end())
            {
              result.LeaseStatus
                  = LeaseStatus(response.GetHeaders().at(_detail::HeaderLeaseStatus));
            }
            return Azure::Response<FileDownloadResult>(std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<FileGetPropertiesResult> GetPropertiesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success.
            FileGetPropertiesResult result;
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);

            for (auto i = response.GetHeaders().lower_bound(_detail::HeaderMetadata);
                 i != response.GetHeaders().end()
                 && i->first.substr(0, 9) == _detail::HeaderMetadata;
                 ++i)
            {
              result.Metadata.emplace(i->first.substr(10), i->second);
            }
            result.FileSize = std::stoll(response.GetHeaders().at(_detail::HeaderContentLength));
            if (response.GetHeaders().find(_detail::HeaderContentType)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentType = response.GetHeaders().at(_detail::HeaderContentType);
            }
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            if (response.GetHeaders().find(_detail::HeaderTransactionalContentHashMd5)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentHash = _internal::FromBase64String(
                  response.GetHeaders().at(_detail::HeaderTransactionalContentHashMd5),
                  HashAlgorithm::Md5);
            }
            if (response.GetHeaders().find(_detail::HeaderContentEncoding)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentEncoding
                  = response.GetHeaders().at(_detail::HeaderContentEncoding);
            }
            if (response.GetHeaders().find(_detail::HeaderCacheControl)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.CacheControl
                  = response.GetHeaders().at(_detail::HeaderCacheControl);
            }
            if (response.GetHeaders().find(_detail::HeaderContentDisposition)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentDisposition
                  = response.GetHeaders().at(_detail::HeaderContentDisposition);
            }
            if (response.GetHeaders().find(_detail::HeaderContentLanguage)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentLanguage
                  = response.GetHeaders().at(_detail::HeaderContentLanguage);
            }
            if (response.GetHeaders().find(_detail::HeaderCopyCompletedOn)
                != response.GetHeaders().end())
            {
              result.CopyCompletedOn = DateTime::Parse(
                  response.GetHeaders().at(_detail::HeaderCopyCompletedOn),
                  DateTime::DateFormat::Rfc1123);
            }
            if (response.GetHeaders().find(_detail::HeaderCopyStatusDescription)
                != response.GetHeaders().end())
            {
              result.CopyStatusDescription
                  = response.GetHeaders().at(_detail::HeaderCopyStatusDescription);
            }
            if (response.GetHeaders().find(_detail::HeaderCopyId) != response.GetHeaders().end())
            {
              result.CopyId = response.GetHeaders().at(_detail::HeaderCopyId);
            }
            if (response.GetHeaders().find(_detail::HeaderCopyProgress)
                != response.GetHeaders().end())
            {
              result.CopyProgress = response.GetHeaders().at(_detail::HeaderCopyProgress);
            }
            if (response.GetHeaders().find(_detail::HeaderCopySource)
                != response.GetHeaders().end())
            {
              result.CopySource = response.GetHeaders().at(_detail::HeaderCopySource);
            }
            if (response.GetHeaders().find(_detail::HeaderCopyStatus)
                != response.GetHeaders().end())
            {
              result.CopyStatus = CopyStatus(response.GetHeaders().at(_detail::HeaderCopyStatus));
            }
            if (response.GetHeaders().find(_detail::HeaderIsServerEncrypted)
                != response.GetHeaders().end())
            {
              result.IsServerEncrypted
                  = response.GetHeaders().at(_detail::HeaderIsServerEncrypted) == "true";
            }
            result.SmbProperties.Attributes
                = FileAttributes(response.GetHeaders().at(_detail::HeaderAttributes));
            result.SmbProperties.CreatedOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderCreatedOn), DateTime::DateFormat::Rfc3339);
            result.SmbProperties.LastWrittenOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastWrittenOn),
                DateTime::DateFormat::Rfc3339);
            result.SmbProperties.ChangedOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderChangedOn), DateTime::DateFormat::Rfc3339);
            result.SmbProperties.PermissionKey
                = response.GetHeaders().at(_detail::HeaderFilePermissionKey);
            result.SmbProperties.FileId = response.GetHeaders().at(_detail::HeaderFileId);
            result.SmbProperties.ParentFileId
                = response.GetHeaders().at(_detail::HeaderParentFileId);
            if (response.GetHeaders().find(_detail::HeaderLeaseDuration)
                != response.GetHeaders().end())
            {
              result.LeaseDuration
                  = LeaseDuration(response.GetHeaders().at(_detail::HeaderLeaseDuration));
            }
            if (response.GetHeaders().find(_detail::HeaderLeaseState)
                != response.GetHeaders().end())
            {
              result.LeaseState = LeaseState(response.GetHeaders().at(_detail::HeaderLeaseState));
            }
            if (response.GetHeaders().find(_detail::HeaderLeaseStatus)
                != response.GetHeaders().end())
            {
              result.LeaseStatus
                  = LeaseStatus(response.GetHeaders().at(_detail::HeaderLeaseStatus));
            }
            return Azure::Response<FileGetPropertiesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<FileDeleteResult> DeleteParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
          {
            // Success (Accepted).
            FileDeleteResult result;
            return Azure::Response<FileDeleteResult>(std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<FileSetHttpHeadersResult> SetHttpHeadersParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success
            FileSetHttpHeadersResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            result.IsServerEncrypted
                = response.GetHeaders().at(_detail::HeaderRequestIsServerEncrypted) == "true";
            result.SmbProperties.PermissionKey
                = response.GetHeaders().at(_detail::HeaderFilePermissionKey);
            result.SmbProperties.Attributes
                = FileAttributes(response.GetHeaders().at(_detail::HeaderAttributes));
            result.SmbProperties.CreatedOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderCreatedOn), DateTime::DateFormat::Rfc3339);
            result.SmbProperties.LastWrittenOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastWrittenOn),
                DateTime::DateFormat::Rfc3339);
            result.SmbProperties.ChangedOn = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderChangedOn), DateTime::DateFormat::Rfc3339);
            result.SmbProperties.FileId = response.GetHeaders().at(_detail::HeaderFileId);
            result.SmbProperties.ParentFileId
                = response.GetHeaders().at(_detail::HeaderParentFileId);
            return Azure::Response<FileSetHttpHeadersResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<FileSetMetadataResult> SetMetadataParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success (OK).
            FileSetMetadataResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.IsServerEncrypted
                = response.GetHeaders().at(_detail::HeaderRequestIsServerEncrypted) == "true";
            return Azure::Response<FileSetMetadataResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<FileAcquireLeaseResult> AcquireLeaseParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // The Acquire operation completed successfully.
            FileAcquireLeaseResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            result.LeaseId = response.GetHeaders().at(_detail::HeaderLeaseId);
            return Azure::Response<FileAcquireLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<FileReleaseLeaseResult> ReleaseLeaseParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // The Release operation completed successfully.
            FileReleaseLeaseResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            return Azure::Response<FileReleaseLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<FileChangeLeaseResult> ChangeLeaseParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // The Change operation completed successfully.
            FileChangeLeaseResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            result.LeaseId = response.GetHeaders().at(_detail::HeaderLeaseId);
            return Azure::Response<FileChangeLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<FileBreakLeaseResult> BreakLeaseParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
          {
            // The Break operation completed successfully.
            FileBreakLeaseResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            if (response.GetHeaders().find(_detail::HeaderLeaseId) != response.GetHeaders().end())
            {
              result.LeaseId = response.GetHeaders().at(_detail::HeaderLeaseId);
            }
            return Azure::Response<FileBreakLeaseResult>(std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<FileUploadRangeResult> UploadRangeParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // Success (Created).
            FileUploadRangeResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            if (response.GetHeaders().find(_detail::HeaderTransactionalContentHashMd5)
                != response.GetHeaders().end())
            {
              result.TransactionalContentHash = _internal::FromBase64String(
                  response.GetHeaders().at(_detail::HeaderTransactionalContentHashMd5),
                  HashAlgorithm::Md5);
            }
            if (response.GetHeaders().find(_detail::HeaderRequestIsServerEncrypted)
                != response.GetHeaders().end())
            {
              result.IsServerEncrypted
                  = response.GetHeaders().at(_detail::HeaderRequestIsServerEncrypted) == "true";
            }
            return Azure::Response<FileUploadRangeResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<FileUploadRangeFromUrlResult> UploadRangeFromUrlParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // Success (Created).
            FileUploadRangeFromUrlResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            result.TransactionalContentHash = _internal::FromBase64String(
                response.GetHeaders().at(_detail::HeaderTransactionalContentHashCrc64),
                HashAlgorithm::Crc64);
            result.IsServerEncrypted
                = response.GetHeaders().at(_detail::HeaderRequestIsServerEncrypted) == "true";
            return Azure::Response<FileUploadRangeFromUrlResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<FileGetRangeListResult> GetRangeListParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success.
            const auto& bodyBuffer = response.GetBody();
            auto reader = _internal::XmlReader(
                reinterpret_cast<const char*>(bodyBuffer.data()), bodyBuffer.size());
            FileGetRangeListResult result = bodyBuffer.empty()
                ? FileGetRangeListResult()
                : FileGetRangeListResultFromRangeList(RangeListFromXml(reader));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.FileSize = std::stoll(response.GetHeaders().at(_detail::HeaderXMsContentLength));
            return Azure::Response<FileGetRangeListResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static RangeList RangeListFromXml(_internal::XmlReader& reader)
        {
          auto result = RangeList();
          enum class XmlTagName
          {
            ClearRange,
            Range,
            Ranges,
            Unknown,
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

              if (std::strcmp(node.Name, "ClearRange") == 0)
              {
                path.emplace_back(XmlTagName::ClearRange);
              }
              else if (std::strcmp(node.Name, "Range") == 0)
              {
                path.emplace_back(XmlTagName::Range);
              }
              else if (std::strcmp(node.Name, "Ranges") == 0)
              {
                path.emplace_back(XmlTagName::Ranges);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }
              if (path.size() == 2 && path[0] == XmlTagName::Ranges && path[1] == XmlTagName::Range)
              {
                result.Ranges.emplace_back(HttpRangeFromXml(reader));
                path.pop_back();
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::Ranges
                  && path[1] == XmlTagName::ClearRange)
              {
                result.ClearRanges.emplace_back(HttpRangeFromXml(reader));
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
            }
          }
          return result;
        }

        static FileGetRangeListResult FileGetRangeListResultFromRangeList(RangeList object)
        {
          FileGetRangeListResult result;
          result.Ranges = std::move(object.Ranges);
          result.ClearRanges = std::move(object.ClearRanges);

          return result;
        }
        static Azure::Response<FileStartCopyResult> StartCopyParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
          {
            // The copy file has been accepted with the specified copy status.
            FileStartCopyResult result;
            result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            result.LastModified = DateTime::Parse(
                response.GetHeaders().at(_detail::HeaderLastModified),
                DateTime::DateFormat::Rfc1123);
            if (response.GetHeaders().find(_detail::HeaderCopyId) != response.GetHeaders().end())
            {
              result.CopyId = response.GetHeaders().at(_detail::HeaderCopyId);
            }
            if (response.GetHeaders().find(_detail::HeaderCopyStatus)
                != response.GetHeaders().end())
            {
              result.CopyStatus = CopyStatus(response.GetHeaders().at(_detail::HeaderCopyStatus));
            }
            return Azure::Response<FileStartCopyResult>(std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<FileAbortCopyResult> AbortCopyParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::NoContent)
          {
            // The delete request was accepted and the file will be deleted.
            FileAbortCopyResult result;
            return Azure::Response<FileAbortCopyResult>(std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<FileListHandlesResult> ListHandlesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success.
            const auto& bodyBuffer = response.GetBody();
            auto reader = _internal::XmlReader(
                reinterpret_cast<const char*>(bodyBuffer.data()), bodyBuffer.size());
            FileListHandlesResult result = bodyBuffer.empty()
                ? FileListHandlesResult()
                : FileListHandlesResultFromListHandlesResponse(ListHandlesResponseFromXml(reader));
            result.HttpHeaders.ContentType = response.GetHeaders().at(_detail::HeaderContentType);
            return Azure::Response<FileListHandlesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static HandleItem HandleItemFromXml(_internal::XmlReader& reader)
        {
          auto result = HandleItem();
          enum class XmlTagName
          {
            ClientIp,
            FileId,
            HandleId,
            LastReconnectTime,
            OpenTime,
            ParentId,
            Path,
            SessionId,
            Unknown,
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

              if (std::strcmp(node.Name, "ClientIp") == 0)
              {
                path.emplace_back(XmlTagName::ClientIp);
              }
              else if (std::strcmp(node.Name, "FileId") == 0)
              {
                path.emplace_back(XmlTagName::FileId);
              }
              else if (std::strcmp(node.Name, "HandleId") == 0)
              {
                path.emplace_back(XmlTagName::HandleId);
              }
              else if (std::strcmp(node.Name, "LastReconnectTime") == 0)
              {
                path.emplace_back(XmlTagName::LastReconnectTime);
              }
              else if (std::strcmp(node.Name, "OpenTime") == 0)
              {
                path.emplace_back(XmlTagName::OpenTime);
              }
              else if (std::strcmp(node.Name, "ParentId") == 0)
              {
                path.emplace_back(XmlTagName::ParentId);
              }
              else if (std::strcmp(node.Name, "Path") == 0)
              {
                path.emplace_back(XmlTagName::Path);
              }
              else if (std::strcmp(node.Name, "SessionId") == 0)
              {
                path.emplace_back(XmlTagName::SessionId);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::ClientIp)
              {
                result.ClientIp = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::FileId)
              {
                result.FileId = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::HandleId)
              {
                result.HandleId = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::LastReconnectTime)
              {
                result.LastReconnectedOn
                    = DateTime::Parse(node.Value, DateTime::DateFormat::Rfc1123);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::OpenTime)
              {
                result.OpenedOn = DateTime::Parse(node.Value, DateTime::DateFormat::Rfc1123);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::ParentId)
              {
                result.ParentId = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::Path)
              {
                result.Path = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::SessionId)
              {
                result.SessionId = node.Value;
              }
            }
          }
          return result;
        }

        static ListHandlesResponse ListHandlesResponseFromXml(_internal::XmlReader& reader)
        {
          auto result = ListHandlesResponse();
          enum class XmlTagName
          {
            Entries,
            EnumerationResults,
            Handle,
            NextMarker,
            Unknown,
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

              if (std::strcmp(node.Name, "Entries") == 0)
              {
                path.emplace_back(XmlTagName::Entries);
              }
              else if (std::strcmp(node.Name, "EnumerationResults") == 0)
              {
                path.emplace_back(XmlTagName::EnumerationResults);
              }
              else if (std::strcmp(node.Name, "Handle") == 0)
              {
                path.emplace_back(XmlTagName::Handle);
              }
              else if (std::strcmp(node.Name, "NextMarker") == 0)
              {
                path.emplace_back(XmlTagName::NextMarker);
              }
              else
              {
                path.emplace_back(XmlTagName::Unknown);
              }
              if (path.size() == 3 && path[0] == XmlTagName::EnumerationResults
                  && path[1] == XmlTagName::Entries && path[2] == XmlTagName::Handle)
              {
                result.HandleList.emplace_back(HandleItemFromXml(reader));
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 2 && path[0] == XmlTagName::EnumerationResults
                  && path[1] == XmlTagName::NextMarker)
              {
                result.ContinuationToken = node.Value;
              }
            }
          }
          return result;
        }

        static FileListHandlesResult FileListHandlesResultFromListHandlesResponse(
            ListHandlesResponse object)
        {
          FileListHandlesResult result;
          result.HandleList = std::move(object.HandleList);
          result.ContinuationToken = std::move(object.ContinuationToken);

          return result;
        }
        static Azure::Response<FileForceCloseHandlesResult> ForceCloseHandlesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success.
            FileForceCloseHandlesResult result;
            if (response.GetHeaders().find(_detail::HeaderContinuationToken)
                != response.GetHeaders().end())
            {
              result.ContinuationToken = response.GetHeaders().at(_detail::HeaderContinuationToken);
            }
            result.NumberOfHandlesClosed
                = std::stoi(response.GetHeaders().at(_detail::HeaderNumberOfHandlesClosed));
            result.NumberOfHandlesFailedToClose
                = std::stoi(response.GetHeaders().at(_detail::HeaderNumberOfHandlesFailedToClose));
            return Azure::Response<FileForceCloseHandlesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }
      };

    }; // class ShareRestClient

  } // namespace _detail

}}}} // namespace Azure::Storage::Files::Shares
