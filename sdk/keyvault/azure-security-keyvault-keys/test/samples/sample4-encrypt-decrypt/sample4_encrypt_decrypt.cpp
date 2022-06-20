// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample demonstrates how to encrypt and decrypt a single block of plain text with an
 * RSA key using the methods of the CryptographyClient.
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

  // Let's create a RSA key which will be used to encrypt and decrypt
  auto rsaKeyName = "CloudRsaKey-" + Azure::Core::Uuid::CreateUuid().ToString();
  auto keyOptions = CreateRsaKeyOptions(rsaKeyName, false);
  keyOptions.KeySize = 2048;
  KeyVaultKey cloudRsaKey = keyClient.CreateRsaKey(keyOptions).Value;
  std::cout << " - Key is returned with name " << cloudRsaKey.Name() << " and type "
            << cloudRsaKey.GetKeyType().ToString() << std::endl;

  CryptographyClient cryptoClient(cloudRsaKey.Id(), credential);

  uint8_t const data[] = "A single block of plaintext";
  std::vector<uint8_t> plaintext(std::begin(data), std::end(data));
  EncryptResult encryptResult
      = cryptoClient.Encrypt(EncryptParameters::RsaOaepParameters(plaintext)).Value;
  std::cout << " - Encrypted data using the algorithm " << encryptResult.Algorithm.ToString()
            << ", with key " << encryptResult.KeyId << ". The resulting encrypted data is: "
            << Azure::Core::Convert::Base64Encode(encryptResult.Ciphertext) << std::endl;

  DecryptResult decryptResult
      = cryptoClient.Decrypt(DecryptParameters::RsaOaepParameters(encryptResult.Ciphertext)).Value;
  std::cout << " - Decrypted data using the algorithm " << decryptResult.Algorithm.ToString()
            << ", with key " << decryptResult.KeyId << ". The resulting decrypted data is: "
            << std::string(decryptResult.Plaintext.begin(), decryptResult.Plaintext.end())
            << std::endl;

  // Delete the key
  auto deleteOperation = keyClient.StartDeleteKey(rsaKeyName);
  deleteOperation.PollUntilDone(2min);
  keyClient.PurgeDeletedKey(rsaKeyName);
}
