// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "../src/private/secret_serializers.hpp"
#include "secret_get_client_deserialize_test.hpp"

#include <azure/keyvault/secrets/secret_client.hpp>

using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Secrets::_detail;

TEST(KeyVaultSecretSerializer, GetClientDeserializePartial1)
{
  auto response = getPartialResponse();

  KeyVaultSecret secret = _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize(response);
  EXPECT_EQ(secret.Attributes.Name.HasValue(), false);
  runPartialExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientDeserializePartial2)
{
  auto response = getPartialResponse();

  KeyVaultSecret secret
      = _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize("name1", response);

  EXPECT_EQ(secret.Attributes.Name.Value(), "name1");
  runPartialExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientDeserializePartial3)
{
  auto response = getPartialResponse();

  KeyVaultSecret secret = KeyVaultSecret("name2");
  _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize(secret, response);

  EXPECT_EQ(secret.Attributes.Name.Value(), "name2");
  runPartialExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientdeserializeFull1)
{
  auto response = getFullResponse();

  KeyVaultSecret secret = _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize(response);
  EXPECT_EQ(secret.Attributes.Name.HasValue(), false);
  runFullExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientdeserializeFull2)
{
  auto response = getFullResponse();

  KeyVaultSecret secret
      = _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize("name1", response);

  EXPECT_EQ(secret.Attributes.Name.Value(), "name1");
  runFullExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientdeserializeFull3)
{
  auto response = getFullResponse();

  KeyVaultSecret secret = KeyVaultSecret("name2");
  _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize(secret, response);

  EXPECT_EQ(secret.Attributes.Name.Value(), "name2");
  runFullExpect(secret);
}
