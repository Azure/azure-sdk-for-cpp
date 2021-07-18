#include "private/secret_serializers.hpp"
#include "private/secret_constants.hpp"

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

/*
// Create from HTTP raw response only.
static KeyVaultSecret KeyVaultSecretDeserialize(Azure::Core::Http::RawResponse const& rawResponse);
*/
// Updates a Key based on an HTTP raw response.
void KeyVaultSecretSerializer::KeyVaultSecretDeserialize(
    KeyVaultSecret& key,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  key.Id = rawResponse.GetReasonPhrase();
}

