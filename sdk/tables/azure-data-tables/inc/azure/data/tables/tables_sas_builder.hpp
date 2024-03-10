// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/data/tables/account_sas_builder.hpp"
#include "azure/data/tables/credentials/shared_key_credential.hpp"

#include <azure/core/datetime.hpp>
#include <azure/core/nullable.hpp>

#include <string>
#include <type_traits>

namespace Azure { namespace Data { namespace Tables { namespace Sas {
  /**
   * @brief Defines the protocols permitted for Storage requests made with a shared
   * access signature.
   */
  enum class TablesSasProtocol
  {
    /**
     * @brief No protocol has been specified. If no value is specified,
     * the service will default to HttpsAndHttp.
     */
    None = 0,

    /**
     * @brief Only requests issued over HTTPS or HTTP will be permitted.
     */
    HttpsAndHttp = 1,

    /**
     * @brief Only requests issued over HTTPS will be permitted.
     */
    Https = 2
  };

  /**
   * @brief Contains the list of
   * permissions that can be set for a table's access policy.
   */
  enum class TablesSasPermissions
  {
    /**
     * @brief Indicates that Read is permitted.
     */
    Read = 1,
    /**
     * @brief Indicates that Add is permitted.
     */
    Add = 2,
    /**
     * @brief Indicates that Delete is permitted.
     */
    Delete = 4,
    /**
     * @brief Indicates that Update is permitted.
     */
    Update = 8,
    /**
     * @brief Indicates that all permissions are set.
     */
    All = ~0
  };

  /** @brief Bitwise OR of two values*/
  inline TablesSasPermissions operator|(TablesSasPermissions lhs, TablesSasPermissions rhs)
  {
    using type = std::underlying_type_t<TablesSasPermissions>;
    return static_cast<TablesSasPermissions>(static_cast<type>(lhs) | static_cast<type>(rhs));
  }

  /** @brief Bitwise AND of two values*/
  inline TablesSasPermissions operator&(TablesSasPermissions lhs, TablesSasPermissions rhs)
  {
    using type = std::underlying_type_t<TablesSasPermissions>;
    return static_cast<TablesSasPermissions>(static_cast<type>(lhs) & static_cast<type>(rhs));
  }

  /**
   * @brief TableSasBuilder is used to generate a Shared Access Signature (SAS) for an Azure
   * Storage Tables.
   */
  struct TablesSasBuilder final
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
    Azure::Nullable<Azure::DateTime> StartsOn;

    /**
     * @brief The time at which the shared access signature becomes invalid. This field must
     * be omitted if it has been specified in an associated stored access policy. This timestamp
     * will be truncated to second.
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
     * @brief An optional unique value up to 64 characters in length that correlates to an
     * access policy specified for the table.
     */
    std::string Identifier;

    /**
     * @brief The name of the table being made accessible.
     */
    std::string TableName;

    /**
     * @brief The optional start of the partition key values range being made available.
     */
    std::string PartitionKeyStart;

    /**
     * @brief The optional end of the partition key values range being made available.
     */
    std::string PartitionKeyEnd;

    /**
     * @brief The optional start of the row key values range being made available.
     */
    std::string RowKeyStart;

    /**
     * @brief The optional end of the partition key values range being made available.
     */
    std::string RowKeyEnd;

    /**
     * @brief Sets the permissions for the table SAS.
     *
     * @param permissions The allowed permissions.
     */
    void SetPermissions(TablesSasPermissions permissions);

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
    std::string GenerateSasToken(
        const Azure::Data::Tables::Credentials::SharedKeyCredential& credential);

    /**
     * @brief Gets the canonical path for the shared access signature.
     *
     * @param credential The storage account's shared key credential.
     * @return Canonical path.
     */
    std::string GetCanonicalName(
        const Azure::Data::Tables::Credentials::SharedKeyCredential& credential) const
    {
      return Azure::Core::_internal::StringExtensions::ToLower(
          "/table/" + credential.AccountName + "/" + TableName);
    }

  private:
    std::string Permissions;
  };

}}}} // namespace Azure::Data::Tables::Sas
