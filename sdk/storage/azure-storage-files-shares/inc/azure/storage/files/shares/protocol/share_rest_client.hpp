
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/http/http.hpp"
#include "azure/core/http/pipeline.hpp"
#include "azure/core/nullable.hpp"
#include "azure/core/response.hpp"
#include "azure/storage/common/crypt.hpp"
#include "azure/storage/common/json.hpp"
#include "azure/storage/common/storage_common.hpp"
#include "azure/storage/common/storage_exception.hpp"
#include "azure/storage/common/xml_wrapper.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  namespace Details {
    constexpr static const char* c_DefaultServiceApiVersion = "2020-02-10";
    constexpr static const char* c_QueryCopyId = "copyid";
    constexpr static const char* c_QueryListSharesInclude = "include";
    constexpr static const char* c_QueryContinuationToken = "marker";
    constexpr static const char* c_QueryMaxResults = "maxresults";
    constexpr static const char* c_QueryPrefix = "prefix";
    constexpr static const char* c_QueryPrevShareSnapshot = "prevsharesnapshot";
    constexpr static const char* c_QueryShareSnapshot = "sharesnapshot";
    constexpr static const char* c_QueryTimeout = "timeout";
    constexpr static const char* c_QueryRestype = "restype";
    constexpr static const char* c_QueryComp = "comp";
    constexpr static const char* c_HeaderVersion = "x-ms-version";
    constexpr static const char* c_HeaderRequestId = "x-ms-client-request-id";
    constexpr static const char* c_HeaderContentLength = "content-length";
    constexpr static const char* c_HeaderContentMd5 = "content-md5";
    constexpr static const char* c_HeaderCopyActionAbortConstant = "x-ms-copy-action";
    constexpr static const char* c_HeaderCopySource = "x-ms-copy-source";
    constexpr static const char* c_HeaderFilePermissionCopyMode = "x-ms-file-permission-copy-mode";
    constexpr static const char* c_HeaderIgnoreReadOnly = "x-ms-file-copy-ignore-read-only";
    constexpr static const char* c_HeaderFileAttributes = "x-ms-file-attributes";
    constexpr static const char* c_HeaderFileCreationTime = "x-ms-file-creation-time";
    constexpr static const char* c_HeaderFileLastWriteTime = "x-ms-file-last-write-time";
    constexpr static const char* c_HeaderSetArchiveAttribute = "x-ms-file-copy-set-archive";
    constexpr static const char* c_HeaderDeletedShareName = "x-ms-deleted-share-name";
    constexpr static const char* c_HeaderDeletedShareVersion = "x-ms-deleted-share-version";
    constexpr static const char* c_HeaderDeleteSnapshots = "x-ms-delete-snapshots";
    constexpr static const char* c_HeaderFileCacheControl = "x-ms-cache-control";
    constexpr static const char* c_HeaderFileContentDisposition = "x-ms-content-disposition";
    constexpr static const char* c_HeaderFileContentEncoding = "x-ms-content-encoding";
    constexpr static const char* c_HeaderFileContentLanguage = "x-ms-content-language";
    constexpr static const char* c_HeaderFileContentType = "x-ms-content-type";
    constexpr static const char* c_HeaderFilePermission = "x-ms-file-permission";
    constexpr static const char* c_HeaderFilePermissionKey = "x-ms-file-permission-key";
    constexpr static const char* c_HeaderFileRangeWriteFromUrl = "x-ms-write";
    constexpr static const char* c_HeaderFileRangeWriteFromUrlDefault = "update";
    constexpr static const char* c_HeaderFileTypeConstant = "x-ms-type";
    constexpr static const char* c_HeaderRangeGetContentMd5 = "x-ms-range-get-content-md5";
    constexpr static const char* c_HeaderHandleId = "x-ms-handle-id";
    constexpr static const char* c_HeaderBreakPeriod = "x-ms-lease-break-period";
    constexpr static const char* c_HeaderDuration = "x-ms-lease-duration";
    constexpr static const char* c_HeaderLeaseId = "x-ms-lease-id";
    constexpr static const char* c_HeaderMetadata = "x-ms-meta";
    constexpr static const char* c_HeaderProposedLeaseId = "x-ms-proposed-lease-id";
    constexpr static const char* c_HeaderRange = "x-ms-range";
    constexpr static const char* c_HeaderRecursive = "x-ms-recursive";
    constexpr static const char* c_HeaderQuota = "x-ms-share-quota";
    constexpr static const char* c_HeaderSourceContentCrc64 = "x-ms-source-content-crc64";
    constexpr static const char* c_HeaderSourceIfMatchCrc64 = "x-ms-source-if-match-crc64";
    constexpr static const char* c_HeaderSourceIfNoneMatchCrc64 = "x-ms-source-if-none-match-crc64";
    constexpr static const char* c_HeaderSourceRange = "x-ms-source-range";
    constexpr static const char* c_HeaderErrorCode = "x-ms-error-code";
    constexpr static const char* c_HeaderETag = "etag";
    constexpr static const char* c_HeaderLastModified = "last-modified";
    constexpr static const char* c_HeaderDate = "date";
    constexpr static const char* c_HeaderProvisionedIops = "x-ms-share-provisioned-iops";
    constexpr static const char* c_HeaderProvisionedIngressMBps
        = "x-ms-share-provisioned-ingress-mbps";
    constexpr static const char* c_HeaderProvisionedEgressMBps
        = "x-ms-share-provisioned-egress-mbps";
    constexpr static const char* c_HeaderNextAllowedQuotaDowngradeTime
        = "x-ms-share-next-allowed-quota-downgrade-time";
    constexpr static const char* c_HeaderLeaseDuration = "x-ms-lease-duration";
    constexpr static const char* c_HeaderLeaseState = "x-ms-lease-state";
    constexpr static const char* c_HeaderLeaseStatus = "x-ms-lease-status";
    constexpr static const char* c_HeaderLeaseTime = "x-ms-lease-time";
    constexpr static const char* c_HeaderClientRequestId = "x-ms-client-request-id";
    constexpr static const char* c_HeaderAction = "x-ms-lease-action";
    constexpr static const char* c_HeaderSnapshot = "x-ms-snapshot";
    constexpr static const char* c_HeaderRequestIsServerEncrypted = "x-ms-request-server-encrypted";
    constexpr static const char* c_HeaderFileChangeTime = "x-ms-file-change-time";
    constexpr static const char* c_HeaderFileId = "x-ms-file-id";
    constexpr static const char* c_HeaderFileParentId = "x-ms-file-parent-id";
    constexpr static const char* c_HeaderIsServerEncrypted = "x-ms-server-encrypted";
    constexpr static const char* c_HeaderContentType = "content-type";
    constexpr static const char* c_HeaderContinuationToken = "x-ms-marker";
    constexpr static const char* c_HeaderNumberOfHandlesClosed = "x-ms-number-of-handles-closed";
    constexpr static const char* c_HeaderNumberOfHandlesFailedToClose
        = "x-ms-number-of-handles-failed";
    constexpr static const char* c_HeaderXMsContentLength = "x-ms-content-length";
    constexpr static const char* c_HeaderContentRange = "content-range";
    constexpr static const char* c_HeaderTransactionalContentMd5 = "content-md5";
    constexpr static const char* c_HeaderContentEncoding = "content-encoding";
    constexpr static const char* c_HeaderCacheControl = "cache-control";
    constexpr static const char* c_HeaderContentDisposition = "content-disposition";
    constexpr static const char* c_HeaderContentLanguage = "content-language";
    constexpr static const char* c_HeaderAcceptRanges = "accept-ranges";
    constexpr static const char* c_HeaderCopyCompletionTime = "x-ms-copy-completion-time";
    constexpr static const char* c_HeaderCopyStatusDescription = "x-ms-copy-status-description";
    constexpr static const char* c_HeaderCopyId = "x-ms-copy-id";
    constexpr static const char* c_HeaderCopyProgress = "x-ms-copy-progress";
    constexpr static const char* c_HeaderCopyStatus = "x-ms-copy-status";
    constexpr static const char* c_HeaderFileType = "x-ms-type";
    constexpr static const char* c_HeaderXMsRange = "x-ms-range";
    constexpr static const char* c_HeaderFileRangeWrite = "x-ms-write";
    constexpr static const char* c_HeaderFileRangeWriteTypeDefault = "update";
    constexpr static const char* c_HeaderXMsContentCrc64 = "x-ms-content-crc64";
  } // namespace Details
  namespace Models {
    struct FileShareHttpHeaders
    {
      std::string CacheControl;
      std::string ContentDisposition;
      std::string ContentEncoding;
      std::string ContentLanguage;
      std::string ContentType;
      std::string ContentMd5;
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
      std::string Start; // The date-time the policy is active.
      std::string Expiry; // The date-time the policy expires.
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
      std::string OpenTime; // Time when the session that previously opened the handle has last been
                            // reconnected. (UTC)
      std::string LastReconnectTime; // Time handle was last connected to (UTC)
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
      std::string LastModified;
      std::string Etag;
      int64_t Quota = int64_t();
      Azure::Core::Nullable<int32_t> ProvisionedIops;
      Azure::Core::Nullable<int32_t> ProvisionedIngressMBps;
      Azure::Core::Nullable<int32_t> ProvisionedEgressMBps;
      Azure::Core::Nullable<std::string> NextAllowedQuotaDowngradeTime;
      std::string DeletedTime;
      int32_t RemainingRetentionDays = int32_t();
      Models::LeaseStatusType LeaseStatus;
      Models::LeaseStateType LeaseState;
      Models::LeaseDurationType LeaseDuration;
    };

    typedef std::map<std::string, std::string> Metadata;

    // A listed Azure Storage share item.
    struct ShareItem
    {
      std::string Name;
      std::string Snapshot;
      bool Deleted = bool();
      std::string Version;
      Models::ShareProperties Properties;
      Models::Metadata ShareMetadata;
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
      std::string LastModified;
    };

    struct ShareGetPropertiesResult
    {
      std::map<std::string, std::string> Metadata;
      std::string ETag;
      std::string LastModified;
      int64_t Quota = int64_t();
      Azure::Core::Nullable<int32_t> ProvisionedIops;
      Azure::Core::Nullable<int32_t> ProvisionedIngressMBps;
      Azure::Core::Nullable<int32_t> ProvisionedEgressMBps;
      Azure::Core::Nullable<std::string> NextAllowedQuotaDowngradeTime;
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
      std::string LastModified;
      Azure::Core::Nullable<int32_t> LeaseTime;
      std::string LeaseId;
    };

    struct ShareReleaseLeaseResult
    {
      std::string ETag;
      std::string LastModified;
      Azure::Core::Nullable<int32_t> LeaseTime;
    };

    struct ShareChangeLeaseResult
    {
      std::string ETag;
      std::string LastModified;
      Azure::Core::Nullable<int32_t> LeaseTime;
      std::string LeaseId;
    };

    struct ShareRenewLeaseResult
    {
      std::string ETag;
      std::string LastModified;
      Azure::Core::Nullable<int32_t> LeaseTime;
      std::string LeaseId;
    };

    struct ShareBreakLeaseResult
    {
      std::string ETag;
      std::string LastModified;
      int32_t LeaseTime = int32_t();
      Azure::Core::Nullable<std::string> LeaseId;
    };

    struct ShareCreateSnapshotResult
    {
      std::string Snapshot;
      std::string ETag;
      std::string LastModified;
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
      std::string LastModified;
    };

    struct ShareSetMetadataResult
    {
      std::string ETag;
      std::string LastModified;
    };

    struct ShareGetAccessPolicyResult
    {
      std::vector<Models::SignedIdentifier> SignedIdentifiers;
      std::string ETag;
      std::string LastModified;
    };

    struct ShareSetAccessPolicyResult
    {
      std::string ETag;
      std::string LastModified;
    };

    struct ShareGetStatisticsResult
    {
      int64_t ShareUsageBytes = int64_t();
      std::string ETag;
      std::string LastModified;
    };

    struct ShareRestoreResult
    {
      std::string ETag;
      std::string LastModified;
    };

    struct DirectoryCreateResult
    {
      std::string ETag;
      std::string LastModified;
      bool IsServerEncrypted = bool();
      std::string FilePermissionKey;
      std::string FileAttributes;
      std::string FileCreationTime;
      std::string FileLastWriteTime;
      std::string FileChangeTime;
      std::string FileId;
      std::string FileParentId;
    };

    struct DirectoryGetPropertiesResult
    {
      std::map<std::string, std::string> Metadata;
      std::string ETag;
      std::string LastModified;
      bool IsServerEncrypted = bool();
      std::string FileAttributes;
      std::string FileCreationTime;
      std::string FileLastWriteTime;
      std::string FileChangeTime;
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
      std::string LastModified;
      bool IsServerEncrypted = bool();
      std::string FilePermissionKey;
      std::string FileAttributes;
      std::string FileCreationTime;
      std::string FileLastWriteTime;
      std::string FileChangeTime;
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
      FileShareHttpHeaders HttpHeaders;
    };

    struct DirectoryListHandlesResult
    {
      std::vector<Models::HandleItem> HandleList;
      std::string ContinuationToken;
      FileShareHttpHeaders HttpHeaders;
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
      std::string LastModified;
      bool IsServerEncrypted = bool();
      std::string FilePermissionKey;
      std::string FileAttributes;
      std::string FileCreationTime;
      std::string FileLastWriteTime;
      std::string FileChangeTime;
      std::string FileId;
      std::string FileParentId;
    };

    struct FileDownloadResult
    {
      std::unique_ptr<Azure::Core::Http::BodyStream> BodyStream;
      std::string LastModified;
      std::map<std::string, std::string> Metadata;
      int64_t ContentLength = int64_t();
      FileShareHttpHeaders HttpHeaders;
      Azure::Core::Nullable<std::string> ContentRange;
      std::string ETag;
      Azure::Core::Nullable<std::string> TransactionalContentMd5;
      std::string AcceptRanges;
      Azure::Core::Nullable<std::string> CopyCompletionTime;
      Azure::Core::Nullable<std::string> CopyStatusDescription;
      Azure::Core::Nullable<std::string> CopyId;
      Azure::Core::Nullable<std::string> CopyProgress;
      Azure::Core::Nullable<std::string> CopySource;
      Azure::Core::Nullable<CopyStatusType> CopyStatus;
      Azure::Core::Nullable<bool> IsServerEncrypted;
      std::string FileAttributes;
      std::string FileCreationTime;
      std::string FileLastWriteTime;
      std::string FileChangeTime;
      std::string FilePermissionKey;
      std::string FileId;
      std::string FileParentId;
      Azure::Core::Nullable<LeaseDurationType> LeaseDuration;
      Azure::Core::Nullable<LeaseStateType> LeaseState;
      Azure::Core::Nullable<LeaseStatusType> LeaseStatus;
    };

    struct FileGetPropertiesResult
    {
      std::string LastModified;
      std::map<std::string, std::string> Metadata;
      std::string FileType;
      int64_t ContentLength = int64_t();
      FileShareHttpHeaders HttpHeaders;
      std::string ETag;
      Azure::Core::Nullable<std::string> CopyCompletionTime;
      Azure::Core::Nullable<std::string> CopyStatusDescription;
      Azure::Core::Nullable<std::string> CopyId;
      Azure::Core::Nullable<std::string> CopyProgress;
      Azure::Core::Nullable<std::string> CopySource;
      Azure::Core::Nullable<CopyStatusType> CopyStatus;
      Azure::Core::Nullable<bool> IsServerEncrypted;
      std::string FileAttributes;
      std::string FileCreationTime;
      std::string FileLastWriteTime;
      std::string FileChangeTime;
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
      std::string LastModified;
      bool IsServerEncrypted = bool();
      std::string FilePermissionKey;
      std::string FileAttributes;
      std::string FileCreationTime;
      std::string FileLastWriteTime;
      std::string FileChangeTime;
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
      std::string LastModified;
      std::string LeaseId;
    };

    struct FileReleaseLeaseResult
    {
      std::string ETag;
      std::string LastModified;
    };

    struct FileChangeLeaseResult
    {
      std::string ETag;
      std::string LastModified;
      std::string LeaseId;
    };

    struct FileBreakLeaseResult
    {
      std::string ETag;
      std::string LastModified;
      Azure::Core::Nullable<std::string> LeaseId;
    };

    struct FileUploadRangeResult
    {
      std::string ETag;
      std::string LastModified;
      std::string TransactionalContentMd5;
      bool IsServerEncrypted = bool();
    };

    struct FileUploadRangeFromUrlResult
    {
      std::string ETag;
      std::string LastModified;
      std::string XMsContentCrc64;
      bool IsServerEncrypted = bool();
    };

    struct FileGetRangeListResult
    {
      std::vector<Models::FileRange> Ranges;
      std::vector<Models::ClearRange> ClearRanges;
      std::string LastModified;
      std::string ETag;
      int64_t FileContentLength = int64_t();
    };

    struct FileStartCopyResult
    {
      std::string ETag;
      std::string LastModified;
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
      FileShareHttpHeaders HttpHeaders;
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "service");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "properties");
          if (setPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(setPropertiesOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderVersion, setPropertiesOptions.ApiVersionParameter);
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
        };

        static Azure::Core::Response<Models::ServiceGetPropertiesResult> GetProperties(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetPropertiesOptions& getPropertiesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "service");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "properties");
          if (getPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(getPropertiesOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderVersion, getPropertiesOptions.ApiVersionParameter);
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
        };

        static Azure::Core::Response<Models::ServiceListSharesSegmentResult> ListSharesSegment(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ListSharesSegmentOptions& listSharesSegmentOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "list");
          if (listSharesSegmentOptions.Prefix.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryPrefix,
                Storage::Details::UrlEncodeQueryParameter(
                    listSharesSegmentOptions.Prefix.GetValue()));
          }
          if (listSharesSegmentOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryContinuationToken,
                Storage::Details::UrlEncodeQueryParameter(
                    listSharesSegmentOptions.ContinuationToken.GetValue()));
          }
          if (listSharesSegmentOptions.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryMaxResults,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listSharesSegmentOptions.MaxResults.GetValue())));
          }
          if (listSharesSegmentOptions.ListSharesInclude.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryListSharesInclude,
                Storage::Details::UrlEncodeQueryParameter(Models::ListSharesIncludeTypeToString(
                    listSharesSegmentOptions.ListSharesInclude.GetValue())));
          }
          if (listSharesSegmentOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listSharesSegmentOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderVersion, listSharesSegmentOptions.ApiVersionParameter);
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
            c_Days,
            c_Enabled,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_Days);
              }
              else if (std::strcmp(node.Name, "Enabled") == 0)
              {
                path.emplace_back(XmlTagName::c_Enabled);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::c_Days)
              {
                result.Days = std::stoi(node.Value);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_Enabled)
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
            c_Enabled,
            c_IncludeAPIs,
            c_RetentionPolicy,
            c_Unknown,
            c_Version,
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
                path.emplace_back(XmlTagName::c_Enabled);
              }
              else if (std::strcmp(node.Name, "IncludeAPIs") == 0)
              {
                path.emplace_back(XmlTagName::c_IncludeAPIs);
              }
              else if (std::strcmp(node.Name, "RetentionPolicy") == 0)
              {
                path.emplace_back(XmlTagName::c_RetentionPolicy);
              }
              else if (std::strcmp(node.Name, "Version") == 0)
              {
                path.emplace_back(XmlTagName::c_Version);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }

              if (path.size() == 1 && path[0] == XmlTagName::c_RetentionPolicy)
              {
                result.RetentionPolicy = ShareRetentionPolicyFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::c_Enabled)
              {
                result.Enabled = (std::strcmp(node.Value, "true") == 0);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_IncludeAPIs)
              {
                result.IncludeApis = (std::strcmp(node.Value, "true") == 0);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_Version)
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
            c_AllowedHeaders,
            c_AllowedMethods,
            c_AllowedOrigins,
            c_ExposedHeaders,
            c_MaxAgeInSeconds,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_AllowedHeaders);
              }
              else if (std::strcmp(node.Name, "AllowedMethods") == 0)
              {
                path.emplace_back(XmlTagName::c_AllowedMethods);
              }
              else if (std::strcmp(node.Name, "AllowedOrigins") == 0)
              {
                path.emplace_back(XmlTagName::c_AllowedOrigins);
              }
              else if (std::strcmp(node.Name, "ExposedHeaders") == 0)
              {
                path.emplace_back(XmlTagName::c_ExposedHeaders);
              }
              else if (std::strcmp(node.Name, "MaxAgeInSeconds") == 0)
              {
                path.emplace_back(XmlTagName::c_MaxAgeInSeconds);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::c_AllowedHeaders)
              {
                result.AllowedHeaders = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_AllowedMethods)
              {
                result.AllowedMethods = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_AllowedOrigins)
              {
                result.AllowedOrigins = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_ExposedHeaders)
              {
                result.ExposedHeaders = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_MaxAgeInSeconds)
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
            c_Enabled,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_Enabled);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::c_Enabled)
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
            c_Multichannel,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_Multichannel);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }

              if (path.size() == 1 && path[0] == XmlTagName::c_Multichannel)
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
            c_SMB,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_SMB);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }

              if (path.size() == 1 && path[0] == XmlTagName::c_SMB)
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
            c_Cors,
            c_CorsRule,
            c_HourMetrics,
            c_MinuteMetrics,
            c_ProtocolSettings,
            c_StorageServiceProperties,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_Cors);
              }
              else if (std::strcmp(node.Name, "CorsRule") == 0)
              {
                path.emplace_back(XmlTagName::c_CorsRule);
              }
              else if (std::strcmp(node.Name, "HourMetrics") == 0)
              {
                path.emplace_back(XmlTagName::c_HourMetrics);
              }
              else if (std::strcmp(node.Name, "MinuteMetrics") == 0)
              {
                path.emplace_back(XmlTagName::c_MinuteMetrics);
              }
              else if (std::strcmp(node.Name, "ProtocolSettings") == 0)
              {
                path.emplace_back(XmlTagName::c_ProtocolSettings);
              }
              else if (std::strcmp(node.Name, "StorageServiceProperties") == 0)
              {
                path.emplace_back(XmlTagName::c_StorageServiceProperties);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }

              if (path.size() == 2 && path[0] == XmlTagName::c_StorageServiceProperties
                  && path[1] == XmlTagName::c_HourMetrics)
              {
                result.HourMetrics = MetricsFromXml(reader);
                path.pop_back();
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::c_StorageServiceProperties
                  && path[1] == XmlTagName::c_MinuteMetrics)
              {
                result.MinuteMetrics = MetricsFromXml(reader);
                path.pop_back();
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::c_StorageServiceProperties
                  && path[1] == XmlTagName::c_ProtocolSettings)
              {
                result.Protocol = ShareProtocolSettingsFromXml(reader);
                path.pop_back();
              }
              else if (
                  path.size() == 3 && path[0] == XmlTagName::c_StorageServiceProperties
                  && path[1] == XmlTagName::c_Cors && path[2] == XmlTagName::c_CorsRule)
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
            c_LeaseStatus,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_LeaseStatus);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::c_LeaseStatus)
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
            c_LeaseState,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_LeaseState);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::c_LeaseState)
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
            c_LeaseDuration,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_LeaseDuration);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::c_LeaseDuration)
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
            c_DeletedTime,
            c_Etag,
            c_LastModified,
            c_LeaseDuration,
            c_LeaseState,
            c_LeaseStatus,
            c_NextAllowedQuotaDowngradeTime,
            c_ProvisionedEgressMBps,
            c_ProvisionedIngressMBps,
            c_ProvisionedIops,
            c_Quota,
            c_RemainingRetentionDays,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_DeletedTime);
              }
              else if (std::strcmp(node.Name, "Etag") == 0)
              {
                path.emplace_back(XmlTagName::c_Etag);
              }
              else if (std::strcmp(node.Name, "Last-Modified") == 0)
              {
                path.emplace_back(XmlTagName::c_LastModified);
              }
              else if (std::strcmp(node.Name, "LeaseDuration") == 0)
              {
                path.emplace_back(XmlTagName::c_LeaseDuration);
              }
              else if (std::strcmp(node.Name, "LeaseState") == 0)
              {
                path.emplace_back(XmlTagName::c_LeaseState);
              }
              else if (std::strcmp(node.Name, "LeaseStatus") == 0)
              {
                path.emplace_back(XmlTagName::c_LeaseStatus);
              }
              else if (std::strcmp(node.Name, "NextAllowedQuotaDowngradeTime") == 0)
              {
                path.emplace_back(XmlTagName::c_NextAllowedQuotaDowngradeTime);
              }
              else if (std::strcmp(node.Name, "ProvisionedEgressMBps") == 0)
              {
                path.emplace_back(XmlTagName::c_ProvisionedEgressMBps);
              }
              else if (std::strcmp(node.Name, "ProvisionedIngressMBps") == 0)
              {
                path.emplace_back(XmlTagName::c_ProvisionedIngressMBps);
              }
              else if (std::strcmp(node.Name, "ProvisionedIops") == 0)
              {
                path.emplace_back(XmlTagName::c_ProvisionedIops);
              }
              else if (std::strcmp(node.Name, "Quota") == 0)
              {
                path.emplace_back(XmlTagName::c_Quota);
              }
              else if (std::strcmp(node.Name, "RemainingRetentionDays") == 0)
              {
                path.emplace_back(XmlTagName::c_RemainingRetentionDays);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }

              if (path.size() == 1 && path[0] == XmlTagName::c_LeaseStatus)
              {
                result.LeaseStatus = LeaseStatusTypeFromXml(reader);
                path.pop_back();
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_LeaseState)
              {
                result.LeaseState = LeaseStateTypeFromXml(reader);
                path.pop_back();
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_LeaseDuration)
              {
                result.LeaseDuration = LeaseDurationTypeFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::c_DeletedTime)
              {
                result.DeletedTime = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_Etag)
              {
                result.Etag = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_LastModified)
              {
                result.LastModified = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_NextAllowedQuotaDowngradeTime)
              {
                result.NextAllowedQuotaDowngradeTime = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_ProvisionedEgressMBps)
              {
                result.ProvisionedEgressMBps = std::stoi(node.Value);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_ProvisionedIngressMBps)
              {
                result.ProvisionedIngressMBps = std::stoi(node.Value);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_ProvisionedIops)
              {
                result.ProvisionedIops = std::stoi(node.Value);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_Quota)
              {
                result.Quota = std::stoll(node.Value);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_RemainingRetentionDays)
              {
                result.RemainingRetentionDays = std::stoi(node.Value);
              }
            }
          }
          return result;
        }

        static std::map<std::string, std::string> MetadataFromXml(
            Storage::Details::XmlReader& reader)
        {
          std::map<std::string, std::string> result;
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
            c_Deleted,
            c_Metadata,
            c_Name,
            c_Properties,
            c_Snapshot,
            c_Unknown,
            c_Version,
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
                path.emplace_back(XmlTagName::c_Deleted);
              }
              else if (std::strcmp(node.Name, "Metadata") == 0)
              {
                path.emplace_back(XmlTagName::c_Metadata);
              }
              else if (std::strcmp(node.Name, "Name") == 0)
              {
                path.emplace_back(XmlTagName::c_Name);
              }
              else if (std::strcmp(node.Name, "Properties") == 0)
              {
                path.emplace_back(XmlTagName::c_Properties);
              }
              else if (std::strcmp(node.Name, "Snapshot") == 0)
              {
                path.emplace_back(XmlTagName::c_Snapshot);
              }
              else if (std::strcmp(node.Name, "Version") == 0)
              {
                path.emplace_back(XmlTagName::c_Version);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }

              if (path.size() == 1 && path[0] == XmlTagName::c_Properties)
              {
                result.Properties = SharePropertiesFromXml(reader);
                path.pop_back();
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_Metadata)
              {
                result.ShareMetadata = MetadataFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::c_Deleted)
              {
                result.Deleted = (std::strcmp(node.Value, "true") == 0);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_Name)
              {
                result.Name = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_Snapshot)
              {
                result.Snapshot = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_Version)
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
            c_EnumerationResults,
            c_Marker,
            c_MaxResults,
            c_NextMarker,
            c_Prefix,
            c_Share,
            c_Shares,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_EnumerationResults);
              }
              else if (std::strcmp(node.Name, "Marker") == 0)
              {
                path.emplace_back(XmlTagName::c_Marker);
              }
              else if (std::strcmp(node.Name, "MaxResults") == 0)
              {
                path.emplace_back(XmlTagName::c_MaxResults);
              }
              else if (std::strcmp(node.Name, "NextMarker") == 0)
              {
                path.emplace_back(XmlTagName::c_NextMarker);
              }
              else if (std::strcmp(node.Name, "Prefix") == 0)
              {
                path.emplace_back(XmlTagName::c_Prefix);
              }
              else if (std::strcmp(node.Name, "Share") == 0)
              {
                path.emplace_back(XmlTagName::c_Share);
              }
              else if (std::strcmp(node.Name, "Shares") == 0)
              {
                path.emplace_back(XmlTagName::c_Shares);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }
              if (path.size() == 3 && path[0] == XmlTagName::c_EnumerationResults
                  && path[1] == XmlTagName::c_Shares && path[2] == XmlTagName::c_Share)
              {
                result.ShareItems.emplace_back(ShareItemFromXml(reader));
                path.pop_back();
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 2 && path[0] == XmlTagName::c_EnumerationResults
                  && path[1] == XmlTagName::c_NextMarker)
              {
                result.ContinuationToken = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::c_EnumerationResults
                  && path[1] == XmlTagName::c_MaxResults)
              {
                result.MaxResults = std::stoi(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::c_EnumerationResults
                  && path[1] == XmlTagName::c_Prefix)
              {
                result.Prefix = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::c_EnumerationResults
                  && path[1] == XmlTagName::c_Marker)
              {
                result.PreviousContinuationToken = node.Value;
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Attribute)
            {
              if (path.size() == 1 && path[0] == XmlTagName::c_EnumerationResults
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
          std::map<std::string, std::string>
              Metadata; // A name-value pair to associate with a file storage object.
          Azure::Core::Nullable<int64_t>
              ShareQuota; // Specifies the maximum size of the share, in gigabytes.
          std::string ApiVersionParameter
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
        };

        static Azure::Core::Response<Models::ShareCreateResult> Create(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const CreateOptions& createOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "share");
          if (createOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(createOptions.Timeout.GetValue())));
          }
          std::set<std::string> metadataKeys;
          for (const auto& pair : createOptions.Metadata)
          {
            std::string key = pair.first;
            std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
              return static_cast<char>(std::tolower(c));
            });
            if (metadataKeys.insert(key).second == false)
            {
              throw std::runtime_error("duplicate keys in metadata");
            }
            request.AddHeader(Details::c_HeaderMetadata + ("-" + pair.first), pair.second);
          }
          metadataKeys.clear();
          if (createOptions.ShareQuota.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderQuota, std::to_string(createOptions.ShareQuota.GetValue()));
          }
          request.AddHeader(Details::c_HeaderVersion, createOptions.ApiVersionParameter);
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "share");
          if (getPropertiesOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    getPropertiesOptions.ShareSnapshot.GetValue()));
          }
          if (getPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(getPropertiesOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderVersion, getPropertiesOptions.ApiVersionParameter);
          if (getPropertiesOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderLeaseId, getPropertiesOptions.LeaseIdOptional.GetValue());
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "share");
          if (deleteOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(deleteOptions.ShareSnapshot.GetValue()));
          }
          if (deleteOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(deleteOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderVersion, deleteOptions.ApiVersionParameter);
          if (deleteOptions.XMsDeleteSnapshots.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderDeleteSnapshots,
                DeleteSnapshotsOptionTypeToString(deleteOptions.XMsDeleteSnapshots.GetValue()));
          }
          if (deleteOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(Details::c_HeaderLeaseId, deleteOptions.LeaseIdOptional.GetValue());
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "lease");
          request.AddHeader(Details::c_HeaderAction, "acquire");
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "share");
          if (acquireLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(acquireLeaseOptions.Timeout.GetValue())));
          }
          request.AddHeader(
              Details::c_HeaderDuration, std::to_string(acquireLeaseOptions.LeaseDuration));
          if (acquireLeaseOptions.ProposedLeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderProposedLeaseId,
                acquireLeaseOptions.ProposedLeaseIdOptional.GetValue());
          }
          request.AddHeader(Details::c_HeaderVersion, acquireLeaseOptions.ApiVersionParameter);
          if (acquireLeaseOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    acquireLeaseOptions.ShareSnapshot.GetValue()));
          }
          if (acquireLeaseOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderRequestId, acquireLeaseOptions.ClientRequestId.GetValue());
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "lease");
          request.AddHeader(Details::c_HeaderAction, "release");
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "share");
          if (releaseLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(releaseLeaseOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderLeaseId, releaseLeaseOptions.LeaseIdRequired);
          request.AddHeader(Details::c_HeaderVersion, releaseLeaseOptions.ApiVersionParameter);
          if (releaseLeaseOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    releaseLeaseOptions.ShareSnapshot.GetValue()));
          }
          if (releaseLeaseOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderRequestId, releaseLeaseOptions.ClientRequestId.GetValue());
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "lease");
          request.AddHeader(Details::c_HeaderAction, "change");
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "share");
          if (changeLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(changeLeaseOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderLeaseId, changeLeaseOptions.LeaseIdRequired);
          if (changeLeaseOptions.ProposedLeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderProposedLeaseId,
                changeLeaseOptions.ProposedLeaseIdOptional.GetValue());
          }
          request.AddHeader(Details::c_HeaderVersion, changeLeaseOptions.ApiVersionParameter);
          if (changeLeaseOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    changeLeaseOptions.ShareSnapshot.GetValue()));
          }
          if (changeLeaseOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderRequestId, changeLeaseOptions.ClientRequestId.GetValue());
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "lease");
          request.AddHeader(Details::c_HeaderAction, "renew");
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "share");
          if (renewLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(renewLeaseOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderLeaseId, renewLeaseOptions.LeaseIdRequired);
          request.AddHeader(Details::c_HeaderVersion, renewLeaseOptions.ApiVersionParameter);
          if (renewLeaseOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    renewLeaseOptions.ShareSnapshot.GetValue()));
          }
          if (renewLeaseOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderRequestId, renewLeaseOptions.ClientRequestId.GetValue());
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "lease");
          request.AddHeader(Details::c_HeaderAction, "break");
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "share");
          if (breakLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(breakLeaseOptions.Timeout.GetValue())));
          }
          if (breakLeaseOptions.LeaseBreakPeriod.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderBreakPeriod,
                std::to_string(breakLeaseOptions.LeaseBreakPeriod.GetValue()));
          }
          if (breakLeaseOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderLeaseId, breakLeaseOptions.LeaseIdOptional.GetValue());
          }
          request.AddHeader(Details::c_HeaderVersion, breakLeaseOptions.ApiVersionParameter);
          if (breakLeaseOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderRequestId, breakLeaseOptions.ClientRequestId.GetValue());
          }
          if (breakLeaseOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryShareSnapshot,
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
          std::map<std::string, std::string>
              Metadata; // A name-value pair to associate with a file storage object.
          std::string ApiVersionParameter
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
        };

        static Azure::Core::Response<Models::ShareCreateSnapshotResult> CreateSnapshot(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const CreateSnapshotOptions& createSnapshotOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "snapshot");
          if (createSnapshotOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(createSnapshotOptions.Timeout.GetValue())));
          }
          std::set<std::string> metadataKeys;
          for (const auto& pair : createSnapshotOptions.Metadata)
          {
            std::string key = pair.first;
            std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
              return static_cast<char>(std::tolower(c));
            });
            if (metadataKeys.insert(key).second == false)
            {
              throw std::runtime_error("duplicate keys in metadata");
            }
            request.AddHeader(Details::c_HeaderMetadata + ("-" + pair.first), pair.second);
          }
          metadataKeys.clear();
          request.AddHeader(Details::c_HeaderVersion, createSnapshotOptions.ApiVersionParameter);
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "filepermission");
          if (createPermissionOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(createPermissionOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderVersion, createPermissionOptions.ApiVersionParameter);
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
        };

        static Azure::Core::Response<Models::ShareGetPermissionResult> GetPermission(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetPermissionOptions& getPermissionOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "filepermission");
          request.AddHeader(
              Details::c_HeaderFilePermissionKey, getPermissionOptions.FilePermissionKeyRequired);
          if (getPermissionOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(getPermissionOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderVersion, getPermissionOptions.ApiVersionParameter);
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "properties");
          if (setQuotaOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(setQuotaOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderVersion, setQuotaOptions.ApiVersionParameter);
          if (setQuotaOptions.ShareQuota.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderQuota, std::to_string(setQuotaOptions.ShareQuota.GetValue()));
          }
          if (setQuotaOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(Details::c_HeaderLeaseId, setQuotaOptions.LeaseIdOptional.GetValue());
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
          std::map<std::string, std::string>
              Metadata; // A name-value pair to associate with a file storage object.
          std::string ApiVersionParameter
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "metadata");
          if (setMetadataOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(setMetadataOptions.Timeout.GetValue())));
          }
          std::set<std::string> metadataKeys;
          for (const auto& pair : setMetadataOptions.Metadata)
          {
            std::string key = pair.first;
            std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
              return static_cast<char>(std::tolower(c));
            });
            if (metadataKeys.insert(key).second == false)
            {
              throw std::runtime_error("duplicate keys in metadata");
            }
            request.AddHeader(Details::c_HeaderMetadata + ("-" + pair.first), pair.second);
          }
          metadataKeys.clear();
          request.AddHeader(Details::c_HeaderVersion, setMetadataOptions.ApiVersionParameter);
          if (setMetadataOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderLeaseId, setMetadataOptions.LeaseIdOptional.GetValue());
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "acl");
          if (getAccessPolicyOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(getAccessPolicyOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderVersion, getAccessPolicyOptions.ApiVersionParameter);
          if (getAccessPolicyOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderLeaseId, getAccessPolicyOptions.LeaseIdOptional.GetValue());
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "acl");
          if (setAccessPolicyOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(setAccessPolicyOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderVersion, setAccessPolicyOptions.ApiVersionParameter);
          if (setAccessPolicyOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderLeaseId, setAccessPolicyOptions.LeaseIdOptional.GetValue());
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "stats");
          if (getStatisticsOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(getStatisticsOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderVersion, getStatisticsOptions.ApiVersionParameter);
          if (getStatisticsOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderLeaseId, getStatisticsOptions.LeaseIdOptional.GetValue());
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "share");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "undelete");
          if (restoreOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(restoreOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderVersion, restoreOptions.ApiVersionParameter);
          if (restoreOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderRequestId, restoreOptions.ClientRequestId.GetValue());
          }
          if (restoreOptions.DeletedShareName.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderDeletedShareName, restoreOptions.DeletedShareName.GetValue());
          }
          if (restoreOptions.DeletedShareVersion.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderDeletedShareVersion,
                restoreOptions.DeletedShareVersion.GetValue());
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
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

            for (auto i = response.GetHeaders().lower_bound(Details::c_HeaderMetadata);
                 i != response.GetHeaders().end()
                 && i->first.substr(0, 9) == Details::c_HeaderMetadata;
                 ++i)
            {
              result.Metadata.emplace(i->first.substr(10), i->second);
            }
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
            result.Quota = std::stoll(response.GetHeaders().at(Details::c_HeaderQuota));
            if (response.GetHeaders().find(Details::c_HeaderProvisionedIops)
                != response.GetHeaders().end())
            {
              result.ProvisionedIops
                  = std::stoi(response.GetHeaders().at(Details::c_HeaderProvisionedIops));
            }
            if (response.GetHeaders().find(Details::c_HeaderProvisionedIngressMBps)
                != response.GetHeaders().end())
            {
              result.ProvisionedIngressMBps
                  = std::stoi(response.GetHeaders().at(Details::c_HeaderProvisionedIngressMBps));
            }
            if (response.GetHeaders().find(Details::c_HeaderProvisionedEgressMBps)
                != response.GetHeaders().end())
            {
              result.ProvisionedEgressMBps
                  = std::stoi(response.GetHeaders().at(Details::c_HeaderProvisionedEgressMBps));
            }
            if (response.GetHeaders().find(Details::c_HeaderNextAllowedQuotaDowngradeTime)
                != response.GetHeaders().end())
            {
              result.NextAllowedQuotaDowngradeTime
                  = response.GetHeaders().at(Details::c_HeaderNextAllowedQuotaDowngradeTime);
            }
            if (response.GetHeaders().find(Details::c_HeaderLeaseDuration)
                != response.GetHeaders().end())
            {
              result.LeaseDuration = LeaseDurationTypeFromString(
                  response.GetHeaders().at(Details::c_HeaderLeaseDuration));
            }
            if (response.GetHeaders().find(Details::c_HeaderLeaseState)
                != response.GetHeaders().end())
            {
              result.LeaseState
                  = LeaseStateTypeFromString(response.GetHeaders().at(Details::c_HeaderLeaseState));
            }
            if (response.GetHeaders().find(Details::c_HeaderLeaseStatus)
                != response.GetHeaders().end())
            {
              result.LeaseStatus = LeaseStatusTypeFromString(
                  response.GetHeaders().at(Details::c_HeaderLeaseStatus));
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
            if (response.GetHeaders().find(Details::c_HeaderLeaseTime)
                != response.GetHeaders().end())
            {
              result.LeaseTime = std::stoi(response.GetHeaders().at(Details::c_HeaderLeaseTime));
            }
            result.LeaseId = response.GetHeaders().at(Details::c_HeaderLeaseId);
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
            if (response.GetHeaders().find(Details::c_HeaderLeaseTime)
                != response.GetHeaders().end())
            {
              result.LeaseTime = std::stoi(response.GetHeaders().at(Details::c_HeaderLeaseTime));
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
            if (response.GetHeaders().find(Details::c_HeaderLeaseTime)
                != response.GetHeaders().end())
            {
              result.LeaseTime = std::stoi(response.GetHeaders().at(Details::c_HeaderLeaseTime));
            }
            result.LeaseId = response.GetHeaders().at(Details::c_HeaderLeaseId);
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
            if (response.GetHeaders().find(Details::c_HeaderLeaseTime)
                != response.GetHeaders().end())
            {
              result.LeaseTime = std::stoi(response.GetHeaders().at(Details::c_HeaderLeaseTime));
            }
            result.LeaseId = response.GetHeaders().at(Details::c_HeaderLeaseId);
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
            if (response.GetHeaders().find(Details::c_HeaderLeaseTime)
                != response.GetHeaders().end())
            {
              result.LeaseTime = std::stoi(response.GetHeaders().at(Details::c_HeaderLeaseTime));
            }
            if (response.GetHeaders().find(Details::c_HeaderLeaseId) != response.GetHeaders().end())
            {
              result.LeaseId = response.GetHeaders().at(Details::c_HeaderLeaseId);
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
            result.Snapshot = response.GetHeaders().at(Details::c_HeaderSnapshot);
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
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
            result.FilePermissionKey = response.GetHeaders().at(Details::c_HeaderFilePermissionKey);
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
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
            c_Expiry,
            c_Permission,
            c_Start,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_Expiry);
              }
              else if (std::strcmp(node.Name, "Permission") == 0)
              {
                path.emplace_back(XmlTagName::c_Permission);
              }
              else if (std::strcmp(node.Name, "Start") == 0)
              {
                path.emplace_back(XmlTagName::c_Start);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::c_Expiry)
              {
                result.Expiry = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_Permission)
              {
                result.Permission = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_Start)
              {
                result.Start = node.Value;
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
            c_AccessPolicy,
            c_Id,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_AccessPolicy);
              }
              else if (std::strcmp(node.Name, "Id") == 0)
              {
                path.emplace_back(XmlTagName::c_Id);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }

              if (path.size() == 1 && path[0] == XmlTagName::c_AccessPolicy)
              {
                result.Policy = AccessPolicyFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::c_Id)
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
            c_SignedIdentifier,
            c_SignedIdentifiers,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_SignedIdentifier);
              }
              else if (std::strcmp(node.Name, "SignedIdentifiers") == 0)
              {
                path.emplace_back(XmlTagName::c_SignedIdentifiers);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }

              if (path.size() == 2 && path[0] == XmlTagName::c_SignedIdentifiers
                  && path[1] == XmlTagName::c_SignedIdentifier)
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
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
              Storage::Details::XmlNodeType::Text, nullptr, object.Start.data()});
          writer.Write(Storage::Details::XmlNode{Storage::Details::XmlNodeType::EndTag});
          writer.Write(
              Storage::Details::XmlNode{Storage::Details::XmlNodeType::StartTag, "Expiry"});
          writer.Write(Storage::Details::XmlNode{
              Storage::Details::XmlNodeType::Text, nullptr, object.Expiry.data()});
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
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
            c_ShareStats,
            c_ShareUsageBytes,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_ShareStats);
              }
              else if (std::strcmp(node.Name, "ShareUsageBytes") == 0)
              {
                path.emplace_back(XmlTagName::c_ShareUsageBytes);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 2 && path[0] == XmlTagName::c_ShareStats
                  && path[1] == XmlTagName::c_ShareUsageBytes)
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
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
          std::map<std::string, std::string>
              Metadata; // A name-value pair to associate with a file storage object.
          std::string ApiVersionParameter
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "directory");
          if (createOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(createOptions.Timeout.GetValue())));
          }
          std::set<std::string> metadataKeys;
          for (const auto& pair : createOptions.Metadata)
          {
            std::string key = pair.first;
            std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
              return static_cast<char>(std::tolower(c));
            });
            if (metadataKeys.insert(key).second == false)
            {
              throw std::runtime_error("duplicate keys in metadata");
            }
            request.AddHeader(Details::c_HeaderMetadata + ("-" + pair.first), pair.second);
          }
          metadataKeys.clear();
          request.AddHeader(Details::c_HeaderVersion, createOptions.ApiVersionParameter);
          if (createOptions.FilePermission.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFilePermission, createOptions.FilePermission.GetValue());
          }
          if (createOptions.FilePermissionKey.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFilePermissionKey, createOptions.FilePermissionKey.GetValue());
          }
          request.AddHeader(Details::c_HeaderFileAttributes, createOptions.FileAttributes);
          request.AddHeader(Details::c_HeaderFileCreationTime, createOptions.FileCreationTime);
          request.AddHeader(Details::c_HeaderFileLastWriteTime, createOptions.FileLastWriteTime);
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
        };

        static Azure::Core::Response<Models::DirectoryGetPropertiesResult> GetProperties(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetPropertiesOptions& getPropertiesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "directory");
          if (getPropertiesOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    getPropertiesOptions.ShareSnapshot.GetValue()));
          }
          if (getPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(getPropertiesOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderVersion, getPropertiesOptions.ApiVersionParameter);
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
        };

        static Azure::Core::Response<Models::DirectoryDeleteResult> Delete(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const DeleteOptions& deleteOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Delete, url);
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "directory");
          if (deleteOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(deleteOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderVersion, deleteOptions.ApiVersionParameter);
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "directory");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "properties");
          if (setPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(setPropertiesOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderVersion, setPropertiesOptions.ApiVersionParameter);
          if (setPropertiesOptions.FilePermission.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFilePermission, setPropertiesOptions.FilePermission.GetValue());
          }
          if (setPropertiesOptions.FilePermissionKey.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFilePermissionKey,
                setPropertiesOptions.FilePermissionKey.GetValue());
          }
          request.AddHeader(Details::c_HeaderFileAttributes, setPropertiesOptions.FileAttributes);
          request.AddHeader(
              Details::c_HeaderFileCreationTime, setPropertiesOptions.FileCreationTime);
          request.AddHeader(
              Details::c_HeaderFileLastWriteTime, setPropertiesOptions.FileLastWriteTime);
          return SetPropertiesParseResult(context, pipeline.Send(context, request));
        }

        struct SetMetadataOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/Setting-Timeouts-for-File-Service-Operations?redirectedfrom=MSDN">Setting
                       // Timeouts for File Service Operations.</a>
          std::map<std::string, std::string>
              Metadata; // A name-value pair to associate with a file storage object.
          std::string ApiVersionParameter
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
        };

        static Azure::Core::Response<Models::DirectorySetMetadataResult> SetMetadata(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetMetadataOptions& setMetadataOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "directory");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "metadata");
          if (setMetadataOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(setMetadataOptions.Timeout.GetValue())));
          }
          std::set<std::string> metadataKeys;
          for (const auto& pair : setMetadataOptions.Metadata)
          {
            std::string key = pair.first;
            std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
              return static_cast<char>(std::tolower(c));
            });
            if (metadataKeys.insert(key).second == false)
            {
              throw std::runtime_error("duplicate keys in metadata");
            }
            request.AddHeader(Details::c_HeaderMetadata + ("-" + pair.first), pair.second);
          }
          metadataKeys.clear();
          request.AddHeader(Details::c_HeaderVersion, setMetadataOptions.ApiVersionParameter);
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
        };

        static Azure::Core::Response<Models::DirectoryListFilesAndDirectoriesSegmentResult>
        ListFilesAndDirectoriesSegment(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ListFilesAndDirectoriesSegmentOptions& listFilesAndDirectoriesSegmentOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(Details::c_QueryRestype, "directory");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "list");
          if (listFilesAndDirectoriesSegmentOptions.Prefix.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryPrefix,
                Storage::Details::UrlEncodeQueryParameter(
                    listFilesAndDirectoriesSegmentOptions.Prefix.GetValue()));
          }
          if (listFilesAndDirectoriesSegmentOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    listFilesAndDirectoriesSegmentOptions.ShareSnapshot.GetValue()));
          }
          if (listFilesAndDirectoriesSegmentOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryContinuationToken,
                Storage::Details::UrlEncodeQueryParameter(
                    listFilesAndDirectoriesSegmentOptions.ContinuationToken.GetValue()));
          }
          if (listFilesAndDirectoriesSegmentOptions.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryMaxResults,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listFilesAndDirectoriesSegmentOptions.MaxResults.GetValue())));
          }
          if (listFilesAndDirectoriesSegmentOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listFilesAndDirectoriesSegmentOptions.Timeout.GetValue())));
          }
          request.AddHeader(
              Details::c_HeaderVersion, listFilesAndDirectoriesSegmentOptions.ApiVersionParameter);
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
        };

        static Azure::Core::Response<Models::DirectoryListHandlesResult> ListHandles(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ListHandlesOptions& listHandlesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "listhandles");
          if (listHandlesOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryContinuationToken,
                Storage::Details::UrlEncodeQueryParameter(
                    listHandlesOptions.ContinuationToken.GetValue()));
          }
          if (listHandlesOptions.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryMaxResults,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listHandlesOptions.MaxResults.GetValue())));
          }
          if (listHandlesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listHandlesOptions.Timeout.GetValue())));
          }
          if (listHandlesOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    listHandlesOptions.ShareSnapshot.GetValue()));
          }
          if (listHandlesOptions.Recursive.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderRecursive,
                (listHandlesOptions.Recursive.GetValue() ? "true" : "false"));
          }
          request.AddHeader(Details::c_HeaderVersion, listHandlesOptions.ApiVersionParameter);
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
        };

        static Azure::Core::Response<Models::DirectoryForceCloseHandlesResult> ForceCloseHandles(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ForceCloseHandlesOptions& forceCloseHandlesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "forceclosehandles");
          if (forceCloseHandlesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(forceCloseHandlesOptions.Timeout.GetValue())));
          }
          if (forceCloseHandlesOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryContinuationToken,
                Storage::Details::UrlEncodeQueryParameter(
                    forceCloseHandlesOptions.ContinuationToken.GetValue()));
          }
          if (forceCloseHandlesOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    forceCloseHandlesOptions.ShareSnapshot.GetValue()));
          }
          request.AddHeader(Details::c_HeaderHandleId, forceCloseHandlesOptions.HandleId);
          if (forceCloseHandlesOptions.Recursive.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderRecursive,
                (forceCloseHandlesOptions.Recursive.GetValue() ? "true" : "false"));
          }
          request.AddHeader(Details::c_HeaderVersion, forceCloseHandlesOptions.ApiVersionParameter);
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
            result.IsServerEncrypted
                = response.GetHeaders().at(Details::c_HeaderRequestIsServerEncrypted) == "true";
            result.FilePermissionKey = response.GetHeaders().at(Details::c_HeaderFilePermissionKey);
            result.FileAttributes = response.GetHeaders().at(Details::c_HeaderFileAttributes);
            result.FileCreationTime = response.GetHeaders().at(Details::c_HeaderFileCreationTime);
            result.FileLastWriteTime = response.GetHeaders().at(Details::c_HeaderFileLastWriteTime);
            result.FileChangeTime = response.GetHeaders().at(Details::c_HeaderFileChangeTime);
            result.FileId = response.GetHeaders().at(Details::c_HeaderFileId);
            result.FileParentId = response.GetHeaders().at(Details::c_HeaderFileParentId);
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

            for (auto i = response.GetHeaders().lower_bound(Details::c_HeaderMetadata);
                 i != response.GetHeaders().end()
                 && i->first.substr(0, 9) == Details::c_HeaderMetadata;
                 ++i)
            {
              result.Metadata.emplace(i->first.substr(10), i->second);
            }
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
            result.IsServerEncrypted
                = response.GetHeaders().at(Details::c_HeaderIsServerEncrypted) == "true";
            result.FileAttributes = response.GetHeaders().at(Details::c_HeaderFileAttributes);
            result.FileCreationTime = response.GetHeaders().at(Details::c_HeaderFileCreationTime);
            result.FileLastWriteTime = response.GetHeaders().at(Details::c_HeaderFileLastWriteTime);
            result.FileChangeTime = response.GetHeaders().at(Details::c_HeaderFileChangeTime);
            result.FilePermissionKey = response.GetHeaders().at(Details::c_HeaderFilePermissionKey);
            result.FileId = response.GetHeaders().at(Details::c_HeaderFileId);
            result.FileParentId = response.GetHeaders().at(Details::c_HeaderFileParentId);
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
            result.IsServerEncrypted
                = response.GetHeaders().at(Details::c_HeaderRequestIsServerEncrypted) == "true";
            result.FilePermissionKey = response.GetHeaders().at(Details::c_HeaderFilePermissionKey);
            result.FileAttributes = response.GetHeaders().at(Details::c_HeaderFileAttributes);
            result.FileCreationTime = response.GetHeaders().at(Details::c_HeaderFileCreationTime);
            result.FileLastWriteTime = response.GetHeaders().at(Details::c_HeaderFileLastWriteTime);
            result.FileChangeTime = response.GetHeaders().at(Details::c_HeaderFileChangeTime);
            result.FileId = response.GetHeaders().at(Details::c_HeaderFileId);
            result.FileParentId = response.GetHeaders().at(Details::c_HeaderFileParentId);
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.IsServerEncrypted
                = response.GetHeaders().at(Details::c_HeaderRequestIsServerEncrypted) == "true";
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
            result.HttpHeaders.ContentType = response.GetHeaders().at(Details::c_HeaderContentType);
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
            c_Name,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_Name);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::c_Name)
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
            c_ContentLength,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_ContentLength);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::c_ContentLength)
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
            c_Name,
            c_Properties,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_Name);
              }
              else if (std::strcmp(node.Name, "Properties") == 0)
              {
                path.emplace_back(XmlTagName::c_Properties);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }

              if (path.size() == 1 && path[0] == XmlTagName::c_Properties)
              {
                result.Properties = FilePropertyFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::c_Name)
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
            c_Directory,
            c_File,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_Directory);
              }
              else if (std::strcmp(node.Name, "File") == 0)
              {
                path.emplace_back(XmlTagName::c_File);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }
              if (path.size() == 1 && path[0] == XmlTagName::c_Directory)
              {
                result.DirectoryItems.emplace_back(DirectoryItemFromXml(reader));
                path.pop_back();
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_File)
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
            c_Entries,
            c_EnumerationResults,
            c_Marker,
            c_MaxResults,
            c_NextMarker,
            c_Prefix,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_Entries);
              }
              else if (std::strcmp(node.Name, "EnumerationResults") == 0)
              {
                path.emplace_back(XmlTagName::c_EnumerationResults);
              }
              else if (std::strcmp(node.Name, "Marker") == 0)
              {
                path.emplace_back(XmlTagName::c_Marker);
              }
              else if (std::strcmp(node.Name, "MaxResults") == 0)
              {
                path.emplace_back(XmlTagName::c_MaxResults);
              }
              else if (std::strcmp(node.Name, "NextMarker") == 0)
              {
                path.emplace_back(XmlTagName::c_NextMarker);
              }
              else if (std::strcmp(node.Name, "Prefix") == 0)
              {
                path.emplace_back(XmlTagName::c_Prefix);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }

              if (path.size() == 2 && path[0] == XmlTagName::c_EnumerationResults
                  && path[1] == XmlTagName::c_Entries)
              {
                result.Segment = FilesAndDirectoriesListSegmentFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 2 && path[0] == XmlTagName::c_EnumerationResults
                  && path[1] == XmlTagName::c_NextMarker)
              {
                result.ContinuationToken = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::c_EnumerationResults
                  && path[1] == XmlTagName::c_MaxResults)
              {
                result.MaxResults = std::stoi(node.Value);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::c_EnumerationResults
                  && path[1] == XmlTagName::c_Prefix)
              {
                result.Prefix = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::c_EnumerationResults
                  && path[1] == XmlTagName::c_Marker)
              {
                result.PreviousContinuationToken = node.Value;
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Attribute)
            {
              if (path.size() == 1 && path[0] == XmlTagName::c_EnumerationResults
                  && (std::strcmp(node.Name, "DirectoryPath") == 0))
              {
                result.DirectoryPath = node.Value;
              }
              else if (
                  path.size() == 1 && path[0] == XmlTagName::c_EnumerationResults
                  && (std::strcmp(node.Name, "ServiceEndpoint") == 0))
              {
                result.ServiceEndpoint = node.Value;
              }
              else if (
                  path.size() == 1 && path[0] == XmlTagName::c_EnumerationResults
                  && (std::strcmp(node.Name, "ShareName") == 0))
              {
                result.ShareName = node.Value;
              }
              else if (
                  path.size() == 1 && path[0] == XmlTagName::c_EnumerationResults
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
            result.HttpHeaders.ContentType = response.GetHeaders().at(Details::c_HeaderContentType);
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
            c_ClientIp,
            c_FileId,
            c_HandleId,
            c_LastReconnectTime,
            c_OpenTime,
            c_ParentId,
            c_Path,
            c_SessionId,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_ClientIp);
              }
              else if (std::strcmp(node.Name, "FileId") == 0)
              {
                path.emplace_back(XmlTagName::c_FileId);
              }
              else if (std::strcmp(node.Name, "HandleId") == 0)
              {
                path.emplace_back(XmlTagName::c_HandleId);
              }
              else if (std::strcmp(node.Name, "LastReconnectTime") == 0)
              {
                path.emplace_back(XmlTagName::c_LastReconnectTime);
              }
              else if (std::strcmp(node.Name, "OpenTime") == 0)
              {
                path.emplace_back(XmlTagName::c_OpenTime);
              }
              else if (std::strcmp(node.Name, "ParentId") == 0)
              {
                path.emplace_back(XmlTagName::c_ParentId);
              }
              else if (std::strcmp(node.Name, "Path") == 0)
              {
                path.emplace_back(XmlTagName::c_Path);
              }
              else if (std::strcmp(node.Name, "SessionId") == 0)
              {
                path.emplace_back(XmlTagName::c_SessionId);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::c_ClientIp)
              {
                result.ClientIp = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_FileId)
              {
                result.FileId = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_HandleId)
              {
                result.HandleId = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_LastReconnectTime)
              {
                result.LastReconnectTime = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_OpenTime)
              {
                result.OpenTime = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_ParentId)
              {
                result.ParentId = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_Path)
              {
                result.Path = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_SessionId)
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
            c_Entries,
            c_EnumerationResults,
            c_Handle,
            c_NextMarker,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_Entries);
              }
              else if (std::strcmp(node.Name, "EnumerationResults") == 0)
              {
                path.emplace_back(XmlTagName::c_EnumerationResults);
              }
              else if (std::strcmp(node.Name, "Handle") == 0)
              {
                path.emplace_back(XmlTagName::c_Handle);
              }
              else if (std::strcmp(node.Name, "NextMarker") == 0)
              {
                path.emplace_back(XmlTagName::c_NextMarker);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }
              if (path.size() == 3 && path[0] == XmlTagName::c_EnumerationResults
                  && path[1] == XmlTagName::c_Entries && path[2] == XmlTagName::c_Handle)
              {
                result.HandleList.emplace_back(HandleItemFromXml(reader));
                path.pop_back();
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 2 && path[0] == XmlTagName::c_EnumerationResults
                  && path[1] == XmlTagName::c_NextMarker)
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
            if (response.GetHeaders().find(Details::c_HeaderContinuationToken)
                != response.GetHeaders().end())
            {
              result.ContinuationToken
                  = response.GetHeaders().at(Details::c_HeaderContinuationToken);
            }
            result.numberOfHandlesClosed
                = std::stoi(response.GetHeaders().at(Details::c_HeaderNumberOfHandlesClosed));
            result.numberOfHandlesFailedToClose = std::stoi(
                response.GetHeaders().at(Details::c_HeaderNumberOfHandlesFailedToClose));
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
          int64_t XMsContentLength; // Specifies the maximum size for the file, up to 4 TB.
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
          Azure::Core::Nullable<std::string> FileContentMd5; // Sets the file's MD5 hash.
          Azure::Core::Nullable<std::string>
              FileContentDisposition; // Sets the file's Content-Disposition header.
          std::map<std::string, std::string>
              Metadata; // A name-value pair to associate with a file storage object.
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
          request.AddHeader(Details::c_HeaderContentLength, "0");
          if (createOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(createOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderVersion, createOptions.ApiVersionParameter);
          request.AddHeader(
              Details::c_HeaderXMsContentLength, std::to_string(createOptions.XMsContentLength));
          request.AddHeader(Details::c_HeaderFileTypeConstant, "file");
          if (createOptions.FileContentType.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFileContentType, createOptions.FileContentType.GetValue());
          }
          if (createOptions.FileContentEncoding.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFileContentEncoding, createOptions.FileContentEncoding.GetValue());
          }
          if (createOptions.FileContentLanguage.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFileContentLanguage, createOptions.FileContentLanguage.GetValue());
          }
          if (createOptions.FileCacheControl.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFileCacheControl, createOptions.FileCacheControl.GetValue());
          }
          if (createOptions.FileContentMd5.HasValue())
          {
            request.AddHeader(Details::c_HeaderContentMd5, createOptions.FileContentMd5.GetValue());
          }
          if (createOptions.FileContentDisposition.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFileContentDisposition,
                createOptions.FileContentDisposition.GetValue());
          }
          std::set<std::string> metadataKeys;
          for (const auto& pair : createOptions.Metadata)
          {
            std::string key = pair.first;
            std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
              return static_cast<char>(std::tolower(c));
            });
            if (metadataKeys.insert(key).second == false)
            {
              throw std::runtime_error("duplicate keys in metadata");
            }
            request.AddHeader(Details::c_HeaderMetadata + ("-" + pair.first), pair.second);
          }
          metadataKeys.clear();
          if (createOptions.FilePermission.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFilePermission, createOptions.FilePermission.GetValue());
          }
          if (createOptions.FilePermissionKey.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFilePermissionKey, createOptions.FilePermissionKey.GetValue());
          }
          request.AddHeader(Details::c_HeaderFileAttributes, createOptions.FileAttributes);
          request.AddHeader(Details::c_HeaderFileCreationTime, createOptions.FileCreationTime);
          request.AddHeader(Details::c_HeaderFileLastWriteTime, createOptions.FileLastWriteTime);
          if (createOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(Details::c_HeaderLeaseId, createOptions.LeaseIdOptional.GetValue());
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(downloadOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderVersion, downloadOptions.ApiVersionParameter);
          if (downloadOptions.Range.HasValue())
          {
            request.AddHeader(Details::c_HeaderRange, downloadOptions.Range.GetValue());
          }
          if (downloadOptions.GetRangeContentMd5.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderRangeGetContentMd5,
                (downloadOptions.GetRangeContentMd5.GetValue() ? "true" : "false"));
          }
          if (downloadOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(Details::c_HeaderLeaseId, downloadOptions.LeaseIdOptional.GetValue());
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
                Details::c_QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    getPropertiesOptions.ShareSnapshot.GetValue()));
          }
          if (getPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(getPropertiesOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderVersion, getPropertiesOptions.ApiVersionParameter);
          if (getPropertiesOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderLeaseId, getPropertiesOptions.LeaseIdOptional.GetValue());
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(deleteOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderVersion, deleteOptions.ApiVersionParameter);
          if (deleteOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(Details::c_HeaderLeaseId, deleteOptions.LeaseIdOptional.GetValue());
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          Azure::Core::Nullable<std::string> FileContentMd5; // Sets the file's MD5 hash.
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
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "properties");
          if (setHttpHeadersOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(setHttpHeadersOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderVersion, setHttpHeadersOptions.ApiVersionParameter);
          if (setHttpHeadersOptions.XMsContentLength.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderXMsContentLength,
                std::to_string(setHttpHeadersOptions.XMsContentLength.GetValue()));
          }
          if (setHttpHeadersOptions.FileContentType.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFileContentType, setHttpHeadersOptions.FileContentType.GetValue());
          }
          if (setHttpHeadersOptions.FileContentEncoding.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFileContentEncoding,
                setHttpHeadersOptions.FileContentEncoding.GetValue());
          }
          if (setHttpHeadersOptions.FileContentLanguage.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFileContentLanguage,
                setHttpHeadersOptions.FileContentLanguage.GetValue());
          }
          if (setHttpHeadersOptions.FileCacheControl.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFileCacheControl,
                setHttpHeadersOptions.FileCacheControl.GetValue());
          }
          if (setHttpHeadersOptions.FileContentMd5.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderContentMd5, setHttpHeadersOptions.FileContentMd5.GetValue());
          }
          if (setHttpHeadersOptions.FileContentDisposition.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFileContentDisposition,
                setHttpHeadersOptions.FileContentDisposition.GetValue());
          }
          if (setHttpHeadersOptions.FilePermission.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFilePermission, setHttpHeadersOptions.FilePermission.GetValue());
          }
          if (setHttpHeadersOptions.FilePermissionKey.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFilePermissionKey,
                setHttpHeadersOptions.FilePermissionKey.GetValue());
          }
          request.AddHeader(Details::c_HeaderFileAttributes, setHttpHeadersOptions.FileAttributes);
          request.AddHeader(
              Details::c_HeaderFileCreationTime, setHttpHeadersOptions.FileCreationTime);
          request.AddHeader(
              Details::c_HeaderFileLastWriteTime, setHttpHeadersOptions.FileLastWriteTime);
          if (setHttpHeadersOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderLeaseId, setHttpHeadersOptions.LeaseIdOptional.GetValue());
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
          std::map<std::string, std::string>
              Metadata; // A name-value pair to associate with a file storage object.
          std::string ApiVersionParameter
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "metadata");
          if (setMetadataOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(setMetadataOptions.Timeout.GetValue())));
          }
          std::set<std::string> metadataKeys;
          for (const auto& pair : setMetadataOptions.Metadata)
          {
            std::string key = pair.first;
            std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
              return static_cast<char>(std::tolower(c));
            });
            if (metadataKeys.insert(key).second == false)
            {
              throw std::runtime_error("duplicate keys in metadata");
            }
            request.AddHeader(Details::c_HeaderMetadata + ("-" + pair.first), pair.second);
          }
          metadataKeys.clear();
          request.AddHeader(Details::c_HeaderVersion, setMetadataOptions.ApiVersionParameter);
          if (setMetadataOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderLeaseId, setMetadataOptions.LeaseIdOptional.GetValue());
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "lease");
          request.AddHeader(Details::c_HeaderAction, "acquire");
          if (acquireLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(acquireLeaseOptions.Timeout.GetValue())));
          }
          request.AddHeader(
              Details::c_HeaderDuration, std::to_string(acquireLeaseOptions.LeaseDuration));
          if (acquireLeaseOptions.ProposedLeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderProposedLeaseId,
                acquireLeaseOptions.ProposedLeaseIdOptional.GetValue());
          }
          request.AddHeader(Details::c_HeaderVersion, acquireLeaseOptions.ApiVersionParameter);
          if (acquireLeaseOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderRequestId, acquireLeaseOptions.ClientRequestId.GetValue());
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "lease");
          request.AddHeader(Details::c_HeaderAction, "release");
          if (releaseLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(releaseLeaseOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderLeaseId, releaseLeaseOptions.LeaseIdRequired);
          request.AddHeader(Details::c_HeaderVersion, releaseLeaseOptions.ApiVersionParameter);
          if (releaseLeaseOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderRequestId, releaseLeaseOptions.ClientRequestId.GetValue());
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "lease");
          request.AddHeader(Details::c_HeaderAction, "change");
          if (changeLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(changeLeaseOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderLeaseId, changeLeaseOptions.LeaseIdRequired);
          if (changeLeaseOptions.ProposedLeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderProposedLeaseId,
                changeLeaseOptions.ProposedLeaseIdOptional.GetValue());
          }
          request.AddHeader(Details::c_HeaderVersion, changeLeaseOptions.ApiVersionParameter);
          if (changeLeaseOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderRequestId, changeLeaseOptions.ClientRequestId.GetValue());
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "lease");
          request.AddHeader(Details::c_HeaderAction, "break");
          if (breakLeaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(breakLeaseOptions.Timeout.GetValue())));
          }
          if (breakLeaseOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderLeaseId, breakLeaseOptions.LeaseIdOptional.GetValue());
          }
          request.AddHeader(Details::c_HeaderVersion, breakLeaseOptions.ApiVersionParameter);
          if (breakLeaseOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderRequestId, breakLeaseOptions.ClientRequestId.GetValue());
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
          Models::FileRangeWriteType
              XMsWrite; // Specify one of the following options: - Update: Writes the bytes
                        // specified by the request body into the specified range. The Range and
                        // Content-Length headers must match to perform the update. - Clear: Clears
                        // the specified range and releases the space used in storage for that
                        // range. To clear a range, set the Content-Length header to zero, and set
                        // the Range header to a value that indicates the range to clear, up to
                        // maximum file size.
          int64_t ContentLength
              = int64_t(); // Specifies the number of bytes being transmitted in the request body.
                           // When the x-ms-write header is set to clear, the value of this header
                           // must be set to zero.
          Azure::Core::Nullable<std::string>
              ContentMd5; // An MD5 hash of the content. This hash is used to verify the integrity
                          // of the data during transport. When the Content-MD5 header is specified,
                          // the File service compares the hash of the content that has arrived with
                          // the header value that was sent. If the two hashes do not match, the
                          // operation will fail with error code 400 (Bad Request).
          std::string ApiVersionParameter
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          Azure::Core::Http::Request request(
              Azure::Core::Http::HttpMethod::Put, std::move(url), &bodyStream);
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "range");
          if (uploadRangeOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(uploadRangeOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderXMsRange, uploadRangeOptions.XMsRange);
          request.AddHeader(
              Details::c_HeaderFileRangeWrite,
              FileRangeWriteTypeToString(uploadRangeOptions.XMsWrite));
          request.AddHeader(
              Details::c_HeaderContentLength, std::to_string(uploadRangeOptions.ContentLength));
          if (uploadRangeOptions.ContentMd5.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderContentMd5, uploadRangeOptions.ContentMd5.GetValue());
          }
          request.AddHeader(Details::c_HeaderVersion, uploadRangeOptions.ApiVersionParameter);
          if (uploadRangeOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderLeaseId, uploadRangeOptions.LeaseIdOptional.GetValue());
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
          Azure::Core::Nullable<std::string>
              SourceContentCrc64; // Specify the crc64 calculated for the range of bytes that must
                                  // be read from the copy source.
          Azure::Core::Nullable<std::string>
              SourceIfMatchCrc64; // Specify the crc64 value to operate only on range with a
                                  // matching crc64 checksum.
          Azure::Core::Nullable<std::string>
              SourceIfNoneMatchCrc64; // Specify the crc64 value to operate only on range without a
                                      // matching crc64 checksum.
          std::string ApiVersionParameter
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "range");
          if (uploadRangeFromUrlOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(uploadRangeFromUrlOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderRange, uploadRangeFromUrlOptions.TargetRange);
          request.AddHeader(Details::c_HeaderCopySource, uploadRangeFromUrlOptions.CopySource);
          if (uploadRangeFromUrlOptions.SourceRange.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderSourceRange, uploadRangeFromUrlOptions.SourceRange.GetValue());
          }
          request.AddHeader(
              Details::c_HeaderFileRangeWriteFromUrl,
              FileRangeWriteFromUrlTypeToString(uploadRangeFromUrlOptions.XMsWrite));
          request.AddHeader(
              Details::c_HeaderContentLength,
              std::to_string(uploadRangeFromUrlOptions.ContentLength));
          if (uploadRangeFromUrlOptions.SourceContentCrc64.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderSourceContentCrc64,
                uploadRangeFromUrlOptions.SourceContentCrc64.GetValue());
          }
          if (uploadRangeFromUrlOptions.SourceIfMatchCrc64.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderSourceIfMatchCrc64,
                uploadRangeFromUrlOptions.SourceIfMatchCrc64.GetValue());
          }
          if (uploadRangeFromUrlOptions.SourceIfNoneMatchCrc64.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderSourceIfNoneMatchCrc64,
                uploadRangeFromUrlOptions.SourceIfNoneMatchCrc64.GetValue());
          }
          request.AddHeader(
              Details::c_HeaderVersion, uploadRangeFromUrlOptions.ApiVersionParameter);
          if (uploadRangeFromUrlOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderLeaseId, uploadRangeFromUrlOptions.LeaseIdOptional.GetValue());
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "rangelist");
          if (getRangeListOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    getRangeListOptions.ShareSnapshot.GetValue()));
          }
          if (getRangeListOptions.PrevShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryPrevShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    getRangeListOptions.PrevShareSnapshot.GetValue()));
          }
          if (getRangeListOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(getRangeListOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderVersion, getRangeListOptions.ApiVersionParameter);
          if (getRangeListOptions.XMsRange.HasValue())
          {
            request.AddHeader(Details::c_HeaderXMsRange, getRangeListOptions.XMsRange.GetValue());
          }
          if (getRangeListOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderLeaseId, getRangeListOptions.LeaseIdOptional.GetValue());
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
          std::map<std::string, std::string>
              Metadata; // A name-value pair to associate with a file storage object.
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
          request.AddHeader(Details::c_HeaderContentLength, "0");
          if (startCopyOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(startCopyOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderVersion, startCopyOptions.ApiVersionParameter);
          std::set<std::string> metadataKeys;
          for (const auto& pair : startCopyOptions.Metadata)
          {
            std::string key = pair.first;
            std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
              return static_cast<char>(std::tolower(c));
            });
            if (metadataKeys.insert(key).second == false)
            {
              throw std::runtime_error("duplicate keys in metadata");
            }
            request.AddHeader(Details::c_HeaderMetadata + ("-" + pair.first), pair.second);
          }
          metadataKeys.clear();
          request.AddHeader(Details::c_HeaderCopySource, startCopyOptions.CopySource);
          if (startCopyOptions.FilePermission.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFilePermission, startCopyOptions.FilePermission.GetValue());
          }
          if (startCopyOptions.FilePermissionKey.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFilePermissionKey, startCopyOptions.FilePermissionKey.GetValue());
          }
          if (startCopyOptions.XMsFilePermissionCopyMode.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFilePermissionCopyMode,
                PermissionCopyModeTypeToString(
                    startCopyOptions.XMsFilePermissionCopyMode.GetValue()));
          }
          if (startCopyOptions.FileCopyIgnoreReadOnly.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderIgnoreReadOnly,
                (startCopyOptions.FileCopyIgnoreReadOnly.GetValue() ? "true" : "false"));
          }
          if (startCopyOptions.FileCopyFileAttributes.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFileAttributes,
                startCopyOptions.FileCopyFileAttributes.GetValue());
          }
          if (startCopyOptions.FileCopyFileCreationTime.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFileCreationTime,
                startCopyOptions.FileCopyFileCreationTime.GetValue());
          }
          if (startCopyOptions.FileCopyFileLastWriteTime.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderFileLastWriteTime,
                startCopyOptions.FileCopyFileLastWriteTime.GetValue());
          }
          if (startCopyOptions.FileCopySetArchiveAttribute.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderSetArchiveAttribute,
                (startCopyOptions.FileCopySetArchiveAttribute.GetValue() ? "true" : "false"));
          }
          if (startCopyOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderLeaseId, startCopyOptions.LeaseIdOptional.GetValue());
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
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
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "copy");
          request.GetUrl().AppendQueryParameter(
              Details::c_QueryCopyId,
              Storage::Details::UrlEncodeQueryParameter(abortCopyOptions.CopyId));
          if (abortCopyOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(abortCopyOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::c_HeaderCopyActionAbortConstant, "abort");
          request.AddHeader(Details::c_HeaderVersion, abortCopyOptions.ApiVersionParameter);
          if (abortCopyOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::c_HeaderLeaseId, abortCopyOptions.LeaseIdOptional.GetValue());
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
        };

        static Azure::Core::Response<Models::FileListHandlesResult> ListHandles(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ListHandlesOptions& listHandlesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "listhandles");
          if (listHandlesOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryContinuationToken,
                Storage::Details::UrlEncodeQueryParameter(
                    listHandlesOptions.ContinuationToken.GetValue()));
          }
          if (listHandlesOptions.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryMaxResults,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listHandlesOptions.MaxResults.GetValue())));
          }
          if (listHandlesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listHandlesOptions.Timeout.GetValue())));
          }
          if (listHandlesOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    listHandlesOptions.ShareSnapshot.GetValue()));
          }
          request.AddHeader(Details::c_HeaderVersion, listHandlesOptions.ApiVersionParameter);
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
              = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to
                                                     // use for this request.
        };

        static Azure::Core::Response<Models::FileForceCloseHandlesResult> ForceCloseHandles(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ForceCloseHandlesOptions& forceCloseHandlesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::c_HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::c_QueryComp, "forceclosehandles");
          if (forceCloseHandlesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(forceCloseHandlesOptions.Timeout.GetValue())));
          }
          if (forceCloseHandlesOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryContinuationToken,
                Storage::Details::UrlEncodeQueryParameter(
                    forceCloseHandlesOptions.ContinuationToken.GetValue()));
          }
          if (forceCloseHandlesOptions.ShareSnapshot.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::c_QueryShareSnapshot,
                Storage::Details::UrlEncodeQueryParameter(
                    forceCloseHandlesOptions.ShareSnapshot.GetValue()));
          }
          request.AddHeader(Details::c_HeaderHandleId, forceCloseHandlesOptions.HandleId);
          request.AddHeader(Details::c_HeaderVersion, forceCloseHandlesOptions.ApiVersionParameter);
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
            result.IsServerEncrypted
                = response.GetHeaders().at(Details::c_HeaderRequestIsServerEncrypted) == "true";
            result.FilePermissionKey = response.GetHeaders().at(Details::c_HeaderFilePermissionKey);
            result.FileAttributes = response.GetHeaders().at(Details::c_HeaderFileAttributes);
            result.FileCreationTime = response.GetHeaders().at(Details::c_HeaderFileCreationTime);
            result.FileLastWriteTime = response.GetHeaders().at(Details::c_HeaderFileLastWriteTime);
            result.FileChangeTime = response.GetHeaders().at(Details::c_HeaderFileChangeTime);
            result.FileId = response.GetHeaders().at(Details::c_HeaderFileId);
            result.FileParentId = response.GetHeaders().at(Details::c_HeaderFileParentId);
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
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);

            for (auto i = response.GetHeaders().lower_bound(Details::c_HeaderMetadata);
                 i != response.GetHeaders().end()
                 && i->first.substr(0, 9) == Details::c_HeaderMetadata;
                 ++i)
            {
              result.Metadata.emplace(i->first.substr(10), i->second);
            }
            result.ContentLength
                = std::stoll(response.GetHeaders().at(Details::c_HeaderContentLength));
            result.HttpHeaders.ContentType = response.GetHeaders().at(Details::c_HeaderContentType);
            if (response.GetHeaders().find(Details::c_HeaderContentRange)
                != response.GetHeaders().end())
            {
              result.ContentRange = response.GetHeaders().at(Details::c_HeaderContentRange);
            }
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            if (response.GetHeaders().find(Details::c_HeaderTransactionalContentMd5)
                != response.GetHeaders().end())
            {
              result.TransactionalContentMd5
                  = response.GetHeaders().at(Details::c_HeaderTransactionalContentMd5);
            }
            if (response.GetHeaders().find(Details::c_HeaderContentEncoding)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentEncoding
                  = response.GetHeaders().at(Details::c_HeaderContentEncoding);
            }
            if (response.GetHeaders().find(Details::c_HeaderCacheControl)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.CacheControl
                  = response.GetHeaders().at(Details::c_HeaderCacheControl);
            }
            if (response.GetHeaders().find(Details::c_HeaderContentDisposition)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentDisposition
                  = response.GetHeaders().at(Details::c_HeaderContentDisposition);
            }
            if (response.GetHeaders().find(Details::c_HeaderContentLanguage)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentLanguage
                  = response.GetHeaders().at(Details::c_HeaderContentLanguage);
            }
            result.AcceptRanges = response.GetHeaders().at(Details::c_HeaderAcceptRanges);
            if (response.GetHeaders().find(Details::c_HeaderCopyCompletionTime)
                != response.GetHeaders().end())
            {
              result.CopyCompletionTime
                  = response.GetHeaders().at(Details::c_HeaderCopyCompletionTime);
            }
            if (response.GetHeaders().find(Details::c_HeaderCopyStatusDescription)
                != response.GetHeaders().end())
            {
              result.CopyStatusDescription
                  = response.GetHeaders().at(Details::c_HeaderCopyStatusDescription);
            }
            if (response.GetHeaders().find(Details::c_HeaderCopyId) != response.GetHeaders().end())
            {
              result.CopyId = response.GetHeaders().at(Details::c_HeaderCopyId);
            }
            if (response.GetHeaders().find(Details::c_HeaderCopyProgress)
                != response.GetHeaders().end())
            {
              result.CopyProgress = response.GetHeaders().at(Details::c_HeaderCopyProgress);
            }
            if (response.GetHeaders().find(Details::c_HeaderCopySource)
                != response.GetHeaders().end())
            {
              result.CopySource = response.GetHeaders().at(Details::c_HeaderCopySource);
            }
            if (response.GetHeaders().find(Details::c_HeaderCopyStatus)
                != response.GetHeaders().end())
            {
              result.CopyStatus
                  = CopyStatusTypeFromString(response.GetHeaders().at(Details::c_HeaderCopyStatus));
            }
            if (response.GetHeaders().find(Details::c_HeaderContentMd5)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentMd5 = response.GetHeaders().at(Details::c_HeaderContentMd5);
            }
            if (response.GetHeaders().find(Details::c_HeaderIsServerEncrypted)
                != response.GetHeaders().end())
            {
              result.IsServerEncrypted
                  = response.GetHeaders().at(Details::c_HeaderIsServerEncrypted) == "true";
            }
            result.FileAttributes = response.GetHeaders().at(Details::c_HeaderFileAttributes);
            result.FileCreationTime = response.GetHeaders().at(Details::c_HeaderFileCreationTime);
            result.FileLastWriteTime = response.GetHeaders().at(Details::c_HeaderFileLastWriteTime);
            result.FileChangeTime = response.GetHeaders().at(Details::c_HeaderFileChangeTime);
            result.FilePermissionKey = response.GetHeaders().at(Details::c_HeaderFilePermissionKey);
            result.FileId = response.GetHeaders().at(Details::c_HeaderFileId);
            result.FileParentId = response.GetHeaders().at(Details::c_HeaderFileParentId);
            if (response.GetHeaders().find(Details::c_HeaderLeaseDuration)
                != response.GetHeaders().end())
            {
              result.LeaseDuration = LeaseDurationTypeFromString(
                  response.GetHeaders().at(Details::c_HeaderLeaseDuration));
            }
            if (response.GetHeaders().find(Details::c_HeaderLeaseState)
                != response.GetHeaders().end())
            {
              result.LeaseState
                  = LeaseStateTypeFromString(response.GetHeaders().at(Details::c_HeaderLeaseState));
            }
            if (response.GetHeaders().find(Details::c_HeaderLeaseStatus)
                != response.GetHeaders().end())
            {
              result.LeaseStatus = LeaseStatusTypeFromString(
                  response.GetHeaders().at(Details::c_HeaderLeaseStatus));
            }
            return Azure::Core::Response<Models::FileDownloadResult>(
                std::move(result), std::move(responsePtr));
          }
          else if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::PartialContent)
          {
            // Succeeded to read a specified range of the file.
            Models::FileDownloadResult result;
            result.BodyStream = response.GetBodyStream();
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);

            for (auto i = response.GetHeaders().lower_bound(Details::c_HeaderMetadata);
                 i != response.GetHeaders().end()
                 && i->first.substr(0, 9) == Details::c_HeaderMetadata;
                 ++i)
            {
              result.Metadata.emplace(i->first.substr(10), i->second);
            }
            result.ContentLength
                = std::stoll(response.GetHeaders().at(Details::c_HeaderContentLength));
            result.HttpHeaders.ContentType = response.GetHeaders().at(Details::c_HeaderContentType);
            if (response.GetHeaders().find(Details::c_HeaderContentRange)
                != response.GetHeaders().end())
            {
              result.ContentRange = response.GetHeaders().at(Details::c_HeaderContentRange);
            }
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            if (response.GetHeaders().find(Details::c_HeaderTransactionalContentMd5)
                != response.GetHeaders().end())
            {
              result.TransactionalContentMd5
                  = response.GetHeaders().at(Details::c_HeaderTransactionalContentMd5);
            }
            if (response.GetHeaders().find(Details::c_HeaderContentEncoding)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentEncoding
                  = response.GetHeaders().at(Details::c_HeaderContentEncoding);
            }
            if (response.GetHeaders().find(Details::c_HeaderCacheControl)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.CacheControl
                  = response.GetHeaders().at(Details::c_HeaderCacheControl);
            }
            if (response.GetHeaders().find(Details::c_HeaderContentDisposition)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentDisposition
                  = response.GetHeaders().at(Details::c_HeaderContentDisposition);
            }
            if (response.GetHeaders().find(Details::c_HeaderContentLanguage)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentLanguage
                  = response.GetHeaders().at(Details::c_HeaderContentLanguage);
            }
            result.AcceptRanges = response.GetHeaders().at(Details::c_HeaderAcceptRanges);
            if (response.GetHeaders().find(Details::c_HeaderCopyCompletionTime)
                != response.GetHeaders().end())
            {
              result.CopyCompletionTime
                  = response.GetHeaders().at(Details::c_HeaderCopyCompletionTime);
            }
            if (response.GetHeaders().find(Details::c_HeaderCopyStatusDescription)
                != response.GetHeaders().end())
            {
              result.CopyStatusDescription
                  = response.GetHeaders().at(Details::c_HeaderCopyStatusDescription);
            }
            if (response.GetHeaders().find(Details::c_HeaderCopyId) != response.GetHeaders().end())
            {
              result.CopyId = response.GetHeaders().at(Details::c_HeaderCopyId);
            }
            if (response.GetHeaders().find(Details::c_HeaderCopyProgress)
                != response.GetHeaders().end())
            {
              result.CopyProgress = response.GetHeaders().at(Details::c_HeaderCopyProgress);
            }
            if (response.GetHeaders().find(Details::c_HeaderCopySource)
                != response.GetHeaders().end())
            {
              result.CopySource = response.GetHeaders().at(Details::c_HeaderCopySource);
            }
            if (response.GetHeaders().find(Details::c_HeaderCopyStatus)
                != response.GetHeaders().end())
            {
              result.CopyStatus
                  = CopyStatusTypeFromString(response.GetHeaders().at(Details::c_HeaderCopyStatus));
            }
            if (response.GetHeaders().find(Details::c_HeaderContentMd5)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentMd5 = response.GetHeaders().at(Details::c_HeaderContentMd5);
            }
            if (response.GetHeaders().find(Details::c_HeaderIsServerEncrypted)
                != response.GetHeaders().end())
            {
              result.IsServerEncrypted
                  = response.GetHeaders().at(Details::c_HeaderIsServerEncrypted) == "true";
            }
            result.FileAttributes = response.GetHeaders().at(Details::c_HeaderFileAttributes);
            result.FileCreationTime = response.GetHeaders().at(Details::c_HeaderFileCreationTime);
            result.FileLastWriteTime = response.GetHeaders().at(Details::c_HeaderFileLastWriteTime);
            result.FileChangeTime = response.GetHeaders().at(Details::c_HeaderFileChangeTime);
            result.FilePermissionKey = response.GetHeaders().at(Details::c_HeaderFilePermissionKey);
            result.FileId = response.GetHeaders().at(Details::c_HeaderFileId);
            result.FileParentId = response.GetHeaders().at(Details::c_HeaderFileParentId);
            if (response.GetHeaders().find(Details::c_HeaderLeaseDuration)
                != response.GetHeaders().end())
            {
              result.LeaseDuration = LeaseDurationTypeFromString(
                  response.GetHeaders().at(Details::c_HeaderLeaseDuration));
            }
            if (response.GetHeaders().find(Details::c_HeaderLeaseState)
                != response.GetHeaders().end())
            {
              result.LeaseState
                  = LeaseStateTypeFromString(response.GetHeaders().at(Details::c_HeaderLeaseState));
            }
            if (response.GetHeaders().find(Details::c_HeaderLeaseStatus)
                != response.GetHeaders().end())
            {
              result.LeaseStatus = LeaseStatusTypeFromString(
                  response.GetHeaders().at(Details::c_HeaderLeaseStatus));
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
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);

            for (auto i = response.GetHeaders().lower_bound(Details::c_HeaderMetadata);
                 i != response.GetHeaders().end()
                 && i->first.substr(0, 9) == Details::c_HeaderMetadata;
                 ++i)
            {
              result.Metadata.emplace(i->first.substr(10), i->second);
            }
            result.FileType = response.GetHeaders().at(Details::c_HeaderFileType);
            result.ContentLength
                = std::stoll(response.GetHeaders().at(Details::c_HeaderContentLength));
            if (response.GetHeaders().find(Details::c_HeaderContentType)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentType
                  = response.GetHeaders().at(Details::c_HeaderContentType);
            }
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            if (response.GetHeaders().find(Details::c_HeaderTransactionalContentMd5)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentMd5
                  = response.GetHeaders().at(Details::c_HeaderTransactionalContentMd5);
            }
            if (response.GetHeaders().find(Details::c_HeaderContentEncoding)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentEncoding
                  = response.GetHeaders().at(Details::c_HeaderContentEncoding);
            }
            if (response.GetHeaders().find(Details::c_HeaderCacheControl)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.CacheControl
                  = response.GetHeaders().at(Details::c_HeaderCacheControl);
            }
            if (response.GetHeaders().find(Details::c_HeaderContentDisposition)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentDisposition
                  = response.GetHeaders().at(Details::c_HeaderContentDisposition);
            }
            if (response.GetHeaders().find(Details::c_HeaderContentLanguage)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentLanguage
                  = response.GetHeaders().at(Details::c_HeaderContentLanguage);
            }
            if (response.GetHeaders().find(Details::c_HeaderCopyCompletionTime)
                != response.GetHeaders().end())
            {
              result.CopyCompletionTime
                  = response.GetHeaders().at(Details::c_HeaderCopyCompletionTime);
            }
            if (response.GetHeaders().find(Details::c_HeaderCopyStatusDescription)
                != response.GetHeaders().end())
            {
              result.CopyStatusDescription
                  = response.GetHeaders().at(Details::c_HeaderCopyStatusDescription);
            }
            if (response.GetHeaders().find(Details::c_HeaderCopyId) != response.GetHeaders().end())
            {
              result.CopyId = response.GetHeaders().at(Details::c_HeaderCopyId);
            }
            if (response.GetHeaders().find(Details::c_HeaderCopyProgress)
                != response.GetHeaders().end())
            {
              result.CopyProgress = response.GetHeaders().at(Details::c_HeaderCopyProgress);
            }
            if (response.GetHeaders().find(Details::c_HeaderCopySource)
                != response.GetHeaders().end())
            {
              result.CopySource = response.GetHeaders().at(Details::c_HeaderCopySource);
            }
            if (response.GetHeaders().find(Details::c_HeaderCopyStatus)
                != response.GetHeaders().end())
            {
              result.CopyStatus
                  = CopyStatusTypeFromString(response.GetHeaders().at(Details::c_HeaderCopyStatus));
            }
            if (response.GetHeaders().find(Details::c_HeaderIsServerEncrypted)
                != response.GetHeaders().end())
            {
              result.IsServerEncrypted
                  = response.GetHeaders().at(Details::c_HeaderIsServerEncrypted) == "true";
            }
            result.FileAttributes = response.GetHeaders().at(Details::c_HeaderFileAttributes);
            result.FileCreationTime = response.GetHeaders().at(Details::c_HeaderFileCreationTime);
            result.FileLastWriteTime = response.GetHeaders().at(Details::c_HeaderFileLastWriteTime);
            result.FileChangeTime = response.GetHeaders().at(Details::c_HeaderFileChangeTime);
            result.FilePermissionKey = response.GetHeaders().at(Details::c_HeaderFilePermissionKey);
            result.FileId = response.GetHeaders().at(Details::c_HeaderFileId);
            result.FileParentId = response.GetHeaders().at(Details::c_HeaderFileParentId);
            if (response.GetHeaders().find(Details::c_HeaderLeaseDuration)
                != response.GetHeaders().end())
            {
              result.LeaseDuration = LeaseDurationTypeFromString(
                  response.GetHeaders().at(Details::c_HeaderLeaseDuration));
            }
            if (response.GetHeaders().find(Details::c_HeaderLeaseState)
                != response.GetHeaders().end())
            {
              result.LeaseState
                  = LeaseStateTypeFromString(response.GetHeaders().at(Details::c_HeaderLeaseState));
            }
            if (response.GetHeaders().find(Details::c_HeaderLeaseStatus)
                != response.GetHeaders().end())
            {
              result.LeaseStatus = LeaseStatusTypeFromString(
                  response.GetHeaders().at(Details::c_HeaderLeaseStatus));
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
            result.IsServerEncrypted
                = response.GetHeaders().at(Details::c_HeaderRequestIsServerEncrypted) == "true";
            result.FilePermissionKey = response.GetHeaders().at(Details::c_HeaderFilePermissionKey);
            result.FileAttributes = response.GetHeaders().at(Details::c_HeaderFileAttributes);
            result.FileCreationTime = response.GetHeaders().at(Details::c_HeaderFileCreationTime);
            result.FileLastWriteTime = response.GetHeaders().at(Details::c_HeaderFileLastWriteTime);
            result.FileChangeTime = response.GetHeaders().at(Details::c_HeaderFileChangeTime);
            result.FileId = response.GetHeaders().at(Details::c_HeaderFileId);
            result.FileParentId = response.GetHeaders().at(Details::c_HeaderFileParentId);
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.IsServerEncrypted
                = response.GetHeaders().at(Details::c_HeaderRequestIsServerEncrypted) == "true";
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
            result.LeaseId = response.GetHeaders().at(Details::c_HeaderLeaseId);
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
            result.LeaseId = response.GetHeaders().at(Details::c_HeaderLeaseId);
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
            if (response.GetHeaders().find(Details::c_HeaderLeaseId) != response.GetHeaders().end())
            {
              result.LeaseId = response.GetHeaders().at(Details::c_HeaderLeaseId);
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
            if (response.GetHeaders().find(Details::c_HeaderTransactionalContentMd5)
                != response.GetHeaders().end())
            {
              result.TransactionalContentMd5
                  = response.GetHeaders().at(Details::c_HeaderTransactionalContentMd5);
            }
            if (response.GetHeaders().find(Details::c_HeaderRequestIsServerEncrypted)
                != response.GetHeaders().end())
            {
              result.IsServerEncrypted
                  = response.GetHeaders().at(Details::c_HeaderRequestIsServerEncrypted) == "true";
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
            result.XMsContentCrc64 = response.GetHeaders().at(Details::c_HeaderXMsContentCrc64);
            result.IsServerEncrypted
                = response.GetHeaders().at(Details::c_HeaderRequestIsServerEncrypted) == "true";
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
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.FileContentLength
                = std::stoll(response.GetHeaders().at(Details::c_HeaderXMsContentLength));
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
            c_End,
            c_Start,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_End);
              }
              else if (std::strcmp(node.Name, "Start") == 0)
              {
                path.emplace_back(XmlTagName::c_Start);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::c_End)
              {
                result.End = std::stoll(node.Value);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_Start)
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
            c_End,
            c_Start,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_End);
              }
              else if (std::strcmp(node.Name, "Start") == 0)
              {
                path.emplace_back(XmlTagName::c_Start);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::c_End)
              {
                result.End = std::stoll(node.Value);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_Start)
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
            c_ClearRange,
            c_Range,
            c_Ranges,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_ClearRange);
              }
              else if (std::strcmp(node.Name, "Range") == 0)
              {
                path.emplace_back(XmlTagName::c_Range);
              }
              else if (std::strcmp(node.Name, "Ranges") == 0)
              {
                path.emplace_back(XmlTagName::c_Ranges);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }
              if (path.size() == 2 && path[0] == XmlTagName::c_Ranges
                  && path[1] == XmlTagName::c_Range)
              {
                result.Ranges.emplace_back(FileRangeFromXml(reader));
                path.pop_back();
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::c_Ranges
                  && path[1] == XmlTagName::c_ClearRange)
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
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
            if (response.GetHeaders().find(Details::c_HeaderCopyId) != response.GetHeaders().end())
            {
              result.CopyId = response.GetHeaders().at(Details::c_HeaderCopyId);
            }
            if (response.GetHeaders().find(Details::c_HeaderCopyStatus)
                != response.GetHeaders().end())
            {
              result.CopyStatus
                  = CopyStatusTypeFromString(response.GetHeaders().at(Details::c_HeaderCopyStatus));
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
            result.HttpHeaders.ContentType = response.GetHeaders().at(Details::c_HeaderContentType);
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
            c_ClientIp,
            c_FileId,
            c_HandleId,
            c_LastReconnectTime,
            c_OpenTime,
            c_ParentId,
            c_Path,
            c_SessionId,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_ClientIp);
              }
              else if (std::strcmp(node.Name, "FileId") == 0)
              {
                path.emplace_back(XmlTagName::c_FileId);
              }
              else if (std::strcmp(node.Name, "HandleId") == 0)
              {
                path.emplace_back(XmlTagName::c_HandleId);
              }
              else if (std::strcmp(node.Name, "LastReconnectTime") == 0)
              {
                path.emplace_back(XmlTagName::c_LastReconnectTime);
              }
              else if (std::strcmp(node.Name, "OpenTime") == 0)
              {
                path.emplace_back(XmlTagName::c_OpenTime);
              }
              else if (std::strcmp(node.Name, "ParentId") == 0)
              {
                path.emplace_back(XmlTagName::c_ParentId);
              }
              else if (std::strcmp(node.Name, "Path") == 0)
              {
                path.emplace_back(XmlTagName::c_Path);
              }
              else if (std::strcmp(node.Name, "SessionId") == 0)
              {
                path.emplace_back(XmlTagName::c_SessionId);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::c_ClientIp)
              {
                result.ClientIp = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_FileId)
              {
                result.FileId = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_HandleId)
              {
                result.HandleId = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_LastReconnectTime)
              {
                result.LastReconnectTime = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_OpenTime)
              {
                result.OpenTime = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_ParentId)
              {
                result.ParentId = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_Path)
              {
                result.Path = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::c_SessionId)
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
            c_Entries,
            c_EnumerationResults,
            c_Handle,
            c_NextMarker,
            c_Unknown,
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
                path.emplace_back(XmlTagName::c_Entries);
              }
              else if (std::strcmp(node.Name, "EnumerationResults") == 0)
              {
                path.emplace_back(XmlTagName::c_EnumerationResults);
              }
              else if (std::strcmp(node.Name, "Handle") == 0)
              {
                path.emplace_back(XmlTagName::c_Handle);
              }
              else if (std::strcmp(node.Name, "NextMarker") == 0)
              {
                path.emplace_back(XmlTagName::c_NextMarker);
              }
              else
              {
                path.emplace_back(XmlTagName::c_Unknown);
              }
              if (path.size() == 3 && path[0] == XmlTagName::c_EnumerationResults
                  && path[1] == XmlTagName::c_Entries && path[2] == XmlTagName::c_Handle)
              {
                result.HandleList.emplace_back(HandleItemFromXml(reader));
                path.pop_back();
              }
            }
            else if (node.Type == Storage::Details::XmlNodeType::Text)
            {
              if (path.size() == 2 && path[0] == XmlTagName::c_EnumerationResults
                  && path[1] == XmlTagName::c_NextMarker)
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
            if (response.GetHeaders().find(Details::c_HeaderContinuationToken)
                != response.GetHeaders().end())
            {
              result.ContinuationToken
                  = response.GetHeaders().at(Details::c_HeaderContinuationToken);
            }
            result.numberOfHandlesClosed
                = std::stoi(response.GetHeaders().at(Details::c_HeaderNumberOfHandlesClosed));
            result.numberOfHandlesFailedToClose = std::stoi(
                response.GetHeaders().at(Details::c_HeaderNumberOfHandlesFailedToClose));
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
