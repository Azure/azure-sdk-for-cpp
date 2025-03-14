// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// Code generated by Microsoft (R) TypeSpec Code Generator.
// Changes may cause incorrect behavior and will be lost if the code is regenerated.

#include "azure/keyvault/secrets/keyvault_secret.hpp"

#include "azure/keyvault/secrets/generated/models/generated_models.hpp"

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets {
  KeyVaultSecret::KeyVaultSecret(std::string const& name, std::string const& value)
      : Name(name), Value(value), Properties(name)
  {
    if (Name.empty())
    {
      throw std::invalid_argument("Name cannot be empty");
    }

    if (Value.HasValue() == false || Value.Value().empty())
    {
      throw std::invalid_argument("Value cannot be empty");
    }
  }

  KeyVaultSecret::KeyVaultSecret(std::string name) : Name(std::move(name))
  {
    if (Name.empty())
    {
      throw std::invalid_argument("Name cannot be empty");
    }
  }

  KeyVaultSecret::KeyVaultSecret(_detail::Models::SecretBundle const& secret)
  {
    Value = secret.Value;
    if (secret.Id.HasValue())
    {
      Id = secret.Id.Value();
      Properties = SecretProperties::CreateFromURL(secret.Id.Value());
      Name = Properties.Name;
    }
    Properties.ContentType = secret.ContentType;
    Properties.KeyId = secret.Kid;
    if (secret.Attributes.HasValue())
    {
      Properties.ExpiresOn = secret.Attributes.Value().Expires;
      Properties.NotBefore = secret.Attributes.Value().NotBefore;
      Properties.Enabled = secret.Attributes.Value().Enabled;
      Properties.CreatedOn = secret.Attributes.Value().Created;
      Properties.UpdatedOn = secret.Attributes.Value().Updated;
      if (secret.Attributes.Value().RecoverableDays.HasValue())
      {
        Properties.RecoverableDays = secret.Attributes.Value().RecoverableDays.Value();
      }
      if (secret.Attributes.Value().RecoveryLevel.HasValue())
      {
        Properties.RecoveryLevel = secret.Attributes.Value().RecoveryLevel.Value().ToString();
      }
    }
    if (secret.Managed.HasValue())
    {
      Properties.Managed = secret.Managed.Value();
    }
    if (secret.Tags.HasValue())
    {
      Properties.Tags = std::unordered_map<std::string, std::string>(
          secret.Tags.Value().begin(), secret.Tags.Value().end());
    }
  }

  KeyVaultSecret::KeyVaultSecret(_detail::Models::SecretItem const& secret)
  {
    if (secret.Id.HasValue())
    {
      Id = secret.Id.Value();
      Properties = SecretProperties::CreateFromURL(secret.Id.Value());
      Name = Properties.Name;
    }
    if (secret.Tags.HasValue())
    {
      Properties.Tags = std::unordered_map<std::string, std::string>(
          secret.Tags.Value().begin(), secret.Tags.Value().end());
    }
    Properties.ContentType = secret.ContentType;
    if (secret.Managed.HasValue())
    {
      Properties.Managed = secret.Managed.Value();
    }
    if (secret.Attributes.HasValue())
    {
      Properties.ExpiresOn = secret.Attributes.Value().Expires;
      Properties.NotBefore = secret.Attributes.Value().NotBefore;
      Properties.Enabled = secret.Attributes.Value().Enabled;
      Properties.CreatedOn = secret.Attributes.Value().Created;
      Properties.UpdatedOn = secret.Attributes.Value().Updated;
      if (secret.Attributes.Value().RecoverableDays.HasValue())
      {
        Properties.RecoverableDays = secret.Attributes.Value().RecoverableDays.Value();
      }
      if (secret.Attributes.Value().RecoveryLevel.HasValue())
      {
        Properties.RecoveryLevel = secret.Attributes.Value().RecoveryLevel.Value().ToString();
      }
    }
  }

  _detail::Models::SecretSetParameters KeyVaultSecret::ToSetSecretParameters() const
  {
    _detail::Models::SecretSetParameters secretParameters;
    if (Properties.ContentType.HasValue())
    {
      secretParameters.ContentType = Properties.ContentType;
    }
    if (Value.HasValue())
    {
      secretParameters.Value = Value.Value();
    }

    secretParameters.Tags
        = std::map<std::string, std::string>(Properties.Tags.begin(), Properties.Tags.end());

    secretParameters.SecretAttributes = _detail::Models::SecretAttributes();
    if (Properties.Enabled.HasValue())
    {
      secretParameters.SecretAttributes.Value().Enabled = Properties.Enabled;
    }
    if (Properties.NotBefore.HasValue())
    {
      secretParameters.SecretAttributes.Value().NotBefore = Properties.NotBefore;
    }
    if (Properties.ExpiresOn.HasValue())
    {
      secretParameters.SecretAttributes.Value().Expires = Properties.ExpiresOn;
    }
    if (Properties.CreatedOn.HasValue())
    {
      secretParameters.SecretAttributes.Value().Created = Properties.CreatedOn;
    }
    if (Properties.UpdatedOn.HasValue())
    {
      secretParameters.SecretAttributes.Value().Updated = Properties.UpdatedOn;
    }
    if (Properties.RecoverableDays.HasValue())
    {
      secretParameters.SecretAttributes.Value().RecoverableDays
          = static_cast<int32_t>(Properties.RecoverableDays.Value());
    }
    if (Properties.RecoveryLevel.HasValue())
    {
      secretParameters.SecretAttributes.Value().RecoveryLevel
          = _detail::Models::DeletionRecoveryLevel(Properties.RecoveryLevel.Value());
    }
    return secretParameters;
  }
}}}} // namespace Azure::Security::KeyVault::Secrets