// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "secret_paged_deserialize_test.hpp"

using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Secrets::_test;
using namespace Azure::Security::KeyVault::Secrets::_detail;
using namespace Azure::Core::Json::_internal;

TEST(KeyVaultSecretPropertiesPagedResponse, SingleWithNext)
{
  auto response = _test::PagedHelpers::GetFirstResponse();

  auto result = _detail::KeyVaultSecretPropertiesPagedResultSerializer::
      KeyVaultSecretPropertiesPagedResponseDeserialize(response);

  EXPECT_EQ(result.Items.size(), 1);
  EXPECT_EQ(
      result.NextPageToken.Value(),
      "https://gearama-test2.vault.azure.net:443/"
      "secrets?api-version=7.2&$skiptoken="
      "eyJOZXh0TWFya2VyIjoiMiE4NCFNREF3TURFM0lYTmxZM0psZEM5VFQwMUZVMFZEVWtWVUlUQXdNREF5T0NFNU9UazVM"
      "VEV5TFRNeFZESXpPalU1T2pVNUxqazVPVGs1T1RsYUlRLS0iLCJUYXJnZXRMb2NhdGlvbiI6MH0&maxresults=1");

  auto item = result.Items[0];
  EXPECT_EQ(item.Enabled.Value(), true);
  EXPECT_EQ(item.RecoverableDays.Value(), 90);
  EXPECT_EQ(item.RecoveryLevel.Value(), "Recoverable+Purgeable");
  EXPECT_EQ(item.Id, "https://gearama-test2.vault.azure.net/secrets/gdfgfd");
}

TEST(KeyVaultSecretPropertiesPagedResponse, MultipleNoNext)
{
  auto response = _test::PagedHelpers::GetMultipleResponse();

  auto result = _detail::KeyVaultSecretPropertiesPagedResultSerializer::
      KeyVaultSecretPropertiesPagedResponseDeserialize(response);

  EXPECT_EQ(result.Items.size(), 3);
  EXPECT_EQ(result.NextPageToken.HasValue(), false);

  auto item = result.Items[0];
  EXPECT_EQ(item.Enabled.Value(), true);
  EXPECT_EQ(item.RecoverableDays.Value(), 90);
  EXPECT_EQ(item.RecoveryLevel.Value(), "Recoverable+Purgeable");
  EXPECT_EQ(
      item.Id,
      "https://gearama-test2.vault.azure.net/secrets/gdfgfd/5a0fdd819481420eac6f3282ce722461");
  EXPECT_EQ(item.Name, "gdfgfd");
  EXPECT_EQ(item.Version, "5a0fdd819481420eac6f3282ce722461");

  item = result.Items[1];
  EXPECT_EQ(item.Enabled.Value(), true);
  EXPECT_EQ(item.RecoverableDays.Value(), 90);
  EXPECT_EQ(item.RecoveryLevel.Value(), "Recoverable+Purgeable");
  EXPECT_EQ(
      item.Id,
      "https://gearama-test2.vault.azure.net/secrets/gdfgfd/8faafbb99216484dbbd75f9dd6bcaadf");
  EXPECT_EQ(item.Name, "gdfgfd");
  EXPECT_EQ(item.Version, "8faafbb99216484dbbd75f9dd6bcaadf");

  item = result.Items[2];
  EXPECT_EQ(item.Enabled.Value(), true);
  EXPECT_EQ(item.RecoverableDays.Value(), 90);
  EXPECT_EQ(item.RecoveryLevel.Value(), "Recoverable+Purgeable");
  EXPECT_EQ(
      item.Id,
      "https://gearama-test2.vault.azure.net/secrets/gdfgfd/d75080822f03400ab4d658bd0e988ac5");
  EXPECT_EQ(item.Name, "gdfgfd");
  EXPECT_EQ(item.Version, "d75080822f03400ab4d658bd0e988ac5");
}

TEST(KeyVaultSecretPropertiesPagedResponse, NoneNoNext)
{
  auto response = _test::PagedHelpers::GetEmptyResponse();

  auto result = _detail::KeyVaultSecretPropertiesPagedResultSerializer::
      KeyVaultSecretPropertiesPagedResponseDeserialize(response);

  EXPECT_EQ(result.Items.size(), 0);
  EXPECT_EQ(result.NextPageToken.HasValue(), false);
}

TEST(KeyVaultSecretDeletedSecretPagedResultSerializer, SingleWithNext)
{
  auto response = _test::PagedHelpers::GetDeletedFirstResponse();

  auto result = _detail::KeyVaultSecretDeletedSecretPagedResultSerializer::
      KeyVaultSecretDeletedSecretPagedResultDeserialize(response);

  EXPECT_EQ(result.Items.size(), 1);
  EXPECT_EQ(result.NextPageToken.Value(), "nextLink");

  auto item = result.Items[0];
  EXPECT_EQ(item.Properties.Enabled.Value(), true);
  EXPECT_EQ(item.Properties.RecoverableDays.Value(), 90);
  EXPECT_EQ(item.Properties.RecoveryLevel.Value(), "Recoverable+Purgeable");
  EXPECT_EQ(item.Id, "https://gearama-test2.vault.azure.net/secrets/eqwewq");
  EXPECT_EQ(item.RecoveryId, "https://gearama-test2.vault.azure.net/deletedsecrets/eqwewq");
}

TEST(KeyVaultSecretDeletedSecretPagedResultSerializer, MultipleNoNext)
{
  auto response = _test::PagedHelpers::GetDeletedMultipleResponse();

  auto result = _detail::KeyVaultSecretDeletedSecretPagedResultSerializer::
      KeyVaultSecretDeletedSecretPagedResultDeserialize(response);

  EXPECT_EQ(result.Items.size(), 3);
  EXPECT_FALSE(result.NextPageToken.HasValue());

  auto item = result.Items[0];
  EXPECT_EQ(item.Properties.Enabled.Value(), true);
  EXPECT_EQ(item.Properties.RecoverableDays.Value(), 90);
  EXPECT_EQ(item.Properties.RecoveryLevel.Value(), "Recoverable+Purgeable");
  EXPECT_EQ(item.Id, "https://gearama-test2.vault.azure.net/secrets/eqwewq");
  EXPECT_EQ(item.RecoveryId, "https://gearama-test2.vault.azure.net/deletedsecrets/eqwewq");

  item = result.Items[1];
  EXPECT_EQ(item.Properties.Enabled.Value(), true);
  EXPECT_EQ(item.Properties.RecoverableDays.Value(), 90);
  EXPECT_EQ(item.Properties.RecoveryLevel.Value(), "Recoverable+Purgeable");
  EXPECT_EQ(item.Id, "https://gearama-test2.vault.azure.net/secrets/someSecret");
  EXPECT_EQ(item.RecoveryId, "https://gearama-test2.vault.azure.net/secrets/someSecret");

  item = result.Items[2];
  EXPECT_EQ(item.Properties.Enabled.Value(), true);
  EXPECT_EQ(item.Properties.RecoverableDays.Value(), 90);
  EXPECT_EQ(item.Properties.RecoveryLevel.Value(), "Recoverable+Purgeable");
  EXPECT_EQ(item.Id, "https://gearama-test2.vault.azure.net/secrets/someSecret2");
  EXPECT_EQ(item.RecoveryId, "https://gearama-test2.vault.azure.net/deletedsecrets/someSecret2");
}

TEST(KeyVaultSecretDeletedSecretPagedResultSerializer, NoneNoNext)
{
  auto response = _test::PagedHelpers::GetEmptyResponse();

  auto result = _detail::KeyVaultSecretDeletedSecretPagedResultSerializer::
      KeyVaultSecretDeletedSecretPagedResultDeserialize(response);

  EXPECT_EQ(result.Items.size(), 0);
  EXPECT_EQ(result.NextPageToken.HasValue(), false);
}
