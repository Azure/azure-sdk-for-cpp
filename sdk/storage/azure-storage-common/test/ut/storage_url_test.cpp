// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/storage/common/internal/storage_url.hpp>

#include <gtest/gtest.h>

namespace Azure { namespace Storage { namespace Test {

  TEST(StorageUrlTest, ParsesStandardEndpoint)
  {
    auto result = _internal::ParseStorageUrl(
        Azure::Core::Url("https://account.blob.core.windows.net/container/blob?versionid=version"));

    ASSERT_TRUE(result.HasValue());
    EXPECT_EQ(result.Value().Service, "blob");
    EXPECT_EQ(result.Value().AccountName, "account");
    EXPECT_EQ(result.Value().ServiceUrl, "https://account.blob.core.windows.net");
    ASSERT_TRUE(result.Value().ContainerName.HasValue());
    EXPECT_EQ(result.Value().ContainerName.Value(), "container");
    EXPECT_EQ(
        result.Value().ContainerUrl.Value(), "https://account.blob.core.windows.net/container");
  }

  TEST(StorageUrlTest, ParsesMixedCaseEndpoints)
  {
    for (const std::string hostname :
         {"Account.BLOB.CORE.WINDOWS.NET",
          "ACCOUNT.blob.core.windows.net",
          "Account-DuAlStAcK-SeCoNdArY.BlOb.CoRe.WiNdOwS.NeT",
          "Account.BlOb.PrEpRoD.CoRe.WiNdOwS.NeT",
          "Account.BlOb.CoRe.ChInAcLoUdApI.Cn",
          "Account.BlOb.CoRe.UsGoVcLoUdApI.NeT",
          "Account-SeCoNdArY.Z50.BlOb.StOrAgE.AzUrE.NeT",
          "Account-SeCoNdArY.CuStOm.BlOb.StOrAgE.AzUrE.NeT"})
    {
      SCOPED_TRACE(hostname);
      const auto serviceUrl = "https://" + hostname;
      const auto absoluteUrl = serviceUrl + "/Container/Blob?versionid=MiXeD";
      const Azure::Core::Url url(absoluteUrl);
      auto result = _internal::ParseStorageUrl(url);

      ASSERT_TRUE(result.HasValue());
      EXPECT_EQ(result.Value().Service, "blob");
      EXPECT_EQ(result.Value().AccountName, "account");
      EXPECT_EQ(result.Value().ServiceUrl, serviceUrl);
      ASSERT_TRUE(result.Value().ContainerName.HasValue());
      EXPECT_EQ(result.Value().ContainerName.Value(), "Container");
      EXPECT_EQ(result.Value().ContainerUrl.Value(), serviceUrl + "/Container");
      EXPECT_EQ(url.GetAbsoluteUrl(), absoluteUrl);
    }
  }

  TEST(StorageUrlTest, RejectsNonStorageEndpoints)
  {
    EXPECT_FALSE(_internal::ParseStorageUrl(Azure::Core::Url("https://example.com")).HasValue());
    EXPECT_FALSE(_internal::ParseStorageUrl(Azure::Core::Url("https://localhost/account/container"))
                     .HasValue());
    EXPECT_FALSE(
        _internal::ParseStorageUrl(Azure::Core::Url("https://storage.contoso.com/container"))
            .HasValue());
  }

  TEST(StorageUrlTest, ParsesSovereignCloudEndpoints)
  {
    auto china = _internal::ParseStorageUrl(
        Azure::Core::Url("https://account.blob.core.chinacloudapi.cn/container/blob"));
    ASSERT_TRUE(china.HasValue());
    EXPECT_EQ(china.Value().Service, "blob");
    EXPECT_EQ(china.Value().AccountName, "account");
    EXPECT_EQ(china.Value().ServiceUrl, "https://account.blob.core.chinacloudapi.cn");

    auto government = _internal::ParseStorageUrl(
        Azure::Core::Url("https://account.file.core.usgovcloudapi.net/share/file"));
    ASSERT_TRUE(government.HasValue());
    EXPECT_EQ(government.Value().Service, "file");
    EXPECT_EQ(government.Value().AccountName, "account");
    EXPECT_EQ(government.Value().ServiceUrl, "https://account.file.core.usgovcloudapi.net");
  }

  TEST(StorageUrlTest, StripsAccountSuffixesRepeatedly)
  {
    auto network = _internal::ParseStorageUrl(
        Azure::Core::Url("https://account-dualstack-secondary.blob.core.windows.net"));

    ASSERT_TRUE(network.HasValue());
    EXPECT_EQ(network.Value().AccountName, "account");

    auto internetRouting = _internal::ParseStorageUrl(
        Azure::Core::Url("https://account-internetrouting-secondary.blob.core.windows.net"));
    ASSERT_TRUE(internetRouting.HasValue());
    EXPECT_EQ(internetRouting.Value().AccountName, "account");

    auto microsoftRouting = _internal::ParseStorageUrl(
        Azure::Core::Url("https://account-microsoftrouting-secondary.dfs.core.windows.net"));
    ASSERT_TRUE(microsoftRouting.HasValue());
    EXPECT_EQ(microsoftRouting.Value().AccountName, "account");
  }

  TEST(StorageUrlTest, ParsesAzureDnsZoneEndpoints)
  {
    auto blob = _internal::ParseStorageUrl(
        Azure::Core::Url("https://account.z1.blob.storage.azure.net/container/blob"));
    ASSERT_TRUE(blob.HasValue());
    EXPECT_EQ(blob.Value().Service, "blob");
    EXPECT_EQ(blob.Value().AccountName, "account");
    EXPECT_EQ(blob.Value().ServiceUrl, "https://account.z1.blob.storage.azure.net");

    auto dfs = _internal::ParseStorageUrl(
        Azure::Core::Url("https://account-secondary.z50.dfs.storage.azure.net/filesystem/file"));
    ASSERT_TRUE(dfs.HasValue());
    EXPECT_EQ(dfs.Value().Service, "dfs");
    EXPECT_EQ(dfs.Value().AccountName, "account");
    EXPECT_EQ(dfs.Value().ServiceUrl, "https://account-secondary.z50.dfs.storage.azure.net");
  }

  TEST(StorageUrlTest, ParsesFlexibleAzureDnsZoneEndpoints)
  {
    auto result = _internal::ParseStorageUrl(
        Azure::Core::Url("https://account-secondary.custom.blob.storage.azure.net/container"));

    ASSERT_TRUE(result.HasValue());
    EXPECT_EQ(result.Value().Service, "blob");
    EXPECT_EQ(result.Value().AccountName, "account");
  }

}}} // namespace Azure::Storage::Test
