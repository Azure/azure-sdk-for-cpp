// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include <azure/core/internal/cryptography/sha_hash.hpp>

#include "key_client_base_test.hpp"

#include <azure/keyvault/keys.hpp>

#include <string>
#include <vector>

using namespace Azure::Security::KeyVault::Keys::Test;
using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Keys::Cryptography;

TEST_P(KeyVaultKeyClientWithParam, RemoteEncrypt)
{
  auto const keyName = GetTestName(true);
  auto const& client = GetClientForTest(keyName);

  CreateRsaKeyOptions rsaKeyOptions(keyName);
  rsaKeyOptions.KeySize = GetParam();
  auto rsaKey = client.CreateRsaKey(rsaKeyOptions).Value;

  // init crypto client from key ID. The remote client will get the key and try to create a local
  // crypto client.
  auto cryptoClient = GetCryptoClient(rsaKey.Id());

  {
    uint8_t plaintextSource[] = "A single block of plaintext";
    std::vector<uint8_t> plaintext(std::begin(plaintextSource), std::end(plaintextSource));

    auto encryptResult
        = cryptoClient->Encrypt(EncryptParameters::RsaOaepParameters(plaintext)).Value;
    EXPECT_EQ(encryptResult.Algorithm.ToString(), EncryptionAlgorithm::RsaOaep.ToString());
    EXPECT_EQ(encryptResult.KeyId, rsaKey.Id());
    EXPECT_TRUE(encryptResult.Ciphertext.size() > 0);

    auto decryptResult
        = cryptoClient->Decrypt(DecryptParameters::RsaOaepParameters(encryptResult.Ciphertext))
              .Value;
    EXPECT_EQ(decryptResult.Algorithm.ToString(), encryptResult.Algorithm.ToString());
    EXPECT_EQ(decryptResult.Plaintext, plaintext);
    EXPECT_EQ(decryptResult.KeyId, encryptResult.KeyId);
  }
}

TEST_P(KeyVaultKeyClientWithParam, RemoteWrap)
{
  auto const keyName = GetTestName(true);
  auto const& client = GetClientForTest(keyName);

  CreateRsaKeyOptions rsaKeyOptions(keyName);
  rsaKeyOptions.KeySize = GetParam();
  auto rsaKey = client.CreateRsaKey(rsaKeyOptions).Value;

  // init crypto client from key ID. The remote client will get the key and try to create a local
  // crypto client.
  auto cryptoClient = GetCryptoClient(rsaKey.Id());

  {
    uint8_t plaintextSource[] = "A single block of plaintext";
    std::vector<uint8_t> plaintext(std::begin(plaintextSource), std::end(plaintextSource));

    auto wrapResult = cryptoClient->WrapKey(KeyWrapAlgorithm::RsaOaep256, plaintext).Value;
    EXPECT_EQ(wrapResult.Algorithm.ToString(), KeyWrapAlgorithm::RsaOaep256.ToString());
    EXPECT_EQ(wrapResult.KeyId, rsaKey.Id());
    EXPECT_TRUE(wrapResult.EncryptedKey.size() > 0);

    auto unwrapResult
        = cryptoClient->UnwrapKey(wrapResult.Algorithm, wrapResult.EncryptedKey).Value;
    EXPECT_EQ(unwrapResult.Algorithm.ToString(), wrapResult.Algorithm.ToString());
    EXPECT_EQ(unwrapResult.Key, plaintext);
    EXPECT_EQ(unwrapResult.KeyId, wrapResult.KeyId);
  }
}

TEST_P(KeyVaultKeyClientWithParam, RemoteSignVerifyRSA256)
{
  auto const keyName = GetTestName(true);
  auto const& client = GetClientForTest(keyName);

  CreateRsaKeyOptions rsaKeyOptions(keyName);
  rsaKeyOptions.KeySize = GetParam();
  auto rsaKey = client.CreateRsaKey(rsaKeyOptions).Value;

  // init crypto client from key ID. The remote client will get the key and try to create a local
  // crypto client.
  auto cryptoClient = GetCryptoClient(rsaKey.Id());
  std::string digestSource("A single block of plaintext");

  // RS256
  {
    Azure::Core::Cryptography::_internal::Sha256Hash sha256;
    auto signatureAlgorithm = SignatureAlgorithm::RS256;
    std::vector<uint8_t> digest
        = sha256.Final(reinterpret_cast<const uint8_t*>(digestSource.data()), digestSource.size());

    auto signResult = cryptoClient->Sign(signatureAlgorithm, digest).Value;
    EXPECT_EQ(signResult.Algorithm.ToString(), signatureAlgorithm.ToString());
    EXPECT_EQ(signResult.KeyId, rsaKey.Id());
    EXPECT_TRUE(signResult.Signature.size() > 0);

    auto verifyResult
        = cryptoClient->Verify(signResult.Algorithm, digest, signResult.Signature).Value;
    EXPECT_EQ(verifyResult.Algorithm.ToString(), verifyResult.Algorithm.ToString());
    EXPECT_EQ(verifyResult.KeyId, rsaKey.Id());
    EXPECT_TRUE(verifyResult.IsValid);
  }

  // PS256
  {
    Azure::Core::Cryptography::_internal::Sha256Hash sha256;
    auto signatureAlgorithm = SignatureAlgorithm::PS256;
    std::vector<uint8_t> digest
        = sha256.Final(reinterpret_cast<const uint8_t*>(digestSource.data()), digestSource.size());

    auto signResult = cryptoClient->Sign(signatureAlgorithm, digest).Value;
    EXPECT_EQ(signResult.Algorithm.ToString(), signatureAlgorithm.ToString());
    EXPECT_EQ(signResult.KeyId, rsaKey.Id());
    EXPECT_TRUE(signResult.Signature.size() > 0);

    auto verifyResult
        = cryptoClient->Verify(signResult.Algorithm, digest, signResult.Signature).Value;
    EXPECT_EQ(verifyResult.Algorithm.ToString(), verifyResult.Algorithm.ToString());
    EXPECT_EQ(verifyResult.KeyId, rsaKey.Id());
    EXPECT_TRUE(verifyResult.IsValid);
  }
}

TEST_F(KeyVaultKeyClient, RemoteSignVerifyES256)
{
  auto const keyName = GetTestName(true);
  auto const& client = GetClientForTest(keyName);
  std::string digestSource("A single block of plaintext");

  // ES256
  {
    CreateEcKeyOptions ecKeyOptions(keyName);
    ecKeyOptions.CurveName = KeyCurveName::P256;
    auto ecKey = client.CreateEcKey(ecKeyOptions).Value;
    auto cryptoClient = GetCryptoClient(ecKey.Id());

    Azure::Core::Cryptography::_internal::Sha256Hash sha256;
    auto signatureAlgorithm = SignatureAlgorithm::ES256;
    std::vector<uint8_t> digest
        = sha256.Final(reinterpret_cast<const uint8_t*>(digestSource.data()), digestSource.size());

    auto signResult = cryptoClient->Sign(signatureAlgorithm, digest).Value;
    EXPECT_EQ(signResult.Algorithm.ToString(), signatureAlgorithm.ToString());
    EXPECT_EQ(signResult.KeyId, ecKey.Id());
    EXPECT_TRUE(signResult.Signature.size() > 0);

    auto verifyResult
        = cryptoClient->Verify(signResult.Algorithm, digest, signResult.Signature).Value;
    EXPECT_EQ(verifyResult.Algorithm.ToString(), verifyResult.Algorithm.ToString());
    EXPECT_EQ(verifyResult.KeyId, ecKey.Id());
    EXPECT_TRUE(verifyResult.IsValid);
  }

  // ES256K
  {
    CreateEcKeyOptions ecKeyOptions(keyName);
    ecKeyOptions.CurveName = KeyCurveName::P256K;
    auto ecKey = client.CreateEcKey(ecKeyOptions).Value;
    auto cryptoClient = GetCryptoClient(ecKey.Id());

    Azure::Core::Cryptography::_internal::Sha256Hash sha256;
    auto signatureAlgorithm = SignatureAlgorithm::ES256K;
    std::vector<uint8_t> digest
        = sha256.Final(reinterpret_cast<const uint8_t*>(digestSource.data()), digestSource.size());

    auto signResult = cryptoClient->Sign(signatureAlgorithm, digest).Value;
    EXPECT_EQ(signResult.Algorithm.ToString(), signatureAlgorithm.ToString());
    EXPECT_EQ(signResult.KeyId, ecKey.Id());
    EXPECT_TRUE(signResult.Signature.size() > 0);

    auto verifyResult
        = cryptoClient->Verify(signResult.Algorithm, digest, signResult.Signature).Value;
    EXPECT_EQ(verifyResult.Algorithm.ToString(), verifyResult.Algorithm.ToString());
    EXPECT_EQ(verifyResult.KeyId, ecKey.Id());
    EXPECT_TRUE(verifyResult.IsValid);
  }
}

TEST_P(KeyVaultKeyClientWithParam, RemoteSignVerifyRSA384)
{
  auto const keyName = GetTestName(true);
  auto const& client = GetClientForTest(keyName);

  CreateRsaKeyOptions rsaKeyOptions(keyName);
  rsaKeyOptions.KeySize = GetParam();
  auto rsaKey = client.CreateRsaKey(rsaKeyOptions).Value;

  // init crypto client from key ID. The remote client will get the key and try to create a local
  // crypto client.
  auto cryptoClient = GetCryptoClient(rsaKey.Id());
  std::string digestSource("A single block of plaintext");

  // RS384
  {
    Azure::Core::Cryptography::_internal::Sha384Hash sha384;
    auto signatureAlgorithm = SignatureAlgorithm::RS384;
    std::vector<uint8_t> digest
        = sha384.Final(reinterpret_cast<const uint8_t*>(digestSource.data()), digestSource.size());

    auto signResult = cryptoClient->Sign(signatureAlgorithm, digest).Value;
    EXPECT_EQ(signResult.Algorithm.ToString(), signatureAlgorithm.ToString());
    EXPECT_EQ(signResult.KeyId, rsaKey.Id());
    EXPECT_TRUE(signResult.Signature.size() > 0);

    auto verifyResult
        = cryptoClient->Verify(signResult.Algorithm, digest, signResult.Signature).Value;
    EXPECT_EQ(verifyResult.Algorithm.ToString(), verifyResult.Algorithm.ToString());
    EXPECT_EQ(verifyResult.KeyId, rsaKey.Id());
    EXPECT_TRUE(verifyResult.IsValid);
  }

  // PS384
  {
    Azure::Core::Cryptography::_internal::Sha384Hash sha384;
    auto signatureAlgorithm = SignatureAlgorithm::PS384;
    std::vector<uint8_t> digest
        = sha384.Final(reinterpret_cast<const uint8_t*>(digestSource.data()), digestSource.size());

    auto signResult = cryptoClient->Sign(signatureAlgorithm, digest).Value;
    EXPECT_EQ(signResult.Algorithm.ToString(), signatureAlgorithm.ToString());
    EXPECT_EQ(signResult.KeyId, rsaKey.Id());
    EXPECT_TRUE(signResult.Signature.size() > 0);

    auto verifyResult
        = cryptoClient->Verify(signResult.Algorithm, digest, signResult.Signature).Value;
    EXPECT_EQ(verifyResult.Algorithm.ToString(), verifyResult.Algorithm.ToString());
    EXPECT_EQ(verifyResult.KeyId, rsaKey.Id());
    EXPECT_TRUE(verifyResult.IsValid);
  }
}

TEST_P(KeyVaultKeyClientWithParam, RemoteSignVerifyDataRSA256)
{
  auto const keyName = GetTestName(true);
  auto const& client = GetClientForTest(keyName);

  CreateRsaKeyOptions rsaKeyOptions(keyName);
  rsaKeyOptions.KeySize = GetParam();
  auto rsaKey = client.CreateRsaKey(rsaKeyOptions).Value;

  // init crypto client from key ID. The remote client will get the key and try to create a local
  // crypto client.
  auto cryptoClient = GetCryptoClient(rsaKey.Id());
  uint8_t dataSource[] = "A single block of plaintext";
  std::vector<uint8_t> data(std::begin(dataSource), std::end(dataSource));

  // RS256
  {
    auto signatureAlgorithm = SignatureAlgorithm::RS256;
    auto signResult = cryptoClient->SignData(signatureAlgorithm, data).Value;
    EXPECT_EQ(signResult.Algorithm.ToString(), signatureAlgorithm.ToString());
    EXPECT_EQ(signResult.KeyId, rsaKey.Id());
    EXPECT_TRUE(signResult.Signature.size() > 0);

    auto verifyResult
        = cryptoClient->VerifyData(signResult.Algorithm, data, signResult.Signature).Value;
    EXPECT_EQ(verifyResult.Algorithm.ToString(), verifyResult.Algorithm.ToString());
    EXPECT_EQ(verifyResult.KeyId, rsaKey.Id());
    EXPECT_TRUE(verifyResult.IsValid);
  }

  // PS256
  {
    auto signatureAlgorithm = SignatureAlgorithm::PS256;
    auto signResult = cryptoClient->SignData(signatureAlgorithm, data).Value;
    EXPECT_EQ(signResult.Algorithm.ToString(), signatureAlgorithm.ToString());
    EXPECT_EQ(signResult.KeyId, rsaKey.Id());
    EXPECT_TRUE(signResult.Signature.size() > 0);

    auto verifyResult
        = cryptoClient->VerifyData(signResult.Algorithm, data, signResult.Signature).Value;
    EXPECT_EQ(verifyResult.Algorithm.ToString(), verifyResult.Algorithm.ToString());
    EXPECT_EQ(verifyResult.KeyId, rsaKey.Id());
    EXPECT_TRUE(verifyResult.IsValid);
  }
}

TEST_P(KeyVaultKeyClientWithParam, GetCryptoFromKeyRemoteEncrypt)
{
  auto const keyName = GetTestName(true);
  auto const& client = GetClientForTest(keyName);

  CreateRsaKeyOptions rsaKeyOptions(keyName);
  rsaKeyOptions.KeySize = GetParam();
  auto rsaKey = client.CreateRsaKey(rsaKeyOptions).Value;

  // Get Crypto client from the key client
  auto cryptoClient = client.GetCryptographyClient(keyName);

  {
    uint8_t plaintextSource[] = "A single block of plaintext";
    std::vector<uint8_t> plaintext(std::begin(plaintextSource), std::end(plaintextSource));

    auto encryptResult
        = cryptoClient.Encrypt(EncryptParameters::RsaOaepParameters(plaintext)).Value;
    EXPECT_EQ(encryptResult.Algorithm.ToString(), EncryptionAlgorithm::RsaOaep.ToString());
    EXPECT_EQ(encryptResult.KeyId, rsaKey.Id());
    EXPECT_TRUE(encryptResult.Ciphertext.size() > 0);

    auto decryptResult
        = cryptoClient.Decrypt(DecryptParameters::RsaOaepParameters(encryptResult.Ciphertext))
              .Value;
    EXPECT_EQ(decryptResult.Algorithm.ToString(), encryptResult.Algorithm.ToString());
    EXPECT_EQ(decryptResult.Plaintext, plaintext);
    EXPECT_EQ(decryptResult.KeyId, encryptResult.KeyId);
  }
}

TEST_P(KeyVaultKeyClientWithParam, GetCryptoFromKeyVersionRemoteEncrypt)
{
  auto const keyName = GetTestName(true);
  auto const& client = GetClientForTest(keyName);

  CreateRsaKeyOptions rsaKeyOptions(keyName);
  rsaKeyOptions.KeySize = GetParam();
  auto rsaKey = client.CreateRsaKey(rsaKeyOptions).Value;

  // Get Crypto client from the key client
  auto cryptoClient = client.GetCryptographyClient(rsaKey.Name(), rsaKey.Properties.Version);

  {
    uint8_t plaintextSource[] = "A single block of plaintext";
    std::vector<uint8_t> plaintext(std::begin(plaintextSource), std::end(plaintextSource));

    auto encryptResult
        = cryptoClient.Encrypt(EncryptParameters::RsaOaepParameters(plaintext)).Value;
    EXPECT_EQ(encryptResult.Algorithm.ToString(), EncryptionAlgorithm::RsaOaep.ToString());
    EXPECT_EQ(encryptResult.KeyId, rsaKey.Id());
    EXPECT_TRUE(encryptResult.Ciphertext.size() > 0);

    auto decryptResult
        = cryptoClient.Decrypt(DecryptParameters::RsaOaepParameters(encryptResult.Ciphertext))
              .Value;
    EXPECT_EQ(decryptResult.Algorithm.ToString(), encryptResult.Algorithm.ToString());
    EXPECT_EQ(decryptResult.Plaintext, plaintext);
    EXPECT_EQ(decryptResult.KeyId, encryptResult.KeyId);
  }
}

namespace {
static std::string GetSuffix(const testing::TestParamInfo<int>& info)
{
  auto stringValue = std::to_string(abs(info.param));
  return info.param < 0 ? "Minus" + stringValue : stringValue;
}
} // namespace

INSTANTIATE_TEST_SUITE_P(
    Crypto,
    KeyVaultKeyClientWithParam,
    ::testing::Values(-215, -100, 0, 13, 55, 233, 987, 1597, 2048, 3072, 4096),
    GetSuffix);