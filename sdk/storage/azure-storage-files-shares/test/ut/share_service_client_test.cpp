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

  const size_t ShareTestSize = 5;

  void FileShareServiceClientTest::SetUp()
  {
    StorageTest::SetUp();
    CHECK_SKIP_TEST();

    m_options = InitClientOptions<Files::Shares::ShareClientOptions>();
    m_fileShareServiceClient = std::make_shared<Files::Shares::ShareServiceClient>(
        Files::Shares::ShareServiceClient::CreateFromConnectionString(
            StandardStorageConnectionString(), m_options));
    m_testName = GetTestName();
    m_testNameLowercase = GetTestNameLowerCase();
    m_sharePrefixA = m_testNameLowercase + "a";
    m_sharePrefixB = m_testNameLowercase + "b";
    m_shareNameSetA.clear();
    m_shareNameSetB.clear();
    for (size_t i = 0; i < ShareTestSize; ++i)
    {
      {
        auto name = m_sharePrefixA + "a" + std::to_string(i);
        m_fileShareServiceClient->GetShareClient(name).Create();
        m_shareNameSetA.emplace_back(std::move(name));
      }
      {
        auto name = m_sharePrefixB + "b" + std::to_string(i);
        m_fileShareServiceClient->GetShareClient(name).Create();
        m_shareNameSetB.emplace_back(std::move(name));
      }
    }
  }

  void FileShareServiceClientTest::TearDown()
  {
    CHECK_SKIP_TEST();
    for (const auto& name : m_shareNameSetA)
    {
      m_fileShareServiceClient->GetShareClient(name).Delete();
    }
    for (const auto& name : m_shareNameSetB)
    {
      m_fileShareServiceClient->GetShareClient(name).Delete();
    }
    StorageTest::TearDown();
  }

  std::vector<Files::Shares::Models::ShareItem> FileShareServiceClientTest::ListAllShares(
      const std::string& prefix)
  {
    std::vector<Files::Shares::Models::ShareItem> result;
    Files::Shares::ListSharesOptions options;
    if (!prefix.empty())
    {
      options.Prefix = prefix;
    }
    for (auto pageResult = m_fileShareServiceClient->ListShares(options); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
      result.insert(result.end(), pageResult.Shares.begin(), pageResult.Shares.end());
    }
    return result;
  }

  TEST_F(FileShareServiceClientTest, ListShares)
  {
    {
      // Normal list without prefix.
      auto result = ListAllShares();
      for (const auto& name : m_shareNameSetA)
      {
        auto iter = std::find_if(
            result.begin(), result.end(), [&name](const Files::Shares::Models::ShareItem& share) {
              return share.Name == name;
            });
        EXPECT_EQ(iter->Name.substr(0U, m_sharePrefixA.size()), m_sharePrefixA);
        EXPECT_NE(result.end(), iter);
      }
      for (const auto& name : m_shareNameSetB)
      {
        auto iter = std::find_if(
            result.begin(), result.end(), [&name](const Files::Shares::Models::ShareItem& share) {
              return share.Name == name;
            });
        EXPECT_EQ(iter->Name.substr(0U, m_sharePrefixB.size()), m_sharePrefixB);
        EXPECT_NE(result.end(), iter);
      }
    }
    {
      // List prefix.
      auto result = ListAllShares(m_sharePrefixA);
      for (const auto& name : m_shareNameSetA)
      {
        auto iter = std::find_if(
            result.begin(), result.end(), [&name](const Files::Shares::Models::ShareItem& share) {
              return share.Name == name;
            });
        EXPECT_EQ(iter->Name.substr(0U, m_sharePrefixA.size()), m_sharePrefixA);
        EXPECT_NE(result.end(), iter);
      }
      for (const auto& name : m_shareNameSetB)
      {
        auto iter = std::find_if(
            result.begin(), result.end(), [&name](const Files::Shares::Models::ShareItem& share) {
              return share.Name == name;
            });
        EXPECT_EQ(result.end(), iter);
      }
    }
    {
      // List max result
      Files::Shares::ListSharesOptions options;
      options.PageSizeHint = 2;
      auto response = m_fileShareServiceClient->ListShares(options);
      EXPECT_LE(2U, response.Shares.size());
    }
  }

  TEST_F(FileShareServiceClientTest, GetProperties)
  {
    auto ret = m_fileShareServiceClient->GetProperties();
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
    auto properties = m_fileShareServiceClient->GetProperties().Value;
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

    EXPECT_NO_THROW(m_fileShareServiceClient->SetProperties(properties));
    // It takes some time before the new properties comes into effect.
    using namespace std::chrono_literals;
    TestSleep(10s);
    auto downloadedProperties = m_fileShareServiceClient->GetProperties().Value;

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

    m_fileShareServiceClient->SetProperties(originalProperties);
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
