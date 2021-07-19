#include "private/secret_serializers.hpp"
#include "private/secret_constants.hpp"

#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>
#include <azure/core/internal/json/json_serializable.hpp>

using namespace Azure::Core::Json::_internal;
using Azure::Core::_internal::PosixTimeConverter;
using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Secrets::_detail;

// Creates a new key based on a name and an HTTP raw response.
KeyVaultSecret KeyVaultSecretSerializer::KeyVaultSecretDeserialize(
    std::string const& name,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  KeyVaultSecret secret(name);
  _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize(secret, rawResponse);
  return secret;
}


// Create from HTTP raw response only.
KeyVaultSecret KeyVaultSecretSerializer::KeyVaultSecretDeserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{
  KeyVaultSecret secret;
  _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize(secret, rawResponse);
  return secret;
}

// Updates a Key based on an HTTP raw response.
void KeyVaultSecretSerializer::KeyVaultSecretDeserialize(
    KeyVaultSecret& secret,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  auto const& body = rawResponse.GetBody();
  auto jsonParser = json::parse(body);

  secret.Value = jsonParser[_detail::ValuePropertyName];
  
  secret.Id = jsonParser[_detail::IdPropertyName];
  if (jsonParser.contains(_detail::KeyIdPropertyName))
  {
    secret.KeyId = jsonParser[_detail::KeyIdPropertyName];
  }


  // Parse URL for the vaultUri, keyVersion
  //_detail::KeyVaultSecretSerializer::ParseKeyUrl(key.Properties, key.Key.Id);
  // "Attributes"
  if (jsonParser.contains(_detail::AttributesPropertyName))
  {
    auto attributes = jsonParser[_detail::AttributesPropertyName];

    JsonOptional::SetIfExists(secret.Properties.Enabled, attributes, _detail::EnabledPropertyName);

    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        secret.Properties.NotBefore,
        attributes,
        _detail::NbfPropertyName,
        PosixTimeConverter::PosixTimeToDateTime);
    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        secret.Properties.ExpiresOn,
        attributes,
        _detail::ExpPropertyName,
        PosixTimeConverter::PosixTimeToDateTime);
    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        secret.Properties.CreatedOn,
        attributes,
        _detail::CreatedPropertyName,
        PosixTimeConverter::PosixTimeToDateTime);
    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        secret.Properties.UpdatedOn,
        attributes,
        _detail::UpdatedPropertyName,
        PosixTimeConverter::PosixTimeToDateTime);
  }

  // "Tags"
  if (jsonParser.contains(_detail::TagsPropertyName))
  {
    auto const& tags = jsonParser[_detail::TagsPropertyName];
    {
      for (auto tag = tags.begin(); tag != tags.end(); ++tag)
      {
        secret.Tags.emplace(tag.key(), tag.value().get<std::string>());
      }
    }
  }

  // managed
  if (jsonParser.contains(_detail::ManagedPropertyName))
  {
    secret.Managed = jsonParser[_detail::ManagedPropertyName].get<bool>();
  }
}

