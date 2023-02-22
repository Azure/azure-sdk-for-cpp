// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake_service_client_test.hpp"

#include <algorithm>
#include <chrono>

namespace Azure { namespace Storage { namespace Blobs { namespace Models {

  bool operator==(const RetentionPolicy& lhs, const RetentionPolicy& rhs)
  {
    if (lhs.IsEnabled != rhs.IsEnabled)
    {
      return false;
    }
    if (lhs.Days.HasValue() != rhs.Days.HasValue())
    {
      return false;
    }
    if (lhs.Days.HasValue() && rhs.Days.HasValue() && lhs.Days.Value() != rhs.Days.Value())
    {
      return false;
    }
    return true;
  }

  bool operator==(const CorsRule& lhs, const CorsRule& rhs)
  {
    return lhs.AllowedHeaders == rhs.AllowedHeaders && lhs.AllowedMethods == rhs.AllowedMethods
        && lhs.AllowedOrigins == rhs.AllowedOrigins && lhs.ExposedHeaders == rhs.ExposedHeaders
        && lhs.MaxAgeInSeconds == rhs.MaxAgeInSeconds;
  }

  bool operator==(const StaticWebsite& lhs, const StaticWebsite& rhs)
  {
    if (lhs.IsEnabled != rhs.IsEnabled)
    {
      return false;
    }
    if (lhs.DefaultIndexDocumentPath.HasValue() != rhs.DefaultIndexDocumentPath.HasValue())
    {
      return false;
    }
    if (lhs.DefaultIndexDocumentPath.HasValue() && rhs.DefaultIndexDocumentPath.HasValue()
        && lhs.DefaultIndexDocumentPath.Value() != rhs.DefaultIndexDocumentPath.Value())
    {
      return false;
    }
    if (lhs.DefaultIndexDocumentPath.HasValue() != rhs.DefaultIndexDocumentPath.HasValue())
    {
      return false;
    }
    if (lhs.ErrorDocument404Path.HasValue() && rhs.ErrorDocument404Path.HasValue()
        && lhs.ErrorDocument404Path.Value() != rhs.ErrorDocument404Path.Value())
    {
      return false;
    }
    if (lhs.IndexDocument.HasValue() && rhs.IndexDocument.HasValue()
        && lhs.IndexDocument.Value() != rhs.IndexDocument.Value())
    {
      return false;
    }
    return true;
  }

}}}} // namespace Azure::Storage::Blobs::Models

namespace Azure { namespace Storage { namespace Test {

  TEST_F(DataLakeServiceClientTest, ListFileSystemsSegment)
  {
    const std::string prefix1 = LowercaseRandomString();
    const std::string prefix2 = LowercaseRandomString();
    std::set<std::string> filesystemSet1;
    std::set<std::string> filesystemSet2;
    for (int i = 0; i < 5; ++i)
    {
      std::string fsName = prefix1 + LowercaseRandomString();

      m_dataLakeServiceClient->GetFileSystemClient(fsName).Create();
      filesystemSet1.insert(fsName);
      fsName = prefix2 + LowercaseRandomString();
      m_dataLakeServiceClient->GetFileSystemClient(fsName).Create();
      filesystemSet2.insert(fsName);
    }
    {
      // Normal list without prefix.
      std::set<std::string> result;
      for (auto page = m_dataLakeServiceClient->ListFileSystems(); page.HasPage();
           page.MoveToNextPage())
      {
        for (auto fs : page.FileSystems)
        {
          result.insert(fs.Name);
        }
      }
      for (const auto& f : filesystemSet1)
      {
        EXPECT_NE(result.find(f), result.end());
      }
      for (const auto& f : filesystemSet2)
      {
        EXPECT_NE(result.find(f), result.end());
      }
    }
    {
      std::set<std::string> result;
      Files::DataLake::ListFileSystemsOptions listOptions;
      listOptions.Prefix = prefix1;
      for (auto page = m_dataLakeServiceClient->ListFileSystems(listOptions); page.HasPage();
           page.MoveToNextPage())
      {
        for (auto fs : page.FileSystems)
        {
          result.insert(fs.Name);
        }
      }
      for (const auto& f : filesystemSet1)
      {
        EXPECT_NE(result.find(f), result.end());
      }
    }
    {
      // List max result
      Files::DataLake::ListFileSystemsOptions options;
      options.PageSizeHint = 2;
      auto response = m_dataLakeServiceClient->ListFileSystems(options);
      EXPECT_LE(2U, response.FileSystems.size());
    }
    for (const auto& fsName : filesystemSet1)
    {
      m_dataLakeServiceClient->GetFileSystemClient(fsName).DeleteIfExists();
    }
    for (const auto& fsName : filesystemSet2)
    {
      m_dataLakeServiceClient->GetFileSystemClient(fsName).DeleteIfExists();
    }
  }

  TEST_F(DataLakeServiceClientTest, DISABLED_ListSystemFileSystems)
  {
    // Disabled temporarily because the test account on the pipeline hasn't system fileSystems.
    // List system type FileSystems
    Files::DataLake::ListFileSystemsOptions options;
    options.Include = Files::DataLake::Models::ListFileSystemsIncludeFlags::System;
    std::vector<std::string> fileSystems;
    for (auto pageResult = m_dataLakeServiceClient->ListFileSystems(options); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
      for (const auto& c : pageResult.FileSystems)
      {
        if (c.Name[0] == '$')
        {
          fileSystems.push_back(c.Name);
        }
      }
    }
    EXPECT_FALSE(fileSystems.empty());
  }

  TEST_F(DataLakeServiceClientTest, AnonymousConstructorsWorks)
  {
    auto keyCredential
        = Azure::Storage::_internal::ParseConnectionString(AdlsGen2ConnectionString())
              .KeyCredential;
    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.StartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    accountSasBuilder.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);
    accountSasBuilder.Services = Sas::AccountSasServices::Blobs;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;
    accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::All);
    auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);

    // Create from Anonymous credential.
    auto datalakeServiceUrl
        = Azure::Storage::Files::DataLake::DataLakeServiceClient::CreateFromConnectionString(
              AdlsGen2ConnectionString())
              .GetUrl();
    auto datalakeServiceClient = Azure::Storage::Files::DataLake::DataLakeServiceClient(
        datalakeServiceUrl + sasToken,
        InitClientOptions<Azure::Storage::Files::DataLake::DataLakeClientOptions>());
    EXPECT_NO_THROW(datalakeServiceClient.ListFileSystems());
  }

  TEST_F(DataLakeServiceClientTest, GetProperties)
  {

    auto ret = m_dataLakeServiceClient->GetProperties();
    auto properties = ret.Value;
    auto logging = properties.Logging;
    EXPECT_FALSE(logging.Version.empty());
    if (logging.RetentionPolicy.IsEnabled)
    {
      EXPECT_TRUE(logging.RetentionPolicy.Days.HasValue());
    }
    auto hourMetrics = properties.HourMetrics;
    if (hourMetrics.IsEnabled)
    {
      EXPECT_FALSE(hourMetrics.Version.empty());
      if (hourMetrics.RetentionPolicy.IsEnabled)
      {
        EXPECT_TRUE(hourMetrics.RetentionPolicy.Days.HasValue());
      }
    }
    auto minuteMetrics = properties.HourMetrics;
    if (minuteMetrics.IsEnabled)
    {
      EXPECT_FALSE(minuteMetrics.Version.empty());
      if (minuteMetrics.RetentionPolicy.IsEnabled)
      {
        EXPECT_TRUE(minuteMetrics.RetentionPolicy.Days.HasValue());
      }
    }
    auto deleteRetentionPolicy = properties.DeleteRetentionPolicy;
    if (deleteRetentionPolicy.IsEnabled)
    {
      EXPECT_TRUE(deleteRetentionPolicy.Days.HasValue());
    }
  }

  TEST_F(DataLakeServiceClientTest, SetProperties)
  {

    auto getServicePropertiesResult = m_dataLakeServiceClient->GetProperties().Value;
    Files::DataLake::Models::DataLakeServiceProperties properties;
    properties.Logging = getServicePropertiesResult.Logging;
    properties.HourMetrics = getServicePropertiesResult.HourMetrics;
    properties.MinuteMetrics = getServicePropertiesResult.MinuteMetrics;
    properties.Cors = getServicePropertiesResult.Cors;
    properties.DefaultServiceVersion = getServicePropertiesResult.DefaultServiceVersion;
    properties.DeleteRetentionPolicy = getServicePropertiesResult.DeleteRetentionPolicy;
    properties.StaticWebsite = getServicePropertiesResult.StaticWebsite;

    auto originalProperties = properties;

    properties.Logging.Delete = !properties.Logging.Delete;
    properties.Logging.Read = !properties.Logging.Read;
    properties.Logging.Write = !properties.Logging.Write;
    properties.Logging.RetentionPolicy.IsEnabled = true;
    properties.Logging.RetentionPolicy.Days = 3;

    properties.HourMetrics.IsEnabled = true;
    properties.HourMetrics.RetentionPolicy.IsEnabled = true;
    properties.HourMetrics.RetentionPolicy.Days = 4;
    properties.HourMetrics.IncludeApis = true;

    properties.MinuteMetrics.IsEnabled = true;
    properties.MinuteMetrics.RetentionPolicy.IsEnabled = true;
    properties.MinuteMetrics.RetentionPolicy.Days = 4;
    properties.MinuteMetrics.IncludeApis = true;

    properties.DefaultServiceVersion = Files::DataLake::_detail::ApiVersion;

    properties.StaticWebsite.IsEnabled = true;
    properties.StaticWebsite.IndexDocument = "index.html";
    properties.StaticWebsite.ErrorDocument404Path = "404.html";
    properties.StaticWebsite.DefaultIndexDocumentPath.Reset();

    Files::DataLake::Models::CorsRule corsRule;
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

    properties.DeleteRetentionPolicy.IsEnabled = true;
    properties.DeleteRetentionPolicy.Days = 7;

    EXPECT_NO_THROW(m_dataLakeServiceClient->SetProperties(properties));

    // It takes some time before the new properties comes into effect.
    using namespace std::chrono_literals;
    TestSleep(10s);
    auto downloadedProperties = m_dataLakeServiceClient->GetProperties().Value;
    EXPECT_EQ(downloadedProperties.Logging.Version, properties.Logging.Version);
    EXPECT_EQ(downloadedProperties.Logging.Delete, properties.Logging.Delete);
    EXPECT_EQ(downloadedProperties.Logging.Read, properties.Logging.Read);
    EXPECT_EQ(downloadedProperties.Logging.Write, properties.Logging.Write);
    EXPECT_EQ(downloadedProperties.Logging.RetentionPolicy, properties.Logging.RetentionPolicy);

    EXPECT_EQ(downloadedProperties.HourMetrics.Version, properties.HourMetrics.Version);
    EXPECT_EQ(downloadedProperties.HourMetrics.IsEnabled, properties.HourMetrics.IsEnabled);
    EXPECT_EQ(
        downloadedProperties.HourMetrics.IncludeApis.HasValue(),
        properties.HourMetrics.IncludeApis.HasValue());
    if (downloadedProperties.HourMetrics.IncludeApis.HasValue()
        == properties.HourMetrics.IncludeApis.HasValue())
    {
      EXPECT_EQ(
          downloadedProperties.HourMetrics.IncludeApis.Value(),
          properties.HourMetrics.IncludeApis.Value());
    }
    EXPECT_EQ(
        downloadedProperties.HourMetrics.RetentionPolicy, properties.HourMetrics.RetentionPolicy);

    EXPECT_EQ(downloadedProperties.MinuteMetrics.Version, properties.MinuteMetrics.Version);
    EXPECT_EQ(downloadedProperties.MinuteMetrics.IsEnabled, properties.MinuteMetrics.IsEnabled);
    EXPECT_EQ(
        downloadedProperties.MinuteMetrics.IncludeApis.HasValue(),
        properties.MinuteMetrics.IncludeApis.HasValue());
    if (downloadedProperties.MinuteMetrics.IncludeApis.HasValue()
        == properties.MinuteMetrics.IncludeApis.HasValue())
    {
      EXPECT_EQ(
          downloadedProperties.MinuteMetrics.IncludeApis.Value(),
          properties.MinuteMetrics.IncludeApis.Value());
    }
    EXPECT_EQ(
        downloadedProperties.MinuteMetrics.RetentionPolicy,
        properties.MinuteMetrics.RetentionPolicy);

    EXPECT_EQ(
        downloadedProperties.DefaultServiceVersion.HasValue(),
        properties.DefaultServiceVersion.HasValue());
    if (downloadedProperties.DefaultServiceVersion.HasValue())
    {
      EXPECT_EQ(
          downloadedProperties.DefaultServiceVersion.Value(),
          properties.DefaultServiceVersion.Value());
    }
    EXPECT_EQ(downloadedProperties.Cors, properties.Cors);

    EXPECT_EQ(downloadedProperties.StaticWebsite, properties.StaticWebsite);

    EXPECT_EQ(downloadedProperties.DeleteRetentionPolicy, properties.DeleteRetentionPolicy);

    auto res = m_dataLakeServiceClient->SetProperties(originalProperties);
  }

}}} // namespace Azure::Storage::Test
