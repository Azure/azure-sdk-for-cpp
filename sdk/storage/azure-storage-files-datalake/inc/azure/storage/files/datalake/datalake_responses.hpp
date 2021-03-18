// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <azure/storage/blobs/blob_responses.hpp>

#include "azure/storage/files/datalake/protocol/datalake_rest_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake { namespace Models {

  // ServiceClient models:

  using UserDelegationKey = Blobs::Models::UserDelegationKey;

  struct FileSystemItemDetails
  {
    Azure::ETag ETag;
    Azure::DateTime LastModified;
    Storage::Metadata Metadata;
    PublicAccessType AccessType = PublicAccessType::None;
    bool HasImmutabilityPolicy = false;
    bool HasLegalHold = false;
    Azure::Nullable<LeaseDurationType> LeaseDuration;
    LeaseStateType LeaseState = LeaseStateType::Available;
    LeaseStatusType LeaseStatus = LeaseStatusType::Unlocked;
  }; // struct FileSystemItemDetails

  struct FileSystemItem
  {
    std::string Name;
    FileSystemItemDetails Details;
  }; // struct BlobContainerItem

  struct ListFileSystemsSinglePageResult
  {
    std::string RequestId;
    std::string ServiceEndpoint;
    std::string Prefix;
    Azure::Nullable<std::string> ContinuationToken;
    std::vector<FileSystemItem> Items;
  }; // struct ListFileSystemsSinglePageResult

  // FileSystemClient models:

  using ListPathsSinglePageResult = _detail::FileSystemListPathsResult;
  using DataLakeSignedIdentifier = Blobs::Models::BlobSignedIdentifier;
  using ListFileSystemsIncludeFlags = Blobs::Models::ListBlobContainersIncludeFlags;

  struct GetFileSystemAccessPolicyResult
  {
    std::string RequestId;
    Azure::ETag ETag;
    Azure::DateTime LastModified;
    PublicAccessType AccessType = PublicAccessType::None;
    std::vector<DataLakeSignedIdentifier> SignedIdentifiers;
  }; // struct DataLakeFileSystemAccessPolciy

  using SetFileSystemAccessPolicyResult = Blobs::Models::SetBlobContainerAccessPolicyResult;

  struct FileSystemProperties
  {
    Azure::ETag ETag;
    DateTime LastModified;
    Storage::Metadata Metadata;
  };

  struct CreateFileSystemResult
  {
    bool Created = true;
    Azure::ETag ETag;
    DateTime LastModified;
    std::string RequestId;
  };

  struct DeleteFileSystemResult
  {
    bool Deleted = true;
    std::string RequestId;
  };

  struct SetFileSystemMetadataResult
  {
    Azure::ETag ETag;
    DateTime LastModified;
    std::string RequestId;
  };

  // PathClient models:

  struct DeletePathResult
  {
    bool Deleted = true;
    std::string RequestId;
  };

  using AcquireLeaseResult = Blobs::Models::AcquireLeaseResult;
  using RenewLeaseResult = Blobs::Models::RenewLeaseResult;
  using ReleaseLeaseResult = Blobs::Models::ReleaseLeaseResult;
  using ChangeLeaseResult = Blobs::Models::ChangeLeaseResult;
  using BreakLeaseResult = Blobs::Models::BreakLeaseResult;
  using RehydratePriority = Blobs::Models::RehydratePriority;
  using ArchiveStatus = Blobs::Models::BlobArchiveStatus;

  struct Acl
  {
    std::string Scope;
    std::string Type;
    std::string Id;
    std::string Permissions;

    /**
     * @brief Creates an Acl based on acl input string.
     * @param aclString the string to be parsed to Acl.
     * @return Acl
     */
    static Acl FromString(const std::string& aclString);

    /**
     * @brief Creates a string from an Acl.
     * @param acl the acl object to be serialized to a string.
     * @return std::string
     */
    static std::string ToString(const Acl& acl);

    /**
     * @brief Creates a vector of Acl from a string that indicates multiple acls.
     * @param aclsString the string that contains multiple acls.
     * @return std::vector<Acl>
     */
    static std::vector<Acl> DeserializeAcls(const std::string& aclsString);

    /**
     * @brief Creates a string that contains several Acls.
     * @param aclsArray the acls to be serialized into a string.
     * @return std::string
     */
    static std::string SerializeAcls(const std::vector<Acl>& cclsArray);
  };

  struct PathProperties
  {
    Azure::ETag ETag;
    DateTime LastModified;
    DateTime CreatedOn;
    int64_t FileSize = 0;
    Storage::Metadata Metadata;
    Azure::Nullable<LeaseDurationType> LeaseDuration;
    Azure::Nullable<LeaseStateType> LeaseState;
    Azure::Nullable<LeaseStatusType> LeaseStatus;
    PathHttpHeaders HttpHeaders;
    Azure::Nullable<bool> IsServerEncrypted;
    Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
    Azure::Nullable<bool> IsAccessTierInferred;
    Azure::Nullable<DateTime> AccessTierChangedOn;
    Azure::Nullable<std::string> CopyId;
    Azure::Nullable<std::string> CopySource;
    Azure::Nullable<Blobs::Models::CopyStatus> CopyStatus;
    Azure::Nullable<std::string> CopyProgress;
    Azure::Nullable<DateTime> CopyCompletedOn;
    Azure::Nullable<DateTime> ExpiresOn;
    Azure::Nullable<DateTime> LastAccessedOn;
    bool IsDirectory = false;
    Azure::Nullable<ArchiveStatus> ArchiveStatus;
    Azure::Nullable<Models::RehydratePriority> RehydratePriority;
    Azure::Nullable<std::string> CopyStatusDescription;
    Azure::Nullable<bool> IsIncrementalCopy;
    Azure::Nullable<std::string> IncrementalCopyDestinationSnapshot;
    Azure::Nullable<std::string> VersionId;
    Azure::Nullable<bool> IsCurrentVersion;
  };

  struct GetPathAccessControlListResult
  {
    Azure::ETag ETag;
    DateTime LastModified;
    std::string Owner;
    std::string Group;
    std::string Permissions;
    std::vector<Acl> Acls;
    std::string RequestId;
  };

  struct SetPathHttpHeadersResult
  {
    Azure::ETag ETag;
    DateTime LastModified;
    std::string RequestId;
  };

  struct SetPathMetadataResult
  {
    Azure::ETag ETag;
    DateTime LastModified;
    std::string RequestId;
  };

  struct CreatePathResult
  {
    bool Created = true;
    Azure::ETag ETag;
    DateTime LastModified;
    Azure::Nullable<int64_t> FileSize;
    std::string RequestId;
  };

  using SetPathAccessControlListResult = _detail::PathSetAccessControlResult;
  using SetPathPermissionsResult = _detail::PathSetAccessControlResult;

  // FileClient models:

  using UploadFileFromResult = Blobs::Models::UploadBlockBlobResult;
  using AppendFileResult = _detail::PathAppendDataResult;
  using FlushFileResult = _detail::PathFlushDataResult;
  using ScheduleFileDeletionResult = Blobs::Models::SetBlobExpiryResult;
  using CopyStatus = Blobs::Models::CopyStatus;

  struct DownloadFileDetails
  {
    Azure::ETag ETag;
    DateTime LastModified;
    Azure::Nullable<LeaseDurationType> LeaseDuration;
    LeaseStateType LeaseState;
    LeaseStatusType LeaseStatus;
    PathHttpHeaders HttpHeaders;
    Storage::Metadata Metadata;
    DateTime CreatedOn;
    Azure::Nullable<DateTime> ExpiresOn;
    Azure::Nullable<DateTime> LastAccessedOn;
    Azure::Nullable<std::string> CopyId;
    Azure::Nullable<std::string> CopySource;
    Azure::Nullable<Models::CopyStatus> CopyStatus;
    Azure::Nullable<std::string> CopyStatusDescription;
    Azure::Nullable<std::string> CopyProgress;
    Azure::Nullable<Azure::DateTime> CopyCompletedOn;
    Azure::Nullable<std::string> VersionId;
    Azure::Nullable<bool> IsCurrentVersion;
    bool IsServerEncrypted = false;
    Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
    Azure::Nullable<std::string> EncryptionScope;
  };

  struct DownloadFileResult
  {
    std::unique_ptr<Azure::Core::IO::BodyStream> Body;
    int64_t FileSize = int64_t();
    Azure::Core::Http::HttpRange ContentRange;
    Azure::Nullable<Storage::ContentHash> TransactionalContentHash;
    DownloadFileDetails Details;
    std::string RequestId;
  };

  struct DeleteFileResult
  {
    bool Deleted = true;
    std::string RequestId;
  };

  struct DownloadFileToResult
  {
    int64_t FileSize = int64_t();
    Azure::Core::Http::HttpRange ContentRange;
    DownloadFileDetails Details;
  };

  using CreateFileResult = CreatePathResult;

  // DirectoryClient models:

  using SetPathAccessControlListRecursiveSinglePageResult
      = _detail::PathSetAccessControlRecursiveResult;
  using UpdatePathAccessControlListRecursiveSinglePageResult
      = SetPathAccessControlListRecursiveSinglePageResult;
  using RemovePathAccessControlListRecursiveSinglePageResult
      = SetPathAccessControlListRecursiveSinglePageResult;
  using CreateDirectoryResult = CreatePathResult;
  using DeleteDirectoryResult = DeletePathResult;

}}}}} // namespace Azure::Storage::Files::DataLake::Models
