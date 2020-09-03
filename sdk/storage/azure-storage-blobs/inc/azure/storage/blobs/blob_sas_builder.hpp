// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include "azure/core/nullable.hpp"
#include "azure/storage/blobs/blob_responses.hpp"
#include "azure/storage/blobs/protocol/blob_rest_client.hpp"
#include "azure/storage/common/account_sas_builder.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  /**
   * @brief Specifies which resources are accessible via the shared access signature.
   */
  enum class BlobSasResource
  {
    /**
     * @brief Grants access to the content and metadata of any blob in the container, and to
     * the list of blobs in the container.
     */
    Container,

    /**
     * @brief Grants access to the content and metadata of the blob.
     */
    Blob,

    /**
     * @brief Grants access to the content and metadata of the specific snapshot, but not
     * the corresponding root blob.
     */
    BlobSnapshot,

    /**
     * @brief Grants access to the content and metadata of the specific version, but not the
     * corresponding root blob.
     */
    BlobVersion,
  };

  /**
   * @brief The list of permissions that can be set for a blob container's access policy.
   */
  enum class BlobContainerSasPermissions
  {
    /**
     * @brief Indicates that Read is permitted.
     */
    Read = 1,

    /**
     * @brief Indicates that Write is permitted.
     */
    Write = 2,

    /**
     * @brief Indicates that Delete is permitted.
     */
    Delete = 4,

    /**
     * @brief Indicates that List is permitted.
     */
    List = 8,

    /**
     * @brief Indicates that Add is permitted.
     */
    Add = 16,

    /**
     * @brief Indicates that Create is permitted.
     */
    Create = 32,

    /**
     * @brief Indicates that reading and writing tags is permitted.
     */
    Tags = 64,

    /**
     * @brief Indicates that deleting previous blob version is permitted.
     */
    DeleteVersion = 128,

    /**
     * @beirf Indicates that all permissions are set.
     */
    All = ~0,
  };

  inline BlobContainerSasPermissions operator|(
      BlobContainerSasPermissions lhs,
      BlobContainerSasPermissions rhs)
  {
    using type = std::underlying_type_t<BlobContainerSasPermissions>;
    return static_cast<BlobContainerSasPermissions>(
        static_cast<type>(lhs) | static_cast<type>(rhs));
  }

  inline BlobContainerSasPermissions operator&(
      BlobContainerSasPermissions lhs,
      BlobContainerSasPermissions rhs)
  {
    using type = std::underlying_type_t<BlobContainerSasPermissions>;
    return static_cast<BlobContainerSasPermissions>(
        static_cast<type>(lhs) & static_cast<type>(rhs));
  }

  std::string BlobContainerSasPermissionsToString(BlobContainerSasPermissions permissions);

  /**
   * @brief The list of permissions that can be set for a blob's access policy.
   */
  enum class BlobSasPermissions
  {
    /**
     * @brief Indicates that Read is permitted.
     */
    Read = 1,

    /**
     * @brief Indicates that Write is permitted.
     */
    Write = 2,

    /**
     * @brief Indicates that Delete is permitted.
     */

    Delete = 4,

    /**
     * @brief Indicates that Add is permitted.
     */
    Add = 8,

    /**
     * @brief Indicates that Create is permitted.
     */
    Create = 16,

    /**
     * @brief Indicates that reading and writing tags is permitted.
     */
    Tags = 32,

    /**
     * @brief Indicates that deleting previous blob version is permitted.
     */
    DeleteVersion = 64,

    /**
     * @beirf Indicates that all permissions are set.
     */
    All = ~0,
  };

  inline BlobSasPermissions operator|(BlobSasPermissions lhs, BlobSasPermissions rhs)
  {
    using type = std::underlying_type_t<BlobSasPermissions>;
    return static_cast<BlobSasPermissions>(static_cast<type>(lhs) | static_cast<type>(rhs));
  }

  inline BlobSasPermissions operator&(BlobSasPermissions lhs, BlobSasPermissions rhs)
  {
    using type = std::underlying_type_t<BlobSasPermissions>;
    return static_cast<BlobSasPermissions>(static_cast<type>(lhs) & static_cast<type>(rhs));
  }

  /**
   * @brief BlobSasBuilder is used to generate a Shared Access Signature (SAS) for an Azure
   * Storage container or blob.
   */
  struct BlobSasBuilder
  {
    /**
     * @brief The storage service version to use to authenticate requests made with this
     * shared access signature, and the service version to use when handling requests made with this
     * shared access signature.
     */
    std::string Version = Details::c_defaultSasVersion;

    /**
     * @brief The optional signed protocol field specifies the protocol permitted for a
     * request made with the SAS.
     */
    SasProtocol Protocol;

    /**
     * @brief Optionally specify the time at which the shared access signature becomes
     * valid.
     */
    Azure::Core::Nullable<std::string> StartsOn;

    /**
     * @brief The time at which the shared access signature becomes invalid. This field must
     * be omitted if it has been specified in an associated stored access policy.
     */
    std::string ExpiresOn;

    /**
     * @brief Specifies an IP address or a range of IP addresses from which to accept
     * requests. If the IP address from which the request originates does not match the IP address
     * or address range specified on the SAS token, the request is not authenticated. When
     * specifying a range of IP addresses, note that the range is inclusive.
     */
    Azure::Core::Nullable<std::string> IPRange;

    /**
     * @brief An optional unique value up to 64 characters in length that correlates to an
     * access policy specified for the container.
     */
    std::string Identifier;

    /**
     * @brief The name of the blob container being made accessible.
     */
    std::string ContainerName;

    /**
     * @brief The name of the blob being made accessible, or empty for a container SAS..
     */
    std::string BlobName;

    /**
     * @brief The name of the blob snapshot being made accessible, or empty for a container
     * SAS and blob SAS.
     */
    std::string Snapshot;

    /**
     * @brief The id of the blob version being made accessible, or empty for a container
     * SAS, blob SAS and blob snapshot SAS.
     */
    std::string BlobVersionId;

    /**
     * @brief Specifies which resources are accessible via the shared access signature.
     */
    BlobSasResource Resource;

    /**
     * @brief Override the value returned for Cache-Control response header..
     */
    std::string CacheControl;

    /**
     * @brief Override the value returned for Content-Disposition response header..
     */
    std::string ContentDisposition;

    /**
     * @brief Override the value returned for Content-Encoding response header..
     */
    std::string ContentEncoding;

    /**
     * @brief Override the value returned for Content-Language response header..
     */
    std::string ContentLanguage;

    /**
     * @brief Override the value returned for Content-Type response header..
     */
    std::string ContentType;

    /**
     * @brief Sets the permissions for the blob container SAS.
     *
     * @param permissions The allowed permissions.
     */
    void SetPermissions(BlobContainerSasPermissions permissions)
    {
      Permissions = BlobContainerSasPermissionsToString(permissions);
    }

    /**
     * @brief Sets the permissions for the blob SAS.
     *
     * @param permissions The allowed permissions.
     */
    void SetPermissions(BlobSasPermissions permissions);

    /**
     * @brief Uses the SharedKeyCredential to sign this shared access signature, to produce
     * the proper SAS query parameters for authentication requests.
     *
     * @param credential The storage account's shared key credential.
     * @return The SAS query parameters used for authenticating requests.
     */
    std::string ToSasQueryParameters(const SharedKeyCredential& credential);

    /**
     * @brief Uses an account's user delegation key to sign this shared access signature, to
     * produce the proper SAS query parameters for authentication requests.
     *
     * @param credential UserDelegationKey retruned from BlobServiceClient.GetUserDelegationKey.
     * @param accountName The name of the storage account.
     * @return The SAS query parameters used for authenticating requests.
     */
    std::string ToSasQueryParameters(
        const UserDelegationKey& userDelegationKey,
        const std::string& accountName);

  private:
    std::string Permissions;
  };

}}} // namespace Azure::Storage::Blobs
