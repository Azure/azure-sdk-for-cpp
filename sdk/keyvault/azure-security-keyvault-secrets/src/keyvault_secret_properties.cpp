// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
/**
 * @file
 * @brief Declares SecretProperties.
 *
 */

#include "./generated/secrets_models.hpp"
#include "azure/keyvault/secrets/keyvault_secret.hpp"
#include "private/secret_serializers.hpp"

using namespace Azure::Security::KeyVault::Secrets;

SecretProperties SecretProperties::CreateFromURL(std::string const& url)
{
  // create a url object to validate the string is valid as url
  Azure::Core::Url urlInstance(url);
  SecretProperties result;
  // parse the url into the result object
  _detail::SecretSerializer::ParseIDUrl(result, urlInstance.GetAbsoluteUrl());
  return result;
}

_detail::Models::SecretUpdateParameters SecretProperties::ToSecretUpdateParameters() const
{
  _detail::Models::SecretUpdateParameters secretParameters;
  if (ContentType.HasValue())
  {
    secretParameters.ContentType = ContentType.Value();
  }

  secretParameters.Tags = std::map<std::string, std::string>(Tags.begin(), Tags.end());
  secretParameters.SecretAttributes = _detail::Models::SecretAttributes();
  if (ExpiresOn.HasValue())
  {
    secretParameters.SecretAttributes.Value().Expires = ExpiresOn;
  }
  if (NotBefore.HasValue())
  {
    secretParameters.SecretAttributes.Value().NotBefore = NotBefore;
  }
  if (Enabled.HasValue())
  {
    secretParameters.SecretAttributes.Value().Enabled = Enabled;
  }
  if (RecoveryLevel.HasValue())
  {
    secretParameters.SecretAttributes.Value().RecoveryLevel
        = _detail::Models::DeletionRecoveryLevel(RecoveryLevel.Value());
  }
  if (RecoverableDays.HasValue())
  {
    secretParameters.SecretAttributes.Value().RecoverableDays
        = static_cast<int32_t>(RecoverableDays.Value());
  }
  if (CreatedOn.HasValue())
  {
    secretParameters.SecretAttributes.Value().Created = CreatedOn.Value();
  }
  if (UpdatedOn.HasValue())
  {
    secretParameters.SecretAttributes.Value().Updated = UpdatedOn.Value();
  }
  return secretParameters;
}
