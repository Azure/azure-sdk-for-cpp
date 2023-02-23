// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

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
    m_shareServiceClient = std::make_shared<Files::Shares::ShareServiceClient>(
        Files::Shares::ShareServiceClient::CreateFromConnectionString(
            StandardStorageConnectionString(), options));
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
      auto response = m_shareServiceClient->ListShares(options);
      EXPECT_LE(2U, response.Shares.size());
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

  TEST_F(FileShareServiceClientTest, DISABLED_SetPremiumFileProperties)
  {
    auto premiumFileShareServiceClient = std::make_shared<Files::Shares::ShareServiceClient>(
        Files::Shares::ShareServiceClient::CreateFromConnectionString(
            PremiumFileConnectionString()));
    auto properties = premiumFileShareServiceClient->GetProperties().Value;
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
    protocolSettings.Settings.Multichannel.Enabled = true;
    properties.Protocol = protocolSettings;

    EXPECT_NO_THROW(premiumFileShareServiceClient->SetProperties(properties));
    // It takes some time before the new properties comes into effect.
    using namespace std::chrono_literals;
    TestSleep(10s);
    auto downloadedProperties = premiumFileShareServiceClient->GetProperties().Value;

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

    premiumFileShareServiceClient->SetProperties(originalProperties);
  }

}}} // namespace Azure::Storage::Test
