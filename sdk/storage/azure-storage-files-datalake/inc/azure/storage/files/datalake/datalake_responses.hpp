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

  struct FileSystemItem
  {
    std::string Name;
    Azure::Core::ETag ETag;
    Azure::Core::DateTime LastModified;
    Storage::Metadata Metadata;
    PublicAccessType AccessType = PublicAccessType::None;
    bool HasImmutabilityPolicy = false;
    bool HasLegalHold = false;
    Azure::Core::Nullable<LeaseDurationType> LeaseDuration;
    LeaseStateType LeaseState = LeaseStateType::Available;
    LeaseStatusType LeaseStatus = LeaseStatusType::Unlocked;
  }; // struct BlobContainerItem

  struct ListFileSystemsSinglePageResult
  {
    std::string RequestId;
    std::string ServiceEndpoint;
    std::string Prefix;
    Azure::Core::Nullable<std::string> ContinuationToken;
    std::vector<FileSystemItem> Items;
  }; // struct ListFileSystemsSinglePageResult

  // FileSystemClient models:

  using ListPathsSinglePageResult = Details::FileSystemListPathsResult;
  using DataLakeSignedIdentifier = Blobs::Models::BlobSignedIdentifier;
  using ListDataLakeFileSystemsIncludeFlags = Blobs::Models::ListBlobContainersIncludeFlags;

  struct GetDataLakeFileSystemAccessPolicyResult
  {
    PublicAccessType AccessType = PublicAccessType::None;
    Azure::Core::ETag ETag;
    Azure::Core::DateTime LastModified;
    std::vector<DataLakeSignedIdentifier> SignedIdentifiers;
    std::string RequestId;
  }; // struct GetDataLakeFileSystemAccessPolicyResult

  using SetDataLakeFileSystemAccessPolicyResult = Blobs::Models::SetBlobContainerAccessPolicyResult;

  struct GetDataLakeFileSystemPropertiesResult
  {
    Azure::Core::ETag ETag;
    Core::DateTime LastModified;
    Storage::Metadata Metadata;
    std::string RequestId;
  };

  struct CreateDataLakeFileSystemResult
  {
    bool Created = true;
    Azure::Core::ETag ETag;
    Core::DateTime LastModified;
    std::string RequestId;
  };

  struct DeleteDataLakeFileSystemResult
  {
    bool Deleted = true;
    std::string RequestId;
  };

  struct SetDataLakeFileSystemMetadataResult
  {
    Azure::Core::ETag ETag;
    Core::DateTime LastModified;
    std::string RequestId;
    std::string NamespaceEnabled;
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
    Azure::Core::ETag ETag;
    Core::DateTime LastModified;
    Core::DateTime CreatedOn;
    int64_t FileSize;
    Storage::Metadata Metadata;
    Azure::Core::Nullable<LeaseDurationType> LeaseDuration;
    Azure::Core::Nullable<LeaseStateType> LeaseState;
    Azure::Core::Nullable<LeaseStatusType> LeaseStatus;
    PathHttpHeaders HttpHeaders;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
    Azure::Core::Nullable<bool> AccessTierInferred;
    Azure::Core::Nullable<Core::DateTime> AccessTierChangedOn;
    Azure::Core::Nullable<std::string> CopyId;
    Azure::Core::Nullable<std::string> CopySource;
    Azure::Core::Nullable<Blobs::Models::CopyStatus> CopyStatus;
    Azure::Core::Nullable<std::string> CopyProgress;
    Azure::Core::Nullable<Core::DateTime> CopyCompletedOn;
    Azure::Core::Nullable<Core::DateTime> ExpiresOn;
    Azure::Core::Nullable<Core::DateTime> LastAccessedOn;
    bool IsDirectory = false;
    Azure::Core::Nullable<DataLakeArchiveStatus> ArchiveStatus;
    Azure::Core::Nullable<Models::RehydratePriority> RehydratePriority;
    Azure::Core::Nullable<std::string> CopyStatusDescription;
    Azure::Core::Nullable<bool> IsIncrementalCopy;
    Azure::Core::Nullable<std::string> IncrementalCopyDestinationSnapshot;
    Azure::Core::Nullable<std::string> VersionId;
    Azure::Core::Nullable<bool> IsCurrentVersion;
    std::string RequestId;
  };

  struct GetDataLakePathAccessControlListResult
  {
    Azure::Core::ETag ETag;
    Core::DateTime LastModified;
    std::string Owner;
    std::string Group;
    std::string Permissions;
    std::vector<Acl> Acls;
    std::string RequestId;
  };

  struct SetDataLakePathHttpHeadersResult
  {
    Azure::Core::ETag ETag;
    Core::DateTime LastModified;
    std::string RequestId;
  };

  struct SetDataLakePathMetadataResult
  {
    Azure::Core::ETag ETag;
    Core::DateTime LastModified;
    std::string RequestId;
  };

  struct CreateDataLakePathResult
  {
    bool Created = true;
    Azure::Core::ETag ETag;
    Core::DateTime LastModified;
    Azure::Core::Nullable<int64_t> FileSize;
    std::string RequestId;
  };

  using SetDataLakePathAccessControlListResult = Details::PathSetAccessControlResult;
  using SetDataLakePathPermissionsResult = Details::PathSetAccessControlResult;

  // FileClient models:

  using UploadDataLakeFileFromResult = Blobs::Models::UploadBlockBlobResult;
  using AppendDataLakeFileResult = Details::PathAppendDataResult;
  using FlushDataLakeFileResult = Details::PathFlushDataResult;
  using ScheduleDataLakeFileDeletionResult = Blobs::Models::SetBlobExpiryResult;
  using CopyStatus = Blobs::Models::CopyStatus;

  struct DownloadDataLakeFileResult
  {
    std::unique_ptr<Azure::Core::Http::BodyStream> Body;
    PathHttpHeaders HttpHeaders;
    int64_t FileSize = int64_t();
    Azure::Core::Http::Range ContentRange;
    Azure::Core::Nullable<Storage::ContentHash> TransactionalContentHash;
    Azure::Core::ETag ETag;
    Core::DateTime LastModified;
    Azure::Core::Nullable<LeaseDurationType> LeaseDuration;
    LeaseStateType LeaseState;
    LeaseStatusType LeaseStatus;
    Storage::Metadata Metadata;
    Core::DateTime CreatedOn;
    Azure::Core::Nullable<Core::DateTime> ExpiresOn;
    Azure::Core::Nullable<Core::DateTime> LastAccessedOn;
    Azure::Core::Nullable<std::string> CopyId;
    Azure::Core::Nullable<std::string> CopySource;
    Azure::Core::Nullable<Models::CopyStatus> CopyStatus;
    Azure::Core::Nullable<std::string> CopyStatusDescription;
    Azure::Core::Nullable<std::string> CopyProgress;
    Azure::Core::Nullable<Azure::Core::DateTime> CopyCompletedOn;
    Azure::Core::Nullable<std::string> VersionId;
    Azure::Core::Nullable<bool> IsCurrentVersion;
    std::string RequestId;
  };

  struct DeleteDataLakeFileResult
  {
    bool Deleted = true;
    std::string RequestId;
  };

  struct DownloadDataLakeFileToResult
  {
    Azure::Core::ETag ETag;
    Core::DateTime LastModified;
    int64_t ContentLength = 0;
    PathHttpHeaders HttpHeaders;
    Storage::Metadata Metadata;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
    std::string RequestId;
  };

  using CreateDataLakeFileResult = CreateDataLakePathResult;

  // DirectoryClient models:

  struct RenameDataLakeDirectoryResult
  {
    Azure::Core::Nullable<std::string> ContinuationToken;
    std::string RequestId;
  };

  using SetDataLakePathAccessControlRecursiveListSinglePageResult
      = Details::PathSetAccessControlRecursiveResult;
  using UpdateDataLakePathAccessControlRecursiveListSinglePageResult
      = SetDataLakePathAccessControlRecursiveListSinglePageResult;
  using RemoveDataLakePathAccessControlRecursiveListSinglePageResult
      = SetDataLakePathAccessControlRecursiveListSinglePageResult;
  using CreateDataLakeDirectoryResult = CreateDataLakePathResult;
  using DeleteDataLakeDirectoryResult = DeleteDataLakePathResult;

}}}}} // namespace Azure::Storage::Files::DataLake::Models
