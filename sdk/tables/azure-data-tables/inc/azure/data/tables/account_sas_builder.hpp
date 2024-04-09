// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/data/tables/credentials/named_key_credential.hpp"
#include "azure/data/tables/enum_operators.hpp"

#include <azure/core/datetime.hpp>
#include <azure/core/nullable.hpp>

#include <string>

namespace Azure { namespace Data { namespace Tables { namespace Sas {
  constexpr const char* HttpsAndHttp = "https,http";
  constexpr const char* HttpsOnly = "https";
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

  namespace _detail {
    inline std::string SasProtocolToString(SasProtocol protocol)
    {
      return protocol == SasProtocol::HttpsAndHttp ? HttpsAndHttp : HttpsOnly;
    }
  } // namespace _detail

  /**
   * @brief Specifies the resource types accessible from an account level shared access
   * signature.
   */
  enum class AccountSasResourceType
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

  /**
   * @brief Specifies the services accessible from an account level shared access signature.
   */
  enum class AccountSasServices
  {
    /**
     * @brief Indicates whether Azure Table Storage resources are accessible from the shared
     * access signature.
     */
    Table = 1,
    /**
     * @brief Indicates all services are accessible from the shared
     * access signature.
     */
    All = ~0,
  };

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
     * @brief Indicates that Add is permitted.
     */
    Add = 8,

    /**
     * @brief Indicates that List is permitted.
     */
    List = 16,

    /**
     * @brief Indicates that Update is permitted.
     */
    Update = 32,

    /**
     * @brief Indicates that all permissions are set.
     */
    All = ~0,
  };

  /**
   * @brief AccountSasBuilder is used to generate an account level Shared Access Signature
   * (SAS) for Azure Storage services.
   */
  class AccountSasBuilder final {
  public:
    /**
     * @brief The optional signed protocol field specifies the protocol permitted for a
     * request made with the SAS.
     */
    SasProtocol Protocol = SasProtocol::HttpsOnly;

    /**
     * @brief Optionally specify the time at which the shared access signature becomes
     * valid.
     */
    Azure::Nullable<Azure::DateTime> StartsOn;

    /**
     * @brief The time at which the shared access signature becomes invalid. This field must
     * be omitted if it has been specified in an associated stored access policy.
     */
    Azure::DateTime ExpiresOn;

    /**
     * @brief Specifies an IP address or a range of IP addresses from which to accept
     * requests. If the IP address from which the request originates does not match the IP address
     * or address range specified on the SAS token, the request is not authenticated. When
     * specifying a range of IP addresses, note that the range is inclusive.
     */
    Azure::Nullable<std::string> IPRange;

    /**
     * @brief The services associated with the shared access signature. The user is
     * restricted to operations with the specified services.
     */
    AccountSasServices Services;

    /**
     * The resource types associated with the shared access signature. The user is
     * restricted to operations on the specified resources.
     */
    AccountSasResourceType ResourceTypes;

    /**
     * @brief Optional encryption scope to use when sending requests authorized with this SAS url.
     */
    std::string EncryptionScope;

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
     * @brief Uses the NamedKeyCredential to sign this shared access signature, to produce
     * the proper SAS query parameters for authentication requests.
     *
     * @param credential The named key credential.
     * @return The SAS query parameters used for authenticating requests.
     */
    std::string GenerateSasToken(
        const Azure::Data::Tables::Credentials::NamedKeyCredential& credential);

  private:
    std::string Permissions;
  };
}}}} // namespace Azure::Data::Tables::Sas
