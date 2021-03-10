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

  using GetUserDelegationKeyResult = Blobs::Models::GetUserDelegationKeyResult;
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
  using ListDataLakeFileSystemsIncludeFlags = Blobs::Models::ListBlobContainersIncludeFlags;

  struct GetDataLakeFileSystemAccessPolicyResult
  {
    PublicAccessType AccessType = PublicAccessType::None;
    Azure::ETag ETag;
    Azure::DateTime LastModified;
    std::vector<DataLakeSignedIdentifier> SignedIdentifiers;
    std::string RequestId;
  }; // struct GetDataLakeFileSystemAccessPolicyResult

  using SetDataLakeFileSystemAccessPolicyResult = Blobs::Models::SetBlobContainerAccessPolicyResult;

  struct GetDataLakeFileSystemPropertiesResult
  {
    Azure::ETag ETag;
    DateTime LastModified;
    Storage::Metadata Metadata;
    std::string RequestId;
  };

  struct CreateDataLakeFileSystemResult
  {
    bool Created = true;
    Azure::ETag ETag;
    DateTime LastModified;
    std::string RequestId;
  };

  struct DeleteDataLakeFileSystemResult
  {
    bool Deleted = true;
    std::string RequestId;
  };

  struct SetDataLakeFileSystemMetadataResult
  {
    Azure::ETag ETag;
    DateTime LastModified;
    std::string RequestId;
  };

  // PathClient models:

  struct DeleteDataLakePathResult
  {
    bool Deleted = true;
    std::string RequestId;
  };

  using AcquireDataLakeLeaseResult = Blobs::Models::AcquireBlobLeaseResult;
  using RenewDataLakeLeaseResult = Blobs::Models::RenewBlobLeaseResult;
  using ReleaseDataLakeLeaseResult = Blobs::Models::ReleaseBlobLeaseResult;
  using ChangeDataLakeLeaseResult = Blobs::Models::ChangeBlobLeaseResult;
  using BreakDataLakeLeaseResult = Blobs::Models::BreakBlobLeaseResult;
  using RehydratePriority = Blobs::Models::RehydratePriority;
  using DataLakeArchiveStatus = Blobs::Models::BlobArchiveStatus;

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
     * @param dataLakeAclsString the string that contains multiple acls.
     * @return std::vector<Acl>
     */
    static std::vector<Acl> DeserializeAcls(const std::string& dataLakeAclsString);

    /**
     * @brief Creates a string that contains several Acls.
     * @param dataLakeAclsArray the acls to be serialized into a string.
     * @return std::string
     */
    static std::string SerializeAcls(const std::vector<Acl>& dataLakeAclsArray);
  };

  struct GetDataLakePathPropertiesResult
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
    Azure::Nullable<DataLakeArchiveStatus> ArchiveStatus;
    Azure::Nullable<Models::RehydratePriority> RehydratePriority;
    Azure::Nullable<std::string> CopyStatusDescription;
    Azure::Nullable<bool> IsIncrementalCopy;
    Azure::Nullable<std::string> IncrementalCopyDestinationSnapshot;
    Azure::Nullable<std::string> VersionId;
    Azure::Nullable<bool> IsCurrentVersion;
    std::string RequestId;
  };

  struct GetDataLakePathAccessControlListResult
  {
    Azure::ETag ETag;
    DateTime LastModified;
    std::string Owner;
    std::string Group;
    std::string Permissions;
    std::vector<Acl> Acls;
    std::string RequestId;
  };

  struct SetDataLakePathHttpHeadersResult
  {
    Azure::ETag ETag;
    DateTime LastModified;
    std::string RequestId;
  };

  struct SetDataLakePathMetadataResult
  {
    Azure::ETag ETag;
    DateTime LastModified;
    std::string RequestId;
  };

  struct CreateDataLakePathResult
  {
    bool Created = true;
    Azure::ETag ETag;
    DateTime LastModified;
    Azure::Nullable<int64_t> FileSize;
    std::string RequestId;
  };

  using SetDataLakePathAccessControlListResult = _detail::PathSetAccessControlResult;
  using SetDataLakePathPermissionsResult = _detail::PathSetAccessControlResult;

  // FileClient models:

  using UploadDataLakeFileFromResult = Blobs::Models::UploadBlockBlobResult;
  using AppendDataLakeFileResult = _detail::PathAppendDataResult;
  using FlushDataLakeFileResult = _detail::PathFlushDataResult;
  using ScheduleDataLakeFileDeletionResult = Blobs::Models::SetBlobExpiryResult;
  using CopyStatus = Blobs::Models::CopyStatus;

  struct DownloadDataLakeFileDetails
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

  struct DownloadDataLakeFileResult
  {
    std::unique_ptr<Azure::Core::IO::BodyStream> Body;
    int64_t FileSize = int64_t();
    Azure::Core::Http::HttpRange ContentRange;
    Azure::Nullable<Storage::ContentHash> TransactionalContentHash;
    DownloadDataLakeFileDetails Details;
    std::string RequestId;
  };

  struct DeleteDataLakeFileResult
  {
    bool Deleted = true;
    std::string RequestId;
  };

  struct DownloadDataLakeFileToResult
  {
    int64_t FileSize = int64_t();
    Azure::Core::Http::HttpRange ContentRange;
    DownloadDataLakeFileDetails Details;
  };

  using CreateDataLakeFileResult = CreateDataLakePathResult;

  // DirectoryClient models:

  struct RenameDataLakeDirectoryResult
  {
    Azure::Nullable<std::string> ContinuationToken;
    std::string RequestId;
  };

  using SetDataLakePathAccessControlListRecursiveSinglePageResult
      = _detail::PathSetAccessControlRecursiveResult;
  using UpdateDataLakePathAccessControlListRecursiveSinglePageResult
      = SetDataLakePathAccessControlListRecursiveSinglePageResult;
  using RemoveDataLakePathAccessControlListRecursiveSinglePageResult
      = SetDataLakePathAccessControlListRecursiveSinglePageResult;
  using CreateDataLakeDirectoryResult = CreateDataLakePathResult;
  using DeleteDataLakeDirectoryResult = DeleteDataLakePathResult;

}}}}} // namespace Azure::Storage::Files::DataLake::Models
