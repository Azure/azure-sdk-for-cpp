#pragma once
#include <azure/core/internal/json/json.hpp>
#include <azure/keyvault/secrets/keyvault_secret.hpp>
#include <azure/core/http/http.hpp>
#include <azure/keyvault/secrets/keyvault_secret_set_parameters.hpp>

using namespace Azure::Security::KeyVault::Secrets;

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets { namespace _detail {
  struct KeyVaultSecretSerializer final
  {
    // Creates a new key based on a name and an HTTP raw response.
    static KeyVaultSecret KeyVaultSecretDeserialize(
        std::string const& name,
        Azure::Core::Http::RawResponse const& rawResponse);

    // Create from HTTP raw response only.
    static KeyVaultSecret KeyVaultSecretDeserialize(
        Azure::Core::Http::RawResponse const& rawResponse);

    // Updates a Key based on an HTTP raw response.
    static void KeyVaultSecretDeserialize(
        KeyVaultSecret& key,
        Azure::Core::Http::RawResponse const& rawResponse);
  };

  struct KeyvaultSecretSetParametersSerializer final
  {
    static std::string KeyvaultSecretSetParametersSerialize(
        KeyVaultSecretSetParameters const& parameters);
  };
}}}}} // namespace Azure::Security::KeyVault::Secrets::_detail
