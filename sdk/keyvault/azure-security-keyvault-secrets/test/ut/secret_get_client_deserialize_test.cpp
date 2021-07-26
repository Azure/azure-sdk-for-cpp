// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "secret_get_client_deserialize_test.hpp"
#include "../src/private/secret_serializers.hpp"

#include "azure/keyvault/secrets/secret_client.hpp"

using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Secrets::_detail;

TEST(KeyVaultSecretSerializer, GetClientDeserializePartial1)
{
  auto response = getPartialResponse();

  KeyVaultSecret secret = _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize(response);
  runPartialExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientDeserializePartial2)
{
  auto response = getPartialResponse();

  KeyVaultSecret secret
      = _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize("name1", response);

  runPartialExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientDeserializePartial3)
{
  auto response = getPartialResponse();

  KeyVaultSecret secret = KeyVaultSecret("name2", "");
  _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize(secret, response);

  runPartialExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientdeserializeFull1)
{
  auto response = getFullResponse();

  KeyVaultSecret secret = _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize(response);
  runFullExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientdeserializeFull2)
{
  auto response = getFullResponse();

  KeyVaultSecret secret
      = _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize("name1", response);

  runFullExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientdeserializeFull3)
{
  auto response = getFullResponse();

  KeyVaultSecret secret = KeyVaultSecret("name2", "");
  _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize(secret, response);

  runFullExpect(secret);
}
