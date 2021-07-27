// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "secret_get_client_deserialize_test.hpp"
#include "../src/private/secret_serializers.hpp"

#include "azure/keyvault/secrets/secret_client.hpp"

using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Secrets::_test;
using namespace Azure::Security::KeyVault::Secrets::_detail;

TEST(KeyVaultSecretSerializer, GetClientDeserializePartial1)
{
  auto response = Helpers::GetPartialResponse();

  KeyVaultSecret secret = _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize(response);
  Helpers::RunPartialExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientDeserializePartial2)
{
  auto response = Helpers::GetPartialResponse();

  KeyVaultSecret secret
      = _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize("name1", response);

  Helpers::RunPartialExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientDeserializePartial3)
{
  auto response = Helpers::GetPartialResponse();

  KeyVaultSecret secret = KeyVaultSecret("name2", "a");
  _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize(secret, response);

  Helpers::RunPartialExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientdeserializeFull1)
{
  auto response = Helpers::GetFullResponse();

  KeyVaultSecret secret = _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize(response);
  Helpers::RunFullExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientdeserializeFull2)
{
  auto response = Helpers::GetFullResponse();

  KeyVaultSecret secret
      = _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize("name1", response);

  Helpers::RunFullExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientdeserializeFull3)
{
  auto response = Helpers::GetFullResponse();

  KeyVaultSecret secret = KeyVaultSecret("name2", "a");
  _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize(secret, response);

  Helpers::RunFullExpect(secret);
}

TEST(KeyVaultDeletedSecretSerializer, GetDeletedClientDeserializeFull1)
{
  auto response = getDeletedFullResponse();

  KeyVaultDeletedSecret secret
      = _detail::KeyVaultDeletedSecretSerializer::KeyVaultDeletedSecretDeserialize(response);

  runFullExpect(secret, false);
  runDeletedExtras(secret);
}

TEST(KeyVaultDeletedSecretSerializer, GetDeletedClientDeserializeFull2)
{
  auto response = getDeletedFullResponse();

  KeyVaultDeletedSecret secret
      = _detail::KeyVaultDeletedSecretSerializer::KeyVaultDeletedSecretDeserialize(
          "name1", response);

  runFullExpect(secret, false);
  runDeletedExtras(secret);
}

TEST(KeyVaultDeletedSecretSerializer, GetDeletedClientDeserializeFull3)
{
  auto response = getDeletedFullResponse();

  KeyVaultDeletedSecret secret = KeyVaultDeletedSecret("name2");
  _detail::KeyVaultDeletedSecretSerializer::KeyVaultDeletedSecretDeserialize(secret, response);

  runFullExpect(secret, false);
  runDeletedExtras(secret);
}
