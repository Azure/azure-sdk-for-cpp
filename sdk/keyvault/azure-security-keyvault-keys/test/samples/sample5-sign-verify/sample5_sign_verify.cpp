// Copyright (c) Microsoft Corporation. All rights reserved.
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

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <azure/core.hpp>
#include <azure/identity.hpp>
#include <azure/keyvault/common/sha.hpp>
#include <azure/keyvault/key_vault_keys.hpp>

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

  auto ecKeyName = "CloudEcKey-" + Azure::Core::Uuid::CreateUuid().ToString();
  auto ecKeyOptions = CreateEcKeyOptions(ecKeyName, false);
  ecKeyOptions.CurveName = KeyCurveName::P256K;

  KeyVaultKey cloudRsaKey = keyClient.CreateRsaKey(keyOptions).Value;
  std::cout << " - Key is returned with name " << cloudRsaKey.Name() << " and type "
            << cloudRsaKey.GetKeyType().ToString() << std::endl;

  KeyVaultKey cloudEcKey = keyClient.CreateEcKey(ecKeyOptions).Value;
  std::cout << " - Key is returned with name " << cloudEcKey.Name() << " and type "
            << cloudEcKey.GetKeyType().ToString() << std::endl;

  CryptographyClient rsaCryptoClient(cloudRsaKey.Id(), credential);

  CryptographyClient ecCryptoClient(cloudEcKey.Id(), credential);

  // digestBase64 simulates some text data that has been hashed using the SHA256 algorithm
  // and then base 64 encoded. It is not relevant for the sample how to create the SHA256
  // hashed digest.
  // Example input data source for the digest:
  // "This is some sample data which we will use to demonstrate sign and verify"
  std::string digestBase64 = "DU9EdhpwhJqnGnieD0qKYEz6e8QPKlOVpYZZro+XtI8=";
  std::vector<uint8_t> digest = Azure::Core::Convert::Base64Decode(digestBase64);

  // Sign and Verify from digest
  SignResult rsaSignResult = rsaCryptoClient.Sign(SignatureAlgorithm::RS256, digest);
  std::cout << " - Signed digest using the algorithm " << rsaSignResult.Algorithm.ToString()
            << ", with key " << rsaSignResult.KeyId << ". The resulting signature is: "
            << Azure::Core::Convert::Base64Encode(rsaSignResult.Signature) << std::endl;

  SignResult ecSignResult = ecCryptoClient.Sign(SignatureAlgorithm::ES256K, digest);
  std::cout << " - Signed digest using the algorithm " << ecSignResult.Algorithm.ToString()
            << ", with key " << ecSignResult.KeyId << ". The resulting signature is: "
            << Azure::Core::Convert::Base64Encode(ecSignResult.Signature) << std::endl;

  VerifyResult rsaVerifyResult
      = rsaCryptoClient.Verify(SignatureAlgorithm::RS256, digest, rsaSignResult.Signature);
  std::cout << " - Verified the signature using the algorithm "
            << rsaVerifyResult.Algorithm.ToString() << ", with key " << rsaVerifyResult.KeyId
            << ". Signature is valid: " << (rsaVerifyResult.IsValid ? "True" : "False")
            << std::endl;

  VerifyResult ecVerifyResult
      = ecCryptoClient.Verify(SignatureAlgorithm::ES256K, digest, ecSignResult.Signature);
  std::cout << " - Verified the signature using the algorithm "
            << ecVerifyResult.Algorithm.ToString() << ", with key " << ecVerifyResult.KeyId
            << ". Signature is valid: " << (ecVerifyResult.IsValid ? "True" : "False") << std::endl;

  // Sign and Verify from data
  SignResult rsaSignDataResult = rsaCryptoClient.SignData(SignatureAlgorithm::RS256, data);
  std::cout << " - Signed data using the algorithm " << rsaSignDataResult.Algorithm.ToString()
            << ", with key " << rsaSignDataResult.KeyId << ". The resulting signature is: "
            << Azure::Core::Convert::Base64Encode(rsaSignDataResult.Signature) << std::endl;

  SignResult ecSignDataResult = ecCryptoClient.SignData(SignatureAlgorithm::ES256K, data);
  std::cout << " - Signed data using the algorithm " << ecSignDataResult.Algorithm.ToString()
            << ", with key " << ecSignDataResult.KeyId << ". The resulting signature is: "
            << Azure::Core::Convert::Base64Encode(ecSignDataResult.Signature) << std::endl;

  VerifyResult rsaVerifyDataResult
      = rsaCryptoClient.VerifyData(SignatureAlgorithm::RS256, data, rsaSignDataResult.Signature);
  std::cout << " - Verified the signature using the algorithm "
            << rsaVerifyDataResult.Algorithm.ToString() << ", with key "
            << rsaVerifyDataResult.KeyId
            << ". Signature is valid: " << (rsaVerifyDataResult.IsValid ? "True" : "False")
            << std::endl;

  VerifyResult ecVerifyDataResult
      = ecCryptoClient.VerifyData(SignatureAlgorithm::ES256K, data, ecSignDataResult.Signature);
  std::cout << " - Verified the signature using the algorithm "
            << ecVerifyDataResult.Algorithm.ToString() << ", with key " << ecVerifyDataResult.KeyId
            << ". Signature is valid: " << (ecVerifyDataResult.IsValid ? "True" : "False")
            << std::endl;

  // Delete the key
  auto deleteOperation = keyClient.StartDeleteKey(rsaKeyName);
  auto ecDeleteOperation = keyClient.StartDeleteKey(ecKeyName);
  deleteOperation.PollUntilDone(2min);
  ecDeleteOperation.PollUntilDone(2min);
  keyClient.PurgeDeletedKey(rsaKeyName);
  keyClient.PurgeDeletedKey(ecKeyName);
}
 
