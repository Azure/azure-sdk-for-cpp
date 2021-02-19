// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <type_traits>

#include <azure/core/nullable.hpp>
#include <azure/storage/common/account_sas_builder.hpp>
#include <azure/storage/common/constants.hpp>

namespace Azure { namespace Storage { namespace Sas {

  /**
   * @brief Specifies which resources are accessible via the shared access signature.
   */
  enum class ShareSasResource
  {
    /**
     * @brief Grants access to the content and metadata of the file.
     */
    Share,

    /**
     * @brief Grants access to the content and metadata of any file in the share, and to the list of
     * directories and files in the share.
     */
    File,
  };

  /**
   * @brief The list of permissions that can be set for a file share's access policy.
   */
  enum class ShareSasPermissions
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
     * @brief Indicates that Create is permitted.
     */
    Create = 16,

    /**
     * @brief Indicates that all permissions are set.
     */
    All = ~0,
  };

  inline ShareSasPermissions operator|(ShareSasPermissions lhs, ShareSasPermissions rhs)
  {
    using type = std::underlying_type_t<ShareSasPermissions>;
    return static_cast<ShareSasPermissions>(static_cast<type>(lhs) | static_cast<type>(rhs));
  }

  inline ShareSasPermissions operator&(ShareSasPermissions lhs, ShareSasPermissions rhs)
  {
    using type = std::underlying_type_t<ShareSasPermissions>;
    return static_cast<ShareSasPermissions>(static_cast<type>(lhs) & static_cast<type>(rhs));
  }

  /**
   * @brief The list of permissions that can be set for a share file's access policy.
   */
  enum class ShareFileSasPermissions
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
     * @brief Indicates that Create is permitted.
     */
    Create = 8,

    /**
     * @brief Indicates that all permissions are set.
     */
    All = ~0,
  };

  inline ShareFileSasPermissions operator|(ShareFileSasPermissions lhs, ShareFileSasPermissions rhs)
  {
    using type = std::underlying_type_t<ShareFileSasPermissions>;
    return static_cast<ShareFileSasPermissions>(static_cast<type>(lhs) | static_cast<type>(rhs));
  }

  inline ShareFileSasPermissions operator&(ShareFileSasPermissions lhs, ShareFileSasPermissions rhs)
  {
    using type = std::underlying_type_t<ShareFileSasPermissions>;
    return static_cast<ShareFileSasPermissions>(static_cast<type>(lhs) & static_cast<type>(rhs));
  }

  /**
   * @brief ShareSasBuilder is used to generate a Shared Access Signature (SAS) for an Azure
   * Storage share or file.
   */
  struct ShareSasBuilder
  {
    /**
     * @brief The optional signed protocol field specifies the protocol permitted for a
     * request made with the SAS.
     */
    SasProtocol Protocol;

    /**
     * @brief Optionally specify the time at which the shared access signature becomes
     * valid. This timestamp will be truncated to second.
     */
    Azure::Core::Nullable<Azure::Core::DateTime> StartsOn;

    /**
     * @brief The time at which the shared access signature becomes invalid. This field must
     * be omitted if it has been specified in an associated stored access policy. This timestamp
     * will be truncated to second.
     */
    Azure::Core::DateTime ExpiresOn;

    /**
     * @brief Specifies an IP address or a range of IP addresses from which to accept
     * requests. If the IP address from which the request originates does not match the IP address
     * or address range specified on the SAS token, the request is not authenticated. When
     * specifying a range of IP addresses, note that the range is inclusive.
     */
    Azure::Core::Nullable<std::string> IPRange;

    /**
     * @brief An optional unique value up to 64 characters in length that correlates to an
     * access policy specified for the share.
     */
    std::string Identifier;

    /**
     * @brief The name of the file share being made accessible.
     */
    std::string ShareName;

    /**
     * @brief The name of the share file being made accessible, or empty for a share SAS..
     */
    std::string FilePath;

    /**
     * @brief Specifies which resources are accessible via the shared access signature.
     */
    ShareSasResource Resource;

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
     * @brief Sets the permissions for the share SAS.
     *
     * @param permissions The allowed permissions.
     */
    void SetPermissions(ShareSasPermissions permissions);

    /**
     * @brief Sets the permissions for the share SAS.
     *
     * @param permissions The allowed permissions.
     */
    void SetPermissions(ShareFileSasPermissions permissions);

    /**
     * @brief Sets the permissions for the SAS using a raw permissions string.
     *
     * @param rawPermissions Raw permissions string for the SAS.
     */
    void SetPermissions(std::string rawPermissions) { Permissions = std::move(rawPermissions); }

    /**
     * @brief Uses the StorageSharedKeyCredential to sign this shared access signature, to produce
     * the proper SAS query parameters for authentication requests.
     *
     * @param credential The storage account's shared key credential.
     * @return The SAS query parameters used for authenticating requests.
     */
    std::string GenerateSasToken(const StorageSharedKeyCredential& credential);

  private:
    std::string Permissions;
  };

}}} // namespace Azure::Storage::Sas
