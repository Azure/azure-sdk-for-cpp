// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "secret_get_client_deserialize_test.hpp"

#include "../src/private/secret_serializers.hpp"
#include "azure/keyvault/secrets/secret_client.hpp"

using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Secrets::_test;
using namespace Azure::Security::KeyVault::Secrets::_detail;

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
