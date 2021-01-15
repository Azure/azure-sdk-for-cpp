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
  using ListFileSystemsSinglePageResult = ServiceListFileSystemsResult;

  // FileSystemClient models:

  using ListPathsSinglePageResult = FileSystemListPathsResult;

  struct GetDataLakeFileSystemPropertiesResult
  {
    std::string ETag;
    Core::DateTime LastModified;
    Storage::Metadata Metadata;
  };

  struct CreateDataLakeFileSystemResult
  {
    bool Created = true;
    std::string ETag;
    Core::DateTime LastModified;
  };

  struct DeleteDataLakeFileSystemResult
  {
    bool Deleted = true;
  };

  using SetDataLakeFileSystemMetadataResult = FileSystemCreateResult;

  // PathClient models:

  struct DeleteDataLakePathResult
  {
    bool Deleted = true;
    Azure::Core::Nullable<std::string> ContinuationToken;
  };

  using AcquireDataLakePathLeaseResult = Blobs::Models::AcquireBlobLeaseResult;
  using RenewDataLakePathLeaseResult = Blobs::Models::RenewBlobLeaseResult;
  using ReleaseDataLakePathLeaseResult = Blobs::Models::ReleaseBlobLeaseResult;
  using ChangeDataLakePathLeaseResult = Blobs::Models::ChangeBlobLeaseResult;
  using BreakDataLakePathLeaseResult = Blobs::Models::BreakBlobLeaseResult;

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
    std::string ETag;
    Core::DateTime LastModified;
    Core::DateTime CreatedOn;
    int64_t ContentLength;
    Storage::Metadata Metadata;
    Azure::Core::Nullable<std::string> LeaseDuration;
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
  };

  struct GetDataLakePathAccessControlResult
  {
    std::string ETag;
    Core::DateTime LastModified;
    std::vector<Acl> Acls;
  };

  struct SetDataLakePathHttpHeadersResult
  {
    std::string ETag;
    Core::DateTime LastModified;
  };

  struct SetDataLakePathMetadataResult
  {
    std::string ETag;
    Core::DateTime LastModified;
  };

  struct CreateDataLakePathResult
  {
    bool Created = true;
    std::string ETag;
    Core::DateTime LastModified;
    Azure::Core::Nullable<int64_t> ContentLength;
  };

  using SetDataLakePathAccessControlResult = PathSetAccessControlResult;

  // FileClient models:

  using UploadDataLakeFileFromResult = Blobs::Models::UploadBlockBlobResult;
  using AppendDataLakeFileDataResult = PathAppendDataResult;
  using FlushDataLakeFileDataResult = PathFlushDataResult;
  using ScheduleDataLakeFileDeletionResult = Blobs::Models::SetBlobExpiryResult;

  struct ReadDataLakeFileResult
  {
    std::unique_ptr<Azure::Core::Http::BodyStream> Body;
    PathHttpHeaders HttpHeaders;
    Azure::Core::Nullable<std::string> ContentRange;
    Azure::Core::Nullable<Storage::ContentHash> TransactionalContentHash;
    std::string ETag;
    Core::DateTime LastModified;
    Azure::Core::Nullable<std::string> LeaseDuration;
    LeaseStateType LeaseState;
    LeaseStatusType LeaseStatus;
    Storage::Metadata Metadata;
    Core::DateTime CreatedOn;
    Azure::Core::Nullable<Core::DateTime> ExpiresOn;
    Azure::Core::Nullable<Core::DateTime> LastAccessedOn;
  };

  struct RenameDataLakeFileResult
  {
  };

  struct DeleteDataLakeFileResult
  {
    bool Deleted = true;
  };

  struct DownloadDataLakeFileToResult
  {
    std::string ETag;
    Core::DateTime LastModified;
    int64_t ContentLength = 0;
    PathHttpHeaders HttpHeaders;
    Storage::Metadata Metadata;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
  };

  using CreateDataLakeFileResult = CreateDataLakePathResult;

  // DirectoryClient models:

  struct RenameDataLakeDirectoryResult
  {
    Azure::Core::Nullable<std::string> ContinuationToken;
  };

  using SetDataLakeDirectoryAccessControlRecursiveResult = PathSetAccessControlRecursiveResult;
  using CreateDataLakeDirectoryResult = CreateDataLakePathResult;
  using DeleteDataLakeDirectoryResult = DeleteDataLakePathResult;

}}}}} // namespace Azure::Storage::Files::DataLake::Models
