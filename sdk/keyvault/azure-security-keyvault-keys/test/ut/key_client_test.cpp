//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "private/key_constants.hpp"
#include "private/key_serializers.hpp"
#include <azure/core/context.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <azure/keyvault/keys.hpp>

#include <exception>
#include <memory>

using namespace Azure::Security::KeyVault::Keys;

TEST(KeyVaultKeyClientUnitTest, initClient)
{
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>("tenantID", "AppId", "SecretId");
  {
    EXPECT_NO_THROW(KeyClient keyClient("http://account.vault.azure.net", credential));
  }
  {
    KeyClientOptions options;
    options.Retry.MaxRetries = 10;
    EXPECT_NO_THROW(KeyClient keyClient("http://account.vault.azure.net", credential, options));
  }
}

TEST(KeyVaultKeyClientUnitTest, ServiceVersion)
{
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>("tenantID", "AppId", "SecretId");
  // 7.3
  EXPECT_NO_THROW(auto options = KeyClientOptions();
                  KeyClient keyClient("http://account.vault.azure.net", credential, options);
                  EXPECT_EQ(options.ApiVersion, "7.3"););
}

TEST(KeyVaultKeyClientUnitTest, GetUrl)
{
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>("tenantID", "AppId", "SecretId");

  auto url = "vaultUrl";
  KeyClient keyClient(url, credential);
  EXPECT_EQ(url, keyClient.GetUrl());
}

TEST(KeyReleaseOptionsUnitTest, None)
{
  KeyReleaseOptions options;
  auto serialized = _detail::KeyReleaseOptionsSerializer::KeyReleaseOptionsSerialize(options);
  auto deserialized = Azure::Core::Json::_internal::json::parse(serialized);

  EXPECT_EQ(options.Target, deserialized[_detail::TargetValue]);
  EXPECT_EQ(nullptr, deserialized[_detail::NonceValue]);
  EXPECT_EQ(nullptr, deserialized[_detail::EncryptionValue]);
}

TEST(KeyReleaseOptionsUnitTest, One)
{
  KeyReleaseOptions options;
  options.Target = "xyz";
  auto serialized = _detail::KeyReleaseOptionsSerializer::KeyReleaseOptionsSerialize(options);
  auto deserialized = Azure::Core::Json::_internal::json::parse(serialized);

  EXPECT_EQ(options.Target, deserialized[_detail::TargetValue]);
  EXPECT_EQ(nullptr, deserialized[_detail::NonceValue]);
  EXPECT_EQ(nullptr, deserialized[_detail::EncryptionValue]);
}

TEST(KeyReleaseOptionsUnitTest, Most)
{
  KeyReleaseOptions options;
  options.Target = "xyz";
  options.Nonce = "abc";
  auto serialized = _detail::KeyReleaseOptionsSerializer::KeyReleaseOptionsSerialize(options);
  auto deserialized = Azure::Core::Json::_internal::json::parse(serialized);

  EXPECT_EQ(options.Target, deserialized[_detail::TargetValue]);
  EXPECT_EQ(options.Nonce.Value(), deserialized[_detail::NonceValue]);
  EXPECT_EQ(nullptr, deserialized[_detail::EncryptionValue]);
}

TEST(KeyReleaseOptionsUnitTest, All)
{
  KeyReleaseOptions options;
  options.Target = "xyz";
  options.Nonce = "abc";
  options.Encryption = KeyEncryptionAlgorithm::CKM_RSA_AES_KEY_WRAP;
  auto serialized = _detail::KeyReleaseOptionsSerializer::KeyReleaseOptionsSerialize(options);
  auto deserialized = Azure::Core::Json::_internal::json::parse(serialized);

  EXPECT_EQ(options.Target, deserialized[_detail::TargetValue]);
  EXPECT_EQ(options.Nonce.Value(), deserialized[_detail::NonceValue]);
  EXPECT_EQ(options.Encryption.Value().ToString(), deserialized[_detail::EncryptionValue]);
}

TEST(KeyEncryptionAlgorithmUnitTest, CheckValues)
{
  EXPECT_EQ(
      KeyEncryptionAlgorithm::CKM_RSA_AES_KEY_WRAP.ToString(), _detail::CKM_RSA_AES_KEY_WRAP_Value);
  EXPECT_EQ(
      KeyEncryptionAlgorithm::RSA_AES_KEY_WRAP_256.ToString(), _detail::RSA_AES_KEY_WRAP_256_Value);
  EXPECT_EQ(
      KeyEncryptionAlgorithm::RSA_AES_KEY_WRAP_384.ToString(), _detail::RSA_AES_KEY_WRAP_384_Value);
}
