// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "share_service_client_test.hpp"

#include <algorithm>
#include <thread>

namespace Azure { namespace Storage { namespace Test {

  namespace {
    bool NullableEquals(const Azure::Nullable<bool>& lhs, const Azure::Nullable<bool>& rhs)
    {
      return (lhs.HasValue() && rhs.HasValue() && (lhs.Value() == rhs.Value()))
          || (!lhs.HasValue() && !rhs.HasValue());
    }
  } // namespace

  void FileShareServiceClientTest::SetUp()
  {
    StorageTest::SetUp();

    auto options = InitStorageClientOptions<Files::Shares::ShareClientOptions>();
    options.ShareTokenIntent = Files::Shares::Models::ShareTokenIntent::Backup;
    if (m_useTokenCredentialByDefault)
    {
      m_shareServiceClient = std::make_shared<Files::Shares::ShareServiceClient>(
          Files::Shares::ShareServiceClient(GetShareServiceUrl(), GetTestCredential(), options));
    }
    else
    {
      m_shareServiceClient = std::make_shared<Files::Shares::ShareServiceClient>(
          Files::Shares::ShareServiceClient::CreateFromConnectionString(
              StandardStorageConnectionString(), options));
    }

    // Most APIs don't work for premium shares
    {
      std::string premiumConnectionString;
      try
      {
        premiumConnectionString = PremiumFileConnectionString();
      }
      catch (...)
      {
      }
      if (!premiumConnectionString.empty())
      {
        m_premiumShareServiceClient = std::make_shared<Files::Shares::ShareServiceClient>(
            Files::Shares::ShareServiceClient::CreateFromConnectionString(
                premiumConnectionString, options));
      }
    }
  }

  Files::Shares::ShareClient FileShareServiceClientTest::GetPremiumShareClientForTest(
      const std::string& shareName,
      Files::Shares::ShareClientOptions clientOptions)
  {
    InitStorageClientOptions(clientOptions);
    auto shareClient = Files::Shares::ShareClient::CreateFromConnectionString(
        PremiumFileConnectionString(), shareName, clientOptions);
    m_resourceCleanupFunctions.push_back([shareClient]() { shareClient.DeleteIfExists(); });

    return shareClient;
  }

  TEST_F(FileShareServiceClientTest, Constructors_LIVEONLY_)
  {
    auto clientOptions = InitStorageClientOptions<Files::Shares::ShareClientOptions>();
    {
      auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
      auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

      auto keyCredential
          = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

      Sas::AccountSasBuilder accountSasBuilder;
      accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
      accountSasBuilder.StartsOn = sasStartsOn;
      accountSasBuilder.ExpiresOn = sasExpiresOn;
      accountSasBuilder.Services = Sas::AccountSasServices::Files;
      accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;
      accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::Read);
      auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);

      auto serviceClient = Files::Shares::ShareServiceClient(
          m_shareServiceClient->GetUrl() + sasToken, clientOptions);
      EXPECT_NO_THROW(serviceClient.GetProperties());
    }
  }

  TEST_F(FileShareServiceClientTest, ListShares)
  {
    std::string prefix1 = LowercaseRandomString();
    std::string prefix2 = LowercaseRandomString();
    std::set<std::string> shareSet1;
    std::set<std::string> shareSet2;
    for (int i = 0; i < 5; ++i)
    {
      auto shareName = prefix1 + LowercaseRandomString();
      auto shareClient = m_shareServiceClient->GetShareClient(shareName);
      shareClient.Create();
      shareSet1.emplace(shareName);
      shareName = prefix2 + LowercaseRandomString();
      shareClient = m_shareServiceClient->GetShareClient(shareName);
      shareClient.Create();
      shareSet2.emplace(shareName);
    }
    {
      // Normal list without prefix.
      std::set<std::string> result;
      for (auto page = m_shareServiceClient->ListShares(); page.HasPage(); page.MoveToNextPage())
      {
        for (const auto& share : page.Shares)
        {
          result.insert(share.Name);
        }
      }
      for (const auto& name : shareSet1)
      {
        EXPECT_NE(result.find(name), result.end());
      }
      for (const auto& name : shareSet2)
      {
        EXPECT_NE(result.find(name), result.end());
      }
    }
    {
      // List prefix.
      std::set<std::string> result;
      Files::Shares::ListSharesOptions options;
      options.Prefix = prefix1;
      for (auto page = m_shareServiceClient->ListShares(options); page.HasPage();
           page.MoveToNextPage())
      {
        for (const auto& share : page.Shares)
        {
          result.insert(share.Name);
        }
      }
      for (const auto& name : shareSet1)
      {
        EXPECT_NE(result.find(name), result.end());
      }
      for (const auto& name : shareSet2)
      {
        EXPECT_EQ(result.find(name), result.end());
      }
    }
    {
      // List max result
      Files::Shares::ListSharesOptions options;
      options.PageSizeHint = 2;
      int numPages = 0;
      for (auto page = m_shareServiceClient->ListShares(options); page.HasPage();
           page.MoveToNextPage())
      {
        EXPECT_LE(page.Shares.size(), 2U);
        ++numPages;
      }
      EXPECT_GT(numPages, 2);
    }
    for (const auto& shareName : shareSet1)
    {
      m_shareServiceClient->GetShareClient(shareName).DeleteIfExists();
    }
    for (const auto& shareName : shareSet2)
    {
      m_shareServiceClient->GetShareClient(shareName).DeleteIfExists();
    }
  }

  TEST_F(FileShareServiceClientTest, ListSharesEnableSnapshotVirtualDirectoryAccess_PLAYBACKONLY_)
  {
    auto premiumFileShareServiceClient = *m_premiumShareServiceClient;
    std::string shareName1 = LowercaseRandomString();
    std::string shareName2 = LowercaseRandomString();
    auto shareClient1 = GetPremiumShareClientForTest(shareName1);
    auto shareClient2 = GetPremiumShareClientForTest(shareName2);
    Files::Shares::CreateShareOptions createOptions;
    createOptions.EnabledProtocols = Files::Shares::Models::ShareProtocols::Nfs;
    shareClient1.Create(createOptions);
    shareClient2.Create(createOptions);

    Files::Shares::SetSharePropertiesOptions setPropertiesOptions;
    setPropertiesOptions.EnableSnapshotVirtualDirectoryAccess = true;
    shareClient1.SetProperties(setPropertiesOptions);
    setPropertiesOptions.EnableSnapshotVirtualDirectoryAccess = false;
    shareClient2.SetProperties(setPropertiesOptions);

    Azure::Nullable<Files::Shares::Models::ShareItem> share1;
    Azure::Nullable<Files::Shares::Models::ShareItem> share2;
    for (auto page = premiumFileShareServiceClient.ListShares(); page.HasPage();
         page.MoveToNextPage())
    {
      for (const auto& share : page.Shares)
      {
        if (share.Name == shareName1)
        {
          share1 = share;
        }
        else if (share.Name == shareName2)
        {
          share2 = share;
        }
      }
    }
    ASSERT_TRUE(share1.HasValue() && share2.HasValue());
    EXPECT_TRUE(
        share1.Value().Details.EnableSnapshotVirtualDirectoryAccess.HasValue()
        && share1.Value().Details.EnableSnapshotVirtualDirectoryAccess.Value());
    EXPECT_TRUE(
        share2.Value().Details.EnableSnapshotVirtualDirectoryAccess.HasValue()
        && !share2.Value().Details.EnableSnapshotVirtualDirectoryAccess.Value());
  }

  TEST_F(FileShareServiceClientTest, GetProperties)
  {
    auto ret = m_shareServiceClient->GetProperties();
    auto properties = ret.Value;
    auto hourMetrics = properties.HourMetrics;
    if (hourMetrics.Enabled)
    {
      EXPECT_FALSE(hourMetrics.Version.empty());
    }
    auto minuteMetrics = properties.HourMetrics;
    if (minuteMetrics.Enabled)
    {
      EXPECT_FALSE(minuteMetrics.Version.empty());
    }
  }

  TEST_F(FileShareServiceClientTest, SetProperties)
  {
    auto properties = m_shareServiceClient->GetProperties().Value;
    properties.Protocol.Reset();
    auto originalProperties = properties;

    properties.HourMetrics.Enabled = true;
    properties.HourMetrics.RetentionPolicy.Enabled = true;
    properties.HourMetrics.RetentionPolicy.Days = 4;
    properties.HourMetrics.IncludeApis = true;

    properties.MinuteMetrics.Enabled = true;
    properties.MinuteMetrics.RetentionPolicy.Enabled = true;
    properties.MinuteMetrics.RetentionPolicy.Days = 3;
    properties.MinuteMetrics.IncludeApis = true;

    Files::Shares::Models::CorsRule corsRule;
    corsRule.AllowedOrigins = "http://www.example1.com";
    corsRule.AllowedMethods = "GET,PUT";
    corsRule.AllowedHeaders = "x-ms-header1,x-ms-header2";
    corsRule.ExposedHeaders = "x-ms-header3";
    corsRule.MaxAgeInSeconds = 10;
    properties.Cors.emplace_back(corsRule);

    corsRule.AllowedOrigins = "http://www.example2.com";
    corsRule.AllowedMethods = "DELETE";
    corsRule.AllowedHeaders = "x-ms-header1";
    corsRule.ExposedHeaders = "x-ms-header2,x-ms-header3";
    corsRule.MaxAgeInSeconds = 20;
    properties.Cors.emplace_back(corsRule);

    EXPECT_NO_THROW(m_shareServiceClient->SetProperties(properties));
    // It takes some time before the new properties comes into effect.
    using namespace std::chrono_literals;
    TestSleep(10s);
    auto downloadedProperties = m_shareServiceClient->GetProperties().Value;

    EXPECT_EQ(downloadedProperties.HourMetrics.Version, properties.HourMetrics.Version);
    EXPECT_EQ(downloadedProperties.HourMetrics.Enabled, properties.HourMetrics.Enabled);
    EXPECT_TRUE(NullableEquals(
        downloadedProperties.HourMetrics.IncludeApis, properties.HourMetrics.IncludeApis));
    EXPECT_EQ(
        downloadedProperties.HourMetrics.RetentionPolicy.Enabled,
        properties.HourMetrics.RetentionPolicy.Enabled);
    EXPECT_EQ(
        downloadedProperties.HourMetrics.RetentionPolicy.Days.HasValue(),
        properties.HourMetrics.RetentionPolicy.Days.HasValue());
    if (properties.HourMetrics.RetentionPolicy.Days.HasValue())
    {
      EXPECT_EQ(
          downloadedProperties.HourMetrics.RetentionPolicy.Days.Value(),
          properties.HourMetrics.RetentionPolicy.Days.Value());
    }

    EXPECT_EQ(downloadedProperties.MinuteMetrics.Version, properties.MinuteMetrics.Version);
    EXPECT_EQ(downloadedProperties.MinuteMetrics.Enabled, properties.MinuteMetrics.Enabled);
    EXPECT_TRUE(NullableEquals(
        downloadedProperties.HourMetrics.IncludeApis, properties.HourMetrics.IncludeApis));
    EXPECT_EQ(
        downloadedProperties.MinuteMetrics.RetentionPolicy.Enabled,
        properties.MinuteMetrics.RetentionPolicy.Enabled);
    EXPECT_EQ(
        downloadedProperties.MinuteMetrics.RetentionPolicy.Days.HasValue(),
        properties.MinuteMetrics.RetentionPolicy.Days.HasValue());
    if (properties.MinuteMetrics.RetentionPolicy.Days.HasValue())
    {
      EXPECT_EQ(
          downloadedProperties.MinuteMetrics.RetentionPolicy.Days.Value(),
          properties.MinuteMetrics.RetentionPolicy.Days.Value());
    }

    EXPECT_EQ(downloadedProperties.Cors.size(), properties.Cors.size());
    for (const auto& cors : downloadedProperties.Cors)
    {
      auto iter = std::find_if(
          properties.Cors.begin(),
          properties.Cors.end(),
          [&cors](const Files::Shares::Models::CorsRule& rule) {
            return rule.AllowedOrigins == cors.AllowedOrigins;
          });
      EXPECT_EQ(iter->AllowedMethods, cors.AllowedMethods);
      EXPECT_EQ(iter->AllowedHeaders, cors.AllowedHeaders);
      EXPECT_EQ(iter->ExposedHeaders, cors.ExposedHeaders);
      EXPECT_EQ(iter->MaxAgeInSeconds, cors.MaxAgeInSeconds);
      EXPECT_NE(properties.Cors.end(), iter);
    }

    m_shareServiceClient->SetProperties(originalProperties);
  }

  TEST_F(FileShareServiceClientTest, SetPremiumFileProperties_LIVEONLY_)
  {
    auto premiumFileShareServiceClient = *m_premiumShareServiceClient;
    auto properties = premiumFileShareServiceClient.GetProperties().Value;
    auto originalProperties = properties;

    properties.HourMetrics.Enabled = true;
    properties.HourMetrics.RetentionPolicy.Enabled = true;
    properties.HourMetrics.RetentionPolicy.Days = 4;
    properties.HourMetrics.IncludeApis = true;

    properties.MinuteMetrics.Enabled = true;
    properties.MinuteMetrics.RetentionPolicy.Enabled = true;
    properties.MinuteMetrics.RetentionPolicy.Days = 3;
    properties.MinuteMetrics.IncludeApis = true;

    Files::Shares::Models::CorsRule corsRule;
    corsRule.AllowedOrigins = "http://www.example1.com";
    corsRule.AllowedMethods = "GET,PUT";
    corsRule.AllowedHeaders = "x-ms-header1,x-ms-header2";
    corsRule.ExposedHeaders = "x-ms-header3";
    corsRule.MaxAgeInSeconds = 10;
    properties.Cors.emplace_back(corsRule);

    corsRule.AllowedOrigins = "http://www.example2.com";
    corsRule.AllowedMethods = "DELETE";
    corsRule.AllowedHeaders = "x-ms-header1";
    corsRule.ExposedHeaders = "x-ms-header2,x-ms-header3";
    corsRule.MaxAgeInSeconds = 20;
    properties.Cors.emplace_back(corsRule);

    auto protocolSettings = Files::Shares::Models::ProtocolSettings();
    protocolSettings.Settings = Files::Shares::Models::SmbSettings();
    protocolSettings.Settings.Multichannel.Enabled = true;
    properties.Protocol = protocolSettings;

    EXPECT_NO_THROW(premiumFileShareServiceClient.SetProperties(properties));
    // It takes some time before the new properties comes into effect.
    using namespace std::chrono_literals;
    TestSleep(10s);
    auto downloadedProperties = premiumFileShareServiceClient.GetProperties().Value;

    EXPECT_EQ(downloadedProperties.HourMetrics.Version, properties.HourMetrics.Version);
    EXPECT_EQ(downloadedProperties.HourMetrics.Enabled, properties.HourMetrics.Enabled);
    EXPECT_TRUE(NullableEquals(
        downloadedProperties.HourMetrics.IncludeApis, properties.HourMetrics.IncludeApis));
    EXPECT_EQ(
        downloadedProperties.HourMetrics.RetentionPolicy.Enabled,
        properties.HourMetrics.RetentionPolicy.Enabled);
    EXPECT_EQ(
        downloadedProperties.HourMetrics.RetentionPolicy.Days.HasValue(),
        properties.HourMetrics.RetentionPolicy.Days.HasValue());
    if (properties.HourMetrics.RetentionPolicy.Days.HasValue())
    {
      EXPECT_EQ(
          downloadedProperties.HourMetrics.RetentionPolicy.Days.Value(),
          properties.HourMetrics.RetentionPolicy.Days.Value());
    }

    EXPECT_EQ(downloadedProperties.MinuteMetrics.Version, properties.MinuteMetrics.Version);
    EXPECT_EQ(downloadedProperties.MinuteMetrics.Enabled, properties.MinuteMetrics.Enabled);
    EXPECT_TRUE(NullableEquals(
        downloadedProperties.HourMetrics.IncludeApis, properties.HourMetrics.IncludeApis));
    EXPECT_EQ(
        downloadedProperties.MinuteMetrics.RetentionPolicy.Enabled,
        properties.MinuteMetrics.RetentionPolicy.Enabled);
    EXPECT_EQ(
        downloadedProperties.MinuteMetrics.RetentionPolicy.Days.HasValue(),
        properties.MinuteMetrics.RetentionPolicy.Days.HasValue());
    if (properties.MinuteMetrics.RetentionPolicy.Days.HasValue())
    {
      EXPECT_EQ(
          downloadedProperties.MinuteMetrics.RetentionPolicy.Days.Value(),
          properties.MinuteMetrics.RetentionPolicy.Days.Value());
    }

    EXPECT_EQ(downloadedProperties.Cors.size(), properties.Cors.size());
    for (const auto& cors : downloadedProperties.Cors)
    {
      auto iter = std::find_if(
          properties.Cors.begin(),
          properties.Cors.end(),
          [&cors](const Files::Shares::Models::CorsRule& rule) {
            return rule.AllowedOrigins == cors.AllowedOrigins;
          });
      EXPECT_EQ(iter->AllowedMethods, cors.AllowedMethods);
      EXPECT_EQ(iter->AllowedHeaders, cors.AllowedHeaders);
      EXPECT_EQ(iter->ExposedHeaders, cors.ExposedHeaders);
      EXPECT_EQ(iter->MaxAgeInSeconds, cors.MaxAgeInSeconds);
      EXPECT_NE(properties.Cors.end(), iter);
    }

    EXPECT_EQ(true, properties.Protocol.Value().Settings.Multichannel.Enabled);

    premiumFileShareServiceClient.SetProperties(originalProperties);
  }

  TEST_F(FileShareServiceClientTest, OAuth_PLAYBACKONLY_)
  {
    auto credential = GetTestCredential();
    auto options = InitStorageClientOptions<Files::Shares::ShareClientOptions>();
    options.ShareTokenIntent = Files::Shares::Models::ShareTokenIntent::Backup;

    auto shareServiceClient
        = Files::Shares::ShareServiceClient(m_shareServiceClient->GetUrl(), credential, options);
    // Get Properties
    Files::Shares::Models::ShareServiceProperties properties;
    EXPECT_NO_THROW(properties = shareServiceClient.GetProperties().Value);

    // Set Properties
    properties.Protocol.Reset();
    EXPECT_NO_THROW(shareServiceClient.SetProperties(properties));

    // List Shares
    EXPECT_NO_THROW(shareServiceClient.ListShares());
  }

  TEST_F(FileShareServiceClientTest, PremiumSharePaidBurst_PLAYBACKONLY_)
  {
    auto shareServiceClient = *m_premiumShareServiceClient;
    auto shareName = LowercaseRandomString();
    auto shareClient = shareServiceClient.GetShareClient(shareName);

    // Create
    Files::Shares::CreateShareOptions createOptions;
    createOptions.EnablePaidBursting = true;
    createOptions.PaidBurstingMaxIops = 1000;
    createOptions.PaidBurstingMaxBandwidthMibps = 5000;
    shareClient.Create(createOptions);

    // Get Properties
    auto properties = shareClient.GetProperties().Value;
    EXPECT_TRUE(properties.PaidBurstingEnabled.HasValue());
    EXPECT_TRUE(properties.PaidBurstingEnabled.Value());
    EXPECT_TRUE(properties.PaidBurstingMaxIops.HasValue());
    EXPECT_EQ(properties.PaidBurstingMaxIops.Value(), 1000);
    EXPECT_TRUE(properties.PaidBurstingMaxBandwidthMibps.HasValue());
    EXPECT_EQ(properties.PaidBurstingMaxBandwidthMibps.Value(), 5000);

    // Set Properties
    Files::Shares::SetSharePropertiesOptions setPropertiesOptions;
    setPropertiesOptions.EnablePaidBursting = true;
    setPropertiesOptions.PaidBurstingMaxIops = 500;
    setPropertiesOptions.PaidBurstingMaxBandwidthMibps = 1000;
    shareClient.SetProperties(setPropertiesOptions);

    // List Shares
    Azure::Nullable<Files::Shares::Models::ShareItem> shareItem;
    for (auto page = shareServiceClient.ListShares(); page.HasPage(); page.MoveToNextPage())
    {
      for (const auto& share : page.Shares)
      {
        if (share.Name == shareName)
        {
          shareItem = share;
        }
      }
    }
    ASSERT_TRUE(shareItem.HasValue());
    EXPECT_TRUE(shareItem.Value().Details.PaidBurstingEnabled.HasValue());
    EXPECT_TRUE(shareItem.Value().Details.PaidBurstingEnabled.Value());
    EXPECT_TRUE(shareItem.Value().Details.PaidBurstingMaxIops.HasValue());
    EXPECT_EQ(shareItem.Value().Details.PaidBurstingMaxIops.Value(), 500);
    EXPECT_TRUE(shareItem.Value().Details.PaidBurstingMaxBandwidthMibps.HasValue());
    EXPECT_EQ(shareItem.Value().Details.PaidBurstingMaxBandwidthMibps.Value(), 1000);

    // Set Properties EnablePaidBursting = false
    setPropertiesOptions.EnablePaidBursting = false;
    setPropertiesOptions.PaidBurstingMaxIops.Reset();
    setPropertiesOptions.PaidBurstingMaxBandwidthMibps.Reset();
    shareClient.SetProperties(setPropertiesOptions);
    properties = shareClient.GetProperties().Value;
    EXPECT_TRUE(properties.PaidBurstingEnabled.HasValue());
    EXPECT_FALSE(properties.PaidBurstingEnabled.Value());

    shareClient.DeleteIfExists();
  }

  TEST_F(FileShareServiceClientTest, ListShares_ProvisionedBilling_PLAYBACKONLY_)
  {
    auto shareServiceClient = *m_shareServiceClient;
    auto shareName = LowercaseRandomString();
    auto shareClient = shareServiceClient.GetShareClient(shareName);

    // Create
    shareClient.Create();

    // List Shares
    Azure::Nullable<Files::Shares::Models::ShareItem> shareItem;
    for (auto page = shareServiceClient.ListShares(); page.HasPage(); page.MoveToNextPage())
    {
      for (const auto& share : page.Shares)
      {
        if (share.Name == shareName)
        {
          shareItem = share;
        }
      }
    }
    ASSERT_TRUE(shareItem.HasValue());
    EXPECT_TRUE(shareItem.Value().Details.IncludedBurstIops.HasValue());
    EXPECT_TRUE(shareItem.Value().Details.MaxBurstCreditsForIops.HasValue());
    EXPECT_TRUE(shareItem.Value().Details.NextAllowedProvisionedIopsDowngradeTime.HasValue());
    EXPECT_TRUE(shareItem.Value().Details.NextAllowedProvisionedBandwidthDowngradeTime.HasValue());

    EXPECT_NO_THROW(shareClient.Delete());
  }

  TEST_F(FileShareServiceClientTest, EncryptionInTransit)
  {
    {
      auto shareServiceClient = *m_shareServiceClient;

      auto properties = shareServiceClient.GetProperties().Value;
      properties.Protocol = Files::Shares::Models::ProtocolSettings();
      properties.Protocol.Value().Settings = Files::Shares::Models::SmbSettings();
      properties.Protocol.Value().SmbSettings = Files::Shares::Models::NewSmbSettings();
      properties.Protocol.Value().SmbSettings.Value().EncryptionInTransit
          = Files::Shares::Models::SmbEncryptionInTransit();
      properties.Protocol.Value().SmbSettings.Value().EncryptionInTransit.Value().Required = true;

      EXPECT_NO_THROW(shareServiceClient.SetProperties(properties));

      // Get Properties
      properties = shareServiceClient.GetProperties().Value;
      EXPECT_TRUE(properties.Protocol.HasValue());
      EXPECT_TRUE(properties.Protocol.Value().SmbSettings.HasValue());
      EXPECT_TRUE(properties.Protocol.Value().SmbSettings.Value().EncryptionInTransit.HasValue());
      EXPECT_TRUE(
          properties.Protocol.Value().SmbSettings.Value().EncryptionInTransit.Value().Required);
    }
    {
      auto premiumShareServiceClient = *m_premiumShareServiceClient;

      auto properties = premiumShareServiceClient.GetProperties().Value;
      properties.Protocol = Files::Shares::Models::ProtocolSettings();
      properties.Protocol.Value().NfsSettings = Files::Shares::Models::NfsSettings();
      properties.Protocol.Value().NfsSettings.Value().EncryptionInTransit
          = Files::Shares::Models::NfsEncryptionInTransit();
      properties.Protocol.Value().NfsSettings.Value().EncryptionInTransit.Value().Required = true;
      EXPECT_NO_THROW(premiumShareServiceClient.SetProperties(properties));

      // Get Properties
      properties = premiumShareServiceClient.GetProperties().Value;
      EXPECT_TRUE(properties.Protocol.HasValue());
      EXPECT_TRUE(properties.Protocol.Value().NfsSettings.HasValue());
      EXPECT_TRUE(properties.Protocol.Value().NfsSettings.Value().EncryptionInTransit.HasValue());
      EXPECT_TRUE(
          properties.Protocol.Value().NfsSettings.Value().EncryptionInTransit.Value().Required);
    }
  }

  TEST_F(FileShareServiceClientTest, PremiumMultiChannelNewSchema)
  {

    auto premiumShareServiceClient = *m_premiumShareServiceClient;

    auto properties = premiumShareServiceClient.GetProperties().Value;
    properties.Protocol = Files::Shares::Models::ProtocolSettings();
    properties.Protocol.Value().SmbSettings = Files::Shares::Models::NewSmbSettings();
    properties.Protocol.Value().SmbSettings.Value().Multichannel
        = Files::Shares::Models::SmbMultichannel();
    properties.Protocol.Value().SmbSettings.Value().Multichannel.Value().Enabled = true;
    EXPECT_NO_THROW(premiumShareServiceClient.SetProperties(properties));

    // Get Properties
    properties = premiumShareServiceClient.GetProperties().Value;
    EXPECT_TRUE(properties.Protocol.HasValue());
    EXPECT_TRUE(properties.Protocol.Value().SmbSettings.HasValue());
    EXPECT_TRUE(properties.Protocol.Value().SmbSettings.Value().Multichannel.HasValue());
    EXPECT_TRUE(properties.Protocol.Value().SmbSettings.Value().Multichannel.Value().Enabled);
    EXPECT_TRUE(properties.Protocol.Value().Settings.Multichannel.Enabled);
  }
}}} // namespace Azure::Storage::Test
