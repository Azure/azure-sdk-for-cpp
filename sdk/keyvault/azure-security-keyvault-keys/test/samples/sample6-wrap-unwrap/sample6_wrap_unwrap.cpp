//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample demonstrates how to sign data with both a RSA key and an EC key using the
 * synchronous methods of the CryptographyClient.
 *
 * @remark The following environment variables must be set before running the sample.
 * - AZURE_KEYVAULT_URL:  To the Key Vault account URL.
 * - AZURE_TENANT_ID:     Tenant ID for the Azure account.
 * - AZURE_CLIENT_ID:     The Client ID to authenticate the request.
 * - AZURE_CLIENT_SECRET: The client secret.
 *
 */

#include "get_env.hpp"

#include <azure/core.hpp>
#include <azure/identity.hpp>
#include <azure/keyvault/keys.hpp>

#include <chrono>
#include <iostream>
#include <vector>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Keys::Cryptography;
using namespace std::chrono_literals;

int main()
{
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);

  KeyClient keyClient(std::getenv("AZURE_KEYVAULT_URL"), credential);

  auto rsaKeyName = "CloudRsaKey-" + Azure::Core::Uuid::CreateUuid().ToString();
  auto keyOptions = CreateRsaKeyOptions(rsaKeyName, false);
  keyOptions.KeySize = 2048;

  KeyVaultKey cloudRsaKey = keyClient.CreateRsaKey(keyOptions).Value;
  std::cout << " - Key is returned with name " << cloudRsaKey.Name() << " and type "
            << cloudRsaKey.GetKeyType().ToString() << std::endl;

  CryptographyClient cryptoClient(cloudRsaKey.Id(), credential);

  // keyDataSource simulates a symmetric private key created locally in the system. It is not
  // relevant for the sample how to create the private key as it depends on the OS.
  // For example, on linux, the key can be created using openSSL.
  uint8_t const keyDataSource[]
      = "MIIBOgIBAAJBAKUFtjMCrEZzg30Rb5EQnFy6fFUTn3wwVPM9yW4Icn7EMk34ic+"
        "3CYytbOqbRQDDUtbyUCdMEu2OZ0RPqL4GWMECAwEAAQJAcHi7HHs25XF3bbeDfbB/"
        "kae8c9PDAEaEr6At+......";
  std::vector<uint8_t> keyData(std::begin(keyDataSource), std::end(keyDataSource));
  std::cout << " - Using a sample generated key: " << Azure::Core::Convert::Base64Encode(keyData)
            << std::endl;

  auto wrapResult = cryptoClient.WrapKey(KeyWrapAlgorithm::RsaOaep, keyData).Value;
  std::cout << " - Encrypted data using the algorithm " << wrapResult.Algorithm.ToString()
            << ", with key " << wrapResult.KeyId << ". The resulting encrypted data is: "
            << Azure::Core::Convert::Base64Encode(wrapResult.EncryptedKey) << std::endl;

  auto unwrapResult
      = cryptoClient.UnwrapKey(KeyWrapAlgorithm::RsaOaep, wrapResult.EncryptedKey).Value;
  std::cout << " - Decrypted data using the algorithm " << unwrapResult.Algorithm.ToString()
            << ", with key " << unwrapResult.KeyId << ". The resulting decrypted data is: "
            << Azure::Core::Convert::Base64Encode(unwrapResult.Key) << std::endl;

  // Delete the key
  auto deleteOperation = keyClient.StartDeleteKey(rsaKeyName);
  deleteOperation.PollUntilDone(2min);
  keyClient.PurgeDeletedKey(rsaKeyName);
}
