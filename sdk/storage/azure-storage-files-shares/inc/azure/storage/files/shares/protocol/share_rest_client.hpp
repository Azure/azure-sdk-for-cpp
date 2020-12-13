
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/datetime.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/pipeline.hpp"
#include "azure/core/nullable.hpp"
#include "azure/core/response.hpp"
#include "azure/core/strings.hpp"
#include "azure/storage/common/crypt.hpp"
#include "azure/storage/common/storage_common.hpp"
#include "azure/storage/common/storage_exception.hpp"
#include "azure/storage/common/xml_wrapper.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  namespace Details {
    constexpr static const char* DefaultServiceApiVersion = "2020-02-10";
    constexpr static const char* QueryCopyId = "copyid";
    constexpr static const char* QueryListSharesInclude = "include";
    constexpr static const char* QueryContinuationToken = "marker";
    constexpr static const char* QueryMaxResults = "maxresults";
    constexpr static const char* QueryPrefix = "prefix";
    constexpr static const char* QueryPrevShareSnapshot = "prevsharesnapshot";
    constexpr static const char* QueryShareSnapshot = "sharesnapshot";
    constexpr static const char* QueryTimeout = "timeout";
    constexpr static const char* QueryRestype = "restype";
    constexpr static const char* QueryComp = "comp";
    constexpr static const char* HeaderVersion = "x-ms-version";
    constexpr static const char* HeaderRequestId = "x-ms-client-request-id";
    constexpr static const char* HeaderContentLength = "content-length";
    constexpr static const char* HeaderContentHash = "content-md5";
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
    constexpr static const char* HeaderFileRangeWriteFromUrlDefault = "update";
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
    constexpr static const char* HeaderSourceContentHash = "x-ms-source-content-crc64";
    constexpr static const char* HeaderSourceIfMatchHash = "x-ms-source-if-match-crc64";
    constexpr static const char* HeaderSourceIfNoneMatchHash = "x-ms-source-if-none-match-crc64";
    constexpr static const char* HeaderSourceRange = "x-ms-source-range";
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
    constexpr static const char* HeaderLeaseTime = "x-ms-lease-time";
    constexpr static const char* HeaderClientRequestId = "x-ms-client-request-id";
    constexpr static const char* HeaderAction = "x-ms-lease-action";
    constexpr static const char* HeaderSnapshot = "x-ms-snapshot";
    constexpr static const char* HeaderRequestIsServerEncrypted = "x-ms-request-server-encrypted";
    constexpr static const char* HeaderFileChangedOn = "x-ms-file-change-time";
    constexpr static const char* HeaderFileId = "x-ms-file-id";
    constexpr static const char* HeaderFileParentId = "x-ms-file-parent-id";
    constexpr static const char* HeaderIsServerEncrypted = "x-ms-server-encrypted";
    constexpr static const char* HeaderContentType = "content-type";
    constexpr static const char* HeaderContinuationToken = "x-ms-marker";
    constexpr static const char* HeaderNumberOfHandlesClosed = "x-ms-number-of-handles-closed";
    constexpr static const char* HeaderNumberOfHandlesFailedToClose
        = "x-ms-number-of-handles-failed";
    constexpr static const char* HeaderXMsContentLength = "x-ms-content-length";
    constexpr static const char* HeaderContentRange = "content-range";
    constexpr static const char* HeaderTransactionalContentHash = "content-md5";
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
    constexpr static const char* HeaderFileType = "x-ms-type";
    constexpr static const char* HeaderXMsRange = "x-ms-range";
    constexpr static const char* HeaderFileRangeWrite = "x-ms-write";
    constexpr static const char* HeaderFileRangeWriteTypeDefault = "update";
  } // namespace Details
  namespace Models {
    struct ShareFileHttpHeaders
    {
      std::string CacheControl;
      std::string ContentDisposition;
      std::string ContentEncoding;
      std::string ContentLanguage;
      std::string ContentType;
      Storage::ContentHash ContentHash;
    };
    // Specifies the option to copy file security descriptor from source file or to set it using the
    // value which is defined by the header value of x-ms-file-permission or
    // x-ms-file-permission-key.
    enum class PermissionCopyModeType
    {
      Source,
      Override,
      Unknown
    };

    // Specifies the option include to delete the base share and all of its snapshots.
    enum class DeleteSnapshotsOptionType
    {
      Include,
      Unknown
    };

    // Only update is supported: - Update: Writes the bytes downloaded from the source url into the
    // specified range.
    enum class FileRangeWriteFromUrlType
    {
      Update,
      Unknown
    };

    // An Access policy.
    struct AccessPolicy
    {
      Core::DateTime StartsOn; // The date-time the policy is active.
      Core::DateTime ExpiresOn; // The date-time the policy expires.
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
    struct FileProperty
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
      Models::FileProperty Properties;
    };

    // Abstract for entries that can be listed from Directory.
    struct FilesAndDirectoriesListSegment
    {
      std::vector<Models::DirectoryItem> DirectoryItems;
      std::vector<Models::FileItem> FileItems;
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
      Core::DateTime OpenedOn; // Time when the session that previously opened the handle has last
                               // been reconnected. (UTC)
      Core::DateTime LastReconnectedOn; // Time handle was last connected to (UTC)
    };

    // When a file or share is leased, specifies whether the lease is of infinite or fixed duration.
    enum class LeaseDurationType
    {
      Infinite,
      Fixed,
      Unknown
    };

    // Lease state of the file or share.
    enum class LeaseStateType
    {
      Available,
      Leased,
      Expired,
      Breaking,
      Broken,
      Unknown
    };

    // The current lease status of the file or share.
    enum class LeaseStatusType
    {
      Locked,
      Unlocked,
      Unknown
    };

    // An enumeration of directories and files.
    struct ListFilesAndDirectoriesSegmentResponse
    {
      std::string ServiceEndpoint;
      std::string ShareName;
      std::string ShareSnapshot;
      std::string DirectoryPath;
      std::string Prefix;
      std::string PreviousContinuationToken;
      int32_t MaxResults = int32_t();
      Models::FilesAndDirectoriesListSegment Segment;
      std::string ContinuationToken;
    };

    // An enumeration of handles.
    struct ListHandlesResponse
    {
      std::vector<Models::HandleItem> HandleList;
      std::string ContinuationToken;
    };

    // Properties of a share.
    struct ShareProperties
    {
      Core::DateTime LastModified;
      std::string Etag;
      int64_t Quota = int64_t();
      Azure::Core::Nullable<int32_t> ProvisionedIops;
      Azure::Core::Nullable<int32_t> ProvisionedIngressMBps;
      Azure::Core::Nullable<int32_t> ProvisionedEgressMBps;
      Azure::Core::Nullable<Core::DateTime> NextAllowedQuotaDowngradeTime;
      Azure::Core::Nullable<Core::DateTime> DeletedOn;
      int32_t RemainingRetentionDays = int32_t();
      Models::LeaseStatusType LeaseStatus = Models::LeaseStatusType::Unknown;
      Models::LeaseStateType LeaseState = Models::LeaseStateType::Unknown;
      Models::LeaseDurationType LeaseDuration = Models::LeaseDurationType::Unknown;
    };

    // A listed Azure Storage share item.
    struct ShareItem
    {
      std::string Name;
      std::string Snapshot;
      bool Deleted = bool();
      std::string Version;
      Models::ShareProperties Properties;
      Storage::Metadata ShareMetadata;
    };

    // An enumeration of shares.
    struct ListSharesResponse
    {
      std::string ServiceEndpoint;
      std::string Prefix;
      std::string PreviousContinuationToken;
      int32_t MaxResults = int32_t();
      std::vector<Models::ShareItem> ShareItems;
      std::string ContinuationToken;
    };

    // The retention policy.
    struct ShareRetentionPolicy
    {
      bool Enabled
          = bool(); // Indicates whether a retention policy is enabled for the File service. If
                    // false, metrics data is retained, and the user is responsible for deleting it.
      Azure::Core::Nullable<int32_t>
          Days; // Indicates the number of days that metrics data should be retained. All data older
                // than this value will be deleted. Metrics data is deleted on a best-effort basis
                // after the retention period expires.
    };

    // Storage Analytics metrics for file service.
    struct Metrics
    {
      std::string Version; // The version of Storage Analytics to configure.
      bool Enabled = bool(); // Indicates whether metrics are enabled for the File service.
      Azure::Core::Nullable<bool> IncludeApis; // Indicates whether metrics should generate summary
                                               // statistics for called API operations.
      Models::ShareRetentionPolicy RetentionPolicy;
    };

    // Settings for SMB multichannel
    struct SmbMultichannel
    {
      bool Enabled = bool(); // If SMB multichannel is enabled.
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

    // Settings for SMB protocol.
    struct SmbSettings
    {
      Models::SmbMultichannel Multichannel; // Settings for SMB Multichannel.
    };

    // Protocol settings
    struct ShareProtocolSettings
    {
      Models::SmbSettings Settings; // Settings for SMB protocol.
    };

    // The list of file ranges
    struct ShareFileRangeList
    {
      std::vector<Models::FileRange> Ranges;
      std::vector<Models::ClearRange> ClearRanges;
    };

    // Stats for the share.
    struct ShareStats
    {
      int64_t ShareUsageBytes
          = int64_t(); // The approximate size of the data stored in bytes. Note that this value may
                       // not include all recently created or recently resized files.
    };

    // Signed identifier.
    struct SignedIdentifier
    {
      std::string Id; // A unique id.
      Models::AccessPolicy Policy; // The access policy.
    };

    // Storage service properties.
    struct StorageServiceProperties
    {
      Models::Metrics HourMetrics; // A summary of request statistics grouped by API in hourly
                                   // aggregates for files.
      Models::Metrics MinuteMetrics; // A summary of request statistics grouped by API in minute
                                     // aggregates for files.
      std::vector<Models::CorsRule> Cors; // The set of CORS rules.
      Azure::Core::Nullable<Models::ShareProtocolSettings> Protocol; // Protocol settings
    };

    // A permission (a security descriptor) at the share level.
    struct SharePermission
    {
      std::string
          Permission; // The permission in the Security Descriptor Definition Language (SDDL).
    };

    // Describes what lease action to take.
    enum class LeaseAction
    {
      Acquire,
      Release,
      Change,
      Renew,
      Break,
      Unknown
    };

    // State of the copy operation identified by 'x-ms-copy-id'.
    enum class CopyStatusType
    {
      Pending,
      Success,
      Aborted,
      Failed,
      Unknown
    };

    // Specify one of the following options: - Update: Writes the bytes specified by the request
    // body into the specified range. The Range and Content-Length headers must match to perform the
    // update. - Clear: Clears the specified range and releases the space used in storage for that
    // range. To clear a range, set the Content-Length header to zero, and set the Range header to a
    // value that indicates the range to clear, up to maximum file size.
    enum class FileRangeWriteType
    {
      Update,
      Clear,
      Unknown
    };

    struct ServiceSetPropertiesResult
    {
    };

    struct ServiceGetPropertiesResult
    {
      Models::Metrics HourMetrics;
      Models::Metrics MinuteMetrics;
      std::vector<Models::CorsRule> Cors;
      Azure::Core::Nullable<Models::ShareProtocolSettings> Protocol;
    };

    struct ServiceListSharesSegmentResult
    {
      std::string ServiceEndpoint;
      std::string Prefix;
      std::string PreviousContinuationToken;
      int32_t MaxResults = int32_t();
      std::vector<Models::ShareItem> ShareItems;
      std::string ContinuationToken;
    };

    struct ShareCreateResult
    {
      std::string ETag;
      Core::DateTime LastModified;
    };

    struct ShareGetPropertiesResult
    {
      Storage::Metadata Metadata;
      std::string ETag;
      Core::DateTime LastModified;
      int64_t Quota = int64_t();
      Azure::Core::Nullable<int32_t> ProvisionedIops;
      Azure::Core::Nullable<int32_t> ProvisionedIngressMBps;
      Azure::Core::Nullable<int32_t> ProvisionedEgressMBps;
      Azure::Core::Nullable<Core::DateTime> NextAllowedQuotaDowngradeTime;
      Azure::Core::Nullable<LeaseDurationType> LeaseDuration;
      Azure::Core::Nullable<LeaseStateType> LeaseState;
      Azure::Core::Nullable<LeaseStatusType> LeaseStatus;
    };

    struct ShareDeleteResult
    {
    };

    struct ShareAcquireLeaseResult
    {
      std::string ETag;
      Core::DateTime LastModified;
      Azure::Core::Nullable<int32_t> LeaseTime;
      std::string LeaseId;
    };

    struct ShareReleaseLeaseResult
    {
      std::string ETag;
      Core::DateTime LastModified;
      Azure::Core::Nullable<int32_t> LeaseTime;
    };

    struct ShareChangeLeaseResult
    {
      std::string ETag;
      Core::DateTime LastModified;
      Azure::Core::Nullable<int32_t> LeaseTime;
      std::string LeaseId;
    };

    struct ShareRenewLeaseResult
    {
      std::string ETag;
      Core::DateTime LastModified;
      Azure::Core::Nullable<int32_t> LeaseTime;
      std::string LeaseId;
    };

    struct ShareBreakLeaseResult
    {
      std::string ETag;
      Core::DateTime LastModified;
      int32_t LeaseTime = int32_t();
      Azure::Core::Nullable<std::string> LeaseId;
    };

    struct ShareCreateSnapshotResult
    {
      std::string Snapshot;
      std::string ETag;
      Core::DateTime LastModified;
    };

    struct ShareCreatePermissionResult
    {
      std::string FilePermissionKey;
    };

    struct ShareGetPermissionResult
    {
      std::string Permission;
    };

    struct ShareSetQuotaResult
    {
      std::string ETag;
      Core::DateTime LastModified;
    };

    struct ShareSetMetadataResult
    {
      std::string ETag;
      Core::DateTime LastModified;
    };

    struct ShareGetAccessPolicyResult
    {
      std::vector<Models::SignedIdentifier> SignedIdentifiers;
      std::string ETag;
      Core::DateTime LastModified;
    };

    struct ShareSetAccessPolicyResult
    {
      std::string ETag;
      Core::DateTime LastModified;
    };

    struct ShareGetStatisticsResult
    {
      int64_t ShareUsageBytes = int64_t();
      std::string ETag;
      Core::DateTime LastModified;
    };

    struct ShareRestoreResult
    {
      std::string ETag;
      Core::DateTime LastModified;
    };

    struct DirectoryCreateResult
    {
      std::string ETag;
      Core::DateTime LastModified;
      bool IsServerEncrypted = bool();
      std::string FilePermissionKey;
      std::string FileAttributes;
      Core::DateTime FileCreatedOn;
      Core::DateTime FileLastWrittenOn;
      Core::DateTime FileChangedOn;
      std::string FileId;
      std::string FileParentId;
    };

    struct DirectoryGetPropertiesResult
    {
      Storage::Metadata Metadata;
      std::string ETag;
      Core::DateTime LastModified;
      bool IsServerEncrypted = bool();
      std::string FileAttributes;
      Core::DateTime FileCreatedOn;
      Core::DateTime FileLastWrittenOn;
      Core::DateTime FileChangedOn;
      std::string FilePermissionKey;
      std::string FileId;
      std::string FileParentId;
    };

    struct DirectoryDeleteResult
    {
    };

    struct DirectorySetPropertiesResult
    {
      std::string ETag;
      Core::DateTime LastModified;
      bool IsServerEncrypted = bool();
      std::string FilePermissionKey;
      std::string FileAttributes;
      Core::DateTime FileCreatedOn;
      Core::DateTime FileLastWrittenOn;
      Core::DateTime FileChangedOn;
      std::string FileId;
      std::string FileParentId;
    };

    struct DirectorySetMetadataResult
    {
      std::string ETag;
      bool IsServerEncrypted = bool();
    };

    struct DirectoryListFilesAndDirectoriesSegmentResult
    {
      std::string ServiceEndpoint;
      std::string ShareName;
      std::string ShareSnapshot;
      std::string DirectoryPath;
      std::string Prefix;
      std::string PreviousContinuationToken;
      int32_t MaxResults = int32_t();
      Models::FilesAndDirectoriesListSegment Segment;
      std::string ContinuationToken;
      ShareFileHttpHeaders HttpHeaders;
    };

    struct DirectoryListHandlesResult
    {
      std::vector<Models::HandleItem> HandleList;
      std::string ContinuationToken;
      ShareFileHttpHeaders HttpHeaders;
    };

    struct DirectoryForceCloseHandlesResult
    {
      Azure::Core::Nullable<std::string> ContinuationToken;
      int32_t numberOfHandlesClosed = int32_t();
      int32_t numberOfHandlesFailedToClose = int32_t();
    };

    struct FileCreateResult
    {
      std::string ETag;
      Core::DateTime LastModified;
      bool IsServerEncrypted = bool();
      std::string FilePermissionKey;
      std::string FileAttributes;
      Core::DateTime FileCreatedOn;
      Core::DateTime FileLastWrittenOn;
      Core::DateTime FileChangedOn;
      std::string FileId;
      std::string FileParentId;
    };

    struct FileDownloadResult
    {
      std::unique_ptr<Azure::Core::Http::BodyStream> BodyStream;
      Core::DateTime LastModified;
      Storage::Metadata Metadata;
      int64_t ContentLength = int64_t();
      ShareFileHttpHeaders HttpHeaders;
      Azure::Core::Nullable<std::string> ContentRange;
      std::string ETag;
      Azure::Core::Nullable<Storage::ContentHash> TransactionalContentHash;
      std::string AcceptRanges;
      Azure::Core::Nullable<Core::DateTime> CopyCompletedOn;
      Azure::Core::Nullable<std::string> CopyStatusDescription;
      Azure::Core::Nullable<std::string> CopyId;
      Azure::Core::Nullable<std::string> CopyProgress;
      Azure::Core::Nullable<std::string> CopySource;
      Azure::Core::Nullable<CopyStatusType> CopyStatus;
      Azure::Core::Nullable<bool> IsServerEncrypted;
      std::string FileAttributes;
      Core::DateTime FileCreatedOn;
      Core::DateTime FileLastWrittenOn;
      Core::DateTime FileChangedOn;
      std::string FilePermissionKey;
      std::string FileId;
      std::string FileParentId;
      Azure::Core::Nullable<LeaseDurationType> LeaseDuration;
      Azure::Core::Nullable<LeaseStateType> LeaseState;
      Azure::Core::Nullable<LeaseStatusType> LeaseStatus;
    };

    struct FileGetPropertiesResult
    {
      Core::DateTime LastModified;
      Storage::Metadata Metadata;
      std::string FileType;
      int64_t ContentLength = int64_t();
      ShareFileHttpHeaders HttpHeaders;
      std::string ETag;
      Azure::Core::Nullable<Core::DateTime> CopyCompletedOn;
      Azure::Core::Nullable<std::string> CopyStatusDescription;
      Azure::Core::Nullable<std::string> CopyId;
      Azure::Core::Nullable<std::string> CopyProgress;
      Azure::Core::Nullable<std::string> CopySource;
      Azure::Core::Nullable<CopyStatusType> CopyStatus;
      Azure::Core::Nullable<bool> IsServerEncrypted;
      std::string FileAttributes;
      Core::DateTime FileCreatedOn;
      Core::DateTime FileLastWrittenOn;
      Core::DateTime FileChangedOn;
      std::string FilePermissionKey;
      std::string FileId;
      std::string FileParentId;
      Azure::Core::Nullable<LeaseDurationType> LeaseDuration;
      Azure::Core::Nullable<LeaseStateType> LeaseState;
      Azure::Core::Nullable<LeaseStatusType> LeaseStatus;
    };

    struct FileDeleteResult
    {
    };

    struct FileSetHttpHeadersResult
    {
      std::string ETag;
      Core::DateTime LastModified;
      bool IsServerEncrypted = bool();
      std::string FilePermissionKey;
      std::string FileAttributes;
      Core::DateTime FileCreatedOn;
      Core::DateTime FileLastWrittenOn;
      Core::DateTime FileChangedOn;
      std::string FileId;
      std::string FileParentId;
    };

    struct FileSetMetadataResult
    {
      std::string ETag;
      bool IsServerEncrypted = bool();
    };

    struct FileAcquireLeaseResult
    {
      std::string ETag;
      Core::DateTime LastModified;
      std::string LeaseId;
    };

    struct FileReleaseLeaseResult
    {
      std::string ETag;
      Core::DateTime LastModified;
    };

    struct FileChangeLeaseResult
    {
      std::string ETag;
      Core::DateTime LastModified;
      std::string LeaseId;
    };

    struct FileBreakLeaseResult
    {
      std::string ETag;
      Core::DateTime LastModified;
      Azure::Core::Nullable<std::string> LeaseId;
    };

    struct FileUploadRangeResult
    {
      std::string ETag;
      Core::DateTime LastModified;
      Storage::ContentHash TransactionalContentHash;
      bool IsServerEncrypted = bool();
    };

    struct FileUploadRangeFromUrlResult
    {
      std::string ETag;
      Core::DateTime LastModified;
      Storage::ContentHash TransactionalContentHash;
      bool IsServerEncrypted = bool();
    };

    struct FileGetRangeListResult
    {
      std::vector<Models::FileRange> Ranges;
      std::vector<Models::ClearRange> ClearRanges;
      Core::DateTime LastModified;
      std::string ETag;
      int64_t FileContentLength = int64_t();
    };

    struct FileStartCopyResult
    {
      std::string ETag;
      Core::DateTime LastModified;
      std::string CopyId;
      CopyStatusType CopyStatus = CopyStatusType::Unknown;
    };

    struct FileAbortCopyResult
    {
    };

    struct FileListHandlesResult
    {
      std::vector<Models::HandleItem> HandleList;
      std::string ContinuationToken;
      ShareFileHttpHeaders HttpHeaders;
    };

    struct FileForceCloseHandlesResult
    {
      Azure::Core::Nullable<std::string> ContinuationToken;
      int32_t numberOfHandlesClosed = int32_t();
      int32_t numberOfHandlesFailedToClose = int32_t();
    };

    enum class ListSharesIncludeType
    {
      None = 0,
      Snapshots = 1,
      Metadata = 2,
      Deleted = 4,

    };

    inline ListSharesIncludeType operator|(ListSharesIncludeType lhs, ListSharesIncludeType rhs)
    {
      using type = std::underlying_type_t<ListSharesIncludeType>;
      return static_cast<ListSharesIncludeType>(static_cast<type>(lhs) | static_cast<type>(rhs));
    }

    inline ListSharesIncludeType& operator|=(ListSharesIncludeType& lhs, ListSharesIncludeType rhs)
    {
      lhs = lhs | rhs;
      return lhs;
    }

    inline ListSharesIncludeType operator&(ListSharesIncludeType lhs, ListSharesIncludeType rhs)
    {
      using type = std::underlying_type_t<ListSharesIncludeType>;
      return static_cast<ListSharesIncludeType>(static_cast<type>(lhs) & static_cast<type>(rhs));
    }

    inline ListSharesIncludeType& operator&=(ListSharesIncludeType& lhs, ListSharesIncludeType rhs)
    {
      lhs = lhs & rhs;
      return lhs;
    }
    inline std::string ListSharesIncludeTypeToString(const Models::ListSharesIncludeType& val)
    {
      Models::ListSharesIncludeType value_list[] = {
          ListSharesIncludeType::Snapshots,
          ListSharesIncludeType::Metadata,
          ListSharesIncludeType::Deleted,
      };
      const char* string_list[] = {
          "snapshots",
          "metadata",
          "deleted",
      };
      std::string result;
      for (std::size_t i = 0; i < sizeof(value_list) / sizeof(Models::ListSharesIncludeType); ++i)
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
  namespace Details {
    inline std::string PermissionCopyModeTypeToString(
        const Models::PermissionCopyModeType& permissionCopyModeType)
    {
      switch (permissionCopyModeType)
      {
        case Models::PermissionCopyModeType::Source:
          return "source";
        case Models::PermissionCopyModeType::Override:
          return "override";
        default:
          return std::string();
      }
    }

    inline Models::PermissionCopyModeType PermissionCopyModeTypeFromString(
        const std::string& permissionCopyModeType)
    {
      if (permissionCopyModeType == "source")
      {
        return Models::PermissionCopyModeType::Source;
      }
      if (permissionCopyModeType == "override")
      {
        return Models::PermissionCopyModeType::Override;
      }
      throw std::runtime_error(
          "Cannot convert " + permissionCopyModeType + " to PermissionCopyModeType");
    }

    inline std::string DeleteSnapshotsOptionTypeToString(
        const Models::DeleteSnapshotsOptionType& deleteSnapshotsOptionType)
    {
      switch (deleteSnapshotsOptionType)
      {
        case Models::DeleteSnapshotsOptionType::Include:
          return "include";
        default:
          return std::string();
      }
    }

    inline Models::DeleteSnapshotsOptionType DeleteSnapshotsOptionTypeFromString(
        const std::string& deleteSnapshotsOptionType)
    {
      if (deleteSnapshotsOptionType == "include")
      {
        return Models::DeleteSnapshotsOptionType::Include;
      }
      throw std::runtime_error(
          "Cannot convert " + deleteSnapshotsOptionType + " to DeleteSnapshotsOptionType");
    }

    inline std::string FileRangeWriteFromUrlTypeToString(
        const Models::FileRangeWriteFromUrlType& fileRangeWriteFromUrlType)
    {
      switch (fileRangeWriteFromUrlType)
      {
        case Models::FileRangeWriteFromUrlType::Update:
          return "update";
        default:
          return std::string();
      }
    }

    inline Models::FileRangeWriteFromUrlType FileRangeWriteFromUrlTypeFromString(
        const std::string& fileRangeWriteFromUrlType)
    {
      if (fileRangeWriteFromUrlType == "update")
      {
        return Models::FileRangeWriteFromUrlType::Update;
      }
      throw std::runtime_error(
          "Cannot convert " + fileRangeWriteFromUrlType + " to FileRangeWriteFromUrlType");
    }

    inline std::string LeaseDurationTypeToString(const Models::LeaseDurationType& leaseDurationType)
    {
      switch (leaseDurationType)
      {
        case Models::LeaseDurationType::Infinite:
          return "infinite";
        case Models::LeaseDurationType::Fixed:
          return "fixed";
        default:
          return std::string();
      }
    }

    inline Models::LeaseDurationType LeaseDurationTypeFromString(
        const std::string& leaseDurationType)
    {
      if (leaseDurationType == "infinite")
      {
        return Models::LeaseDurationType::Infinite;
      }
      if (leaseDurationType == "fixed")
      {
        return Models::LeaseDurationType::Fixed;
      }
      throw std::runtime_error("Cannot convert " + leaseDurationType + " to LeaseDurationType");
    }

    inline std::string LeaseStateTypeToString(const Models::LeaseStateType& leaseStateType)
    {
      switch (leaseStateType)
      {
        case Models::LeaseStateType::Available:
          return "available";
        case Models::LeaseStateType::Leased:
          return "leased";
        case Models::LeaseStateType::Expired:
          return "expired";
        case Models::LeaseStateType::Breaking:
          return "breaking";
        case Models::LeaseStateType::Broken:
          return "broken";
        default:
          return std::string();
      }
    }

    inline Models::LeaseStateType LeaseStateTypeFromString(const std::string& leaseStateType)
    {
      if (leaseStateType == "available")
      {
        return Models::LeaseStateType::Available;
      }
      if (leaseStateType == "leased")
      {
        return Models::LeaseStateType::Leased;
      }
      if (leaseStateType == "expired")
      {
        return Models::LeaseStateType::Expired;
      }
      if (leaseStateType == "breaking")
      {
        return Models::LeaseStateType::Breaking;
      }
      if (leaseStateType == "broken")
      {
        return Models::LeaseStateType::Broken;
      }
      throw std::runtime_error("Cannot convert " + leaseStateType + " to LeaseStateType");
    }

    inline std::string LeaseStatusTypeToString(const Models::LeaseStatusType& leaseStatusType)
    {
      switch (leaseStatusType)
      {
        case Models::LeaseStatusType::Locked:
          return "locked";
        case Models::LeaseStatusType::Unlocked:
          return "unlocked";
        default:
          return std::string();
      }
    }

    inline Models::LeaseStatusType LeaseStatusTypeFromString(const std::string& leaseStatusType)
    {
      if (leaseStatusType == "locked")
      {
        return Models::LeaseStatusType::Locked;
      }
      if (leaseStatusType == "unlocked")
      {
        return Models::LeaseStatusType::Unlocked;
      }
      throw std::runtime_error("Cannot convert " + leaseStatusType + " to LeaseStatusType");
    }

    inline std::string LeaseActionToString(const Models::LeaseAction& leaseAction)
    {
      switch (leaseAction)
      {
        case Models::LeaseAction::Acquire:
          return "acquire";
        case Models::LeaseAction::Release:
          return "release";
        case Models::LeaseAction::Change:
          return "change";
        case Models::LeaseAction::Renew:
          return "renew";
        case Models::LeaseAction::Break:
          return "break";
        default:
          return std::string();
      }
    }

    inline Models::LeaseAction LeaseActionFromString(const std::string& leaseAction)
    {
      if (leaseAction == "acquire")
      {
        return Models::LeaseAction::Acquire;
      }
      if (leaseAction == "release")
      {
        return Models::LeaseAction::Release;
      }
      if (leaseAction == "change")
      {
        return Models::LeaseAction::Change;
      }
      if (leaseAction == "renew")
      {
        return Models::LeaseAction::Renew;
      }
      if (leaseAction == "break")
      {
        return Models::LeaseAction::Break;
      }
      throw std::runtime_error("Cannot convert " + leaseAction + " to LeaseAction");
    }

    inline std::string CopyStatusTypeToString(const Models::CopyStatusType& copyStatusType)
    {
      switch (copyStatusType)
      {
        case Models::CopyStatusType::Pending:
          return "pending";
        case Models::CopyStatusType::Success:
          return "success";
        case Models::CopyStatusType::Aborted:
          return "aborted";
        case Models::CopyStatusType::Failed:
          return "failed";
        default:
          return std::string();
      }
    }

    inline Models::CopyStatusType CopyStatusTypeFromString(const std::string& copyStatusType)
    {
      if (copyStatusType == "pending")
      {
        return Models::CopyStatusType::Pending;
      }
      if (copyStatusType == "success")
      {
        return Models::CopyStatusType::Success;
      }
      if (copyStatusType == "aborted")
      {
        return Models::CopyStatusType::Aborted;
      }
      if (copyStatusType == "failed")
      {
        return Models::CopyStatusType::Failed;
      }
      throw std::runtime_error("Cannot convert " + copyStatusType + " to CopyStatusType");
    }

    inline std::string FileRangeWriteTypeToString(
        const Models::FileRangeWriteType& fileRangeWriteType)
    {
      switch (fileRangeWriteType)
      {
        case Models::FileRangeWriteType::Update:
          return "update";
        case Models::FileRangeWriteType::Clear:
          return "clear";
        default:
          return std::string();
      }
    }

    inline Models::FileRangeWriteType FileRangeWriteTypeFromString(
        const std::string& fileRangeWriteType)
    {
      if (fileRangeWriteType == "update")
      {
        return Models::FileRangeWriteType::Update;
      }
      if (fileRangeWriteType == "clear")
      {
        return Models::FileRangeWriteType::Clear;
      }
      throw std::runtime_error("Cannot convert " + fileRangeWriteType + " to FileRangeWriteType");
    }

    class ShareRestClient {
    public:
      class Service {
      public:
        struct SetPropertiesOptions
        {
          Models::StorageServiceProperties ServiceProperties; // The StorageService properties.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        };

        static Azure::Core::Response<Models::ServiceSetPropertiesResult> SetProperties(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetPropertiesOptions& setPropertiesOptions)
        {

          std::string xml_body;
          {
            Storage::Details::XmlWriter writer;
            StorageServicePropertiesToXml(writer, setPropertiesOptions.ServiceProperties);
            writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::End});
            xml_body = writer.GetDocument();
          }
          auto body = Azure::Core::Http::MemoryBodyStream(
              reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, &body);
          request.AddHeader("Content-Length", std::to_string(body.Length()));
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "service");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "properties");
          if (setPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(setPropertiesOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, setPropertiesOptions.ApiVersionParameter);
          return SetPropertiesParseResult(context, pipeline.Send(context, request));
        }

        struct GetPropertiesOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        };

        static Azure::Core::Response<Models::ServiceGetPropertiesResult> GetProperties(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetPropertiesOptions& getPropertiesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "service");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "properties");
          if (getPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(getPropertiesOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, getPropertiesOptions.ApiVersionParameter);
          return GetPropertiesParseResult(context, pipeline.Send(context, request));
        }

        struct ListSharesSegmentOptions
        {
          Azure::Core::Nullable<std::string> Prefix; // Filters the results to return only entries
                                                     // whose name begins with the specified prefix.
          Azure::Core::Nullable<std::string>
              ContinuationToken; // A string value that identifies the portion of the list to be
                                 // returned with the next list operation. The operation returns a
                                 // marker value within the response body if the list returned was
                                 // not complete. The marker value may then be used in a subsequent
                                 // call to request the next set of list items. The marker value is
                                 // opaque to the client.
          Azure::Core::Nullable<int32_t>
              MaxResults; // Specifies the maximum number of entries to return. If the request does
                          // not specify maxresults, or specifies a value greater than 5,000, the
                          // server will return up to 5,000 items.
          Azure::Core::Nullable<Models::ListSharesIncludeType>
              ListSharesInclude; // Include this parameter to specify one or more datasets to
                                 // include in the response.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        };

        static Azure::Core::Response<Models::ServiceListSharesSegmentResult> ListSharesSegment(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ListSharesSegmentOptions& listSharesSegmentOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "list");
          if (listSharesSegmentOptions.Prefix.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryPrefix,
                Storage::Details::UrlEncodeQueryParameter(
                    listSharesSegmentOptions.Prefix.GetValue()));
          }
          if (listSharesSegmentOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryContinuationToken,
                Storage::Details::UrlEncodeQueryParameter(
                    listSharesSegmentOptions.ContinuationToken.GetValue()));
          }
          if (listSharesSegmentOptions.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryMaxResults,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listSharesSegmentOptions.MaxResults.GetValue())));
          }
          if (listSharesSegmentOptions.ListSharesInclude.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryListSharesInclude,
                Storage::Details::UrlEncodeQueryParameter(Models::ListSharesIncludeTypeToString(
                    listSharesSegmentOptions.ListSharesInclude.GetValue())));
          }
          if (listSharesSegmentOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listSharesSegmentOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, listSharesSegmentOptions.ApiVersionParameter);
          return ListSharesSegmentParseResult(context, pipeline.Send(context, request));
        }

      private:
        static Azure::Core::Response<Models::ServiceSetPropertiesResult> SetPropertiesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
          {
            // Success (Accepted)
            Models::ServiceSetPropertiesResult result;
            return Azure::Core::Response<Models::ServiceSetPropertiesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static void ShareRetentionPolicyToXml(
            Storage::Details::XmlWriter& writer,
            const Models::ShareRetentionPolicy& object)
        {
          writer.Write(
              Storage::Details::XmlNode{Storage::Details::XmlNodeType::StartTag, "Enabled"});
          writer.Write(Storage::Details::XmlNode{
              Storage::Details::XmlNodeType::Text, nullptr, object.Enabled ? "true" : "false"});
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
          if (object.Days.HasValue())
          {
            writer.Write(
                Storage::Details::XmlNode{Storage::Details::XmlNodeType::StartTag, "Days"});
            writer.Write(Storage::Details::XmlNode{
                Storage::Details::XmlNodeType::Text,
                nullptr,
                std::to_string(object.Days.GetValue()).data()});
            writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
          }
        }

        static void MetricsToXml(Storage::Details::XmlWriter& writer, const Models::Metrics& object)
        {
          writer.Write(
              Storage::Details::XmlNode{Storage::Details::XmlNodeType::StartTag, "Version"});
          writer.Write(Storage::Details::XmlNode{
              Storage::Details::XmlNodeType::Text, nullptr, object.Version.data()});
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
          writer.Write(
              Storage::Details::XmlNode{Storage::Details::XmlNodeType::StartTag, "Enabled"});
          writer.Write(Storage::Details::XmlNode{
              Storage::Details::XmlNodeType::Text, nullptr, object.Enabled ? "true" : "false"});
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
          if (object.IncludeApis.HasValue())
          {
            writer.Write(
                Storage::Details::XmlNode{Storage::Details::XmlNodeType::StartTag, "IncludeAPIs"});
            writer.Write(Storage::Details::XmlNode{
                Storage::Details::XmlNodeType::Text,
                nullptr,
                object.IncludeApis.GetValue() ? "true" : "false"});
            writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
          }
          writer.Write(Storage::Details::XmlNode{
              Storage::Details::XmlNodeType::StartTag, "RetentionPolicy"});
          ShareRetentionPolicyToXml(writer, object.RetentionPolicy);
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
        }

        static void CorsRuleToXml(
            Storage::Details::XmlWriter& writer,
            const Models::CorsRule& object)
        {
          writer.Write(
              Storage::Details::XmlNode{Storage::Details::XmlNodeType::StartTag, "CorsRule"});
          writer.Write(
              Storage::Details::XmlNode{Storage::Details::XmlNodeType::StartTag, "AllowedOrigins"});
          writer.Write(Storage::Details::XmlNode{
              Storage::Details::XmlNodeType::Text, nullptr, object.AllowedOrigins.data()});
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
          writer.Write(
              Storage::Details::XmlNode{Storage::Details::XmlNodeType::StartTag, "AllowedMethods"});
          writer.Write(Storage::Details::XmlNode{
              Storage::Details::XmlNodeType::Text, nullptr, object.AllowedMethods.data()});
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
          writer.Write(
              Storage::Details::XmlNode{Storage::Details::XmlNodeType::StartTag, "AllowedHeaders"});
          writer.Write(Storage::Details::XmlNode{
              Storage::Details::XmlNodeType::Text, nullptr, object.AllowedHeaders.data()});
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
          writer.Write(
              Storage::Details::XmlNode{Storage::Details::XmlNodeType::StartTag, "ExposedHeaders"});
          writer.Write(Storage::Details::XmlNode{
              Storage::Details::XmlNodeType::Text, nullptr, object.ExposedHeaders.data()});
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
          writer.Write(Storage::Details::XmlNode{
              Storage::Details::XmlNodeType::StartTag, "MaxAgeInSeconds"});
          writer.Write(Storage::Details::XmlNode{
              Storage::Details::XmlNodeType::Text,
              nullptr,
              std::to_string(object.MaxAgeInSeconds).data()});
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
        }

        static void SmbMultichannelToXml(
            Storage::Details::XmlWriter& writer,
            const Models::SmbMultichannel& object)
        {
          writer.Write(
              Storage::Details::XmlNode{Storage::Details::XmlNodeType::StartTag, "Multichannel"});
          writer.Write(
              Storage::Details::XmlNode{Storage::Details::XmlNodeType::StartTag, "Enabled"});
          writer.Write(Storage::Details::XmlNode{
              Storage::Details::XmlNodeType::Text, nullptr, object.Enabled ? "true" : "false"});
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
        }

        static void SmbSettingsToXml(
            Storage::Details::XmlWriter& writer,
            const Models::SmbSettings& object)
        {
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::StartTag, "SMB"});
          SmbMultichannelToXml(writer, object.Multichannel);
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
        }

        static void ShareProtocolSettingsToXml(
            Storage::Details::XmlWriter& writer,
            const Models::ShareProtocolSettings& object)
        {
          writer.Write(Storage::Details::XmlNode{
              Storage::Details::XmlNodeType::StartTag, "ProtocolSettings"});
          SmbSettingsToXml(writer, object.Settings);
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
        }

        static void StorageServicePropertiesToXml(
            Storage::Details::XmlWriter& writer,
            const Models::StorageServiceProperties& object)
        {
          writer.Write(Storage::Details::XmlNode{
              Storage::Details::XmlNodeType::StartTag, "StorageServiceProperties"});
          writer.Write(
              Storage::Details::XmlNode{Storage::Details::XmlNodeType::StartTag, "HourMetrics"});
          MetricsToXml(writer, object.HourMetrics);
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
          writer.Write(
              Storage::Details::XmlNode{Storage::Details::XmlNodeType::StartTag, "MinuteMetrics"});
          MetricsToXml(writer, object.MinuteMetrics);
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
          if (object.Cors.size() > 0)
          {
            writer.Write(
                Storage::Details::XmlNode{Storage::Details::XmlNodeType::StartTag, "Cors"});
            for (const auto& item : object.Cors)
            {
              CorsRuleToXml(writer, item);
            }
            writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
          }
          if (object.Protocol.HasValue())
          {
            ShareProtocolSettingsToXml(writer, object.Protocol.GetValue());
          }
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
        }
        static Azure::Core::Response<Models::ServiceGetPropertiesResult> GetPropertiesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success.
            const auto& bodyBuffer = response.GetBody();
            auto reader = Storage::Details::XmlReader(
                reinterpret_cast<const char*>(bodyBuffer.data()), bodyBuffer.size());
            Models::ServiceGetPropertiesResult result = bodyBuffer.empty()
                ? Models::ServiceGetPropertiesResult()
                : ServiceGetPropertiesResultFromStorageServiceProperties(
                    StorageServicePropertiesFromXml(reader));
            return Azure::Core::Response<Models::ServiceGetPropertiesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Models::ShareRetentionPolicy ShareRetentionPolicyFromXml(
            Storage::Details::XmlReader& reader)
        {
          auto result = Models::ShareRetentionPolicy();
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
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::Text)
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

        static Models::Metrics MetricsFromXml(Storage::Details::XmlReader& reader)
        {
          auto result = Models::Metrics();
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
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
                result.RetentionPolicy = ShareRetentionPolicyFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
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

        static Models::CorsRule CorsRuleFromXml(Storage::Details::XmlReader& reader)
        {
          auto result = Models::CorsRule();
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
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::Text)
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

        static Models::SmbMultichannel SmbMultichannelFromXml(Storage::Details::XmlReader& reader)
        {
          auto result = Models::SmbMultichannel();
          enum class XmlTagName
          {
            Enabled,
            Unknown,
          };
          std::vector<XmlTagName> path;

          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::Enabled)
              {
                result.Enabled = (std::strcmp(node.Value, "true") == 0);
              }
            }
          }
          return result;
        }

        static Models::SmbSettings SmbSettingsFromXml(Storage::Details::XmlReader& reader)
        {
          auto result = Models::SmbSettings();
          enum class XmlTagName
          {
            Multichannel,
            Unknown,
          };
          std::vector<XmlTagName> path;

          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
            }
          }
          return result;
        }

        static Models::ShareProtocolSettings ShareProtocolSettingsFromXml(
            Storage::Details::XmlReader& reader)
        {
          auto result = Models::ShareProtocolSettings();
          enum class XmlTagName
          {
            SMB,
            Unknown,
          };
          std::vector<XmlTagName> path;

          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
            }
          }
          return result;
        }

        static Models::StorageServiceProperties StorageServicePropertiesFromXml(
            Storage::Details::XmlReader& reader)
        {
          auto result = Models::StorageServiceProperties();
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
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
                result.Protocol = ShareProtocolSettingsFromXml(reader);
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
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
            }
          }
          return result;
        }

        static Models::ServiceGetPropertiesResult
        ServiceGetPropertiesResultFromStorageServiceProperties(
            Models::StorageServiceProperties object)
        {
          Models::ServiceGetPropertiesResult result;
          result.HourMetrics = std::move(object.HourMetrics);
          result.MinuteMetrics = std::move(object.MinuteMetrics);
          result.Cors = std::move(object.Cors);
          result.Protocol = std::move(object.Protocol);

          return result;
        }
        static Azure::Core::Response<Models::ServiceListSharesSegmentResult>
        ListSharesSegmentParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success.
            const auto& bodyBuffer = response.GetBody();
            auto reader = Storage::Details::XmlReader(
                reinterpret_cast<const char*>(bodyBuffer.data()), bodyBuffer.size());
            Models::ServiceListSharesSegmentResult result = bodyBuffer.empty()
                ? Models::ServiceListSharesSegmentResult()
                : ServiceListSharesSegmentResultFromListSharesResponse(
                    ListSharesResponseFromXml(reader));
            return Azure::Core::Response<Models::ServiceListSharesSegmentResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Models::LeaseStatusType LeaseStatusTypeFromXml(Storage::Details::XmlReader& reader)
        {
          auto result = Models::LeaseStatusType::Unknown;
          enum class XmlTagName
          {
            LeaseStatus,
            Unknown,
          };
          std::vector<XmlTagName> path;

          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::LeaseStatus)
              {
                result = LeaseStatusTypeFromString(node.Value);
              }
            }
          }
          return result;
        }

        static Models::LeaseStateType LeaseStateTypeFromXml(Storage::Details::XmlReader& reader)
        {
          auto result = Models::LeaseStateType::Unknown;
          enum class XmlTagName
          {
            LeaseState,
            Unknown,
          };
          std::vector<XmlTagName> path;

          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::LeaseState)
              {
                result = LeaseStateTypeFromString(node.Value);
              }
            }
          }
          return result;
        }

        static Models::LeaseDurationType LeaseDurationTypeFromXml(
            Storage::Details::XmlReader& reader)
        {
          auto result = Models::LeaseDurationType::Unknown;
          enum class XmlTagName
          {
            LeaseDuration,
            Unknown,
          };
          std::vector<XmlTagName> path;

          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::LeaseDuration)
              {
                result = LeaseDurationTypeFromString(node.Value);
              }
            }
          }
          return result;
        }

        static Models::ShareProperties SharePropertiesFromXml(Storage::Details::XmlReader& reader)
        {
          auto result = Models::ShareProperties();
          enum class XmlTagName
          {
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
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
            {

              if (std::strcmp(node.Name, "DeletedTime") == 0)
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
                result.LeaseStatus = LeaseStatusTypeFromXml(reader);
                path.pop_back();
              }
              else if (path.size() == 1 && path[0] == XmlTagName::LeaseState)
              {
                result.LeaseState = LeaseStateTypeFromXml(reader);
                path.pop_back();
              }
              else if (path.size() == 1 && path[0] == XmlTagName::LeaseDuration)
              {
                result.LeaseDuration = LeaseDurationTypeFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::DeletedTime)
              {
                result.DeletedOn
                    = Core::DateTime::Parse(node.Value, Core::DateTime::DateFormat::Rfc1123);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::Etag)
              {
                result.Etag = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::LastModified)
              {
                result.LastModified
                    = Core::DateTime::Parse(node.Value, Core::DateTime::DateFormat::Rfc1123);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::NextAllowedQuotaDowngradeTime)
              {
                result.NextAllowedQuotaDowngradeTime
                    = Core::DateTime::Parse(node.Value, Core::DateTime::DateFormat::Rfc1123);
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

        static Metadata MetadataFromXml(Storage::Details::XmlReader& reader)
        {
          Metadata result;
          int depth = 0;
          std::string key;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
            {
              if (depth++ == 0)
              {
                key = node.Name;
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
            {
              if (depth-- == 0)
              {
                break;
              }
            }
            else if (depth == 1 && node.Type == Storage::Details::XmlNodeType::Text)
            {
              result.emplace(std::move(key), std::string(node.Value));
            }
          }
          return result;
        }

        static Models::ShareItem ShareItemFromXml(Storage::Details::XmlReader& reader)
        {
          auto result = Models::ShareItem();
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
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
                result.Properties = SharePropertiesFromXml(reader);
                path.pop_back();
              }
              else if (path.size() == 1 && path[0] == XmlTagName::Metadata)
              {
                result.ShareMetadata = MetadataFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
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

        static Models::ListSharesResponse ListSharesResponseFromXml(
            Storage::Details::XmlReader& reader)
        {
          auto result = Models::ListSharesResponse();
          enum class XmlTagName
          {
            EnumerationResults,
            Marker,
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
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
            {

              if (std::strcmp(node.Name, "EnumerationResults") == 0)
              {
                path.emplace_back(XmlTagName::EnumerationResults);
              }
              else if (std::strcmp(node.Name, "Marker") == 0)
              {
                path.emplace_back(XmlTagName::Marker);
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
                result.ShareItems.emplace_back(ShareItemFromXml(reader));
                path.pop_back();
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
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
                result.MaxResults = std::stoi(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::EnumerationResults
                  && path[1] == XmlTagName::Prefix)
              {
                result.Prefix = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::EnumerationResults
                  && path[1] == XmlTagName::Marker)
              {
                result.PreviousContinuationToken = node.Value;
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Attribute)
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

        static Models::ServiceListSharesSegmentResult
        ServiceListSharesSegmentResultFromListSharesResponse(Models::ListSharesResponse object)
        {
          Models::ServiceListSharesSegmentResult result;
          result.ServiceEndpoint = std::move(object.ServiceEndpoint);
          result.Prefix = std::move(object.Prefix);
          result.PreviousContinuationToken = std::move(object.PreviousContinuationToken);
          result.MaxResults = object.MaxResults;
          result.ShareItems = std::move(object.ShareItems);
          result.ContinuationToken = std::move(object.ContinuationToken);

          return result;
        }
      };

      class Share {
      public:
        struct CreateOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          Storage::Metadata Metadata; // A name-value pair to associate with a file storage object.
          Azure::Core::Nullable<int64_t>
              ShareQuota; // Specifies the maximum size of the share, in gigabytes.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        };

        static Azure::Core::Response<Models::ShareCreateResult> Create(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const CreateOptions& createOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "share");
          if (createOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(createOptions.Timeout.GetValue())));
          }
          for (const auto& pair : createOptions.Metadata)
          {
            request.AddHeader(Details::HeaderMetadata + ("-" + pair.first), pair.second);
          }
          if (createOptions.ShareQuota.HasValue())
          {
            request.AddHeader(
                Details::HeaderQuota, std::to_string(createOptions.ShareQuota.GetValue()));
          }
          request.AddHeader(Details::HeaderVersion, createOptions.ApiVersionParameter);
          return CreateParseResult(context, pipeline.Send(context, request));
        }

        struct GetPropertiesOptions
        {
          Azure::Core::Nullable<std::string>
              ShareSnapshot; // The snapshot parameter is an opaque DateTime value that, when
                             // present, specifies the share snapshot to query.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
        };

        static Azure::Core::Response<Models::ShareGetPropertiesResult> GetProperties(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetPropertiesOptions& getPropertiesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "share");
          if (getPropertiesOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    getPropertiesOptions.ShareSnapshot.GetValue()));
          }
          if (getPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(getPropertiesOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, getPropertiesOptions.ApiVersionParameter);
          if (getPropertiesOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::HeaderLeaseId, getPropertiesOptions.LeaseIdOptional.GetValue());
          }
          return GetPropertiesParseResult(context, pipeline.Send(context, request));
        }

        struct DeleteOptions
        {
          Azure::Core::Nullable<std::string>
              ShareSnapshot; // The snapshot parameter is an opaque DateTime value that, when
                             // present, specifies the share snapshot to query.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<Models::DeleteSnapshotsOptionType>
              XMsDeleteSnapshots; // Specifies the option include to delete the base share and all
                                  // of its snapshots.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
        };

        static Azure::Core::Response<Models::ShareDeleteResult> Delete(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const DeleteOptions& deleteOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Delete, url);
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "share");
          if (deleteOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(deleteOptions.ShareSnapshot.GetValue()));
          }
          if (deleteOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(deleteOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, deleteOptions.ApiVersionParameter);
          if (deleteOptions.XMsDeleteSnapshots.HasValue())
          {
            request.AddHeader(
                Details::HeaderDeleteSnapshots,
                DeleteSnapshotsOptionTypeToString(deleteOptions.XMsDeleteSnapshots.GetValue()));
          }
          if (deleteOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(Details::HeaderLeaseId, deleteOptions.LeaseIdOptional.GetValue());
          }
          return DeleteParseResult(context, pipeline.Send(context, request));
        }

        struct AcquireLeaseOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          int32_t LeaseDuration
              = int32_t(); // Specifies the duration of the lease, in seconds, or negative one (-1)
                           // for a lease that never expires. A non-infinite lease can be between 15
                           // and 60 seconds. A lease duration cannot be changed using renew or
                           // change.
          Azure::Core::Nullable<std::string>
              ProposedLeaseIdOptional; // Proposed lease ID, in a GUID string format. The File
                                       // service returns 400 (Invalid request) if the proposed
                                       // lease ID is not in the correct format. See Guid
                                       // Constructor (String) for a list of valid GUID string
                                       // formats.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              ShareSnapshot; // The snapshot parameter is an opaque DateTime value that, when
                             // present, specifies the share snapshot to query.
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
        };

        static Azure::Core::Response<Models::ShareAcquireLeaseResult> AcquireLease(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const AcquireLeaseOptions& acquireLeaseOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "lease");
          request.AddHeader(Details::HeaderAction, "acquire");
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "share");
          if (acquireLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(acquireLeaseOptions.Timeout.GetValue())));
          }
          request.AddHeader(
              Details::HeaderDuration, std::to_string(acquireLeaseOptions.LeaseDuration));
          if (acquireLeaseOptions.ProposedLeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::HeaderProposedLeaseId,
                acquireLeaseOptions.ProposedLeaseIdOptional.GetValue());
          }
          request.AddHeader(Details::HeaderVersion, acquireLeaseOptions.ApiVersionParameter);
          if (acquireLeaseOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    acquireLeaseOptions.ShareSnapshot.GetValue()));
          }
          if (acquireLeaseOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderRequestId, acquireLeaseOptions.ClientRequestId.GetValue());
          }
          return AcquireLeaseParseResult(context, pipeline.Send(context, request));
        }

        struct ReleaseLeaseOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string LeaseIdRequired; // Specifies the current lease ID on the resource.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              ShareSnapshot; // The snapshot parameter is an opaque DateTime value that, when
                             // present, specifies the share snapshot to query.
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
        };

        static Azure::Core::Response<Models::ShareReleaseLeaseResult> ReleaseLease(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ReleaseLeaseOptions& releaseLeaseOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "lease");
          request.AddHeader(Details::HeaderAction, "release");
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "share");
          if (releaseLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(releaseLeaseOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderLeaseId, releaseLeaseOptions.LeaseIdRequired);
          request.AddHeader(Details::HeaderVersion, releaseLeaseOptions.ApiVersionParameter);
          if (releaseLeaseOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    releaseLeaseOptions.ShareSnapshot.GetValue()));
          }
          if (releaseLeaseOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderRequestId, releaseLeaseOptions.ClientRequestId.GetValue());
          }
          return ReleaseLeaseParseResult(context, pipeline.Send(context, request));
        }

        struct ChangeLeaseOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string LeaseIdRequired; // Specifies the current lease ID on the resource.
          Azure::Core::Nullable<std::string>
              ProposedLeaseIdOptional; // Proposed lease ID, in a GUID string format. The File
                                       // service returns 400 (Invalid request) if the proposed
                                       // lease ID is not in the correct format. See Guid
                                       // Constructor (String) for a list of valid GUID string
                                       // formats.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              ShareSnapshot; // The snapshot parameter is an opaque DateTime value that, when
                             // present, specifies the share snapshot to query.
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
        };

        static Azure::Core::Response<Models::ShareChangeLeaseResult> ChangeLease(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ChangeLeaseOptions& changeLeaseOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "lease");
          request.AddHeader(Details::HeaderAction, "change");
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "share");
          if (changeLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(changeLeaseOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderLeaseId, changeLeaseOptions.LeaseIdRequired);
          if (changeLeaseOptions.ProposedLeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::HeaderProposedLeaseId,
                changeLeaseOptions.ProposedLeaseIdOptional.GetValue());
          }
          request.AddHeader(Details::HeaderVersion, changeLeaseOptions.ApiVersionParameter);
          if (changeLeaseOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    changeLeaseOptions.ShareSnapshot.GetValue()));
          }
          if (changeLeaseOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderRequestId, changeLeaseOptions.ClientRequestId.GetValue());
          }
          return ChangeLeaseParseResult(context, pipeline.Send(context, request));
        }

        struct RenewLeaseOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string LeaseIdRequired; // Specifies the current lease ID on the resource.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              ShareSnapshot; // The snapshot parameter is an opaque DateTime value that, when
                             // present, specifies the share snapshot to query.
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
        };

        static Azure::Core::Response<Models::ShareRenewLeaseResult> RenewLease(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const RenewLeaseOptions& renewLeaseOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "lease");
          request.AddHeader(Details::HeaderAction, "renew");
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "share");
          if (renewLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(renewLeaseOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderLeaseId, renewLeaseOptions.LeaseIdRequired);
          request.AddHeader(Details::HeaderVersion, renewLeaseOptions.ApiVersionParameter);
          if (renewLeaseOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    renewLeaseOptions.ShareSnapshot.GetValue()));
          }
          if (renewLeaseOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderRequestId, renewLeaseOptions.ClientRequestId.GetValue());
          }
          return RenewLeaseParseResult(context, pipeline.Send(context, request));
        }

        struct BreakLeaseOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          Azure::Core::Nullable<int32_t>
              LeaseBreakPeriod; // For a break operation, proposed duration the lease should
                                // continue before it is broken, in seconds, between 0 and 60. This
                                // break period is only used if it is shorter than the time
                                // remaining on the lease. If longer, the time remaining on the
                                // lease is used. A new lease will not be available before the break
                                // period has expired, but the lease may be held for longer than the
                                // break period. If this header does not appear with a break
                                // operation, a fixed-duration lease breaks after the remaining
                                // lease period elapses, and an infinite lease breaks immediately.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
          Azure::Core::Nullable<std::string>
              ShareSnapshot; // The snapshot parameter is an opaque DateTime value that, when
                             // present, specifies the share snapshot to query.
        };

        static Azure::Core::Response<Models::ShareBreakLeaseResult> BreakLease(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const BreakLeaseOptions& breakLeaseOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "lease");
          request.AddHeader(Details::HeaderAction, "break");
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "share");
          if (breakLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(breakLeaseOptions.Timeout.GetValue())));
          }
          if (breakLeaseOptions.LeaseBreakPeriod.HasValue())
          {
            request.AddHeader(
                Details::HeaderBreakPeriod,
                std::to_string(breakLeaseOptions.LeaseBreakPeriod.GetValue()));
          }
          if (breakLeaseOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(Details::HeaderLeaseId, breakLeaseOptions.LeaseIdOptional.GetValue());
          }
          request.AddHeader(Details::HeaderVersion, breakLeaseOptions.ApiVersionParameter);
          if (breakLeaseOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderRequestId, breakLeaseOptions.ClientRequestId.GetValue());
          }
          if (breakLeaseOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    breakLeaseOptions.ShareSnapshot.GetValue()));
          }
          return BreakLeaseParseResult(context, pipeline.Send(context, request));
        }

        struct CreateSnapshotOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          Storage::Metadata Metadata; // A name-value pair to associate with a file storage object.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        };

        static Azure::Core::Response<Models::ShareCreateSnapshotResult> CreateSnapshot(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const CreateSnapshotOptions& createSnapshotOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "snapshot");
          if (createSnapshotOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(createSnapshotOptions.Timeout.GetValue())));
          }
          for (const auto& pair : createSnapshotOptions.Metadata)
          {
            request.AddHeader(Details::HeaderMetadata + ("-" + pair.first), pair.second);
          }
          request.AddHeader(Details::HeaderVersion, createSnapshotOptions.ApiVersionParameter);
          return CreateSnapshotParseResult(context, pipeline.Send(context, request));
        }

        struct CreatePermissionOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Models::SharePermission
              Permission; // A permission (a security descriptor) at the share level.
        };

        static Azure::Core::Response<Models::ShareCreatePermissionResult> CreatePermission(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const CreatePermissionOptions& createPermissionOptions)
        {

          std::string json_body;
          {
            nlohmann::json json;
            SharePermissionToJson(json, createPermissionOptions.Permission);
            json_body = json.dump();
          }
          auto body = Azure::Core::Http::MemoryBodyStream(
              reinterpret_cast<const uint8_t*>(json_body.data()), json_body.length());
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, &body);
          request.AddHeader("Content-Length", std::to_string(body.Length()));
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "filepermission");
          if (createPermissionOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(createPermissionOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, createPermissionOptions.ApiVersionParameter);
          return CreatePermissionParseResult(context, pipeline.Send(context, request));
        }

        struct GetPermissionOptions
        {
          std::string
              FilePermissionKeyRequired; // Key of the permission to be set for the directory/file.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        };

        static Azure::Core::Response<Models::ShareGetPermissionResult> GetPermission(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetPermissionOptions& getPermissionOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "filepermission");
          request.AddHeader(
              Details::HeaderFilePermissionKey, getPermissionOptions.FilePermissionKeyRequired);
          if (getPermissionOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(getPermissionOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, getPermissionOptions.ApiVersionParameter);
          return GetPermissionParseResult(context, pipeline.Send(context, request));
        }

        struct SetQuotaOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<int64_t>
              ShareQuota; // Specifies the maximum size of the share, in gigabytes.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
        };

        static Azure::Core::Response<Models::ShareSetQuotaResult> SetQuota(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetQuotaOptions& setQuotaOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "properties");
          if (setQuotaOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(setQuotaOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, setQuotaOptions.ApiVersionParameter);
          if (setQuotaOptions.ShareQuota.HasValue())
          {
            request.AddHeader(
                Details::HeaderQuota, std::to_string(setQuotaOptions.ShareQuota.GetValue()));
          }
          if (setQuotaOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(Details::HeaderLeaseId, setQuotaOptions.LeaseIdOptional.GetValue());
          }
          return SetQuotaParseResult(context, pipeline.Send(context, request));
        }

        struct SetMetadataOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          Storage::Metadata Metadata; // A name-value pair to associate with a file storage object.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
        };

        static Azure::Core::Response<Models::ShareSetMetadataResult> SetMetadata(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetMetadataOptions& setMetadataOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "metadata");
          if (setMetadataOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(setMetadataOptions.Timeout.GetValue())));
          }
          for (const auto& pair : setMetadataOptions.Metadata)
          {
            request.AddHeader(Details::HeaderMetadata + ("-" + pair.first), pair.second);
          }
          request.AddHeader(Details::HeaderVersion, setMetadataOptions.ApiVersionParameter);
          if (setMetadataOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::HeaderLeaseId, setMetadataOptions.LeaseIdOptional.GetValue());
          }
          return SetMetadataParseResult(context, pipeline.Send(context, request));
        }

        struct GetAccessPolicyOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
        };

        static Azure::Core::Response<Models::ShareGetAccessPolicyResult> GetAccessPolicy(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetAccessPolicyOptions& getAccessPolicyOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "acl");
          if (getAccessPolicyOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(getAccessPolicyOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, getAccessPolicyOptions.ApiVersionParameter);
          if (getAccessPolicyOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::HeaderLeaseId, getAccessPolicyOptions.LeaseIdOptional.GetValue());
          }
          return GetAccessPolicyParseResult(context, pipeline.Send(context, request));
        }

        struct SetAccessPolicyOptions
        {
          std::vector<Models::SignedIdentifier> ShareAcl; // The ACL for the share.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
        };

        static Azure::Core::Response<Models::ShareSetAccessPolicyResult> SetAccessPolicy(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetAccessPolicyOptions& setAccessPolicyOptions)
        {

          std::string xml_body;
          {
            Storage::Details::XmlWriter writer;
            SignedIdentifiersToXml(writer, setAccessPolicyOptions.ShareAcl);
            writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::End});
            xml_body = writer.GetDocument();
          }
          auto body = Azure::Core::Http::MemoryBodyStream(
              reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, &body);
          request.AddHeader("Content-Length", std::to_string(body.Length()));
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "acl");
          if (setAccessPolicyOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(setAccessPolicyOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, setAccessPolicyOptions.ApiVersionParameter);
          if (setAccessPolicyOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::HeaderLeaseId, setAccessPolicyOptions.LeaseIdOptional.GetValue());
          }
          return SetAccessPolicyParseResult(context, pipeline.Send(context, request));
        }

        struct GetStatisticsOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
        };

        static Azure::Core::Response<Models::ShareGetStatisticsResult> GetStatistics(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetStatisticsOptions& getStatisticsOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "stats");
          if (getStatisticsOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(getStatisticsOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, getStatisticsOptions.ApiVersionParameter);
          if (getStatisticsOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::HeaderLeaseId, getStatisticsOptions.LeaseIdOptional.GetValue());
          }
          return GetStatisticsParseResult(context, pipeline.Send(context, request));
        }

        struct RestoreOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
          Azure::Core::Nullable<std::string>
              DeletedShareName; // Specifies the name of the preivously-deleted share.
          Azure::Core::Nullable<std::string>
              DeletedShareVersion; // Specifies the version of the preivously-deleted share.
        };

        static Azure::Core::Response<Models::ShareRestoreResult> Restore(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const RestoreOptions& restoreOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "undelete");
          if (restoreOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(restoreOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, restoreOptions.ApiVersionParameter);
          if (restoreOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(Details::HeaderRequestId, restoreOptions.ClientRequestId.GetValue());
          }
          if (restoreOptions.DeletedShareName.HasValue())
          {
            request.AddHeader(
                Details::HeaderDeletedShareName, restoreOptions.DeletedShareName.GetValue());
          }
          if (restoreOptions.DeletedShareVersion.HasValue())
          {
            request.AddHeader(
                Details::HeaderDeletedShareVersion, restoreOptions.DeletedShareVersion.GetValue());
          }
          return RestoreParseResult(context, pipeline.Send(context, request));
        }

      private:
        static Azure::Core::Response<Models::ShareCreateResult> CreateParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // Success, Share created.
            Models::ShareCreateResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            return Azure::Core::Response<Models::ShareCreateResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::ShareGetPropertiesResult> GetPropertiesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success
            Models::ShareGetPropertiesResult result;

            for (auto i = response.GetHeaders().lower_bound(Details::HeaderMetadata);
                 i != response.GetHeaders().end()
                 && i->first.substr(0, 9) == Details::HeaderMetadata;
                 ++i)
            {
              result.Metadata.emplace(i->first.substr(10), i->second);
            }
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            result.Quota = std::stoll(response.GetHeaders().at(Details::HeaderQuota));
            if (response.GetHeaders().find(Details::HeaderProvisionedIops)
                != response.GetHeaders().end())
            {
              result.ProvisionedIops
                  = std::stoi(response.GetHeaders().at(Details::HeaderProvisionedIops));
            }
            if (response.GetHeaders().find(Details::HeaderProvisionedIngressMBps)
                != response.GetHeaders().end())
            {
              result.ProvisionedIngressMBps
                  = std::stoi(response.GetHeaders().at(Details::HeaderProvisionedIngressMBps));
            }
            if (response.GetHeaders().find(Details::HeaderProvisionedEgressMBps)
                != response.GetHeaders().end())
            {
              result.ProvisionedEgressMBps
                  = std::stoi(response.GetHeaders().at(Details::HeaderProvisionedEgressMBps));
            }
            if (response.GetHeaders().find(Details::HeaderNextAllowedQuotaDowngradeTime)
                != response.GetHeaders().end())
            {
              result.NextAllowedQuotaDowngradeTime = Core::DateTime::Parse(
                  response.GetHeaders().at(Details::HeaderNextAllowedQuotaDowngradeTime),
                  Core::DateTime::DateFormat::Rfc1123);
            }
            if (response.GetHeaders().find(Details::HeaderLeaseDuration)
                != response.GetHeaders().end())
            {
              result.LeaseDuration = LeaseDurationTypeFromString(
                  response.GetHeaders().at(Details::HeaderLeaseDuration));
            }
            if (response.GetHeaders().find(Details::HeaderLeaseState)
                != response.GetHeaders().end())
            {
              result.LeaseState
                  = LeaseStateTypeFromString(response.GetHeaders().at(Details::HeaderLeaseState));
            }
            if (response.GetHeaders().find(Details::HeaderLeaseStatus)
                != response.GetHeaders().end())
            {
              result.LeaseStatus
                  = LeaseStatusTypeFromString(response.GetHeaders().at(Details::HeaderLeaseStatus));
            }
            return Azure::Core::Response<Models::ShareGetPropertiesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::ShareDeleteResult> DeleteParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
          {
            // Accepted
            Models::ShareDeleteResult result;
            return Azure::Core::Response<Models::ShareDeleteResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::ShareAcquireLeaseResult> AcquireLeaseParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // The Acquire operation completed successfully.
            Models::ShareAcquireLeaseResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            if (response.GetHeaders().find(Details::HeaderLeaseTime) != response.GetHeaders().end())
            {
              result.LeaseTime = std::stoi(response.GetHeaders().at(Details::HeaderLeaseTime));
            }
            result.LeaseId = response.GetHeaders().at(Details::HeaderLeaseId);
            return Azure::Core::Response<Models::ShareAcquireLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::ShareReleaseLeaseResult> ReleaseLeaseParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // The Release operation completed successfully.
            Models::ShareReleaseLeaseResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            if (response.GetHeaders().find(Details::HeaderLeaseTime) != response.GetHeaders().end())
            {
              result.LeaseTime = std::stoi(response.GetHeaders().at(Details::HeaderLeaseTime));
            }
            return Azure::Core::Response<Models::ShareReleaseLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::ShareChangeLeaseResult> ChangeLeaseParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // The Change operation completed successfully.
            Models::ShareChangeLeaseResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            if (response.GetHeaders().find(Details::HeaderLeaseTime) != response.GetHeaders().end())
            {
              result.LeaseTime = std::stoi(response.GetHeaders().at(Details::HeaderLeaseTime));
            }
            result.LeaseId = response.GetHeaders().at(Details::HeaderLeaseId);
            return Azure::Core::Response<Models::ShareChangeLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::ShareRenewLeaseResult> RenewLeaseParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // The Renew operation completed successfully.
            Models::ShareRenewLeaseResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            if (response.GetHeaders().find(Details::HeaderLeaseTime) != response.GetHeaders().end())
            {
              result.LeaseTime = std::stoi(response.GetHeaders().at(Details::HeaderLeaseTime));
            }
            result.LeaseId = response.GetHeaders().at(Details::HeaderLeaseId);
            return Azure::Core::Response<Models::ShareRenewLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::ShareBreakLeaseResult> BreakLeaseParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
          {
            // The Break operation completed successfully.
            Models::ShareBreakLeaseResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            if (response.GetHeaders().find(Details::HeaderLeaseTime) != response.GetHeaders().end())
            {
              result.LeaseTime = std::stoi(response.GetHeaders().at(Details::HeaderLeaseTime));
            }
            if (response.GetHeaders().find(Details::HeaderLeaseId) != response.GetHeaders().end())
            {
              result.LeaseId = response.GetHeaders().at(Details::HeaderLeaseId);
            }
            return Azure::Core::Response<Models::ShareBreakLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::ShareCreateSnapshotResult> CreateSnapshotParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // Success, Share snapshot created.
            Models::ShareCreateSnapshotResult result;
            result.Snapshot = response.GetHeaders().at(Details::HeaderSnapshot);
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            return Azure::Core::Response<Models::ShareCreateSnapshotResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::ShareCreatePermissionResult>
        CreatePermissionParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // Success, Share level permission created.
            Models::ShareCreatePermissionResult result;
            result.FilePermissionKey = response.GetHeaders().at(Details::HeaderFilePermissionKey);
            return Azure::Core::Response<Models::ShareCreatePermissionResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static void SharePermissionToJson(
            nlohmann::json& node,
            const Models::SharePermission& object)
        {
          node["permission"] = object.Permission;
        }

        static Azure::Core::Response<Models::ShareGetPermissionResult> GetPermissionParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success
            const auto& bodyBuffer = response.GetBody();
            Models::ShareGetPermissionResult result = bodyBuffer.empty()
                ? Models::ShareGetPermissionResult()
                : ShareGetPermissionResultFromSharePermission(
                    SharePermissionFromJson(nlohmann::json::parse(bodyBuffer)));
            return Azure::Core::Response<Models::ShareGetPermissionResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Models::SharePermission SharePermissionFromJson(const nlohmann::json& node)
        {
          Models::SharePermission result;
          result.Permission = node["permission"].get<std::string>();
          return result;
        }

        static Models::ShareGetPermissionResult ShareGetPermissionResultFromSharePermission(
            Models::SharePermission object)
        {
          Models::ShareGetPermissionResult result;
          result.Permission = std::move(object.Permission);

          return result;
        }
        static Azure::Core::Response<Models::ShareSetQuotaResult> SetQuotaParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success
            Models::ShareSetQuotaResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            return Azure::Core::Response<Models::ShareSetQuotaResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::ShareSetMetadataResult> SetMetadataParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success
            Models::ShareSetMetadataResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            return Azure::Core::Response<Models::ShareSetMetadataResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::ShareGetAccessPolicyResult> GetAccessPolicyParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success
            const auto& bodyBuffer = response.GetBody();
            auto reader = Storage::Details::XmlReader(
                reinterpret_cast<const char*>(bodyBuffer.data()), bodyBuffer.size());
            Models::ShareGetAccessPolicyResult result = bodyBuffer.empty()
                ? Models::ShareGetAccessPolicyResult()
                : ShareGetAccessPolicyResultFromSignedIdentifiers(SignedIdentifiersFromXml(reader));
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            return Azure::Core::Response<Models::ShareGetAccessPolicyResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Models::AccessPolicy AccessPolicyFromXml(Storage::Details::XmlReader& reader)
        {
          auto result = Models::AccessPolicy();
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
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::Expiry)
              {
                result.ExpiresOn
                    = Core::DateTime::Parse(node.Value, Core::DateTime::DateFormat::Rfc3339);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::Permission)
              {
                result.Permission = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::Start)
              {
                result.StartsOn
                    = Core::DateTime::Parse(node.Value, Core::DateTime::DateFormat::Rfc3339);
              }
            }
          }
          return result;
        }

        static Models::SignedIdentifier SignedIdentifierFromXml(Storage::Details::XmlReader& reader)
        {
          auto result = Models::SignedIdentifier();
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
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::Id)
              {
                result.Id = node.Value;
              }
            }
          }
          return result;
        }

        static std::vector<Models::SignedIdentifier> SignedIdentifiersFromXml(
            Storage::Details::XmlReader& reader)
        {
          auto result = std::vector<Models::SignedIdentifier>();
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
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
            }
          }
          return result;
        }

        static Models::ShareGetAccessPolicyResult ShareGetAccessPolicyResultFromSignedIdentifiers(
            std::vector<Models::SignedIdentifier> object)
        {
          Models::ShareGetAccessPolicyResult result;
          result.SignedIdentifiers = std::move(object);

          return result;
        }
        static Azure::Core::Response<Models::ShareSetAccessPolicyResult> SetAccessPolicyParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success.
            Models::ShareSetAccessPolicyResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            return Azure::Core::Response<Models::ShareSetAccessPolicyResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static void AccessPolicyToXml(
            Storage::Details::XmlWriter& writer,
            const Models::AccessPolicy& object)
        {
          writer.Write(
              Storage::Details::XmlNode{Storage::Details::XmlNodeType::StartTag, "AccessPolicy"});
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::StartTag, "Start"});
          writer.Write(Storage::Details::XmlNode{
              Storage::Details::XmlNodeType::Text,
              nullptr,
              object.StartsOn.GetRfc3339String(Core::DateTime::TimeFractionFormat::AllDigits)
                  .data()});
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
          writer.Write(
              Storage::Details::XmlNode{Storage::Details::XmlNodeType::StartTag, "Expiry"});
          writer.Write(Storage::Details::XmlNode{
              Storage::Details::XmlNodeType::Text,
              nullptr,
              object.ExpiresOn.GetRfc3339String(Core::DateTime::TimeFractionFormat::AllDigits)
                  .data()});
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
          writer.Write(
              Storage::Details::XmlNode{Storage::Details::XmlNodeType::StartTag, "Permission"});
          writer.Write(Storage::Details::XmlNode{
              Storage::Details::XmlNodeType::Text, nullptr, object.Permission.data()});
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
        }

        static void SignedIdentifierToXml(
            Storage::Details::XmlWriter& writer,
            const Models::SignedIdentifier& object)
        {
          writer.Write(Storage::Details::XmlNode{
              Storage::Details::XmlNodeType::StartTag, "SignedIdentifier"});
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::StartTag, "Id"});
          writer.Write(Storage::Details::XmlNode{
              Storage::Details::XmlNodeType::Text, nullptr, object.Id.data()});
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
          AccessPolicyToXml(writer, object.Policy);
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
        }

        static void SignedIdentifiersToXml(
            Storage::Details::XmlWriter& writer,
            const std::vector<Models::SignedIdentifier>& object)
        {
          writer.Write(Storage::Details::XmlNode{
              Storage::Details::XmlNodeType::StartTag, "SignedIdentifiers"});
          for (const auto& item : object)
          {
            SignedIdentifierToXml(writer, item);
          }
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
        }
        static Azure::Core::Response<Models::ShareGetStatisticsResult> GetStatisticsParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success
            const auto& bodyBuffer = response.GetBody();
            auto reader = Storage::Details::XmlReader(
                reinterpret_cast<const char*>(bodyBuffer.data()), bodyBuffer.size());
            Models::ShareGetStatisticsResult result = bodyBuffer.empty()
                ? Models::ShareGetStatisticsResult()
                : ShareGetStatisticsResultFromShareStats(ShareStatsFromXml(reader));
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            return Azure::Core::Response<Models::ShareGetStatisticsResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Models::ShareStats ShareStatsFromXml(Storage::Details::XmlReader& reader)
        {
          auto result = Models::ShareStats();
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
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 2 && path[0] == XmlTagName::ShareStats
                  && path[1] == XmlTagName::ShareUsageBytes)
              {
                result.ShareUsageBytes = std::stoll(node.Value);
              }
            }
          }
          return result;
        }

        static Models::ShareGetStatisticsResult ShareGetStatisticsResultFromShareStats(
            Models::ShareStats object)
        {
          Models::ShareGetStatisticsResult result;
          result.ShareUsageBytes = object.ShareUsageBytes;

          return result;
        }
        static Azure::Core::Response<Models::ShareRestoreResult> RestoreParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // Created
            Models::ShareRestoreResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            return Azure::Core::Response<Models::ShareRestoreResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }
      };

      class Directory {
      public:
        struct CreateOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          Storage::Metadata Metadata; // A name-value pair to associate with a file storage object.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              FilePermission; // If specified the permission (security descriptor) shall be set for
                              // the directory/file. This header can be used if Permission size is
                              // <= 8KB, else x-ms-file-permission-key header shall be used. Default
                              // value: Inherit. If SDDL is specified as input, it must have owner,
                              // group and dacl. Note: Only one of the x-ms-file-permission or
                              // x-ms-file-permission-key should be specified.
          Azure::Core::Nullable<std::string>
              FilePermissionKey; // Key of the permission to be set for the directory/file. Note:
                                 // Only one of the x-ms-file-permission or x-ms-file-permission-key
                                 // should be specified.
          std::string FileAttributes; // If specified, the provided file attributes shall be set.
                                      // Default value: Archive for file and Directory for
                                      // directory. None can also be specified as default.
          std::string FileCreationTime; // Creation time for the file/directory. Default value: Now.
          std::string
              FileLastWriteTime; // Last write time for the file/directory. Default value: Now.
        };

        static Azure::Core::Response<Models::DirectoryCreateResult> Create(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const CreateOptions& createOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "directory");
          if (createOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(createOptions.Timeout.GetValue())));
          }
          for (const auto& pair : createOptions.Metadata)
          {
            request.AddHeader(Details::HeaderMetadata + ("-" + pair.first), pair.second);
          }
          request.AddHeader(Details::HeaderVersion, createOptions.ApiVersionParameter);
          if (createOptions.FilePermission.HasValue())
          {
            request.AddHeader(
                Details::HeaderFilePermission, createOptions.FilePermission.GetValue());
          }
          if (createOptions.FilePermissionKey.HasValue())
          {
            request.AddHeader(
                Details::HeaderFilePermissionKey, createOptions.FilePermissionKey.GetValue());
          }
          request.AddHeader(Details::HeaderFileAttributes, createOptions.FileAttributes);
          request.AddHeader(Details::HeaderFileCreatedOn, createOptions.FileCreationTime);
          request.AddHeader(Details::HeaderFileLastWrittenOn, createOptions.FileLastWriteTime);
          return CreateParseResult(context, pipeline.Send(context, request));
        }

        struct GetPropertiesOptions
        {
          Azure::Core::Nullable<std::string>
              ShareSnapshot; // The snapshot parameter is an opaque DateTime value that, when
                             // present, specifies the share snapshot to query.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        };

        static Azure::Core::Response<Models::DirectoryGetPropertiesResult> GetProperties(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetPropertiesOptions& getPropertiesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "directory");
          if (getPropertiesOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    getPropertiesOptions.ShareSnapshot.GetValue()));
          }
          if (getPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(getPropertiesOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, getPropertiesOptions.ApiVersionParameter);
          return GetPropertiesParseResult(context, pipeline.Send(context, request));
        }

        struct DeleteOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        };

        static Azure::Core::Response<Models::DirectoryDeleteResult> Delete(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const DeleteOptions& deleteOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Delete, url);
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "directory");
          if (deleteOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(deleteOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, deleteOptions.ApiVersionParameter);
          return DeleteParseResult(context, pipeline.Send(context, request));
        }

        struct SetPropertiesOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              FilePermission; // If specified the permission (security descriptor) shall be set for
                              // the directory/file. This header can be used if Permission size is
                              // <= 8KB, else x-ms-file-permission-key header shall be used. Default
                              // value: Inherit. If SDDL is specified as input, it must have owner,
                              // group and dacl. Note: Only one of the x-ms-file-permission or
                              // x-ms-file-permission-key should be specified.
          Azure::Core::Nullable<std::string>
              FilePermissionKey; // Key of the permission to be set for the directory/file. Note:
                                 // Only one of the x-ms-file-permission or x-ms-file-permission-key
                                 // should be specified.
          std::string FileAttributes; // If specified, the provided file attributes shall be set.
                                      // Default value: Archive for file and Directory for
                                      // directory. None can also be specified as default.
          std::string FileCreationTime; // Creation time for the file/directory. Default value: Now.
          std::string
              FileLastWriteTime; // Last write time for the file/directory. Default value: Now.
        };

        static Azure::Core::Response<Models::DirectorySetPropertiesResult> SetProperties(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetPropertiesOptions& setPropertiesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "directory");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "properties");
          if (setPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(setPropertiesOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, setPropertiesOptions.ApiVersionParameter);
          if (setPropertiesOptions.FilePermission.HasValue())
          {
            request.AddHeader(
                Details::HeaderFilePermission, setPropertiesOptions.FilePermission.GetValue());
          }
          if (setPropertiesOptions.FilePermissionKey.HasValue())
          {
            request.AddHeader(
                Details::HeaderFilePermissionKey,
                setPropertiesOptions.FilePermissionKey.GetValue());
          }
          request.AddHeader(Details::HeaderFileAttributes, setPropertiesOptions.FileAttributes);
          request.AddHeader(Details::HeaderFileCreatedOn, setPropertiesOptions.FileCreationTime);
          request.AddHeader(
              Details::HeaderFileLastWrittenOn, setPropertiesOptions.FileLastWriteTime);
          return SetPropertiesParseResult(context, pipeline.Send(context, request));
        }

        struct SetMetadataOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          Storage::Metadata Metadata; // A name-value pair to associate with a file storage object.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        };

        static Azure::Core::Response<Models::DirectorySetMetadataResult> SetMetadata(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetMetadataOptions& setMetadataOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "directory");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "metadata");
          if (setMetadataOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(setMetadataOptions.Timeout.GetValue())));
          }
          for (const auto& pair : setMetadataOptions.Metadata)
          {
            request.AddHeader(Details::HeaderMetadata + ("-" + pair.first), pair.second);
          }
          request.AddHeader(Details::HeaderVersion, setMetadataOptions.ApiVersionParameter);
          return SetMetadataParseResult(context, pipeline.Send(context, request));
        }

        struct ListFilesAndDirectoriesSegmentOptions
        {
          Azure::Core::Nullable<std::string> Prefix; // Filters the results to return only entries
                                                     // whose name begins with the specified prefix.
          Azure::Core::Nullable<std::string>
              ShareSnapshot; // The snapshot parameter is an opaque DateTime value that, when
                             // present, specifies the share snapshot to query.
          Azure::Core::Nullable<std::string>
              ContinuationToken; // A string value that identifies the portion of the list to be
                                 // returned with the next list operation. The operation returns a
                                 // marker value within the response body if the list returned was
                                 // not complete. The marker value may then be used in a subsequent
                                 // call to request the next set of list items. The marker value is
                                 // opaque to the client.
          Azure::Core::Nullable<int32_t>
              MaxResults; // Specifies the maximum number of entries to return. If the request does
                          // not specify maxresults, or specifies a value greater than 5,000, the
                          // server will return up to 5,000 items.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        };

        static Azure::Core::Response<Models::DirectoryListFilesAndDirectoriesSegmentResult>
        ListFilesAndDirectoriesSegment(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ListFilesAndDirectoriesSegmentOptions& listFilesAndDirectoriesSegmentOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(Details::QueryRestype, "directory");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "list");
          if (listFilesAndDirectoriesSegmentOptions.Prefix.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryPrefix,
                Storage::Details::UrlEncodeQueryParameter(
                    listFilesAndDirectoriesSegmentOptions.Prefix.GetValue()));
          }
          if (listFilesAndDirectoriesSegmentOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    listFilesAndDirectoriesSegmentOptions.ShareSnapshot.GetValue()));
          }
          if (listFilesAndDirectoriesSegmentOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryContinuationToken,
                Storage::Details::UrlEncodeQueryParameter(
                    listFilesAndDirectoriesSegmentOptions.ContinuationToken.GetValue()));
          }
          if (listFilesAndDirectoriesSegmentOptions.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryMaxResults,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listFilesAndDirectoriesSegmentOptions.MaxResults.GetValue())));
          }
          if (listFilesAndDirectoriesSegmentOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listFilesAndDirectoriesSegmentOptions.Timeout.GetValue())));
          }
          request.AddHeader(
              Details::HeaderVersion, listFilesAndDirectoriesSegmentOptions.ApiVersionParameter);
          return ListFilesAndDirectoriesSegmentParseResult(
              context, pipeline.Send(context, request));
        }

        struct ListHandlesOptions
        {
          Azure::Core::Nullable<std::string>
              ContinuationToken; // A string value that identifies the portion of the list to be
                                 // returned with the next list operation. The operation returns a
                                 // marker value within the response body if the list returned was
                                 // not complete. The marker value may then be used in a subsequent
                                 // call to request the next set of list items. The marker value is
                                 // opaque to the client.
          Azure::Core::Nullable<int32_t>
              MaxResults; // Specifies the maximum number of entries to return. If the request does
                          // not specify maxresults, or specifies a value greater than 5,000, the
                          // server will return up to 5,000 items.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          Azure::Core::Nullable<std::string>
              ShareSnapshot; // The snapshot parameter is an opaque DateTime value that, when
                             // present, specifies the share snapshot to query.
          Azure::Core::Nullable<bool>
              Recursive; // Specifies operation should apply to the directory specified in the URI,
                         // its files, its subdirectories and their files.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        };

        static Azure::Core::Response<Models::DirectoryListHandlesResult> ListHandles(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ListHandlesOptions& listHandlesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "listhandles");
          if (listHandlesOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryContinuationToken,
                Storage::Details::UrlEncodeQueryParameter(
                    listHandlesOptions.ContinuationToken.GetValue()));
          }
          if (listHandlesOptions.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryMaxResults,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listHandlesOptions.MaxResults.GetValue())));
          }
          if (listHandlesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listHandlesOptions.Timeout.GetValue())));
          }
          if (listHandlesOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    listHandlesOptions.ShareSnapshot.GetValue()));
          }
          if (listHandlesOptions.Recursive.HasValue())
          {
            request.AddHeader(
                Details::HeaderRecursive,
                (listHandlesOptions.Recursive.GetValue() ? "true" : "false"));
          }
          request.AddHeader(Details::HeaderVersion, listHandlesOptions.ApiVersionParameter);
          return ListHandlesParseResult(context, pipeline.Send(context, request));
        }

        struct ForceCloseHandlesOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          Azure::Core::Nullable<std::string>
              ContinuationToken; // A string value that identifies the portion of the list to be
                                 // returned with the next list operation. The operation returns a
                                 // marker value within the response body if the list returned was
                                 // not complete. The marker value may then be used in a subsequent
                                 // call to request the next set of list items. The marker value is
                                 // opaque to the client.
          Azure::Core::Nullable<std::string>
              ShareSnapshot; // The snapshot parameter is an opaque DateTime value that, when
                             // present, specifies the share snapshot to query.
          std::string HandleId; // Specifies handle ID opened on the file or directory to be closed.
                                // Asterisk (*) is a wildcard that specifies all handles.
          Azure::Core::Nullable<bool>
              Recursive; // Specifies operation should apply to the directory specified in the URI,
                         // its files, its subdirectories and their files.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        };

        static Azure::Core::Response<Models::DirectoryForceCloseHandlesResult> ForceCloseHandles(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ForceCloseHandlesOptions& forceCloseHandlesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "forceclosehandles");
          if (forceCloseHandlesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(forceCloseHandlesOptions.Timeout.GetValue())));
          }
          if (forceCloseHandlesOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryContinuationToken,
                Storage::Details::UrlEncodeQueryParameter(
                    forceCloseHandlesOptions.ContinuationToken.GetValue()));
          }
          if (forceCloseHandlesOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    forceCloseHandlesOptions.ShareSnapshot.GetValue()));
          }
          request.AddHeader(Details::HeaderHandleId, forceCloseHandlesOptions.HandleId);
          if (forceCloseHandlesOptions.Recursive.HasValue())
          {
            request.AddHeader(
                Details::HeaderRecursive,
                (forceCloseHandlesOptions.Recursive.GetValue() ? "true" : "false"));
          }
          request.AddHeader(Details::HeaderVersion, forceCloseHandlesOptions.ApiVersionParameter);
          return ForceCloseHandlesParseResult(context, pipeline.Send(context, request));
        }

      private:
        static Azure::Core::Response<Models::DirectoryCreateResult> CreateParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // Success, Directory created.
            Models::DirectoryCreateResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            result.IsServerEncrypted
                = response.GetHeaders().at(Details::HeaderRequestIsServerEncrypted) == "true";
            result.FilePermissionKey = response.GetHeaders().at(Details::HeaderFilePermissionKey);
            result.FileAttributes = response.GetHeaders().at(Details::HeaderFileAttributes);
            result.FileCreatedOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileCreatedOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FileLastWrittenOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileLastWrittenOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FileChangedOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileChangedOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FileId = response.GetHeaders().at(Details::HeaderFileId);
            result.FileParentId = response.GetHeaders().at(Details::HeaderFileParentId);
            return Azure::Core::Response<Models::DirectoryCreateResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::DirectoryGetPropertiesResult> GetPropertiesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success.
            Models::DirectoryGetPropertiesResult result;

            for (auto i = response.GetHeaders().lower_bound(Details::HeaderMetadata);
                 i != response.GetHeaders().end()
                 && i->first.substr(0, 9) == Details::HeaderMetadata;
                 ++i)
            {
              result.Metadata.emplace(i->first.substr(10), i->second);
            }
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            result.IsServerEncrypted
                = response.GetHeaders().at(Details::HeaderIsServerEncrypted) == "true";
            result.FileAttributes = response.GetHeaders().at(Details::HeaderFileAttributes);
            result.FileCreatedOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileCreatedOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FileLastWrittenOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileLastWrittenOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FileChangedOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileChangedOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FilePermissionKey = response.GetHeaders().at(Details::HeaderFilePermissionKey);
            result.FileId = response.GetHeaders().at(Details::HeaderFileId);
            result.FileParentId = response.GetHeaders().at(Details::HeaderFileParentId);
            return Azure::Core::Response<Models::DirectoryGetPropertiesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::DirectoryDeleteResult> DeleteParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
          {
            // Success (Accepted).
            Models::DirectoryDeleteResult result;
            return Azure::Core::Response<Models::DirectoryDeleteResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::DirectorySetPropertiesResult> SetPropertiesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success
            Models::DirectorySetPropertiesResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            result.IsServerEncrypted
                = response.GetHeaders().at(Details::HeaderRequestIsServerEncrypted) == "true";
            result.FilePermissionKey = response.GetHeaders().at(Details::HeaderFilePermissionKey);
            result.FileAttributes = response.GetHeaders().at(Details::HeaderFileAttributes);
            result.FileCreatedOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileCreatedOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FileLastWrittenOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileLastWrittenOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FileChangedOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileChangedOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FileId = response.GetHeaders().at(Details::HeaderFileId);
            result.FileParentId = response.GetHeaders().at(Details::HeaderFileParentId);
            return Azure::Core::Response<Models::DirectorySetPropertiesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::DirectorySetMetadataResult> SetMetadataParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success (OK).
            Models::DirectorySetMetadataResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.IsServerEncrypted
                = response.GetHeaders().at(Details::HeaderRequestIsServerEncrypted) == "true";
            return Azure::Core::Response<Models::DirectorySetMetadataResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::DirectoryListFilesAndDirectoriesSegmentResult>
        ListFilesAndDirectoriesSegmentParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success.
            const auto& bodyBuffer = response.GetBody();
            auto reader = Storage::Details::XmlReader(
                reinterpret_cast<const char*>(bodyBuffer.data()), bodyBuffer.size());
            Models::DirectoryListFilesAndDirectoriesSegmentResult result = bodyBuffer.empty()
                ? Models::DirectoryListFilesAndDirectoriesSegmentResult()
                : DirectoryListFilesAndDirectoriesSegmentResultFromListFilesAndDirectoriesSegmentResponse(
                    ListFilesAndDirectoriesSegmentResponseFromXml(reader));
            result.HttpHeaders.ContentType = response.GetHeaders().at(Details::HeaderContentType);
            return Azure::Core::Response<Models::DirectoryListFilesAndDirectoriesSegmentResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Models::DirectoryItem DirectoryItemFromXml(Storage::Details::XmlReader& reader)
        {
          auto result = Models::DirectoryItem();
          enum class XmlTagName
          {
            Name,
            Unknown,
          };
          std::vector<XmlTagName> path;

          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::Name)
              {
                result.Name = node.Value;
              }
            }
          }
          return result;
        }

        static Models::FileProperty FilePropertyFromXml(Storage::Details::XmlReader& reader)
        {
          auto result = Models::FileProperty();
          enum class XmlTagName
          {
            ContentLength,
            Unknown,
          };
          std::vector<XmlTagName> path;

          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::ContentLength)
              {
                result.ContentLength = std::stoll(node.Value);
              }
            }
          }
          return result;
        }

        static Models::FileItem FileItemFromXml(Storage::Details::XmlReader& reader)
        {
          auto result = Models::FileItem();
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
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
                result.Properties = FilePropertyFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::Name)
              {
                result.Name = node.Value;
              }
            }
          }
          return result;
        }

        static Models::FilesAndDirectoriesListSegment FilesAndDirectoriesListSegmentFromXml(
            Storage::Details::XmlReader& reader)
        {
          auto result = Models::FilesAndDirectoriesListSegment();
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
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
            }
          }
          return result;
        }

        static Models::ListFilesAndDirectoriesSegmentResponse
        ListFilesAndDirectoriesSegmentResponseFromXml(Storage::Details::XmlReader& reader)
        {
          auto result = Models::ListFilesAndDirectoriesSegmentResponse();
          enum class XmlTagName
          {
            Entries,
            EnumerationResults,
            Marker,
            MaxResults,
            NextMarker,
            Prefix,
            Unknown,
          };
          std::vector<XmlTagName> path;

          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
            {

              if (std::strcmp(node.Name, "Entries") == 0)
              {
                path.emplace_back(XmlTagName::Entries);
              }
              else if (std::strcmp(node.Name, "EnumerationResults") == 0)
              {
                path.emplace_back(XmlTagName::EnumerationResults);
              }
              else if (std::strcmp(node.Name, "Marker") == 0)
              {
                path.emplace_back(XmlTagName::Marker);
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
                result.Segment = FilesAndDirectoriesListSegmentFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
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
                result.MaxResults = std::stoi(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::EnumerationResults
                  && path[1] == XmlTagName::Prefix)
              {
                result.Prefix = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::EnumerationResults
                  && path[1] == XmlTagName::Marker)
              {
                result.PreviousContinuationToken = node.Value;
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Attribute)
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

        static Models::DirectoryListFilesAndDirectoriesSegmentResult
        DirectoryListFilesAndDirectoriesSegmentResultFromListFilesAndDirectoriesSegmentResponse(
            Models::ListFilesAndDirectoriesSegmentResponse object)
        {
          Models::DirectoryListFilesAndDirectoriesSegmentResult result;
          result.ServiceEndpoint = std::move(object.ServiceEndpoint);
          result.ShareName = std::move(object.ShareName);
          result.ShareSnapshot = std::move(object.ShareSnapshot);
          result.DirectoryPath = std::move(object.DirectoryPath);
          result.Prefix = std::move(object.Prefix);
          result.PreviousContinuationToken = std::move(object.PreviousContinuationToken);
          result.MaxResults = object.MaxResults;
          result.Segment = std::move(object.Segment);
          result.ContinuationToken = std::move(object.ContinuationToken);

          return result;
        }
        static Azure::Core::Response<Models::DirectoryListHandlesResult> ListHandlesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success.
            const auto& bodyBuffer = response.GetBody();
            auto reader = Storage::Details::XmlReader(
                reinterpret_cast<const char*>(bodyBuffer.data()), bodyBuffer.size());
            Models::DirectoryListHandlesResult result = bodyBuffer.empty()
                ? Models::DirectoryListHandlesResult()
                : DirectoryListHandlesResultFromListHandlesResponse(
                    ListHandlesResponseFromXml(reader));
            result.HttpHeaders.ContentType = response.GetHeaders().at(Details::HeaderContentType);
            return Azure::Core::Response<Models::DirectoryListHandlesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Models::HandleItem HandleItemFromXml(Storage::Details::XmlReader& reader)
        {
          auto result = Models::HandleItem();
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
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::Text)
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
                    = Core::DateTime::Parse(node.Value, Core::DateTime::DateFormat::Rfc1123);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::OpenTime)
              {
                result.OpenedOn
                    = Core::DateTime::Parse(node.Value, Core::DateTime::DateFormat::Rfc1123);
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

        static Models::ListHandlesResponse ListHandlesResponseFromXml(
            Storage::Details::XmlReader& reader)
        {
          auto result = Models::ListHandlesResponse();
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
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::Text)
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

        static Models::DirectoryListHandlesResult DirectoryListHandlesResultFromListHandlesResponse(
            Models::ListHandlesResponse object)
        {
          Models::DirectoryListHandlesResult result;
          result.HandleList = std::move(object.HandleList);
          result.ContinuationToken = std::move(object.ContinuationToken);

          return result;
        }
        static Azure::Core::Response<Models::DirectoryForceCloseHandlesResult>
        ForceCloseHandlesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success.
            Models::DirectoryForceCloseHandlesResult result;
            if (response.GetHeaders().find(Details::HeaderContinuationToken)
                != response.GetHeaders().end())
            {
              result.ContinuationToken = response.GetHeaders().at(Details::HeaderContinuationToken);
            }
            result.numberOfHandlesClosed
                = std::stoi(response.GetHeaders().at(Details::HeaderNumberOfHandlesClosed));
            result.numberOfHandlesFailedToClose
                = std::stoi(response.GetHeaders().at(Details::HeaderNumberOfHandlesFailedToClose));
            return Azure::Core::Response<Models::DirectoryForceCloseHandlesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }
      };

      class File {
      public:
        struct CreateOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          int64_t XMsContentLength
              = int64_t(); // Specifies the maximum size for the file, up to 4 TB.
          Azure::Core::Nullable<std::string>
              FileContentType; // Sets the MIME content type of the file. The default type is
                               // 'application/octet-stream'.
          Azure::Core::Nullable<std::string>
              FileContentEncoding; // Specifies which content encodings have been applied to the
                                   // file.
          Azure::Core::Nullable<std::string>
              FileContentLanguage; // Specifies the natural languages used by this resource.
          Azure::Core::Nullable<std::string>
              FileCacheControl; // Sets the file's cache control. The File service stores this value
                                // but does not use or modify it.
          Azure::Core::Nullable<Storage::ContentHash>
              ContentMd5; // An MD5 hash of the content. This hash is used to verify the integrity
                          // of the data during transport. When the Content-MD5 header is specified,
                          // the File service compares the hash of the content that has arrived with
                          // the header value that was sent. If the two hashes do not match, the
                          // operation will fail with error code 400 (Bad Request).
          Azure::Core::Nullable<std::string>
              FileContentDisposition; // Sets the file's Content-Disposition header.
          Storage::Metadata Metadata; // A name-value pair to associate with a file storage object.
          Azure::Core::Nullable<std::string>
              FilePermission; // If specified the permission (security descriptor) shall be set for
                              // the directory/file. This header can be used if Permission size is
                              // <= 8KB, else x-ms-file-permission-key header shall be used. Default
                              // value: Inherit. If SDDL is specified as input, it must have owner,
                              // group and dacl. Note: Only one of the x-ms-file-permission or
                              // x-ms-file-permission-key should be specified.
          Azure::Core::Nullable<std::string>
              FilePermissionKey; // Key of the permission to be set for the directory/file. Note:
                                 // Only one of the x-ms-file-permission or x-ms-file-permission-key
                                 // should be specified.
          std::string FileAttributes; // If specified, the provided file attributes shall be set.
                                      // Default value: Archive for file and Directory for
                                      // directory. None can also be specified as default.
          std::string FileCreationTime; // Creation time for the file/directory. Default value: Now.
          std::string
              FileLastWriteTime; // Last write time for the file/directory. Default value: Now.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
        };

        static Azure::Core::Response<Models::FileCreateResult> Create(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const CreateOptions& createOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          if (createOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(createOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, createOptions.ApiVersionParameter);
          request.AddHeader(
              Details::HeaderXMsContentLength, std::to_string(createOptions.XMsContentLength));
          request.AddHeader(Details::HeaderFileTypeConstant, "file");
          if (createOptions.FileContentType.HasValue())
          {
            request.AddHeader(
                Details::HeaderFileContentType, createOptions.FileContentType.GetValue());
          }
          if (createOptions.FileContentEncoding.HasValue())
          {
            request.AddHeader(
                Details::HeaderFileContentEncoding, createOptions.FileContentEncoding.GetValue());
          }
          if (createOptions.FileContentLanguage.HasValue())
          {
            request.AddHeader(
                Details::HeaderFileContentLanguage, createOptions.FileContentLanguage.GetValue());
          }
          if (createOptions.FileCacheControl.HasValue())
          {
            request.AddHeader(
                Details::HeaderFileCacheControl, createOptions.FileCacheControl.GetValue());
          }
          if (createOptions.ContentMd5.HasValue())
          {
            request.AddHeader(
                Details::HeaderContentHash,
                Storage::Details::ToBase64String(createOptions.ContentMd5.GetValue()));
          }
          if (createOptions.FileContentDisposition.HasValue())
          {
            request.AddHeader(
                Details::HeaderFileContentDisposition,
                createOptions.FileContentDisposition.GetValue());
          }
          for (const auto& pair : createOptions.Metadata)
          {
            request.AddHeader(Details::HeaderMetadata + ("-" + pair.first), pair.second);
          }
          if (createOptions.FilePermission.HasValue())
          {
            request.AddHeader(
                Details::HeaderFilePermission, createOptions.FilePermission.GetValue());
          }
          if (createOptions.FilePermissionKey.HasValue())
          {
            request.AddHeader(
                Details::HeaderFilePermissionKey, createOptions.FilePermissionKey.GetValue());
          }
          request.AddHeader(Details::HeaderFileAttributes, createOptions.FileAttributes);
          request.AddHeader(Details::HeaderFileCreatedOn, createOptions.FileCreationTime);
          request.AddHeader(Details::HeaderFileLastWrittenOn, createOptions.FileLastWriteTime);
          if (createOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(Details::HeaderLeaseId, createOptions.LeaseIdOptional.GetValue());
          }
          return CreateParseResult(context, pipeline.Send(context, request));
        }

        struct DownloadOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              Range; // Return file data only from the specified byte range.
          Azure::Core::Nullable<bool>
              GetRangeContentMd5; // When this header is set to true and specified together with the
                                  // Range header, the service returns the MD5 hash for the range,
                                  // as long as the range is less than or equal to 4 MB in size.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
        };

        static Azure::Core::Response<Models::FileDownloadResult> Download(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const DownloadOptions& downloadOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url, true);
          if (downloadOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(downloadOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, downloadOptions.ApiVersionParameter);
          if (downloadOptions.Range.HasValue())
          {
            request.AddHeader(Details::HeaderRange, downloadOptions.Range.GetValue());
          }
          if (downloadOptions.GetRangeContentMd5.HasValue())
          {
            request.AddHeader(
                Details::HeaderRangeGetContentMd5,
                (downloadOptions.GetRangeContentMd5.GetValue() ? "true" : "false"));
          }
          if (downloadOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(Details::HeaderLeaseId, downloadOptions.LeaseIdOptional.GetValue());
          }
          return DownloadParseResult(context, pipeline.Send(context, request));
        }

        struct GetPropertiesOptions
        {
          Azure::Core::Nullable<std::string>
              ShareSnapshot; // The snapshot parameter is an opaque DateTime value that, when
                             // present, specifies the share snapshot to query.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
        };

        static Azure::Core::Response<Models::FileGetPropertiesResult> GetProperties(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetPropertiesOptions& getPropertiesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Head, url);
          if (getPropertiesOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    getPropertiesOptions.ShareSnapshot.GetValue()));
          }
          if (getPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(getPropertiesOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, getPropertiesOptions.ApiVersionParameter);
          if (getPropertiesOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::HeaderLeaseId, getPropertiesOptions.LeaseIdOptional.GetValue());
          }
          return GetPropertiesParseResult(context, pipeline.Send(context, request));
        }

        struct DeleteOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
        };

        static Azure::Core::Response<Models::FileDeleteResult> Delete(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const DeleteOptions& deleteOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Delete, url);
          if (deleteOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(deleteOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, deleteOptions.ApiVersionParameter);
          if (deleteOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(Details::HeaderLeaseId, deleteOptions.LeaseIdOptional.GetValue());
          }
          return DeleteParseResult(context, pipeline.Send(context, request));
        }

        struct SetHttpHeadersOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<int64_t>
              XMsContentLength; // Resizes a file to the specified size. If the specified byte value
                                // is less than the current size of the file, then all ranges above
                                // the specified byte value are cleared.
          Azure::Core::Nullable<std::string>
              FileContentType; // Sets the MIME content type of the file. The default type is
                               // 'application/octet-stream'.
          Azure::Core::Nullable<std::string>
              FileContentEncoding; // Specifies which content encodings have been applied to the
                                   // file.
          Azure::Core::Nullable<std::string>
              FileContentLanguage; // Specifies the natural languages used by this resource.
          Azure::Core::Nullable<std::string>
              FileCacheControl; // Sets the file's cache control. The File service stores this value
                                // but does not use or modify it.
          Azure::Core::Nullable<Storage::ContentHash>
              ContentMd5; // An MD5 hash of the content. This hash is used to verify the integrity
                          // of the data during transport. When the Content-MD5 header is specified,
                          // the File service compares the hash of the content that has arrived with
                          // the header value that was sent. If the two hashes do not match, the
                          // operation will fail with error code 400 (Bad Request).
          Azure::Core::Nullable<std::string>
              FileContentDisposition; // Sets the file's Content-Disposition header.
          Azure::Core::Nullable<std::string>
              FilePermission; // If specified the permission (security descriptor) shall be set for
                              // the directory/file. This header can be used if Permission size is
                              // <= 8KB, else x-ms-file-permission-key header shall be used. Default
                              // value: Inherit. If SDDL is specified as input, it must have owner,
                              // group and dacl. Note: Only one of the x-ms-file-permission or
                              // x-ms-file-permission-key should be specified.
          Azure::Core::Nullable<std::string>
              FilePermissionKey; // Key of the permission to be set for the directory/file. Note:
                                 // Only one of the x-ms-file-permission or x-ms-file-permission-key
                                 // should be specified.
          std::string FileAttributes; // If specified, the provided file attributes shall be set.
                                      // Default value: Archive for file and Directory for
                                      // directory. None can also be specified as default.
          std::string FileCreationTime; // Creation time for the file/directory. Default value: Now.
          std::string
              FileLastWriteTime; // Last write time for the file/directory. Default value: Now.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
        };

        static Azure::Core::Response<Models::FileSetHttpHeadersResult> SetHttpHeaders(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetHttpHeadersOptions& setHttpHeadersOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "properties");
          if (setHttpHeadersOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(setHttpHeadersOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, setHttpHeadersOptions.ApiVersionParameter);
          if (setHttpHeadersOptions.XMsContentLength.HasValue())
          {
            request.AddHeader(
                Details::HeaderXMsContentLength,
                std::to_string(setHttpHeadersOptions.XMsContentLength.GetValue()));
          }
          if (setHttpHeadersOptions.FileContentType.HasValue())
          {
            request.AddHeader(
                Details::HeaderFileContentType, setHttpHeadersOptions.FileContentType.GetValue());
          }
          if (setHttpHeadersOptions.FileContentEncoding.HasValue())
          {
            request.AddHeader(
                Details::HeaderFileContentEncoding,
                setHttpHeadersOptions.FileContentEncoding.GetValue());
          }
          if (setHttpHeadersOptions.FileContentLanguage.HasValue())
          {
            request.AddHeader(
                Details::HeaderFileContentLanguage,
                setHttpHeadersOptions.FileContentLanguage.GetValue());
          }
          if (setHttpHeadersOptions.FileCacheControl.HasValue())
          {
            request.AddHeader(
                Details::HeaderFileCacheControl, setHttpHeadersOptions.FileCacheControl.GetValue());
          }
          if (setHttpHeadersOptions.ContentMd5.HasValue())
          {
            request.AddHeader(
                Details::HeaderContentHash,
                Storage::Details::ToBase64String(setHttpHeadersOptions.ContentMd5.GetValue()));
          }
          if (setHttpHeadersOptions.FileContentDisposition.HasValue())
          {
            request.AddHeader(
                Details::HeaderFileContentDisposition,
                setHttpHeadersOptions.FileContentDisposition.GetValue());
          }
          if (setHttpHeadersOptions.FilePermission.HasValue())
          {
            request.AddHeader(
                Details::HeaderFilePermission, setHttpHeadersOptions.FilePermission.GetValue());
          }
          if (setHttpHeadersOptions.FilePermissionKey.HasValue())
          {
            request.AddHeader(
                Details::HeaderFilePermissionKey,
                setHttpHeadersOptions.FilePermissionKey.GetValue());
          }
          request.AddHeader(Details::HeaderFileAttributes, setHttpHeadersOptions.FileAttributes);
          request.AddHeader(Details::HeaderFileCreatedOn, setHttpHeadersOptions.FileCreationTime);
          request.AddHeader(
              Details::HeaderFileLastWrittenOn, setHttpHeadersOptions.FileLastWriteTime);
          if (setHttpHeadersOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::HeaderLeaseId, setHttpHeadersOptions.LeaseIdOptional.GetValue());
          }
          return SetHttpHeadersParseResult(context, pipeline.Send(context, request));
        }

        struct SetMetadataOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          Storage::Metadata Metadata; // A name-value pair to associate with a file storage object.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
        };

        static Azure::Core::Response<Models::FileSetMetadataResult> SetMetadata(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetMetadataOptions& setMetadataOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "metadata");
          if (setMetadataOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(setMetadataOptions.Timeout.GetValue())));
          }
          for (const auto& pair : setMetadataOptions.Metadata)
          {
            request.AddHeader(Details::HeaderMetadata + ("-" + pair.first), pair.second);
          }
          request.AddHeader(Details::HeaderVersion, setMetadataOptions.ApiVersionParameter);
          if (setMetadataOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::HeaderLeaseId, setMetadataOptions.LeaseIdOptional.GetValue());
          }
          return SetMetadataParseResult(context, pipeline.Send(context, request));
        }

        struct AcquireLeaseOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          int32_t LeaseDuration
              = int32_t(); // Specifies the duration of the lease, in seconds, or negative one (-1)
                           // for a lease that never expires. A non-infinite lease can be between 15
                           // and 60 seconds. A lease duration cannot be changed using renew or
                           // change.
          Azure::Core::Nullable<std::string>
              ProposedLeaseIdOptional; // Proposed lease ID, in a GUID string format. The File
                                       // service returns 400 (Invalid request) if the proposed
                                       // lease ID is not in the correct format. See Guid
                                       // Constructor (String) for a list of valid GUID string
                                       // formats.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
        };

        static Azure::Core::Response<Models::FileAcquireLeaseResult> AcquireLease(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const AcquireLeaseOptions& acquireLeaseOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "lease");
          request.AddHeader(Details::HeaderAction, "acquire");
          if (acquireLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(acquireLeaseOptions.Timeout.GetValue())));
          }
          request.AddHeader(
              Details::HeaderDuration, std::to_string(acquireLeaseOptions.LeaseDuration));
          if (acquireLeaseOptions.ProposedLeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::HeaderProposedLeaseId,
                acquireLeaseOptions.ProposedLeaseIdOptional.GetValue());
          }
          request.AddHeader(Details::HeaderVersion, acquireLeaseOptions.ApiVersionParameter);
          if (acquireLeaseOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderRequestId, acquireLeaseOptions.ClientRequestId.GetValue());
          }
          return AcquireLeaseParseResult(context, pipeline.Send(context, request));
        }

        struct ReleaseLeaseOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string LeaseIdRequired; // Specifies the current lease ID on the resource.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
        };

        static Azure::Core::Response<Models::FileReleaseLeaseResult> ReleaseLease(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ReleaseLeaseOptions& releaseLeaseOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "lease");
          request.AddHeader(Details::HeaderAction, "release");
          if (releaseLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(releaseLeaseOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderLeaseId, releaseLeaseOptions.LeaseIdRequired);
          request.AddHeader(Details::HeaderVersion, releaseLeaseOptions.ApiVersionParameter);
          if (releaseLeaseOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderRequestId, releaseLeaseOptions.ClientRequestId.GetValue());
          }
          return ReleaseLeaseParseResult(context, pipeline.Send(context, request));
        }

        struct ChangeLeaseOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string LeaseIdRequired; // Specifies the current lease ID on the resource.
          Azure::Core::Nullable<std::string>
              ProposedLeaseIdOptional; // Proposed lease ID, in a GUID string format. The File
                                       // service returns 400 (Invalid request) if the proposed
                                       // lease ID is not in the correct format. See Guid
                                       // Constructor (String) for a list of valid GUID string
                                       // formats.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
        };

        static Azure::Core::Response<Models::FileChangeLeaseResult> ChangeLease(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ChangeLeaseOptions& changeLeaseOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "lease");
          request.AddHeader(Details::HeaderAction, "change");
          if (changeLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(changeLeaseOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderLeaseId, changeLeaseOptions.LeaseIdRequired);
          if (changeLeaseOptions.ProposedLeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::HeaderProposedLeaseId,
                changeLeaseOptions.ProposedLeaseIdOptional.GetValue());
          }
          request.AddHeader(Details::HeaderVersion, changeLeaseOptions.ApiVersionParameter);
          if (changeLeaseOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderRequestId, changeLeaseOptions.ClientRequestId.GetValue());
          }
          return ChangeLeaseParseResult(context, pipeline.Send(context, request));
        }

        struct BreakLeaseOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
        };

        static Azure::Core::Response<Models::FileBreakLeaseResult> BreakLease(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const BreakLeaseOptions& breakLeaseOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "lease");
          request.AddHeader(Details::HeaderAction, "break");
          if (breakLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(breakLeaseOptions.Timeout.GetValue())));
          }
          if (breakLeaseOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(Details::HeaderLeaseId, breakLeaseOptions.LeaseIdOptional.GetValue());
          }
          request.AddHeader(Details::HeaderVersion, breakLeaseOptions.ApiVersionParameter);
          if (breakLeaseOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderRequestId, breakLeaseOptions.ClientRequestId.GetValue());
          }
          return BreakLeaseParseResult(context, pipeline.Send(context, request));
        }

        struct UploadRangeOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string
              XMsRange; // Specifies the range of bytes to be written. Both the start and end of the
                        // range must be specified. For an update operation, the range can be up to
                        // 4 MB in size. For a clear operation, the range can be up to the value of
                        // the file's full size. The File service accepts only a single byte range
                        // for the Range and 'x-ms-range' headers, and the byte range must be
                        // specified in the following format: bytes=startByte-endByte.
          Models::FileRangeWriteType XMsWrite = Models::FileRangeWriteType::
              Unknown; // Specify one of the following options: - Update: Writes the bytes specified
                       // by the request body into the specified range. The Range and Content-Length
                       // headers must match to perform the update. - Clear: Clears the specified
                       // range and releases the space used in storage for that range. To clear a
                       // range, set the Content-Length header to zero, and set the Range header to
                       // a value that indicates the range to clear, up to maximum file size.
          int64_t ContentLength
              = int64_t(); // Specifies the number of bytes being transmitted in the request body.
                           // When the x-ms-write header is set to clear, the value of this header
                           // must be set to zero.
          Azure::Core::Nullable<Storage::ContentHash>
              ContentMd5; // An MD5 hash of the content. This hash is used to verify the integrity
                          // of the data during transport. When the Content-MD5 header is specified,
                          // the File service compares the hash of the content that has arrived with
                          // the header value that was sent. If the two hashes do not match, the
                          // operation will fail with error code 400 (Bad Request).
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
        };

        static Azure::Core::Response<Models::FileUploadRangeResult> UploadRange(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::BodyStream& bodyStream,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const UploadRangeOptions& uploadRangeOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url, &bodyStream);
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "range");
          if (uploadRangeOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(uploadRangeOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderXMsRange, uploadRangeOptions.XMsRange);
          request.AddHeader(
              Details::HeaderFileRangeWrite,
              FileRangeWriteTypeToString(uploadRangeOptions.XMsWrite));
          request.AddHeader(
              Details::HeaderContentLength, std::to_string(uploadRangeOptions.ContentLength));
          if (uploadRangeOptions.ContentMd5.HasValue())
          {
            request.AddHeader(
                Details::HeaderContentHash,
                Storage::Details::ToBase64String(uploadRangeOptions.ContentMd5.GetValue()));
          }
          request.AddHeader(Details::HeaderVersion, uploadRangeOptions.ApiVersionParameter);
          if (uploadRangeOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::HeaderLeaseId, uploadRangeOptions.LeaseIdOptional.GetValue());
          }
          return UploadRangeParseResult(context, pipeline.Send(context, request));
        }

        struct UploadRangeFromUrlOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string TargetRange; // Writes data to the specified byte range in the file.
          std::string
              CopySource; // Specifies the URL of the source file or blob, up to 2 KB in length. To
                          // copy a file to another file within the same storage account, you may
                          // use Shared Key to authenticate the source file. If you are copying a
                          // file from another storage account, or if you are copying a blob from
                          // the same storage account or another storage account, then you must
                          // authenticate the source file or blob using a shared access signature.
                          // If the source is a public blob, no authentication is required to
                          // perform the copy operation. A file in a share snapshot can also be
                          // specified as a copy source.
          Azure::Core::Nullable<std::string>
              SourceRange; // Bytes of source data in the specified range.
          Models::FileRangeWriteFromUrlType
              XMsWrite; // Only update is supported: - Update: Writes the bytes downloaded from the
                        // source url into the specified range.
          int64_t ContentLength
              = int64_t(); // Specifies the number of bytes being transmitted in the request body.
                           // When the x-ms-write header is set to clear, the value of this header
                           // must be set to zero.
          Azure::Core::Nullable<Storage::ContentHash>
              SourceContentCrc64; // Specify the crc64 calculated for the range of bytes that must
                                  // be read from the copy source.
          Azure::Core::Nullable<Storage::ContentHash>
              SourceIfMatchCrc64; // Specify the crc64 value to operate only on range with a
                                  // matching crc64 checksum.
          Azure::Core::Nullable<Storage::ContentHash>
              SourceIfNoneMatchCrc64; // Specify the crc64 value to operate only on range without a
                                      // matching crc64 checksum.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
        };

        static Azure::Core::Response<Models::FileUploadRangeFromUrlResult> UploadRangeFromUrl(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const UploadRangeFromUrlOptions& uploadRangeFromUrlOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "range");
          if (uploadRangeFromUrlOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(uploadRangeFromUrlOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderRange, uploadRangeFromUrlOptions.TargetRange);
          request.AddHeader(Details::HeaderCopySource, uploadRangeFromUrlOptions.CopySource);
          if (uploadRangeFromUrlOptions.SourceRange.HasValue())
          {
            request.AddHeader(
                Details::HeaderSourceRange, uploadRangeFromUrlOptions.SourceRange.GetValue());
          }
          request.AddHeader(
              Details::HeaderFileRangeWriteFromUrl,
              FileRangeWriteFromUrlTypeToString(uploadRangeFromUrlOptions.XMsWrite));
          request.AddHeader(
              Details::HeaderContentLength,
              std::to_string(uploadRangeFromUrlOptions.ContentLength));
          if (uploadRangeFromUrlOptions.SourceContentCrc64.HasValue())
          {
            request.AddHeader(
                Details::HeaderSourceContentHash,
                Storage::Details::ToBase64String(
                    uploadRangeFromUrlOptions.SourceContentCrc64.GetValue()));
          }
          if (uploadRangeFromUrlOptions.SourceIfMatchCrc64.HasValue())
          {
            request.AddHeader(
                Details::HeaderSourceIfMatchHash,
                Storage::Details::ToBase64String(
                    uploadRangeFromUrlOptions.SourceIfMatchCrc64.GetValue()));
          }
          if (uploadRangeFromUrlOptions.SourceIfNoneMatchCrc64.HasValue())
          {
            request.AddHeader(
                Details::HeaderSourceIfNoneMatchHash,
                Storage::Details::ToBase64String(
                    uploadRangeFromUrlOptions.SourceIfNoneMatchCrc64.GetValue()));
          }
          request.AddHeader(Details::HeaderVersion, uploadRangeFromUrlOptions.ApiVersionParameter);
          if (uploadRangeFromUrlOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::HeaderLeaseId, uploadRangeFromUrlOptions.LeaseIdOptional.GetValue());
          }
          return UploadRangeFromUrlParseResult(context, pipeline.Send(context, request));
        }

        struct GetRangeListOptions
        {
          Azure::Core::Nullable<std::string>
              ShareSnapshot; // The snapshot parameter is an opaque DateTime value that, when
                             // present, specifies the share snapshot to query.
          Azure::Core::Nullable<std::string>
              PrevShareSnapshot; // The previous snapshot parameter is an opaque DateTime value
                                 // that, when present, specifies the previous snapshot.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              XMsRange; // Specifies the range of bytes over which to list ranges, inclusively.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
        };

        static Azure::Core::Response<Models::FileGetRangeListResult> GetRangeList(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetRangeListOptions& getRangeListOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "rangelist");
          if (getRangeListOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    getRangeListOptions.ShareSnapshot.GetValue()));
          }
          if (getRangeListOptions.PrevShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryPrevShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    getRangeListOptions.PrevShareSnapshot.GetValue()));
          }
          if (getRangeListOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(getRangeListOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, getRangeListOptions.ApiVersionParameter);
          if (getRangeListOptions.XMsRange.HasValue())
          {
            request.AddHeader(Details::HeaderXMsRange, getRangeListOptions.XMsRange.GetValue());
          }
          if (getRangeListOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::HeaderLeaseId, getRangeListOptions.LeaseIdOptional.GetValue());
          }
          return GetRangeListParseResult(context, pipeline.Send(context, request));
        }

        struct StartCopyOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Storage::Metadata Metadata; // A name-value pair to associate with a file storage object.
          std::string
              CopySource; // Specifies the URL of the source file or blob, up to 2 KB in length. To
                          // copy a file to another file within the same storage account, you may
                          // use Shared Key to authenticate the source file. If you are copying a
                          // file from another storage account, or if you are copying a blob from
                          // the same storage account or another storage account, then you must
                          // authenticate the source file or blob using a shared access signature.
                          // If the source is a public blob, no authentication is required to
                          // perform the copy operation. A file in a share snapshot can also be
                          // specified as a copy source.
          Azure::Core::Nullable<std::string>
              FilePermission; // If specified the permission (security descriptor) shall be set for
                              // the directory/file. This header can be used if Permission size is
                              // <= 8KB, else x-ms-file-permission-key header shall be used. Default
                              // value: Inherit. If SDDL is specified as input, it must have owner,
                              // group and dacl. Note: Only one of the x-ms-file-permission or
                              // x-ms-file-permission-key should be specified.
          Azure::Core::Nullable<std::string>
              FilePermissionKey; // Key of the permission to be set for the directory/file. Note:
                                 // Only one of the x-ms-file-permission or x-ms-file-permission-key
                                 // should be specified.
          Azure::Core::Nullable<Models::PermissionCopyModeType>
              XMsFilePermissionCopyMode; // Specifies the option to copy file security descriptor
                                         // from source file or to set it using the value which is
                                         // defined by the header value of x-ms-file-permission or
                                         // x-ms-file-permission-key.
          Azure::Core::Nullable<bool>
              FileCopyIgnoreReadOnly; // Specifies the option to overwrite the target file if it
                                      // already exists and has read-only attribute set.
          Azure::Core::Nullable<std::string>
              FileCopyFileAttributes; // Specifies either the option to copy file attributes from a
                                      // source file(source) to a target file or a list of
                                      // attributes to set on a target file.
          Azure::Core::Nullable<std::string>
              FileCopyFileCreationTime; // Specifies either the option to copy file creation time
                                        // from a source file(source) to a target file or a time
                                        // value in ISO 8601 format to set as creation time on a
                                        // target file.
          Azure::Core::Nullable<std::string>
              FileCopyFileLastWriteTime; // Specifies either the option to copy file last write time
                                         // from a source file(source) to a target file or a time
                                         // value in ISO 8601 format to set as last write time on a
                                         // target file.
          Azure::Core::Nullable<bool>
              FileCopySetArchiveAttribute; // Specifies the option to set archive attribute on a
                                           // target file. True means archive attribute will be set
                                           // on a target file despite attribute overrides or a
                                           // source file state.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
        };

        static Azure::Core::Response<Models::FileStartCopyResult> StartCopy(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const StartCopyOptions& startCopyOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          if (startCopyOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(startCopyOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, startCopyOptions.ApiVersionParameter);
          for (const auto& pair : startCopyOptions.Metadata)
          {
            request.AddHeader(Details::HeaderMetadata + ("-" + pair.first), pair.second);
          }
          request.AddHeader(Details::HeaderCopySource, startCopyOptions.CopySource);
          if (startCopyOptions.FilePermission.HasValue())
          {
            request.AddHeader(
                Details::HeaderFilePermission, startCopyOptions.FilePermission.GetValue());
          }
          if (startCopyOptions.FilePermissionKey.HasValue())
          {
            request.AddHeader(
                Details::HeaderFilePermissionKey, startCopyOptions.FilePermissionKey.GetValue());
          }
          if (startCopyOptions.XMsFilePermissionCopyMode.HasValue())
          {
            request.AddHeader(
                Details::HeaderFilePermissionCopyMode,
                PermissionCopyModeTypeToString(
                    startCopyOptions.XMsFilePermissionCopyMode.GetValue()));
          }
          if (startCopyOptions.FileCopyIgnoreReadOnly.HasValue())
          {
            request.AddHeader(
                Details::HeaderIgnoreReadOnly,
                (startCopyOptions.FileCopyIgnoreReadOnly.GetValue() ? "true" : "false"));
          }
          if (startCopyOptions.FileCopyFileAttributes.HasValue())
          {
            request.AddHeader(
                Details::HeaderFileAttributes, startCopyOptions.FileCopyFileAttributes.GetValue());
          }
          if (startCopyOptions.FileCopyFileCreationTime.HasValue())
          {
            request.AddHeader(
                Details::HeaderFileCreatedOn, startCopyOptions.FileCopyFileCreationTime.GetValue());
          }
          if (startCopyOptions.FileCopyFileLastWriteTime.HasValue())
          {
            request.AddHeader(
                Details::HeaderFileLastWrittenOn,
                startCopyOptions.FileCopyFileLastWriteTime.GetValue());
          }
          if (startCopyOptions.FileCopySetArchiveAttribute.HasValue())
          {
            request.AddHeader(
                Details::HeaderSetArchiveAttribute,
                (startCopyOptions.FileCopySetArchiveAttribute.GetValue() ? "true" : "false"));
          }
          if (startCopyOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(Details::HeaderLeaseId, startCopyOptions.LeaseIdOptional.GetValue());
          }
          return StartCopyParseResult(context, pipeline.Send(context, request));
        }

        struct AbortCopyOptions
        {
          std::string CopyId; // The copy identifier provided in the x-ms-copy-id header of the
                              // original Copy File operation.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
        };

        static Azure::Core::Response<Models::FileAbortCopyResult> AbortCopy(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const AbortCopyOptions& abortCopyOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "copy");
          request.GetUrl().AppendQueryParameter(
              Details::QueryCopyId,
              Storage::Details::UrlEncodeQueryParameter(abortCopyOptions.CopyId));
          if (abortCopyOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(abortCopyOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderCopyActionAbortConstant, "abort");
          request.AddHeader(Details::HeaderVersion, abortCopyOptions.ApiVersionParameter);
          if (abortCopyOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(Details::HeaderLeaseId, abortCopyOptions.LeaseIdOptional.GetValue());
          }
          return AbortCopyParseResult(context, pipeline.Send(context, request));
        }

        struct ListHandlesOptions
        {
          Azure::Core::Nullable<std::string>
              ContinuationToken; // A string value that identifies the portion of the list to be
                                 // returned with the next list operation. The operation returns a
                                 // marker value within the response body if the list returned was
                                 // not complete. The marker value may then be used in a subsequent
                                 // call to request the next set of list items. The marker value is
                                 // opaque to the client.
          Azure::Core::Nullable<int32_t>
              MaxResults; // Specifies the maximum number of entries to return. If the request does
                          // not specify maxresults, or specifies a value greater than 5,000, the
                          // server will return up to 5,000 items.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          Azure::Core::Nullable<std::string>
              ShareSnapshot; // The snapshot parameter is an opaque DateTime value that, when
                             // present, specifies the share snapshot to query.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        };

        static Azure::Core::Response<Models::FileListHandlesResult> ListHandles(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ListHandlesOptions& listHandlesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "listhandles");
          if (listHandlesOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryContinuationToken,
                Storage::Details::UrlEncodeQueryParameter(
                    listHandlesOptions.ContinuationToken.GetValue()));
          }
          if (listHandlesOptions.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryMaxResults,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listHandlesOptions.MaxResults.GetValue())));
          }
          if (listHandlesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listHandlesOptions.Timeout.GetValue())));
          }
          if (listHandlesOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    listHandlesOptions.ShareSnapshot.GetValue()));
          }
          request.AddHeader(Details::HeaderVersion, listHandlesOptions.ApiVersionParameter);
          return ListHandlesParseResult(context, pipeline.Send(context, request));
        }

        struct ForceCloseHandlesOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          Azure::Core::Nullable<std::string>
              ContinuationToken; // A string value that identifies the portion of the list to be
                                 // returned with the next list operation. The operation returns a
                                 // marker value within the response body if the list returned was
                                 // not complete. The marker value may then be used in a subsequent
                                 // call to request the next set of list items. The marker value is
                                 // opaque to the client.
          Azure::Core::Nullable<std::string>
              ShareSnapshot; // The snapshot parameter is an opaque DateTime value that, when
                             // present, specifies the share snapshot to query.
          std::string HandleId; // Specifies handle ID opened on the file or directory to be closed.
                                // Asterisk (*) is a wildcard that specifies all handles.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        };

        static Azure::Core::Response<Models::FileForceCloseHandlesResult> ForceCloseHandles(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ForceCloseHandlesOptions& forceCloseHandlesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "forceclosehandles");
          if (forceCloseHandlesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(forceCloseHandlesOptions.Timeout.GetValue())));
          }
          if (forceCloseHandlesOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryContinuationToken,
                Storage::Details::UrlEncodeQueryParameter(
                    forceCloseHandlesOptions.ContinuationToken.GetValue()));
          }
          if (forceCloseHandlesOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    forceCloseHandlesOptions.ShareSnapshot.GetValue()));
          }
          request.AddHeader(Details::HeaderHandleId, forceCloseHandlesOptions.HandleId);
          request.AddHeader(Details::HeaderVersion, forceCloseHandlesOptions.ApiVersionParameter);
          return ForceCloseHandlesParseResult(context, pipeline.Send(context, request));
        }

      private:
        static Azure::Core::Response<Models::FileCreateResult> CreateParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // Success, File created.
            Models::FileCreateResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            result.IsServerEncrypted
                = response.GetHeaders().at(Details::HeaderRequestIsServerEncrypted) == "true";
            result.FilePermissionKey = response.GetHeaders().at(Details::HeaderFilePermissionKey);
            result.FileAttributes = response.GetHeaders().at(Details::HeaderFileAttributes);
            result.FileCreatedOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileCreatedOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FileLastWrittenOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileLastWrittenOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FileChangedOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileChangedOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FileId = response.GetHeaders().at(Details::HeaderFileId);
            result.FileParentId = response.GetHeaders().at(Details::HeaderFileParentId);
            return Azure::Core::Response<Models::FileCreateResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::FileDownloadResult> DownloadParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Succeeded to read the entire file.
            Models::FileDownloadResult result;
            result.BodyStream = response.GetBodyStream();
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);

            for (auto i = response.GetHeaders().lower_bound(Details::HeaderMetadata);
                 i != response.GetHeaders().end()
                 && i->first.substr(0, 9) == Details::HeaderMetadata;
                 ++i)
            {
              result.Metadata.emplace(i->first.substr(10), i->second);
            }
            result.ContentLength
                = std::stoll(response.GetHeaders().at(Details::HeaderContentLength));
            result.HttpHeaders.ContentType = response.GetHeaders().at(Details::HeaderContentType);
            if (response.GetHeaders().find(Details::HeaderContentRange)
                != response.GetHeaders().end())
            {
              result.ContentRange = response.GetHeaders().at(Details::HeaderContentRange);
            }
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            if (response.GetHeaders().find(Details::HeaderTransactionalContentHash)
                != response.GetHeaders().end())
            {
              result.TransactionalContentHash = Storage::Details::FromBase64String(
                  response.GetHeaders().at(Details::HeaderTransactionalContentHash),
                  HashAlgorithm::Md5);
            }
            if (response.GetHeaders().find(Details::HeaderContentEncoding)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentEncoding
                  = response.GetHeaders().at(Details::HeaderContentEncoding);
            }
            if (response.GetHeaders().find(Details::HeaderCacheControl)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.CacheControl
                  = response.GetHeaders().at(Details::HeaderCacheControl);
            }
            if (response.GetHeaders().find(Details::HeaderContentDisposition)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentDisposition
                  = response.GetHeaders().at(Details::HeaderContentDisposition);
            }
            if (response.GetHeaders().find(Details::HeaderContentLanguage)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentLanguage
                  = response.GetHeaders().at(Details::HeaderContentLanguage);
            }
            result.AcceptRanges = response.GetHeaders().at(Details::HeaderAcceptRanges);
            if (response.GetHeaders().find(Details::HeaderCopyCompletedOn)
                != response.GetHeaders().end())
            {
              result.CopyCompletedOn = Core::DateTime::Parse(
                  response.GetHeaders().at(Details::HeaderCopyCompletedOn),
                  Core::DateTime::DateFormat::Rfc1123);
            }
            if (response.GetHeaders().find(Details::HeaderCopyStatusDescription)
                != response.GetHeaders().end())
            {
              result.CopyStatusDescription
                  = response.GetHeaders().at(Details::HeaderCopyStatusDescription);
            }
            if (response.GetHeaders().find(Details::HeaderCopyId) != response.GetHeaders().end())
            {
              result.CopyId = response.GetHeaders().at(Details::HeaderCopyId);
            }
            if (response.GetHeaders().find(Details::HeaderCopyProgress)
                != response.GetHeaders().end())
            {
              result.CopyProgress = response.GetHeaders().at(Details::HeaderCopyProgress);
            }
            if (response.GetHeaders().find(Details::HeaderCopySource)
                != response.GetHeaders().end())
            {
              result.CopySource = response.GetHeaders().at(Details::HeaderCopySource);
            }
            if (response.GetHeaders().find(Details::HeaderCopyStatus)
                != response.GetHeaders().end())
            {
              result.CopyStatus
                  = CopyStatusTypeFromString(response.GetHeaders().at(Details::HeaderCopyStatus));
            }
            if (response.GetHeaders().find(Details::HeaderContentHash)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentHash = Storage::Details::FromBase64String(
                  response.GetHeaders().at(Details::HeaderContentHash), HashAlgorithm::Md5);
            }
            if (response.GetHeaders().find(Details::HeaderIsServerEncrypted)
                != response.GetHeaders().end())
            {
              result.IsServerEncrypted
                  = response.GetHeaders().at(Details::HeaderIsServerEncrypted) == "true";
            }
            result.FileAttributes = response.GetHeaders().at(Details::HeaderFileAttributes);
            result.FileCreatedOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileCreatedOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FileLastWrittenOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileLastWrittenOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FileChangedOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileChangedOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FilePermissionKey = response.GetHeaders().at(Details::HeaderFilePermissionKey);
            result.FileId = response.GetHeaders().at(Details::HeaderFileId);
            result.FileParentId = response.GetHeaders().at(Details::HeaderFileParentId);
            if (response.GetHeaders().find(Details::HeaderLeaseDuration)
                != response.GetHeaders().end())
            {
              result.LeaseDuration = LeaseDurationTypeFromString(
                  response.GetHeaders().at(Details::HeaderLeaseDuration));
            }
            if (response.GetHeaders().find(Details::HeaderLeaseState)
                != response.GetHeaders().end())
            {
              result.LeaseState
                  = LeaseStateTypeFromString(response.GetHeaders().at(Details::HeaderLeaseState));
            }
            if (response.GetHeaders().find(Details::HeaderLeaseStatus)
                != response.GetHeaders().end())
            {
              result.LeaseStatus
                  = LeaseStatusTypeFromString(response.GetHeaders().at(Details::HeaderLeaseStatus));
            }
            return Azure::Core::Response<Models::FileDownloadResult>(
                std::move(result), std::move(responsePtr));
          }
          else if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::PartialContent)
          {
            // Succeeded to read a specified range of the file.
            Models::FileDownloadResult result;
            result.BodyStream = response.GetBodyStream();
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);

            for (auto i = response.GetHeaders().lower_bound(Details::HeaderMetadata);
                 i != response.GetHeaders().end()
                 && i->first.substr(0, 9) == Details::HeaderMetadata;
                 ++i)
            {
              result.Metadata.emplace(i->first.substr(10), i->second);
            }
            result.ContentLength
                = std::stoll(response.GetHeaders().at(Details::HeaderContentLength));
            result.HttpHeaders.ContentType = response.GetHeaders().at(Details::HeaderContentType);
            if (response.GetHeaders().find(Details::HeaderContentRange)
                != response.GetHeaders().end())
            {
              result.ContentRange = response.GetHeaders().at(Details::HeaderContentRange);
            }
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            if (response.GetHeaders().find(Details::HeaderTransactionalContentHash)
                != response.GetHeaders().end())
            {
              result.TransactionalContentHash = Storage::Details::FromBase64String(
                  response.GetHeaders().at(Details::HeaderTransactionalContentHash),
                  HashAlgorithm::Md5);
            }
            if (response.GetHeaders().find(Details::HeaderContentEncoding)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentEncoding
                  = response.GetHeaders().at(Details::HeaderContentEncoding);
            }
            if (response.GetHeaders().find(Details::HeaderCacheControl)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.CacheControl
                  = response.GetHeaders().at(Details::HeaderCacheControl);
            }
            if (response.GetHeaders().find(Details::HeaderContentDisposition)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentDisposition
                  = response.GetHeaders().at(Details::HeaderContentDisposition);
            }
            if (response.GetHeaders().find(Details::HeaderContentLanguage)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentLanguage
                  = response.GetHeaders().at(Details::HeaderContentLanguage);
            }
            result.AcceptRanges = response.GetHeaders().at(Details::HeaderAcceptRanges);
            if (response.GetHeaders().find(Details::HeaderCopyCompletedOn)
                != response.GetHeaders().end())
            {
              result.CopyCompletedOn = Core::DateTime::Parse(
                  response.GetHeaders().at(Details::HeaderCopyCompletedOn),
                  Core::DateTime::DateFormat::Rfc1123);
            }
            if (response.GetHeaders().find(Details::HeaderCopyStatusDescription)
                != response.GetHeaders().end())
            {
              result.CopyStatusDescription
                  = response.GetHeaders().at(Details::HeaderCopyStatusDescription);
            }
            if (response.GetHeaders().find(Details::HeaderCopyId) != response.GetHeaders().end())
            {
              result.CopyId = response.GetHeaders().at(Details::HeaderCopyId);
            }
            if (response.GetHeaders().find(Details::HeaderCopyProgress)
                != response.GetHeaders().end())
            {
              result.CopyProgress = response.GetHeaders().at(Details::HeaderCopyProgress);
            }
            if (response.GetHeaders().find(Details::HeaderCopySource)
                != response.GetHeaders().end())
            {
              result.CopySource = response.GetHeaders().at(Details::HeaderCopySource);
            }
            if (response.GetHeaders().find(Details::HeaderCopyStatus)
                != response.GetHeaders().end())
            {
              result.CopyStatus
                  = CopyStatusTypeFromString(response.GetHeaders().at(Details::HeaderCopyStatus));
            }
            if (response.GetHeaders().find(Details::HeaderContentHash)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentHash = Storage::Details::FromBase64String(
                  response.GetHeaders().at(Details::HeaderContentHash), HashAlgorithm::Md5);
            }
            if (response.GetHeaders().find(Details::HeaderIsServerEncrypted)
                != response.GetHeaders().end())
            {
              result.IsServerEncrypted
                  = response.GetHeaders().at(Details::HeaderIsServerEncrypted) == "true";
            }
            result.FileAttributes = response.GetHeaders().at(Details::HeaderFileAttributes);
            result.FileCreatedOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileCreatedOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FileLastWrittenOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileLastWrittenOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FileChangedOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileChangedOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FilePermissionKey = response.GetHeaders().at(Details::HeaderFilePermissionKey);
            result.FileId = response.GetHeaders().at(Details::HeaderFileId);
            result.FileParentId = response.GetHeaders().at(Details::HeaderFileParentId);
            if (response.GetHeaders().find(Details::HeaderLeaseDuration)
                != response.GetHeaders().end())
            {
              result.LeaseDuration = LeaseDurationTypeFromString(
                  response.GetHeaders().at(Details::HeaderLeaseDuration));
            }
            if (response.GetHeaders().find(Details::HeaderLeaseState)
                != response.GetHeaders().end())
            {
              result.LeaseState
                  = LeaseStateTypeFromString(response.GetHeaders().at(Details::HeaderLeaseState));
            }
            if (response.GetHeaders().find(Details::HeaderLeaseStatus)
                != response.GetHeaders().end())
            {
              result.LeaseStatus
                  = LeaseStatusTypeFromString(response.GetHeaders().at(Details::HeaderLeaseStatus));
            }
            return Azure::Core::Response<Models::FileDownloadResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::FileGetPropertiesResult> GetPropertiesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success.
            Models::FileGetPropertiesResult result;
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);

            for (auto i = response.GetHeaders().lower_bound(Details::HeaderMetadata);
                 i != response.GetHeaders().end()
                 && i->first.substr(0, 9) == Details::HeaderMetadata;
                 ++i)
            {
              result.Metadata.emplace(i->first.substr(10), i->second);
            }
            result.FileType = response.GetHeaders().at(Details::HeaderFileType);
            result.ContentLength
                = std::stoll(response.GetHeaders().at(Details::HeaderContentLength));
            if (response.GetHeaders().find(Details::HeaderContentType)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentType = response.GetHeaders().at(Details::HeaderContentType);
            }
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            if (response.GetHeaders().find(Details::HeaderTransactionalContentHash)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentHash = Storage::Details::FromBase64String(
                  response.GetHeaders().at(Details::HeaderTransactionalContentHash),
                  HashAlgorithm::Md5);
            }
            if (response.GetHeaders().find(Details::HeaderContentEncoding)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentEncoding
                  = response.GetHeaders().at(Details::HeaderContentEncoding);
            }
            if (response.GetHeaders().find(Details::HeaderCacheControl)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.CacheControl
                  = response.GetHeaders().at(Details::HeaderCacheControl);
            }
            if (response.GetHeaders().find(Details::HeaderContentDisposition)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentDisposition
                  = response.GetHeaders().at(Details::HeaderContentDisposition);
            }
            if (response.GetHeaders().find(Details::HeaderContentLanguage)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentLanguage
                  = response.GetHeaders().at(Details::HeaderContentLanguage);
            }
            if (response.GetHeaders().find(Details::HeaderCopyCompletedOn)
                != response.GetHeaders().end())
            {
              result.CopyCompletedOn = Core::DateTime::Parse(
                  response.GetHeaders().at(Details::HeaderCopyCompletedOn),
                  Core::DateTime::DateFormat::Rfc1123);
            }
            if (response.GetHeaders().find(Details::HeaderCopyStatusDescription)
                != response.GetHeaders().end())
            {
              result.CopyStatusDescription
                  = response.GetHeaders().at(Details::HeaderCopyStatusDescription);
            }
            if (response.GetHeaders().find(Details::HeaderCopyId) != response.GetHeaders().end())
            {
              result.CopyId = response.GetHeaders().at(Details::HeaderCopyId);
            }
            if (response.GetHeaders().find(Details::HeaderCopyProgress)
                != response.GetHeaders().end())
            {
              result.CopyProgress = response.GetHeaders().at(Details::HeaderCopyProgress);
            }
            if (response.GetHeaders().find(Details::HeaderCopySource)
                != response.GetHeaders().end())
            {
              result.CopySource = response.GetHeaders().at(Details::HeaderCopySource);
            }
            if (response.GetHeaders().find(Details::HeaderCopyStatus)
                != response.GetHeaders().end())
            {
              result.CopyStatus
                  = CopyStatusTypeFromString(response.GetHeaders().at(Details::HeaderCopyStatus));
            }
            if (response.GetHeaders().find(Details::HeaderIsServerEncrypted)
                != response.GetHeaders().end())
            {
              result.IsServerEncrypted
                  = response.GetHeaders().at(Details::HeaderIsServerEncrypted) == "true";
            }
            result.FileAttributes = response.GetHeaders().at(Details::HeaderFileAttributes);
            result.FileCreatedOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileCreatedOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FileLastWrittenOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileLastWrittenOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FileChangedOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileChangedOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FilePermissionKey = response.GetHeaders().at(Details::HeaderFilePermissionKey);
            result.FileId = response.GetHeaders().at(Details::HeaderFileId);
            result.FileParentId = response.GetHeaders().at(Details::HeaderFileParentId);
            if (response.GetHeaders().find(Details::HeaderLeaseDuration)
                != response.GetHeaders().end())
            {
              result.LeaseDuration = LeaseDurationTypeFromString(
                  response.GetHeaders().at(Details::HeaderLeaseDuration));
            }
            if (response.GetHeaders().find(Details::HeaderLeaseState)
                != response.GetHeaders().end())
            {
              result.LeaseState
                  = LeaseStateTypeFromString(response.GetHeaders().at(Details::HeaderLeaseState));
            }
            if (response.GetHeaders().find(Details::HeaderLeaseStatus)
                != response.GetHeaders().end())
            {
              result.LeaseStatus
                  = LeaseStatusTypeFromString(response.GetHeaders().at(Details::HeaderLeaseStatus));
            }
            return Azure::Core::Response<Models::FileGetPropertiesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::FileDeleteResult> DeleteParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
          {
            // Success (Accepted).
            Models::FileDeleteResult result;
            return Azure::Core::Response<Models::FileDeleteResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::FileSetHttpHeadersResult> SetHttpHeadersParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success
            Models::FileSetHttpHeadersResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            result.IsServerEncrypted
                = response.GetHeaders().at(Details::HeaderRequestIsServerEncrypted) == "true";
            result.FilePermissionKey = response.GetHeaders().at(Details::HeaderFilePermissionKey);
            result.FileAttributes = response.GetHeaders().at(Details::HeaderFileAttributes);
            result.FileCreatedOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileCreatedOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FileLastWrittenOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileLastWrittenOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FileChangedOn = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderFileChangedOn),
                Core::DateTime::DateFormat::Rfc3339);
            result.FileId = response.GetHeaders().at(Details::HeaderFileId);
            result.FileParentId = response.GetHeaders().at(Details::HeaderFileParentId);
            return Azure::Core::Response<Models::FileSetHttpHeadersResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::FileSetMetadataResult> SetMetadataParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success (OK).
            Models::FileSetMetadataResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.IsServerEncrypted
                = response.GetHeaders().at(Details::HeaderRequestIsServerEncrypted) == "true";
            return Azure::Core::Response<Models::FileSetMetadataResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::FileAcquireLeaseResult> AcquireLeaseParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // The Acquire operation completed successfully.
            Models::FileAcquireLeaseResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            result.LeaseId = response.GetHeaders().at(Details::HeaderLeaseId);
            return Azure::Core::Response<Models::FileAcquireLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::FileReleaseLeaseResult> ReleaseLeaseParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // The Release operation completed successfully.
            Models::FileReleaseLeaseResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            return Azure::Core::Response<Models::FileReleaseLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::FileChangeLeaseResult> ChangeLeaseParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // The Change operation completed successfully.
            Models::FileChangeLeaseResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            result.LeaseId = response.GetHeaders().at(Details::HeaderLeaseId);
            return Azure::Core::Response<Models::FileChangeLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::FileBreakLeaseResult> BreakLeaseParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
          {
            // The Break operation completed successfully.
            Models::FileBreakLeaseResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            if (response.GetHeaders().find(Details::HeaderLeaseId) != response.GetHeaders().end())
            {
              result.LeaseId = response.GetHeaders().at(Details::HeaderLeaseId);
            }
            return Azure::Core::Response<Models::FileBreakLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::FileUploadRangeResult> UploadRangeParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // Success (Created).
            Models::FileUploadRangeResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            if (response.GetHeaders().find(Details::HeaderTransactionalContentHash)
                != response.GetHeaders().end())
            {
              result.TransactionalContentHash = Storage::Details::FromBase64String(
                  response.GetHeaders().at(Details::HeaderTransactionalContentHash),
                  HashAlgorithm::Md5);
            }
            if (response.GetHeaders().find(Details::HeaderRequestIsServerEncrypted)
                != response.GetHeaders().end())
            {
              result.IsServerEncrypted
                  = response.GetHeaders().at(Details::HeaderRequestIsServerEncrypted) == "true";
            }
            return Azure::Core::Response<Models::FileUploadRangeResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::FileUploadRangeFromUrlResult>
        UploadRangeFromUrlParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // Success (Created).
            Models::FileUploadRangeFromUrlResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            result.TransactionalContentHash = Storage::Details::FromBase64String(
                response.GetHeaders().at(Details::HeaderTransactionalContentHash),
                HashAlgorithm::Crc64);
            result.IsServerEncrypted
                = response.GetHeaders().at(Details::HeaderRequestIsServerEncrypted) == "true";
            return Azure::Core::Response<Models::FileUploadRangeFromUrlResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::FileGetRangeListResult> GetRangeListParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success.
            const auto& bodyBuffer = response.GetBody();
            auto reader = Storage::Details::XmlReader(
                reinterpret_cast<const char*>(bodyBuffer.data()), bodyBuffer.size());
            Models::FileGetRangeListResult result = bodyBuffer.empty()
                ? Models::FileGetRangeListResult()
                : FileGetRangeListResultFromShareFileRangeList(ShareFileRangeListFromXml(reader));
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.FileContentLength
                = std::stoll(response.GetHeaders().at(Details::HeaderXMsContentLength));
            return Azure::Core::Response<Models::FileGetRangeListResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Models::FileRange FileRangeFromXml(Storage::Details::XmlReader& reader)
        {
          auto result = Models::FileRange();
          enum class XmlTagName
          {
            End,
            Start,
            Unknown,
          };
          std::vector<XmlTagName> path;

          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
            {

              if (std::strcmp(node.Name, "End") == 0)
              {
                path.emplace_back(XmlTagName::End);
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
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::End)
              {
                result.End = std::stoll(node.Value);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::Start)
              {
                result.Start = std::stoll(node.Value);
              }
            }
          }
          return result;
        }

        static Models::ClearRange ClearRangeFromXml(Storage::Details::XmlReader& reader)
        {
          auto result = Models::ClearRange();
          enum class XmlTagName
          {
            End,
            Start,
            Unknown,
          };
          std::vector<XmlTagName> path;

          while (true)
          {
            auto node = reader.Read();
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
            {

              if (std::strcmp(node.Name, "End") == 0)
              {
                path.emplace_back(XmlTagName::End);
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
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::End)
              {
                result.End = std::stoll(node.Value);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::Start)
              {
                result.Start = std::stoll(node.Value);
              }
            }
          }
          return result;
        }

        static Models::ShareFileRangeList ShareFileRangeListFromXml(
            Storage::Details::XmlReader& reader)
        {
          auto result = Models::ShareFileRangeList();
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
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
                result.Ranges.emplace_back(FileRangeFromXml(reader));
                path.pop_back();
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::Ranges
                  && path[1] == XmlTagName::ClearRange)
              {
                result.ClearRanges.emplace_back(ClearRangeFromXml(reader));
                path.pop_back();
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
            }
          }
          return result;
        }

        static Models::FileGetRangeListResult FileGetRangeListResultFromShareFileRangeList(
            Models::ShareFileRangeList object)
        {
          Models::FileGetRangeListResult result;
          result.Ranges = std::move(object.Ranges);
          result.ClearRanges = std::move(object.ClearRanges);

          return result;
        }
        static Azure::Core::Response<Models::FileStartCopyResult> StartCopyParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
          {
            // The copy file has been accepted with the specified copy status.
            Models::FileStartCopyResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            if (response.GetHeaders().find(Details::HeaderCopyId) != response.GetHeaders().end())
            {
              result.CopyId = response.GetHeaders().at(Details::HeaderCopyId);
            }
            if (response.GetHeaders().find(Details::HeaderCopyStatus)
                != response.GetHeaders().end())
            {
              result.CopyStatus
                  = CopyStatusTypeFromString(response.GetHeaders().at(Details::HeaderCopyStatus));
            }
            return Azure::Core::Response<Models::FileStartCopyResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::FileAbortCopyResult> AbortCopyParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::NoContent)
          {
            // The delete request was accepted and the file will be deleted.
            Models::FileAbortCopyResult result;
            return Azure::Core::Response<Models::FileAbortCopyResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::FileListHandlesResult> ListHandlesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success.
            const auto& bodyBuffer = response.GetBody();
            auto reader = Storage::Details::XmlReader(
                reinterpret_cast<const char*>(bodyBuffer.data()), bodyBuffer.size());
            Models::FileListHandlesResult result = bodyBuffer.empty()
                ? Models::FileListHandlesResult()
                : FileListHandlesResultFromListHandlesResponse(ListHandlesResponseFromXml(reader));
            result.HttpHeaders.ContentType = response.GetHeaders().at(Details::HeaderContentType);
            return Azure::Core::Response<Models::FileListHandlesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Models::HandleItem HandleItemFromXml(Storage::Details::XmlReader& reader)
        {
          auto result = Models::HandleItem();
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
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::Text)
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
                    = Core::DateTime::Parse(node.Value, Core::DateTime::DateFormat::Rfc1123);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::OpenTime)
              {
                result.OpenedOn
                    = Core::DateTime::Parse(node.Value, Core::DateTime::DateFormat::Rfc1123);
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

        static Models::ListHandlesResponse ListHandlesResponseFromXml(
            Storage::Details::XmlReader& reader)
        {
          auto result = Models::ListHandlesResponse();
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
            if (node.Type == Storage::Details::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == Storage::Details::XmlNodeType::EndTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::StartTag)
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
            else if (node.Type == Storage::Details::XmlNodeType::Text)
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

        static Models::FileListHandlesResult FileListHandlesResultFromListHandlesResponse(
            Models::ListHandlesResponse object)
        {
          Models::FileListHandlesResult result;
          result.HandleList = std::move(object.HandleList);
          result.ContinuationToken = std::move(object.ContinuationToken);

          return result;
        }
        static Azure::Core::Response<Models::FileForceCloseHandlesResult>
        ForceCloseHandlesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Success.
            Models::FileForceCloseHandlesResult result;
            if (response.GetHeaders().find(Details::HeaderContinuationToken)
                != response.GetHeaders().end())
            {
              result.ContinuationToken = response.GetHeaders().at(Details::HeaderContinuationToken);
            }
            result.numberOfHandlesClosed
                = std::stoi(response.GetHeaders().at(Details::HeaderNumberOfHandlesClosed));
            result.numberOfHandlesFailedToClose
                = std::stoi(response.GetHeaders().at(Details::HeaderNumberOfHandlesFailedToClose));
            return Azure::Core::Response<Models::FileForceCloseHandlesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }
      };

    }; // class ShareRestClient

  } // namespace Details

}}}} // namespace Azure::Storage::Files::Shares
