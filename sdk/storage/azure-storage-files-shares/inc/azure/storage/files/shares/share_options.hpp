// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/storage/files/shares/rest_client.hpp"

#include <azure/core/internal/client_options.hpp>
#include <azure/core/internal/extendable_enumeration.hpp>
#include <azure/core/nullable.hpp>
#include <azure/storage/common/access_conditions.hpp>

#include <memory>
#include <string>
#include <vector>

/* cSpell:ignore dacl */

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  namespace Models {
    enum class RolePermissions
    {
      /*
       * @brief No permissions.
       */
      None = 0,

      /*
       * @brief The execute permission.
       */
      Execute = 1,

      /*
       * @brief The write permission.
       */
      Write = 2,

      /*
       * @brief The read permission.
       */
      Read = 4,
    };

    inline RolePermissions operator|(const RolePermissions& lhs, const RolePermissions& rhs)
    {
      using type = std::underlying_type_t<RolePermissions>;
      return static_cast<RolePermissions>(static_cast<type>(lhs) | static_cast<type>(rhs));
    }

    inline RolePermissions operator&(const RolePermissions& lhs, const RolePermissions& rhs)
    {
      using type = std::underlying_type_t<RolePermissions>;
      return static_cast<RolePermissions>(static_cast<type>(lhs) & static_cast<type>(rhs));
    }

    /**
     * @brief The mode permissions of the file or directory.
     */
    struct NfsFileMode final
    {
      /**
       * @brief Permissions the owner has over the file or directory.
       */
      RolePermissions Owner;

      /**
       * @brief Permissions the group has over the file or directory.
       */
      RolePermissions Group;

      /**
       * @brief Permissions other have over the file or directory.
       */
      RolePermissions Other;

      /**
       * @brief Set effective user ID (setuid) on the file or directory.
       */
      bool EffectiveUserIdentity = false;

      /**
       * @brief Set effective group ID (setgid) on the file or directory.
       */
      bool EffectiveGroupIdentity = false;

      /**
       * @brief The sticky bit may be set on directories. The files in that directory may only be
       * renamed or deleted by the file's owner, the directory's owner, or the root user.
       */
      bool StickyBit = false;

      /**
       * @brief Returns the octal representation of NfsFileMode as a string.
       */
      std::string ToOctalFileMode() const;

      /**
       * @brief Returns NfsFileMode as a string in symbolic notation.
       */
      std::string ToSymbolicFileMode() const;

      /**
       * @brief Returns a NfsFileMode from the octal string representation.
       *
       * @param modeString A 4-digit octal string representation of a File Mode.
       */
      static NfsFileMode ParseOctalFileMode(const std::string& modeString);

      /**
       * @brief Returns a NfsFileMode from the symbolic string representation.
       *
       * @param modeString A 9-character symbolic string representation of a File Mode.
       */
      static NfsFileMode ParseSymbolicFileMode(const std::string& modeString);
    };

    /**
     * @brief NFS properties. Note that these properties only apply to files or directories in
     * premium NFS file accounts.
     */
    struct FilePosixProperties final
    {
      /**
       * NFS only. The mode of the file or directory.
       */
      Nullable<NfsFileMode> FileMode;

      /**
       * NFS only. The owner of the file or directory.
       */
      Nullable<std::string> Owner;

      /**
       * NFS only. The owning group of the file or directory.
       */
      Nullable<std::string> Group;

      /**
       * NFS only. Type of the file or directory.
       */
      Nullable<Models::NfsFileType> NfsFileType;

      /**
       * NFS only. The link count of the file or directory.
       */
      Nullable<std::int64_t> LinkCount;
    };
  } // namespace Models

  /**
   * Smb Properties to copy from the source file.
   */
  enum class CopyableFileSmbPropertyFlags
  {
    /**
     * None.
     */
    None = 0,

    /**
     * File Attributes.
     */
    FileAttributes = 1,

    /**
     * Created On.
     */
    CreatedOn = 2,

    /**
     * Last Written On.
     */
    LastWrittenOn = 4,

    /**
     * Changed On.
     */
    ChangedOn = 8,

    /**
     * Permission
     */
    Permission = 16,

    /**
     * All.
     */
    All = ~None
  };

  inline CopyableFileSmbPropertyFlags operator|(
      CopyableFileSmbPropertyFlags lhs,
      CopyableFileSmbPropertyFlags rhs)
  {
    using type = std::underlying_type_t<CopyableFileSmbPropertyFlags>;
    return static_cast<CopyableFileSmbPropertyFlags>(
        static_cast<type>(lhs) | static_cast<type>(rhs));
  }
  inline CopyableFileSmbPropertyFlags& operator|=(
      CopyableFileSmbPropertyFlags& lhs,
      CopyableFileSmbPropertyFlags rhs)
  {
    lhs = lhs | rhs;
    return lhs;
  }
  inline CopyableFileSmbPropertyFlags operator&(
      CopyableFileSmbPropertyFlags lhs,
      CopyableFileSmbPropertyFlags rhs)
  {
    using type = std::underlying_type_t<CopyableFileSmbPropertyFlags>;
    return static_cast<CopyableFileSmbPropertyFlags>(
        static_cast<type>(lhs) & static_cast<type>(rhs));
  }
  inline CopyableFileSmbPropertyFlags& operator&=(
      CopyableFileSmbPropertyFlags& lhs,
      CopyableFileSmbPropertyFlags rhs)
  {
    lhs = lhs & rhs;
    return lhs;
  }

  /**
   * @brief Audiences available for share service
   *
   */
  class ShareAudience final : public Azure::Core::_internal::ExtendableEnumeration<ShareAudience> {
  public:
    /**
     * @brief Construct a new ShareAudience object
     *
     * @param shareAudience The Azure Active Directory audience to use when forming authorization
     * scopes. For the Language service, this value corresponds to a URL that identifies the Azure
     * cloud where the resource is located. For more information: See
     * https://learn.microsoft.com/en-us/azure/storage/blobs/authorize-access-azure-active-directory
     */
    explicit ShareAudience(std::string shareAudience)
        : ExtendableEnumeration(std::move(shareAudience))
    {
    }

    /**
     * @brief The service endpoint for a given storage account. Use this method to acquire a token
     * for authorizing requests to that specific Azure Storage account and service only.
     *
     * @param storageAccountName he storage account name used to populate the service endpoint.
     * @return The service endpoint for a given storage account.
     */
    static ShareAudience CreateShareServiceAccountAudience(const std::string& storageAccountName)
    {
      return ShareAudience("https://" + storageAccountName + ".file.core.windows.net/");
    }

    /**
     * @brief Default Audience. Use to acquire a token for authorizing requests to any Azure
     * Storage account.
     */
    AZ_STORAGE_FILES_SHARES_DLLEXPORT const static ShareAudience DefaultAudience;
  };

  /**
   * Configures whether to do content validation for blob uploads and downloads.
   */
  struct TransferValidationOptions final
  {
    /**
     * @brief The algorithm used for storage checksum.
     */
    StorageChecksumAlgorithm Algorithm = StorageChecksumAlgorithm::None;
  };

  /**
   * @brief Client options used to initialize share clients.
   */
  struct ShareClientOptions final : Azure::Core::_internal::ClientOptions
  {
    /**
     * API version used by this client.
     */
    std::string ApiVersion;

    /**
     * If set to true, trailing dot (.) will be allowed to suffix directory and file names.
     * If false, the trailing dot will be trimmed.
     * Supported by x-ms-version 2022-11-02 and above.
     */
    Nullable<bool> AllowTrailingDot;

    /**
     * If set to true, trailing dot (.) will be allowed to source file names.
     * If false, the trailing dot will be trimmed.
     * Supported by x-ms-version 2022-11-02 and above.
     */
    Nullable<bool> AllowSourceTrailingDot;

    /**
     * Share Token Intent.  For use with token authentication.  Used to indicate the intent of the
     * request. This is currently required when using token authentication.
     */
    Nullable<Models::ShareTokenIntent> ShareTokenIntent;

    /**
     * The Audience to use for authentication with Azure Active Directory (AAD).
     * #Azure::Storage::Files::Shares::ShareAudience::DefaultAudience will be assumed if
     * Audience is not set.
     */
    Azure::Nullable<ShareAudience> Audience;

    /**
     * @brief Optional. Configures whether to do content validation for file uploads.
     */
    Azure::Nullable<TransferValidationOptions> UploadValidationOptions;

    /**
     * @brief Optional. Configures whether to do content validation for file downloads.
     */
    Azure::Nullable<TransferValidationOptions> DownloadValidationOptions;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareServiceClient::ListShares.
   */
  struct ListSharesOptions final
  {
    /**
     * Filters the results to return only entries whose name begins with the specified
     * prefix.
     */
    Azure::Nullable<std::string> Prefix;

    /**
     * A string value that identifies the portion of the list to be returned with the next
     * list operation. The operation returns a marker value within the response body if the list
     * returned was not complete. The marker value may then be used in a subsequent call to
     * request the next set of list items. The marker value is opaque to the client.
     */
    Azure::Nullable<std::string> ContinuationToken;

    /**
     * Specifies the maximum number of entries to return. If the request does not specify
     * PageSizeHint, or specifies a value greater than 5,000, the server will return up to 5,000
     * items.
     */
    Azure::Nullable<int32_t> PageSizeHint;

    /**
     * Include this parameter to specify one or more datasets to include in the response.
     */
    Azure::Nullable<Models::ListSharesIncludeFlags> ListSharesIncludeFlags;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareServiceClient::SetProperties.
   */
  struct SetServicePropertiesOptions final
  {
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareServiceClient::GetProperties.
   */
  struct GetServicePropertiesOptions final
  {
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareServiceClient::GetUserDelegationKey.
   */
  struct GetUserDelegationKeyOptions final
  {
    /**
     * @brief Start time for the key's validity. The time should be specified in UTC, and
     * will be truncated to second.
     */
    Azure::DateTime StartsOn = std::chrono::system_clock::now();

    /**
     * The delegated user tenant id in Azure AD.
     */
    Nullable<std::string> DelegatedUserTid;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareClient::Create.
   */
  struct CreateShareOptions final
  {
    /**
     * A name-value pair to associate with a file storage object.
     */
    Storage::Metadata Metadata;

    /**
     * Specifies the access tier of the share. This is only valid for standard file account
     * and the value can only be one of `Hot`, `Cool` or `TransactionOptimized`
     */
    Azure::Nullable<Models::AccessTier> AccessTier;

    /**
     * Specifies the maximum size of the share, in gigabytes.
     */
    Azure::Nullable<int64_t> ShareQuotaInGiB;

    /**
     * Specifies the enabled protocols on the share. If they're not specified, the default is SMB.
     */
    Azure::Nullable<Models::ShareProtocols> EnabledProtocols;

    /**
     * Specifies the root squashing behavior on the share when NFS is enabled. If it's not
     * specified, the default is NoRootSquash.
     */
    Azure::Nullable<Models::ShareRootSquash> RootSquash;

    /**
     * Version 2023-08-03 and newer. Specifies whether the snapshot virtual directory should be
     * accessible at the root of share mount point when NFS is enabled. This header is only
     * returned for shares, not for snapshots.
     */
    Nullable<bool> EnableSnapshotVirtualDirectoryAccess;

    /**
     * Optional. Boolean. Version 2023-11-03 and newer. Default if not specified is false. This
     * property enables paid bursting.
     */
    Nullable<bool> EnablePaidBursting;

    /**
     * Optional. Integer. Version 2023-11-03 and newer. Default if not specified is the maximum IOPS
     * the file share can support. Current maximum for a file share is 102,400 IOPS.
     */
    Nullable<std::int64_t> PaidBurstingMaxIops;

    /**
     * Optional. Integer. Version 2023-11-03 and newer. Default if not specified is the maximum
     * throughput the file share can support. Current maximum for a file share is 10,340 MiB/sec.
     */
    Nullable<std::int64_t> PaidBurstingMaxBandwidthMibps;

    /**
     * Optional. Integer. Version 2025-01-05 and newer. The provisioned IOPS of the share. For SSD,
     * minimum IOPS is 3,000 and maximum is 100,000. For HDD, minimum IOPS is 500 and maximum is
     * 50,000.
     */
    Nullable<std::int64_t> ProvisionedMaxIops;

    /**
     * Optional. Integer. Version 2025-01-05 and newer. The provisioned throughput of the share. For
     * SSD, minimum  throughput is 125 MiB/sec and maximum is 10,340 MiB/sec. For HDD, minimum
     * throughput is 60 MiB/sec and maximum is 5,125 MiB/sec.
     */
    Nullable<std::int64_t> ProvisionedMaxBandwidthMibps;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareClient::Delete.
   */
  struct DeleteShareOptions final
  {
    /**
     * Specifies the option include to delete the base share and all of its snapshots.
     */
    Azure::Nullable<bool> DeleteSnapshots;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareClient::CreateSnapshot.
   */
  struct CreateShareSnapshotOptions final
  {
    /**
     * The metadata to be set on the snapshot of the share.
     */
    Storage::Metadata Metadata;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareClient::GetProperties.
   */
  struct GetSharePropertiesOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareClient::SetProperties.
   */
  struct SetSharePropertiesOptions final
  {
    /**
     * Specifies the access tier of the share. This is only valid for standard file account
     * and the value can only be one of `Hot`, `Cool` or `TransactionOptimized`
     */
    Azure::Nullable<Models::AccessTier> AccessTier;

    /**
     * Specifies the maximum size of the share, in gigabytes.
     */
    Azure::Nullable<int64_t> ShareQuotaInGiB;

    /**
     * Specifies the root squashing behavior on the share when NFS is enabled. If it's not
     * specified, the default is NoRootSquash.
     */
    Azure::Nullable<Models::ShareRootSquash> RootSquash;

    /**
     * Version 2023-08-03 and newer. Specifies whether the snapshot virtual directory should be
     * accessible at the root of share mount point when NFS is enabled. This header is only
     * returned for shares, not for snapshots.
     */
    Nullable<bool> EnableSnapshotVirtualDirectoryAccess;

    /**
     * Optional. Boolean. Version 2023-11-03 and newer. Default if not specified is false. This
     * property enables paid bursting.
     */
    Nullable<bool> EnablePaidBursting;

    /**
     * Optional. Integer. Version 2023-11-03 and newer. Default if not specified is the maximum IOPS
     * the file share can support. Current maximum for a file share is 102,400 IOPS.
     */
    Nullable<std::int64_t> PaidBurstingMaxIops;

    /**
     * Optional. Integer. Version 2023-11-03 and newer. Default if not specified is the maximum
     * throughput the file share can support. Current maximum for a file share is 10,340 MiB/sec.
     */
    Nullable<std::int64_t> PaidBurstingMaxBandwidthMibps;

    /**
     * Optional. Boolean. Version 2025-01-05 and newer. Sets the max provisioned IOPs for a share.
     * For SSD, min IOPs is 3,000 and max is 100,000. For HDD, min IOPs is 500 and max is 50,000.
     */
    Nullable<std::int64_t> ProvisionedMaxIops;

    /**
     * Optional. Boolean. Version 2025-01-05 and newer. Sets the max provisioned brandwith for a
     * share.  For SSD, min bandwidth is 125 MiB/sec and max is 10,340 MiB/sec. For HDD, min
     * bandwidth is 60 MiB/sec and max is 5,120 MiB/sec.
     */
    Nullable<std::int64_t> ProvisionedMaxBandwidthMibps;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareClient::SetMetadata.
   */
  struct SetShareMetadataOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareClient::GetAccessPolicy.
   */
  struct GetShareAccessPolicyOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareClient::SetAccessPolicy.
   */
  struct SetShareAccessPolicyOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareClient::GetStatistics.
   */
  struct GetShareStatisticsOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareClient::CreatePermission.
   */
  struct CreateSharePermissionOptions final
  {
    /**
     * Optional. Available for version 2024-11-04 and later. Specifies
     * the format in which the permission is returned.If FilePermissionFormat is unspecified or
     * explicitly set to SDDL format, the permission will be
     * returned in SDDL format.
     */
    Nullable<Models::FilePermissionFormat> FilePermissionFormat;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareClient::GetPermission.
   */
  struct GetSharePermissionOptions final
  {
    /**
     * Optional. Available for version 2024-11-04 and later. Specifies
     * the format in which the permission is returned.If FilePermissionFormat is unspecified or
     * explicitly set to SDDL format, the permission will be
     * returned in SDDL format.
     */
    Nullable<Models::FilePermissionFormat> FilePermissionFormat;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareDirectoryClient::Create.
   */
  struct CreateDirectoryOptions final
  {
    /**
     * A name-value pair to associate with a directory object.
     */
    Storage::Metadata Metadata;

    /**
     * This permission is the security descriptor for the directory specified in the Security
     * Descriptor Definition Language (SDDL). If not specified, 'inherit' is used.
     */
    Azure::Nullable<std::string> DirectoryPermission;

    /**
     * Optional. Available for version 2024-11-04 and later. Specifies
     * the format in which the permission is returned.If DirectoryPermissionFormat is unspecified or
     * explicitly set to SDDL format, the permission will be
     * returned in SDDL format.
     */
    Nullable<Models::FilePermissionFormat> DirectoryPermissionFormat;

    /**
     * SMB properties to set for the directory.
     */
    Models::FileSmbProperties SmbProperties;

    /**
     * The NFS related properties for the file.
     */
    Models::FilePosixProperties PosixProperties;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareDirectoryClient::RenameFile.
   */
  struct RenameFileOptions final
  {
    /**
     * A boolean value for if the destination file already exists, whether this request will
     * overwrite the file or not. If true, the rename will succeed and will overwrite the
     * destination file. If not provided or if false and the destination file does exist, the
     * request will not overwrite the destination file. If provided and the destination file doesn't
     * exist, the rename will succeed.
     */
    Azure::Nullable<bool> ReplaceIfExists;

    /**
     * A boolean value that specifies whether the ReadOnly attribute on a preexisting destination
     * file should be respected. If true, the rename will succeed, otherwise, a previous file at the
     * destination with the ReadOnly attribute set will cause the rename to fail. ReplaceIfExists
     * must also be true.
     */
    Azure::Nullable<bool> IgnoreReadOnly;

    /**
     * Specify the access condition for the path.
     */
    LeaseAccessConditions AccessConditions;

    /**
     * The access condition for source path.
     */
    LeaseAccessConditions SourceAccessConditions;

    /**
     * SMB properties to set for the directory.
     */
    Models::FileSmbProperties SmbProperties;

    /**
     * If specified the permission (security descriptor) shall be set for the directory.
     * This option can be used if Permission size is <= 8KB, else SmbProperties.PermissionKey
     * shall be used.A value of preserve may be passed to keep an existing value unchanged.
     */
    Azure::Nullable<std::string> FilePermission;

    /**
     * Optional. Available for version 2024-11-04 and later. Specifies
     * the format in which the permission is returned.If FilePermissionFormat is unspecified or
     * explicitly set to SDDL format, the permission will be
     * returned in SDDL format.
     */
    Nullable<Models ::FilePermissionFormat> FilePermissionFormat;

    /**
     * A name-value pair to associate with a file storage object.
     */
    Storage::Metadata Metadata;

    /**
     * Content type to set on the File.
     */
    Azure::Nullable<std::string> ContentType;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareDirectoryClient::RenameSubdirectory.
   */
  struct RenameDirectoryOptions final
  {
    /**
     * A boolean value for if the destination directory already exists, whether this request will
     * overwrite the file or not. If true, the rename will succeed and will overwrite the
     * destination directory. If not provided or if false and the destination directory does exist,
     * the request will not overwrite the destination directory. If provided and the destination
     * file doesn't exist, the rename will succeed.
     */
    Azure::Nullable<bool> ReplaceIfExists;

    /**
     * A boolean value that specifies whether the ReadOnly attribute on a preexisting destination
     * directory should be respected. If true, the rename will succeed, otherwise, a previous file
     * at the destination with the ReadOnly attribute set will cause the rename to fail.
     * ReplaceIfExists must also be true.
     */
    Azure::Nullable<bool> IgnoreReadOnly;

    /**
     * Specify the access condition for the path.
     */
    LeaseAccessConditions AccessConditions;

    /**
     * The access condition for source path.
     */
    LeaseAccessConditions SourceAccessConditions;

    /**
     * SMB properties to set for the directory.
     */
    Models::FileSmbProperties SmbProperties;

    /**
     * If specified the permission (security descriptor) shall be set for the directory.
     * This option can be used if Permission size is <= 8KB, else SmbProperties.PermissionKey
     * shall be used.A value of preserve may be passed to keep an existing value unchanged.
     */
    Azure::Nullable<std::string> FilePermission;

    /**
     * Optional. Available for version 2024-11-04 and later. Specifies
     * the format in which the permission is returned.If FilePermissionFormat is unspecified or
     * explicitly set to SDDL format, the permission will be
     * returned in SDDL format.
     */
    Nullable<Models ::FilePermissionFormat> FilePermissionFormat;

    /**
     * A name-value pair to associate with a file storage object.
     */
    Storage::Metadata Metadata;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareDirectoryClient::Delete.
   */
  struct DeleteDirectoryOptions final
  {
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareDirectoryClient::GetProperties.
   */
  struct GetDirectoryPropertiesOptions final
  {
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareDirectoryClient::SetProperties.
   */
  struct SetDirectoryPropertiesOptions final
  {
    /**
     * If specified the permission (security descriptor) shall be set for the directory.
     * This option can be used if Permission size is <= 8KB, else SmbProperties.PermissionKey
     * shall be used. Default value: 'inherit'. If SDDL is specified as input, it must have owner,
     * group and dacl.
     */
    Azure::Nullable<std::string> FilePermission;

    /**
     * Optional. Available for version 2024-11-04 and later. Specifies
     * the format in which the permission is returned.If FilePermissionFormat is unspecified or
     * explicitly set to SDDL format format, the permission will be
     * returned in SDDL format.
     */
    Nullable<Models ::FilePermissionFormat> FilePermissionFormat;

    /**
     * The NFS related properties for the file.
     */
    Models::FilePosixProperties PosixProperties;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareDirectoryClient::SetMetadata.
   */
  struct SetDirectoryMetadataOptions final
  {
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareDirectoryClient::ListFilesAndDirectories.
   */
  struct ListFilesAndDirectoriesOptions final
  {
    /**
     * Filters the results to return only entries whose name begins with the specified
     * prefix.
     */
    Azure::Nullable<std::string> Prefix;

    /**
     * A string value that identifies the portion of the list to be returned with the next
     * list operation. The operation returns a marker value within the response body if the list
     * returned was not complete. The marker value may then be used in a subsequent call to
     * request the next set of list items. The marker value is opaque to the client.
     */
    Azure::Nullable<std::string> ContinuationToken;

    /**
     * Specifies the maximum number of entries to return. If the request does not specify
     * PageSizeHint, or specifies a value greater than 5,000, the server will return up to 5,000
     * items.
     */
    Azure::Nullable<int32_t> PageSizeHint;

    /**
     * Include this parameter to specify one or more datasets to include in the response.
     */
    Models::ListFilesIncludeFlags Include = Models::ListFilesIncludeFlags ::None;

    /**
     * This header is implicitly assumed to be true if include query parameter is not empty. If
     * true, the Content-Length property will be up to date.
     */
    Nullable<bool> IncludeExtendedInfo;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareDirectoryClient::ListHandles.
   */
  struct ListDirectoryHandlesOptions final
  {
    /**
     * A string value that identifies the portion of the list to be returned with the next
     * list operation. The operation returns a marker value within the response body if the list
     * returned was not complete. The marker value may then be used in a subsequent call to
     * request the next set of list items. The marker value is opaque to the client.
     */
    Azure::Nullable<std::string> ContinuationToken;

    /**
     * Specifies the maximum number of entries to return. If the request does not specify
     * PageSizeHint, or specifies a value greater than 5,000, the server will return up to 5,000
     * items.
     */
    Azure::Nullable<int32_t> PageSizeHint;

    /**
     * Specifies operation should apply to the directory specified in the URI, its files, its
     * subdirectories and their files.
     */
    Azure::Nullable<bool> Recursive;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareDirectoryClient::ForceCloseHandle.
   */
  struct ForceCloseDirectoryHandleOptions final
  {
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareDirectoryClient::ForceCloseAllHandles.
   */
  struct ForceCloseAllDirectoryHandlesOptions final
  {
    /**
     * A string value that identifies the portion of the list to be returned with the next
     * close operation. The operation returns a marker value within the response body if the force
     * close was not complete. The marker value may then be used in a subsequent call to
     * close the next handle. The marker value is opaque to the client.
     */
    Azure::Nullable<std::string> ContinuationToken;

    /**
     * @brief Specifies operation should apply to the directory specified in the URI, its files, its
     * subdirectories and their files.
     */
    Azure::Nullable<bool> Recursive;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::Create.
   */
  struct CreateFileOptions final
  {
    /**
     * This permission is the security descriptor for the file specified in the Security
     * Descriptor Definition Language (SDDL) or base64 encoded
     * binary format. If not specified, 'inherit' is used.
     */
    Azure::Nullable<std::string> Permission;

    /**
     * Optional. Available for version 2024-11-04 and later. Specifies
     * the format in which the permission is returned.If FilePermissionFormat is unspecified or
     * explicitly set to SDDL format format, the permission will be
     * returned in SDDL format.
     */
    Nullable<Models ::FilePermissionFormat> FilePermissionFormat;

    /**
     * SMB properties to set for the file.
     */
    Models::FileSmbProperties SmbProperties;

    /**
     * Specifies the HttpHeaders of the file.
     */
    Models::FileHttpHeaders HttpHeaders;

    /**
     * A name-value pair to associate with a file storage object.
     */
    Storage::Metadata Metadata;

    /**
     * The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;

    /**
     * The NFS related properties for the file.
     */
    Models::FilePosixProperties PosixProperties;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::Delete.
   */
  struct DeleteFileOptions final
  {
    /**
     * The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::Download.
   */
  struct DownloadFileOptions final
  {
    /**
     * Downloads only the bytes of the file from this range.
     */
    Azure::Nullable<Core::Http::HttpRange> Range;

    /**
     * When specified together with Range, service returns hash for the range as long as the
     * range is less than or equal to 4 MiB in size. Only MD5 is supported for now.
     */
    Azure::Nullable<HashAlgorithm> RangeHashAlgorithm;

    /**
     * The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;

    /**
     * @brief Optional. Configures whether to do content validation for blob downloads.
     */
    Azure::Nullable<TransferValidationOptions> ValidationOptions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::StartCopy.
   */
  struct StartFileCopyOptions final
  {
    /**
     * A name-value pair to associate with a file storage object.
     */
    Storage::Metadata Metadata;

    /**
     * This permission is the security descriptor for the file specified in the Security
     * Descriptor Definition Language (SDDL) or base64 encoded
     * binary format. If not specified, 'inherit' is used.
     */
    Azure::Nullable<std::string> Permission;

    /**
     * Optional. Available for version 2024-11-04 and later. Specifies
     * the format in which the permission is returned.If FilePermissionFormat is unspecified or
     * explicitly set to SDDL format format, the permission will be
     * returned in SDDL format.
     */
    Nullable<Models ::FilePermissionFormat> FilePermissionFormat;

    /**
     * SMB properties to set for the destination file.
     */
    Models::FileSmbProperties SmbProperties;

    /**
     * Specifies the option to copy file security descriptor from source file or to set it
     * using the value which is defined by the SMB properties.
     */
    Azure::Nullable<Models::PermissionCopyMode> PermissionCopyMode;

    /**
     * Smb Properties to copy from the source file.
     * If this flag is nullable, it will use the value of source file(except ChangedOn, it will be
     * default value) if the property is not set.
     * If this flag is disabled, it will use the default
     * value of destination file if the property is not set.
     * If this flag is enabled, it will use the value of source file no
     * matter the property is set or not.
     */
    Azure::Nullable<CopyableFileSmbPropertyFlags> SmbPropertiesToCopy;

    /**
     * Specifies the option to overwrite the target file if it already exists and has
     * read-only attribute set.
     */
    Azure::Nullable<bool> IgnoreReadOnly;

    /**
     * Specifies the option to set archive attribute on a target file. True means archive
     * attribute will be set on a target file despite attribute overrides or a source file state.
     */
    Azure::Nullable<bool> SetArchiveAttribute;

    /**
     * The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;

    /**
     * The NFS related properties for the file.
     */
    Models::FilePosixProperties PosixProperties;

    /**
     * Optional, only applicable to NFS Files. If not populated, the destination file will have the
     * default File Mode.
     */
    Azure::Nullable<Models::ModeCopyMode> ModeCopyMode;

    /**
     * Optional, only applicable to NFS Files. If not populated, the destination file will have the
     * default Owner and Group.
     */
    Azure::Nullable<Models::OwnerCopyMode> OwnerCopyMode;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::AbortCopy.
   */
  struct AbortFileCopyOptions final
  {
    /**
     * The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::GetProperties.
   */
  struct GetFilePropertiesOptions final
  {
    /**
     * The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::SetProperties.
   */
  struct SetFilePropertiesOptions final
  {
    /**
     * This permission is the security descriptor for the file specified in the Security
     * Descriptor Definition Language (SDDL) or base64 encoded
     * binary format. If not specified, 'inherit' is used.
     */
    Azure::Nullable<std::string> Permission;

    /**
     * Optional. Available for version 2024-11-04 and later. Specifies
     * the format in which the permission is returned.If FilePermissionFormat is unspecified or
     * explicitly set to SDDL format format, the permission will be
     * returned in SDDL format.
     */
    Nullable<Models ::FilePermissionFormat> FilePermissionFormat;

    /**
     * Specify this to resize a file to the specified value.
     */
    Azure::Nullable<int64_t> Size;

    /**
     * The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;

    /**
     * The NFS related properties for the file.
     */
    Models::FilePosixProperties PosixProperties;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::SetMetadata.
   */
  struct SetFileMetadataOptions final
  {
    /**
     * The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::UploadRange.
   */
  struct UploadFileRangeOptions final
  {
    /**
     * An MD5 hash of the content. This hash is used to verify the integrity of the data
     * during transport. When the TransactionalContentHash parameter is specified, the File service
     * compares the hash of the content that has arrived with the header value that was sent. If the
     * two hashes do not match, the operation will fail with error code 400 (Bad Request).
     */
    Azure::Nullable<ContentHash> TransactionalContentHash;

    /**
     * The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;

    /**
     * Specifies if the file last write time should be set to the current time,
     * or the last write time currently associated with the file should be preserved.
     */
    Azure::Nullable<Models::FileLastWrittenMode> FileLastWrittenMode;

    /**
     * @brief Optional. Configures whether to do content validation for file uploads.
     */
    Azure::Nullable<TransferValidationOptions> ValidationOptions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::ClearRange.
   */
  struct ClearFileRangeOptions final
  {
    /**
     * The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;

    /**
     * Specifies if the file last write time should be set to the current time,
     * or the last write time currently associated with the file should be preserved.
     */
    Azure::Nullable<Models::FileLastWrittenMode> FileLastWrittenMode;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareFileClient::UploadRangeFromUri.
   */
  struct UploadFileRangeFromUriOptions final
  {
    /**
     * Specify the hash calculated for the range of bytes that must be read from the copy
     * source.
     */
    Azure::Nullable<ContentHash> TransactionalContentHash;

    /**
     * Specify the access condition for the source. Only ContentHash with Crc64 is supported.
     */
    ContentHashAccessConditions SourceAccessCondition;

    /**
     * The operation will only succeed if the lease access condition is met.
     */
    LeaseAccessConditions AccessConditions;

    /**
     * Specifies if the file last write time should be set to the current time,
     * or the last write time currently associated with the file should be preserved.
     */
    Azure::Nullable<Models::FileLastWrittenMode> FileLastWrittenMode;

    /**
     * @brief Optional. Source authorization used to access the source file.
     * The format is: \<scheme\> \<signature\>
     * Only Bearer type is supported. Credentials should be a valid OAuth access token to copy
     * source.
     */
    std::string SourceAuthorization;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::GetRangeList.
   */
  struct GetFileRangeListOptions final
  {
    /**
     * The range to be get from service.
     */
    Azure::Nullable<Core::Http::HttpRange> Range;

    /**
     * The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;

    /**
     * This header is allowed only when PreviousSnapshot query parameter is set.
     * Determines whether the changed ranges for a file that has been renamed or moved between the
     * target snapshot (or the live file) and the previous snapshot should be listed. If the value
     * is true, the valid changed ranges for the file will be returned. If the value is false, the
     * operation will result in a failure with 409 (Conflict) response.
     */
    Azure::Nullable<bool> IncludeRenames;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::ListHandles.
   */
  struct ListFileHandlesOptions final
  {
    /**
     * A string value that identifies the portion of the list to be returned with the next
     * list operation. The operation returns a marker value within the response body if the list
     * returned was not complete. The marker value may then be used in a subsequent call to request
     * the next set of list items. The marker value is opaque to the client.
     */
    Azure::Nullable<std::string> ContinuationToken;

    /**
     * Specifies the maximum number of entries to return. If the request does not specify
     * PageSizeHint, or specifies a value greater than 5,000, the server will return up to 5,000
     * items.
     */
    Azure::Nullable<int32_t> PageSizeHint;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareFileClient::ForceCloseHandle.
   */
  struct ForceCloseFileHandleOptions final
  {
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareFileClient::ForceCloseAllHandles.
   */
  struct ForceCloseAllFileHandlesOptions final
  {
    /**
     * A string value that identifies the portion of the list to be returned with the next
     * close operation. The operation returns a marker value within the response body if the force
     * close was not complete. The marker value may then be used in a subsequent call to
     * close the next handle. The marker value is opaque to the client.
     */
    Azure::Nullable<std::string> ContinuationToken;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::DownloadTo.
   */
  struct DownloadFileToOptions final
  {
    /**
     * Downloads only the bytes of the file from this range.
     */
    Azure::Nullable<Core::Http::HttpRange> Range;

    /**
     * @brief Optional. Configures whether to do content validation for file downloads.
     */
    Azure::Nullable<TransferValidationOptions> ValidationOptions;

    /**
     * @brief Options for parallel transfer.
     */
    struct
    {
      /**
       * The size of the first range request in bytes. Files smaller than this limit will be
       * downloaded in a single request. Files larger than this limit will continue being downloaded
       * in chunks of size ChunkSize.
       */
      int64_t InitialChunkSize = 256 * 1024 * 1024;

      /**
       * The maximum number of bytes in a single request.
       */
      int64_t ChunkSize = 4 * 1024 * 1024;

      /**
       * The maximum number of threads that may be used in a parallel transfer.
       */
      int32_t Concurrency = 5;
    } TransferOptions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::UploadFrom.
   */
  struct UploadFileFromOptions final
  {
    /**
     * The standard HTTP header system properties to set.
     */
    Models::FileHttpHeaders HttpHeaders;

    /**
     * Name-value pairs associated with the file as metadata.
     */
    Storage::Metadata Metadata;

    /**
     * SMB properties to set for the destination file.
     */
    Models::FileSmbProperties SmbProperties;

    /**
     * If specified the permission (security descriptor) shall be set for the directory.
     * This option can be used if Permission size is <= 8KB, else SmbProperties.PermissionKey
     * shall be used. Default value: 'inherit'. If SDDL is specified as input, it must have owner,
     * group and dacl.
     */
    Azure::Nullable<std::string> FilePermission;

    /**
     * Optional. Available for version 2024-11-04 and later. Specifies
     * the format in which the permission is returned.If FilePermissionFormat is unspecified or
     * explicitly set to SDDL format, the permission will be
     * returned in SDDL format.
     */
    Nullable<Models::FilePermissionFormat> FilePermissionFormat;

    /**
     * The NFS related properties for the file.
     */
    Models::FilePosixProperties PosixProperties;

    /**
     * @brief Optional. Configures whether to do content validation for file uploads.
     */
    Azure::Nullable<TransferValidationOptions> ValidationOptions;

    /**
     * @brief Options for parallel transfer.
     */
    struct
    {
      /**
       * File smaller than this will be uploaded with a single upload operation. This value
       * cannot be larger than 4 MiB.
       */
      int64_t SingleUploadThreshold = 4 * 1024 * 1024;

      /**
       * The maximum number of bytes in a single request.
       */
      int64_t ChunkSize = 4 * 1024 * 1024;

      /**
       * The maximum number of threads that may be used in a parallel transfer.
       */
      int32_t Concurrency = 5;
    } TransferOptions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareLeaseClient::Acquire.
   */
  struct AcquireLeaseOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareLeaseClient::Change.
   */
  struct ChangeLeaseOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareLeaseClient::Release.
   */
  struct ReleaseLeaseOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareLeaseClient::Break.
   */
  struct BreakLeaseOptions final
  {
    /**
     * Proposed duration the lease should continue before it is broken, in seconds,
     * between 0 and 60. This break period is only used if it is shorter than the time remaining on
     * the lease. If longer, the time remaining on the lease is used. A new lease will not be
     * available before the break period has expired, but the lease may be held for longer than the
     * break period.
     */
    Azure::Nullable<int32_t> BreakPeriod;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareLeaseClient::Renew.
   */
  struct RenewLeaseOptions final
  {
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareFileClient::CreateSymbolicLink.
   */
  struct CreateSymbolicLinkOptions final
  {
    /**
     * A name-value pair to associate with a file storage object.
     */
    Storage::Metadata Metadata;

    /**
     * Creation time for the file or directory.
     */
    Nullable<DateTime> CreatedOn;

    /**
     * Last write time for the file or directory.
     */
    Nullable<DateTime> LastWrittenOn;

    /**
     * Specify the access condition for the path.
     */
    LeaseAccessConditions AccessConditions;

    /**
     * NFS only. The owner user identifier (UID) to be set on the symbolic link. The default value
     * is 0 (root).
     */
    Nullable<std::string> Owner;

    /**
     * NFS only. The owner group identifier (GID) to be set on the symbolic link. The default value
     * is 0 (root group).
     */
    Nullable<std::string> Group;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareFileClient::GetSymbolicLink.
   */
  struct GetSymbolicLinkOptions final
  {
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareFileClient::CreateHardLink.
   */
  struct CreateHardLinkOptions final
  {
    /**
     * Specify the access condition for the path.
     */
    LeaseAccessConditions AccessConditions;
  };
}}}} // namespace Azure::Storage::Files::Shares
