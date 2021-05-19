// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <azure/storage/blobs/blob_responses.hpp>

#include "azure/storage/files/datalake/datalake_options.hpp"
#include "azure/storage/files/datalake/protocol/datalake_rest_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  class DataLakeServiceClient;
  class DataLakePathClient;

  namespace Models {

    // ServiceClient models:

    using UserDelegationKey = Blobs::Models::UserDelegationKey;

    /**
     * @brief The detailed information of a file system.
     */
    struct FileSystemItemDetails final
    {
      /**
       * An HTTP entity tag associated with the file system.
       */
      Azure::ETag ETag;

      /**
       * The data and time the file system was last modified.
       */
      Azure::DateTime LastModified;

      /**
       * The Metadata of the file system.
       */
      Storage::Metadata Metadata;

      /**
       * The public access type of the file system.
       */
      PublicAccessType AccessType = PublicAccessType::None;

      /**
       * A boolean that indicates if the file system has immutability policy.
       */
      bool HasImmutabilityPolicy = false;

      /**
       * A boolean that indicates if the file system has legal hold.
       */
      bool HasLegalHold = false;

      /**
       * The duration of the lease on the file system if it has one.
       */
      Azure::Nullable<Models::LeaseDuration> LeaseDuration;

      /**
       * The lease state of the file system.
       */
      Models::LeaseState LeaseState = Models::LeaseState::Available;

      /**
       * The lease status of the file system.
       */
      Models::LeaseStatus LeaseStatus = Models::LeaseStatus::Unlocked;
    }; // struct FileSystemItemDetails

    /**
     * @brief The file system item returned when listing the file systems.
     */
    struct FileSystemItem final
    {
      /**
       * The name of the file system.
       */
      std::string Name;

      /**
       * The detailed information of the file system.
       */
      FileSystemItemDetails Details;
    }; // struct BlobContainerItem

    // FileSystemClient models:

    /**
     * @brief The access policy of a file system.
     */
    struct FileSystemAccessPolicy final
    {
      /**
       * The public access type of the file system.
       */
      PublicAccessType AccessType = PublicAccessType::None;

      /**
       * The signed identifiers of the file system.
       */
      std::vector<SignedIdentifier> SignedIdentifiers;
    };

    using SetFileSystemAccessPolicyResult = Blobs::Models::SetBlobContainerAccessPolicyResult;

    /**
     * @brief The properties of a file system.
     */
    struct FileSystemProperties final
    {
      /**
       * An HTTP entity tag associated with the file system.
       */
      Azure::ETag ETag;

      /**
       * The data and time the file system was last modified.
       */
      DateTime LastModified;

      /**
       * The Metadata of the file system.
       */
      Storage::Metadata Metadata;
    };

    /**
     * @brief The information returned when creating the file system.
     */
    struct CreateFileSystemResult final
    {
      /**
       * If the object is created.
       */
      bool Created = true;

      /**
       * An HTTP entity tag associated with the file system.
       */
      Azure::ETag ETag;

      /**
       * The data and time the file system was last modified.
       */
      DateTime LastModified;
    };

    /**
     * @brief The information returned when deleting the file system.
     */
    struct DeleteFileSystemResult final
    {
      /**
       * If the object is deleted.
       */
      bool Deleted = true;
    };

    /**
     * @brief The information returned when setting the filesystem's metadata
     */
    struct SetFileSystemMetadataResult final
    {
      /**
       * An HTTP entity tag associated with the file system.
       */
      Azure::ETag ETag;

      /**
       * The data and time the file system was last modified.
       */
      DateTime LastModified;
    };

    // PathClient models:

    /**
     * @brief The information returned when deleting the path.
     */
    struct DeletePathResult final
    {
      /**
       * If the object is deleted.
       */
      bool Deleted = true;
    };

    using AcquireLeaseResult = Blobs::Models::AcquireLeaseResult;
    using RenewLeaseResult = Blobs::Models::RenewLeaseResult;
    using ReleaseLeaseResult = Blobs::Models::ReleaseLeaseResult;
    using ChangeLeaseResult = Blobs::Models::ChangeLeaseResult;
    using BreakLeaseResult = Blobs::Models::BreakLeaseResult;
    using RehydratePriority = Blobs::Models::RehydratePriority;
    using ArchiveStatus = Blobs::Models::ArchiveStatus;

    /**
     * @brief An access control object.
     */
    struct Acl final
    {
      /**
       * The scope of the ACL.
       */
      std::string Scope;

      /**
       * The type of the ACL.
       */
      std::string Type;

      /**
       * The ID of the ACL.
       */
      std::string Id;

      /**
       * The permissions of the ACL.
       */
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
      static std::string SerializeAcls(const std::vector<Acl>& aclsArray);
    };

    /**
     * @brief The properties of the path.
     */
    struct PathProperties final
    {
      /**
       * An HTTP entity tag associated with the path.
       */
      Azure::ETag ETag;

      /**
       * The data and time the path was last modified.
       */
      DateTime LastModified;

      /**
       * The date and time at which the path was created.
       */
      DateTime CreatedOn;

      /**
       * The size of the file.
       */
      int64_t FileSize = 0;

      /**
       * The metadata of the path.
       */
      Storage::Metadata Metadata;

      /**
       * The duration of the lease on the path.
       */
      Azure::Nullable<Models::LeaseDuration> LeaseDuration;

      /**
       * The state of the lease on the path.
       */
      Azure::Nullable<Models::LeaseState> LeaseState;

      /**
       * The status of the lease on the path.
       */
      Azure::Nullable<Models::LeaseStatus> LeaseStatus;

      /**
       * The common HTTP headers of the path.
       */
      PathHttpHeaders HttpHeaders;

      /**
       * A boolean indicates if the server is encrypted.
       */
      Azure::Nullable<bool> IsServerEncrypted;

      /**
       * The encryption key's SHA256.
       */
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;

      /**
       * The copy ID of the path, if the path is created from a copy operation.
       */
      Azure::Nullable<std::string> CopyId;

      /**
       * The copy source of the path, if the path is created from a copy operation.
       */
      Azure::Nullable<std::string> CopySource;

      /**
       * The copy status of the path, if the path is created from a copy operation.
       */
      Azure::Nullable<Blobs::Models::CopyStatus> CopyStatus;

      /**
       * The copy progress of the path, if the path is created from a copy operation.
       */
      Azure::Nullable<std::string> CopyProgress;

      /**
       * The copy completion time of the path, if the path is created from a copy operation.
       */
      Azure::Nullable<DateTime> CopyCompletedOn;

      /**
       * The expiry time of the path.
       */
      Azure::Nullable<DateTime> ExpiresOn;

      /**
       * The time this path is last accessed on.
       */
      Azure::Nullable<DateTime> LastAccessedOn;

      /**
       * A boolean indicates if the path is a directory.
       */
      bool IsDirectory = false;

      /**
       * The archive status of the path.
       */
      Azure::Nullable<Models::ArchiveStatus> ArchiveStatus;

      /**
       * The rehydrate priority of the path.
       */
      Azure::Nullable<Models::RehydratePriority> RehydratePriority;

      /**
       * The copy status's description of the path, if the path is created from a copy operation.
       */
      Azure::Nullable<std::string> CopyStatusDescription;

      /**
       * A boolean indicates if the path has been incremental copied.
       */
      Azure::Nullable<bool> IsIncrementalCopy;

      /**
       * The incremental copy destination snapshot of the path.
       */
      Azure::Nullable<std::string> IncrementalCopyDestinationSnapshot;

      /**
       * The version ID of the path.
       */
      Azure::Nullable<std::string> VersionId;

      /**
       * A boolean indicates if the path is in its current version.
       */
      Azure::Nullable<bool> IsCurrentVersion;
    };

    /**
     * @brief The access control list of a path.
     */
    struct PathAccessControlList final
    {
      /**
       * The owner of the path.
       */
      std::string Owner;

      /**
       * The group of the path.
       */
      std::string Group;

      /**
       * The permission of the path.
       */
      std::string Permissions;

      /**
       * The acls of the path.
       */
      std::vector<Acl> Acls;
    };

    /**
     * @brief The information returned when setting the path's HTTP headers.
     */
    struct SetPathHttpHeadersResult final
    {
      /**
       * An HTTP entity tag associated with the path.
       */
      Azure::ETag ETag;

      /**
       * The data and time the path was last modified.
       */
      DateTime LastModified;
    };

    /**
     * @brief The information returned when setting the path's metadata.
     */
    struct SetPathMetadataResult final
    {
      /**
       * An HTTP entity tag associated with the path.
       */
      Azure::ETag ETag;

      /**
       * The data and time the path was last modified.
       */
      DateTime LastModified;
    };

    /**
     * @brief The information returned when creating a path.
     */
    struct CreatePathResult final
    {
      /**
       * A boolean indicates if the path is created.
       */
      bool Created = true;

      /**
       * An HTTP entity tag associated with the path.
       */
      Azure::ETag ETag;

      /**
       * The data and time the path was last modified.
       */
      DateTime LastModified;

      /**
       * The size of the file.
       */
      Azure::Nullable<int64_t> FileSize;
    };

    using SetPathPermissionsResult = SetPathAccessControlListResult;

    // FileClient models:

    using UploadFileFromResult = Blobs::Models::UploadBlockBlobResult;
    using ScheduleFileDeletionResult = Blobs::Models::SetBlobExpiryResult;
    using CopyStatus = Blobs::Models::CopyStatus;

    /**
     * @brief The detailed information returned when downloading a file.
     */
    struct DownloadFileDetails final
    {
      /**
       * An HTTP entity tag associated with the file.
       */
      Azure::ETag ETag;

      /**
       * The data and time the file was last modified.
       */
      DateTime LastModified;

      /**
       * The lease duration of the file.
       */
      Azure::Nullable<Models::LeaseDuration> LeaseDuration;

      /**
       * The lease state of the file.
       */
      Models::LeaseState LeaseState;

      /**
       * The lease status of the file.
       */
      Models::LeaseStatus LeaseStatus;

      /**
       * The common HTTP headers of the file.
       */
      PathHttpHeaders HttpHeaders;

      /**
       * The metadata of the file.
       */
      Storage::Metadata Metadata;

      /**
       * The time this file is created on.
       */
      DateTime CreatedOn;

      /**
       * The time this file expires on.
       */
      Azure::Nullable<DateTime> ExpiresOn;

      /**
       * The time this file is last accessed on.
       */
      Azure::Nullable<DateTime> LastAccessedOn;

      /**
       * The copy ID of the file, if the file is created from a copy operation.
       */
      Azure::Nullable<std::string> CopyId;

      /**
       * The copy source of the file, if the file is created from a copy operation.
       */
      Azure::Nullable<std::string> CopySource;

      /**
       * The copy status of the file, if the file is created from a copy operation.
       */
      Azure::Nullable<Models::CopyStatus> CopyStatus;

      /**
       * The copy status's description of the file, if the file is created from a copy operation.
       */
      Azure::Nullable<std::string> CopyStatusDescription;

      /**
       * The copy progress of the file, if the file is created from a copy operation.
       */
      Azure::Nullable<std::string> CopyProgress;

      /**
       * The copy completed time of the file, if the file is created from a copy operation.
       */
      Azure::Nullable<Azure::DateTime> CopyCompletedOn;

      /**
       * The version ID of the file.
       */
      Azure::Nullable<std::string> VersionId;

      /**
       * If the file is in its current version.
       */
      Azure::Nullable<bool> IsCurrentVersion;

      /**
       * A boolean indicates if the service is encrypted.
       */
      bool IsServerEncrypted = false;

      /**
       * The encryption key's SHA256.
       */
      Azure::Nullable<std::vector<uint8_t>> EncryptionKeySha256;

      /**
       * The encryption scope.
       */
      Azure::Nullable<std::string> EncryptionScope;
    };

    /**
     * @brief The content and information returned when downloading a file.
     */
    struct DownloadFileResult final
    {
      /**
       * The body of the downloaded result.
       */
      std::unique_ptr<Azure::Core::IO::BodyStream> Body;

      /**
       * The size of the file.
       */
      int64_t FileSize = int64_t();

      /**
       * The range of the downloaded content.
       */
      Azure::Core::Http::HttpRange ContentRange;

      /**
       * The transactional hash of the downloaded content.
       */
      Azure::Nullable<Storage::ContentHash> TransactionalContentHash;

      /**
       * The detailed information of the downloaded file.
       */
      DownloadFileDetails Details;
    };

    /**
     * @brief The information returned when deleting a file.
     */
    struct DeleteFileResult final
    {
      /**
       * A boolean indicates if the file is deleted.
       */
      bool Deleted = true;
    };

    /**
     * @brief The information returned when downloading a file to a specific destination.
     */
    struct DownloadFileToResult final
    {
      /**
       * The size of the file.
       */
      int64_t FileSize = int64_t();

      /**
       * The range of the downloaded content.
       */
      Azure::Core::Http::HttpRange ContentRange;

      /**
       * The detailed information of the downloaded file.
       */
      DownloadFileDetails Details;
    };

    using CreateFileResult = CreatePathResult;

    // DirectoryClient models:

    using CreateDirectoryResult = CreatePathResult;
    using DeleteDirectoryResult = DeletePathResult;

  } // namespace Models

  /**
   * @brief Response type for
   * #Azure::Storage::Files::DataLake::DataLakeServiceClient::ListFileSystems.
   */
  class ListFileSystemsPagedResponse final
      : public Azure::Core::PagedResponse<ListFileSystemsPagedResponse> {
  public:
    /**
     * Service endpoint.
     */
    std::string ServiceEndpoint;
    /**
     * File system name prefix that's used to filter the result.
     */
    std::string Prefix;
    /**
     * File system items.
     */
    std::vector<Models::FileSystemItem> FileSystems;

  private:
    void OnNextPage(const Azure::Core::Context& context);

    std::shared_ptr<DataLakeServiceClient> m_dataLakeServiceClient;
    ListFileSystemsOptions m_operationOptions;

    friend class DataLakeServiceClient;
    friend class Azure::Core::PagedResponse<ListFileSystemsPagedResponse>;
  };

  /**
   * @brief Response type for #Azure::Storage::Files::DataLake::DataLakeFileSystemClient::ListPaths
   * and #Azure::Storage::Files::DataLake::DataLakeDirectoryClient::ListPaths.
   */
  class ListPathsPagedResponse final : public Azure::Core::PagedResponse<ListPathsPagedResponse> {
  public:
    /**
     * Path items.
     */
    std::vector<Models::PathItem> Paths;

  private:
    void OnNextPage(const Azure::Core::Context& context);

    std::function<ListPathsPagedResponse(std::string, const Azure::Core::Context&)>
        m_onNextPageFunc;

    friend class DataLakeFileSystemClient;
    friend class DataLakeDirectoryClient;
    friend class Azure::Core::PagedResponse<ListPathsPagedResponse>;
  };

  /**
   * @brief Response type for
   * #Azure::Storage::Files::DataLake::DataLakePathClient::SetAccessControlListRecursive.
   */
  class SetPathAccessControlListRecursivePagedResponse final
      : public Azure::Core::PagedResponse<SetPathAccessControlListRecursivePagedResponse> {
  public:
    /**
     * Number of directories where Access Control List has been updated successfully.
     */
    int32_t NumberOfSuccessfulDirectories = 0;
    /**
     * Number of files where Access Control List has been updated successfully.
     */
    int32_t NumberOfSuccessfulFiles = 0;
    /**
     * Number of paths where Access Control List update has failed.
     */
    int32_t NumberOfFailures = 0;
    /**
     * A collection of path entries that failed to update ACL.
     */
    std::vector<Models::AclFailedEntry> FailedEntries;

  private:
    void OnNextPage(const Azure::Core::Context& context);

    std::shared_ptr<DataLakePathClient> m_dataLakePathClient;
    SetPathAccessControlListRecursiveOptions m_operationOptions;
    std::vector<Models::Acl> m_acls;
    _detail::PathSetAccessControlRecursiveMode m_mode;

    friend class DataLakePathClient;
    friend class Azure::Core::PagedResponse<SetPathAccessControlListRecursivePagedResponse>;
  };

  using UpdatePathAccessControlListRecursivePagedResponse
      = SetPathAccessControlListRecursivePagedResponse;
  using RemovePathAccessControlListRecursivePagedResponse
      = SetPathAccessControlListRecursivePagedResponse;

}}}} // namespace Azure::Storage::Files::DataLake
