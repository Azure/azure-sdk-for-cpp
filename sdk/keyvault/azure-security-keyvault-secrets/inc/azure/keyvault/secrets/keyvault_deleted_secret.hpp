// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Keyvault Deleted Secret definition
 */

#pragma once
#include <azure/core/datetime.hpp>
#include <azure/keyvault/secrets/keyvault_secret.hpp>
namespace Azure { namespace Security { namespace KeyVault { namespace Secrets { namespace _detail {
  struct DeletedSecretSerializer;
  struct DeletedSecretPagedResultSerializer;
}}}}} // namespace Azure::Security::KeyVault::Secrets::_detail

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets {
  /**
   * @brief A Deleted Secret consisting of its previous id, attributes and its tags,
   * as well as information on when it will be purged.
   */
  struct DeletedSecret : public KeyVaultSecret
  {
    friend struct Azure::Security::KeyVault::Secrets::_detail::DeletedSecretSerializer;
    friend struct Azure::Security::KeyVault::Secrets::_detail::DeletedSecretPagedResultSerializer;
    /**
     * @brief A Deleted Secret consisting of its previous id, attributes and its tags,
     * as well as information on when it will be purged.
     */
    std::string RecoveryId;

    /**
     * @brief The time when the secret is scheduled to be purged, in UTC.
     */
    Azure::DateTime ScheduledPurgeDate() { return m_scheduledPurgeDate; };

    /**
     * @brief The time when the secret was deleted, in UTC.
     */
    Azure::DateTime DeletedOn() { return m_deletedOn; };

    /**
     * @brief Default constructor.
     */
    DeletedSecret() = default;

    /**
     * @brief Constructor.
     *
     * @param name Name of the deleted secret.
     */
    DeletedSecret(std::string name) : KeyVaultSecret(std::move(name)) {}

  protected:
    Azure::DateTime m_scheduledPurgeDate;
    Azure::DateTime m_deletedOn;
  };
}}}} // namespace Azure::Security::KeyVault::Secrets
