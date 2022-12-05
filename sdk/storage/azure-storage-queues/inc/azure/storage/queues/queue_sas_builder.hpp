// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
/**
 * @file
 * @brief Defines Queue SAS builder.
 *
 */

#pragma once

#include <string>
#include <type_traits>

#include <azure/core/nullable.hpp>
#include <azure/storage/common/account_sas_builder.hpp>
#include <azure/storage/common/internal/constants.hpp>

namespace Azure { namespace Storage { namespace Sas {

  /**
   * @brief The list of permissions that can be set for a queue's access policy.
   */
  enum class QueueSasPermissions
  {
    /**
     * @brief Read metadata and properties, including message count. Peek at messages.
     */
    Read = 1,

    /**
     * @brief Add messages to the queue.
     */
    Add = 2,

    /**
     * @brief Update messages in the queue.
     */
    Update = 4,

    /**
     * @brief Get and delete messages from the queue.
     */
    Process = 8,

    /**
     * @brief Indicates that all permissions are set.
     */
    All = ~0,
  };

  inline QueueSasPermissions operator|(QueueSasPermissions lhs, QueueSasPermissions rhs)
  {
    using type = std::underlying_type_t<QueueSasPermissions>;
    return static_cast<QueueSasPermissions>(static_cast<type>(lhs) | static_cast<type>(rhs));
  }

  inline QueueSasPermissions operator&(QueueSasPermissions lhs, QueueSasPermissions rhs)
  {
    using type = std::underlying_type_t<QueueSasPermissions>;
    return static_cast<QueueSasPermissions>(static_cast<type>(lhs) & static_cast<type>(rhs));
  }

  /**
   * @brief ShareSasBuilder is used to generate a Shared Access Signature (SAS) for an Azure
   * Storage queue.
   */
  struct QueueSasBuilder final
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
     * access policy specified for the queue.
     */
    std::string Identifier;

    /**
     * @brief The name of the queue being made accessible.
     */
    std::string QueueName;

    /**
     * @brief Sets the permissions for the queue SAS.
     *
     * @param permissions The allowed permissions.
     */
    void SetPermissions(QueueSasPermissions permissions);

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