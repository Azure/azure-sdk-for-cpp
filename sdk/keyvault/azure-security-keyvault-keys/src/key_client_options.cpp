// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/keyvault/keys/key_client_options.hpp>
#include "./generated/keys_models.hpp"
using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Keys::_detail;
_detail::Models::KeyCreateParameters CreateKeyOptions::ToKeyCreateParameters() const {
  Models::KeyCreateParameters keyCreateParameters;
  Models::KeyAttributes attributes;
  std::vector<Models::JsonWebKeyOperation> operations;
  for (auto operation : KeyOperations)
  {
    operations.push_back(Models::JsonWebKeyOperation(operation.ToString()));
  }
  keyCreateParameters.KeyOps = operations;
  std::map<std::string, std::string> tags;
  for (auto const& tag : Tags)
  {
    tags.insert({tag.first, tag.second});
  }
  keyCreateParameters.Tags = tags;
  attributes.NotBefore = NotBefore;
  attributes.Expires = ExpiresOn;
  attributes.Enabled = Enabled;
  attributes.Exportable = Exportable;
  if (ReleasePolicy.HasValue())
  {
    Models::KeyReleasePolicy releasePolicy;
    if (ReleasePolicy.Value().ContentType.HasValue())
    {
      releasePolicy.ContentType = ReleasePolicy.Value().ContentType.Value();
    }

    releasePolicy.EncodedPolicy = std::vector<uint8_t>(
        ReleasePolicy.Value().EncodedPolicy.begin(),
        ReleasePolicy.Value().EncodedPolicy.end());
    releasePolicy.Immutable = ReleasePolicy.Value().Immutable;
    keyCreateParameters.ReleasePolicy = releasePolicy;
  }
  keyCreateParameters.KeyAttributes = attributes;
  return keyCreateParameters;
}