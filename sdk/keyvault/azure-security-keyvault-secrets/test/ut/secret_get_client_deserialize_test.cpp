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

  Secret secret = _detail::SecretSerializer::Deserialize(response);
  Helpers::RunPartialExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientDeserializePartial2)
{
  auto response = Helpers::GetPartialResponse();

  Secret secret = _detail::SecretSerializer::Deserialize("name1", response);

  Helpers::RunPartialExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientDeserializePartial3)
{
  auto response = Helpers::GetPartialResponse();

  Secret secret = Secret("name2", "a");
  _detail::SecretSerializer::Deserialize(secret, response);

  Helpers::RunPartialExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientdeserializeFull1)
{
  auto response = Helpers::GetFullResponse();

  Secret secret = _detail::SecretSerializer::Deserialize(response);
  Helpers::RunFullExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientdeserializeFull2)
{
  auto response = Helpers::GetFullResponse();

  Secret secret = _detail::SecretSerializer::Deserialize("name1", response);

  Helpers::RunFullExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientdeserializeFull3)
{
  auto response = Helpers::GetFullResponse();

  Secret secret = Secret("name2", "a");
  _detail::SecretSerializer::Deserialize(secret, response);

  Helpers::RunFullExpect(secret);
}

TEST(KeyVaultDeletedSecretSerializer, GetDeletedClientDeserializeFull1)
{
  auto response = Helpers::GetDeletedFullResponse();

  DeletedSecret secret
      = _detail::KeyVaultDeletedSecretSerializer::KeyVaultDeletedSecretDeserialize(response);

  Helpers::RunFullExpect(secret, false);
  Helpers::RunDeletedExtras(secret);
}

TEST(KeyVaultDeletedSecretSerializer, GetDeletedClientDeserializeFull2)
{
  auto response = Helpers::GetDeletedFullResponse();

  DeletedSecret secret = _detail::KeyVaultDeletedSecretSerializer::KeyVaultDeletedSecretDeserialize(
      "name1", response);

  Helpers::RunFullExpect(secret, false);
  Helpers::RunDeletedExtras(secret);
}

TEST(KeyVaultDeletedSecretSerializer, GetDeletedClientDeserializeFull3)
{
  auto response = Helpers::GetDeletedFullResponse();

  DeletedSecret secret = DeletedSecret("name2");
  _detail::KeyVaultDeletedSecretSerializer::KeyVaultDeletedSecretDeserialize(secret, response);

  Helpers::RunFullExpect(secret, false);
  Helpers::RunDeletedExtras(secret);
}
