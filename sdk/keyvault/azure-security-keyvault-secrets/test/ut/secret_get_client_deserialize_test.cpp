//  Copyright (c) Microsoft Corporation. All rights reserved.
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

  KeyVaultSecret secret = _detail::SecretSerializer::Deserialize(response);
  Helpers::RunPartialExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientDeserializePartial2)
{
  auto response = Helpers::GetPartialResponse();

  KeyVaultSecret secret = _detail::SecretSerializer::Deserialize("name1", response);

  Helpers::RunPartialExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientDeserializePartial3)
{
  auto response = Helpers::GetPartialResponse();

  KeyVaultSecret secret = KeyVaultSecret("name2", "a");
  _detail::SecretSerializer::Deserialize(secret, response);

  Helpers::RunPartialExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientdeserializeFull1)
{
  auto response = Helpers::GetFullResponse();

  KeyVaultSecret secret = _detail::SecretSerializer::Deserialize(response);
  Helpers::RunFullExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientdeserializeFull2)
{
  auto response = Helpers::GetFullResponse();

  KeyVaultSecret secret = _detail::SecretSerializer::Deserialize("name1", response);

  Helpers::RunFullExpect(secret);
}

TEST(KeyVaultSecretSerializer, GetClientdeserializeFull3)
{
  auto response = Helpers::GetFullResponse();

  KeyVaultSecret secret = KeyVaultSecret("name2", "a");
  _detail::SecretSerializer::Deserialize(secret, response);

  Helpers::RunFullExpect(secret);
}

TEST(DeletedSecretSerializer, GetDeletedClientDeserializeFull1)
{
  auto response = Helpers::GetDeletedFullResponse();

  DeletedSecret secret = _detail::DeletedSecretSerializer::Deserialize(response);

  Helpers::RunFullExpect(secret, false);
  Helpers::RunDeletedExtras(secret);
}

TEST(DeletedSecretSerializer, GetDeletedClientDeserializeFull2)
{
  auto response = Helpers::GetDeletedFullResponse();

  DeletedSecret secret = _detail::DeletedSecretSerializer::Deserialize("name1", response);

  Helpers::RunFullExpect(secret, false);
  Helpers::RunDeletedExtras(secret);
}

TEST(DeletedSecretSerializer, GetDeletedClientDeserializeFull3)
{
  auto response = Helpers::GetDeletedFullResponse();

  DeletedSecret secret = DeletedSecret("name2");
  _detail::DeletedSecretSerializer::Deserialize(secret, response);

  Helpers::RunFullExpect(secret, false);
  Helpers::RunDeletedExtras(secret);
}

TEST(SecretProperties, FactoryValid)
{
  std::string url(
      "https://myvault.vault.azure.net/secrets/my_secret_name/4387e9f3d6e14c459867679a90fd0f79");
  SecretProperties props = SecretProperties::CreateFromURL(url);
  EXPECT_EQ(props.Name, "my_secret_name");
  EXPECT_EQ(props.Version, "4387e9f3d6e14c459867679a90fd0f79");
  EXPECT_EQ(props.Id, url);
  EXPECT_EQ(props.VaultUrl, "https://myvault.vault.azure.net");
}
