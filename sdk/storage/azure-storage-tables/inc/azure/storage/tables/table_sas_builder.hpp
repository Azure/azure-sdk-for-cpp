// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
/**
 * @file
 * @brief Defines Table SAS builder.
 *
 */

#pragma once

#include <azure/core/nullable.hpp>
#include <azure/storage/common/account_sas_builder.hpp>
#include <azure/storage/common/internal/constants.hpp>

#include <string>
#include <type_traits>

namespace Azure { namespace Storage { namespace Sas {

  /**
   * @brief The list of permissions that can be set for a table's access policy.
   */
  enum class TableSasPermissions
  {
    Read = 1,
    Add = 2,
    Update = 4,
    Process = 8,
    All = ~0,
  };

  /** @brief Bitwise OR of two values*/
  inline TableSasPermissions operator|(TableSasPermissions lhs, TableSasPermissions rhs)
  {
    using type = std::underlying_type_t<TableSasPermissions>;
    return static_cast<TableSasPermissions>(static_cast<type>(lhs) | static_cast<type>(rhs));
  }

  /** @brief Bitwise AND of two values*/
  inline TableSasPermissions operator&(TableSasPermissions lhs, TableSasPermissions rhs)
  {
    using type = std::underlying_type_t<TableSasPermissions>;
    return static_cast<TableSasPermissions>(static_cast<type>(lhs) & static_cast<type>(rhs));
  }

  /**
   * @brief ShareSasBuilder is used to generate a Shared Access Signature (SAS) for an Azure
   * Storage table.
   */
  struct TableSasBuilder final
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
     * @brief The name of the queue being made accessible.
     */
    // std::string QueueName;

    /**
     * @brief Sets the permissions for the table SAS.
     *
     * @param permissions The allowed permissions.
     */
    void SetPermissions(TableSasPermissions permissions);

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
