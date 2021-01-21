// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <type_traits>

#include <azure/core/datetime.hpp>
#include <azure/core/nullable.hpp>

#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/storage_credential.hpp"

namespace Azure { namespace Storage { namespace Sas {

  /**
   * @brief Defines the protocols permitted for Storage requests made with a shared access
   * signature.
   */
  enum class SasProtocol
  {
    /**
     * @brief Only requests issued over HTTPS or HTTP will be permitted.
     */
    HttpsAndHttp,

    /**
     * @brief Only requests issued over HTTPS will be permitted.
     */
    HttpsOnly,
  };

  namespace Details {
    inline std::string SasProtocolToString(SasProtocol protocol)
    {
      return protocol == SasProtocol::HttpsAndHttp ? "https,http" : "https";
    }
  } // namespace Details

  /**
   * @brief Specifies the resource types accessible from an account level shared access
   * signature.
   */
  enum class AccountSasResource
  {
    /**
     * @brief Indicates whether service-level APIs are accessible from this shared access
     * signature.
     */
    Service = 1,

    /**
     * @brief Indicates whether container-level APIs are accessible from this shared
     * access signature.
     */
    Container = 2,

    /**
     * @brief Indicates whether object-level APIs for blobs, queue messages, and files are
     * accessible from this shared access signature.
     */
    Object = 4,

    /**
     * @brief Indicates all service-level APIs are accessible from this shared access
     * signature.
     */
    All = ~0,
  };

  inline AccountSasResource operator|(AccountSasResource lhs, AccountSasResource rhs)
  {
    using type = std::underlying_type_t<AccountSasResource>;
    return static_cast<AccountSasResource>(static_cast<type>(lhs) | static_cast<type>(rhs));
  }

  inline AccountSasResource operator&(AccountSasResource lhs, AccountSasResource rhs)
  {
    using type = std::underlying_type_t<AccountSasResource>;
    return static_cast<AccountSasResource>(static_cast<type>(lhs) & static_cast<type>(rhs));
  }

  /**
   * @brief Specifies the services accessible from an account level shared access signature.
   */
  enum class AccountSasServices
  {
    /**
     * @brief Indicates whether Azure Blob Storage resources are accessible from the shared
     * access signature.
     */
    Blobs = 1,

    /**
     * @brief Indicates whether Azure Queue Storage resources are accessible from the shared
     * access signature.
     */
    Queue = 2,

    /**
     * @brief Indicates whether Azure File Storage resources are accessible from the shared
     * access signature.
     */
    Files = 4,

    /**
     * @brief Indicates all services are accessible from the shared
     * access signature.
     */
    All = ~0,
  };

  inline AccountSasServices operator|(AccountSasServices lhs, AccountSasServices rhs)
  {
    using type = std::underlying_type_t<AccountSasServices>;
    return static_cast<AccountSasServices>(static_cast<type>(lhs) | static_cast<type>(rhs));
  }

  inline AccountSasServices operator&(AccountSasServices lhs, AccountSasServices rhs)
  {
    using type = std::underlying_type_t<AccountSasServices>;
    return static_cast<AccountSasServices>(static_cast<type>(lhs) & static_cast<type>(rhs));
  }

  /**
   * @brief The list of permissions that can be set for an account's access policy.
   */
  enum class AccountSasPermissions
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
     * @brief Indicates that deleting previous blob version is permitted.
     */
    DeleteVersion = 8,

    /**
     * @brief Indicates that List is permitted.
     */
    List = 16,

    /**
     * @brief Indicates that Add is permitted.
     */
    Add = 32,

    /**
     * @brief Indicates that Create is permitted.
     */
    Create = 64,

    /**
     * @brief Indicates that Update is permitted.
     */
    Update = 128,

    /**
     * @brief Indicates that Process is permitted.
     */
    Process = 256,

    /**
     * @brief Indicates that reading and writing tags is permitted.
     */
    Tags = 512,

    /**
     * @brief Indicates that filtering by tags is permitted.
     */
    Filter = 1024,

    /**
     * @brief Indicates that all permissions are set.
     */
    All = ~0,
  };

  inline AccountSasPermissions operator|(AccountSasPermissions lhs, AccountSasPermissions rhs)
  {
    using type = std::underlying_type_t<AccountSasPermissions>;
    return static_cast<AccountSasPermissions>(static_cast<type>(lhs) | static_cast<type>(rhs));
  }

  inline AccountSasPermissions operator&(AccountSasPermissions lhs, AccountSasPermissions rhs)
  {
    using type = std::underlying_type_t<AccountSasPermissions>;
    return static_cast<AccountSasPermissions>(static_cast<type>(lhs) & static_cast<type>(rhs));
  }

  /**
   * @brief AccountSasBuilder is used to generate an account level Shared Access Signature
   * (SAS) for Azure Storage services.
   */
  struct AccountSasBuilder
  {
    /**
     * @brief The optional signed protocol field specifies the protocol permitted for a
     * request made with the SAS.
     */
    SasProtocol Protocol = SasProtocol::HttpsOnly;

    /**
     * @brief Optionally specify the time at which the shared access signature becomes
     * valid.
     */
    Azure::Core::Nullable<Azure::Core::DateTime> StartsOn;

    /**
     * @brief The time at which the shared access signature becomes invalid. This field must
     * be omitted if it has been specified in an associated stored access policy.
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
     * @brief The services associated with the shared access signature. The user is
     * restricted to operations with the specified services.
     */
    AccountSasServices Services;

    /**
     * The resource types associated with the shared access signature. The user is
     * restricted to operations on the specified resources.
     */
    AccountSasResource ResourceTypes;

    /**
     * @brief Sets the permissions for an account SAS.
     *
     * @param permissions The
     * allowed permissions.
     */
    void SetPermissions(AccountSasPermissions permissions);

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
     * @param credential
     * The storage account's shared key credential.
     * @return The SAS query parameters used for
     * authenticating requests.
     */
    std::string GenerateSasToken(const StorageSharedKeyCredential& credential);

  private:
    std::string Permissions;
  };

}}} // namespace Azure::Storage::Sas
