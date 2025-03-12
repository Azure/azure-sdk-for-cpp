// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Keyvault Deleted Secret definition
 */

#pragma once
#include <azure/core/datetime.hpp>
#include <azure/keyvault/secrets/generated/models/generated_models.hpp>
#include <azure/keyvault/secrets/keyvault_secret.hpp>
namespace Azure { namespace Security { namespace KeyVault { namespace Secrets {
  class SecretClient;
  class DeletedSecretPagedResponse;
  /**
   * @brief A Deleted Secret consisting of its previous id, attributes and its tags,
   * as well as information on when it will be purged.
   */
  struct DeletedSecret : public KeyVaultSecret
  {
    /**
     * @brief A Deleted Secret consisting of its previous id, attributes and its tags,
     * as well as information on when it will be purged.
     */
    std::string RecoveryId;

    /**
     * @brief The time when the secret is scheduled to be purged, in UTC.
     */
    Azure::Nullable<Azure::DateTime> ScheduledPurgeDate;

    /**
     * @brief The time when the secret was deleted, in UTC.
     */
    Azure::Nullable<Azure::DateTime> DeletedOn;

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

  private:
    DeletedSecret(Generated::Models::DeletedSecretBundle const& deletedSecret)
    {
      if (deletedSecret.RecoveryId.HasValue())
      {
        RecoveryId = deletedSecret.RecoveryId.Value();
      }
      if (deletedSecret.ScheduledPurgeDate.HasValue())
      {
        ScheduledPurgeDate = deletedSecret.ScheduledPurgeDate.Value();
      }
      if (deletedSecret.DeletedDate.HasValue())
      {
        DeletedOn = deletedSecret.DeletedDate.Value();
      }
      if (deletedSecret.Value.HasValue())
      {
        Value = deletedSecret.Value;
      }
      if (deletedSecret.Id.HasValue())
      {
        Properties = SecretProperties::CreateFromURL(deletedSecret.RecoveryId.Value());
        Id = deletedSecret.Id.Value();
        Properties.Id = deletedSecret.Id.Value();
        Name = Properties.Name;
      }
      if (deletedSecret.ContentType.HasValue())
      {
        Properties.ContentType = deletedSecret.ContentType;
      }
      Properties.KeyId = deletedSecret.Kid;
      if (deletedSecret.Managed.HasValue())
      {
        Properties.Managed = deletedSecret.Managed.Value();
      }
      if (deletedSecret.Attributes.HasValue())
      {
        Properties.ExpiresOn = deletedSecret.Attributes.Value().Expires;
        Properties.NotBefore = deletedSecret.Attributes.Value().NotBefore;
        Properties.Enabled = deletedSecret.Attributes.Value().Enabled;
        Properties.CreatedOn = deletedSecret.Attributes.Value().Created;
        Properties.UpdatedOn = deletedSecret.Attributes.Value().Updated;
        if (deletedSecret.Attributes.Value().RecoverableDays.HasValue())
        {
          Properties.RecoverableDays = deletedSecret.Attributes.Value().RecoverableDays.Value();
        }
      }
      if (deletedSecret.Tags.HasValue())
      {
        Properties.Tags = std::unordered_map<std::string, std::string>(
            deletedSecret.Tags.Value().begin(), deletedSecret.Tags.Value().end());
      }
    };
    DeletedSecret(Generated::Models::DeletedSecretItem const& deletedSecret)
    {
      if (deletedSecret.RecoveryId.HasValue())
      {
        RecoveryId = deletedSecret.RecoveryId.Value();
      }
      if (deletedSecret.ScheduledPurgeDate.HasValue())
      {
        ScheduledPurgeDate = deletedSecret.ScheduledPurgeDate.Value();
      }
      if (deletedSecret.DeletedDate.HasValue())
      {
        DeletedOn = deletedSecret.DeletedDate.Value();
      }
      if (deletedSecret.RecoveryId.HasValue())
      {
        Properties = SecretProperties::CreateFromURL(deletedSecret.Id.Value());
        Id = deletedSecret.Id.Value();
        Properties.Id = deletedSecret.Id.Value();
      }
      if (deletedSecret.ContentType.HasValue())
      {
        Properties.ContentType = deletedSecret.ContentType;
      }
      if (deletedSecret.Managed.HasValue())
      {
        Properties.Managed = deletedSecret.Managed.Value();
      }
      if (deletedSecret.Attributes.HasValue())
      {
        Properties.ExpiresOn = deletedSecret.Attributes.Value().Expires;
        Properties.NotBefore = deletedSecret.Attributes.Value().NotBefore;
        Properties.Enabled = deletedSecret.Attributes.Value().Enabled;
        Properties.CreatedOn = deletedSecret.Attributes.Value().Created;
        Properties.UpdatedOn = deletedSecret.Attributes.Value().Updated;
        if (deletedSecret.Attributes.Value().RecoverableDays.HasValue())
        {
          Properties.RecoverableDays = deletedSecret.Attributes.Value().RecoverableDays.Value();
        }
      }
      if (deletedSecret.Tags.HasValue())
      {
        Properties.Tags = std::unordered_map<std::string, std::string>(
            deletedSecret.Tags.Value().begin(), deletedSecret.Tags.Value().end());
      }
    };
    friend class SecretClient;
    friend class DeletedSecretOperation;
    friend class DeletedSecretPagedResponse;
  };
}}}} // namespace Azure::Security::KeyVault::Secrets
