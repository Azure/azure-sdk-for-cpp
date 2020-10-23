// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs.hpp"
#include "test_base.hpp"

#include <thread>

namespace Azure { namespace Storage { namespace Blobs {

  bool operator==(
      const Azure::Storage::Blobs::BlobRetentionPolicy& lhs,
      const Azure::Storage::Blobs::BlobRetentionPolicy& rhs)
  {
    if (lhs.Enabled != rhs.Enabled)
    {
      return false;
    }
    if (lhs.Days.HasValue() != rhs.Days.HasValue())
    {
      return false;
    }
    if (lhs.Days.HasValue() && rhs.Days.HasValue() && lhs.Days.GetValue() != rhs.Days.GetValue())
    {
      return false;
    }
    return true;
  }

  bool operator==(
      const Azure::Storage::Blobs::BlobCorsRule& lhs,
      const Azure::Storage::Blobs::BlobCorsRule& rhs)
  {
    return lhs.AllowedHeaders == rhs.AllowedHeaders && lhs.AllowedMethods == rhs.AllowedMethods
        && lhs.AllowedOrigins == rhs.AllowedOrigins && lhs.ExposedHeaders == rhs.ExposedHeaders
        && lhs.MaxAgeInSeconds == rhs.MaxAgeInSeconds;
  }

  bool operator==(
      const Azure::Storage::Blobs::BlobStaticWebsite& lhs,
      const Azure::Storage::Blobs::BlobStaticWebsite& rhs)
  {
    if (lhs.Enabled != rhs.Enabled)
    {
      return false;
    }
    if (lhs.DefaultIndexDocumentPath.HasValue() != rhs.DefaultIndexDocumentPath.HasValue())
    {
      return false;
    }
    if (lhs.DefaultIndexDocumentPath.HasValue() && rhs.DefaultIndexDocumentPath.HasValue()
        && lhs.DefaultIndexDocumentPath.GetValue() != rhs.DefaultIndexDocumentPath.GetValue())
    {
      return false;
    }
    if (lhs.DefaultIndexDocumentPath.HasValue() != rhs.DefaultIndexDocumentPath.HasValue())
    {
      return false;
    }
    if (lhs.ErrorDocument404Path.HasValue() && rhs.ErrorDocument404Path.HasValue()
        && lhs.ErrorDocument404Path.GetValue() != rhs.ErrorDocument404Path.GetValue())
    {
      return false;
    }
    if (lhs.IndexDocument.HasValue() && rhs.IndexDocument.HasValue()
        && lhs.IndexDocument.GetValue() != rhs.IndexDocument.GetValue())
    {
      return false;
    }
    return true;
  }

}}} // namespace Azure::Storage::Blobs

namespace Azure { namespace Storage { namespace Test {

  class BlobServiceClientTest : public ::testing::Test {
  protected:
    BlobServiceClientTest()
        : m_blobServiceClient(Azure::Storage::Blobs::BlobServiceClient::CreateFromConnectionString(
            StandardStorageConnectionString()))
    {
    }

    Azure::Storage::Blobs::BlobServiceClient m_blobServiceClient;
  };

  TEST_F(BlobServiceClientTest, ListContainers)
  {
    const std::string prefix1 = "prefix1-" + LowercaseRandomString() + "-";
    const std::string prefix2 = "prefix2-" + LowercaseRandomString() + "-";

    std::set<std::string> p1Containers;
    std::set<std::string> p2Containers;
    std::set<std::string> p1p2Containers;

    for (int i = 0; i < 5; ++i)
    {
      std::string containerName = prefix1 + LowercaseRandomString();
      auto containerClient = m_blobServiceClient.GetBlobContainerClient(containerName);
      containerClient.Create();
      p1Containers.insert(containerName);
      p1p2Containers.insert(containerName);
    }
    for (int i = 0; i < 5; ++i)
    {
      std::string containerName = prefix2 + LowercaseRandomString();
      auto containerClient = m_blobServiceClient.GetBlobContainerClient(containerName);
      containerClient.Create();
      p2Containers.insert(containerName);
      p1p2Containers.insert(containerName);
    }

    Azure::Storage::Blobs::ListContainersSegmentOptions options;
    options.MaxResults = 4;
    std::set<std::string> listContainers;
    do
    {
      auto res = m_blobServiceClient.ListBlobContainersSegment(options);
      EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderRequestId).empty());
      EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderDate).empty());
      EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderXMsVersion).empty());
      EXPECT_FALSE(res->ServiceEndpoint.empty());

      options.ContinuationToken = res->ContinuationToken;
      for (const auto& container : res->Items)
      {
        listContainers.insert(container.Name);
      }
    } while (!options.ContinuationToken.GetValue().empty());
    EXPECT_TRUE(std::includes(
        listContainers.begin(),
        listContainers.end(),
        p1p2Containers.begin(),
        p1p2Containers.end()));

    options.Prefix = prefix1;
    listContainers.clear();
    do
    {
      auto res = m_blobServiceClient.ListBlobContainersSegment(options);
      EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderRequestId).empty());
      EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderDate).empty());
      EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderXMsVersion).empty());
      EXPECT_FALSE(res->ServiceEndpoint.empty());

      options.ContinuationToken = res->ContinuationToken;
      for (const auto& container : res->Items)
      {
        EXPECT_FALSE(container.Name.empty());
        EXPECT_FALSE(container.ETag.empty());
        EXPECT_FALSE(container.LastModified.empty());
        EXPECT_FALSE(container.IsDeleted);
        EXPECT_FALSE(container.VersionId.HasValue());
        EXPECT_FALSE(container.DeletedTime.HasValue());
        EXPECT_FALSE(container.RemainingRetentionDays.HasValue());
        EXPECT_EQ(container.DefaultEncryptionScope, c_AccountEncryptionKey);
        EXPECT_FALSE(container.PreventEncryptionScopeOverride);
        listContainers.insert(container.Name);
      }
    } while (!options.ContinuationToken.GetValue().empty());
    EXPECT_TRUE(std::includes(
        listContainers.begin(), listContainers.end(), p1Containers.begin(), p1Containers.end()));

    for (const auto& container : p1p2Containers)
    {
      auto container_client = m_blobServiceClient.GetBlobContainerClient(container);
      container_client.Delete();
    }
  }

  TEST_F(BlobServiceClientTest, GetProperties)
  {
    auto ret = m_blobServiceClient.GetProperties();
    auto properties = *ret;
    auto logging = properties.Logging;
    EXPECT_FALSE(logging.Version.empty());
    if (logging.RetentionPolicy.Enabled)
    {
      EXPECT_TRUE(logging.RetentionPolicy.Days.HasValue());
    }
    auto hourMetrics = properties.HourMetrics;
    if (hourMetrics.Enabled)
    {
      EXPECT_FALSE(hourMetrics.Version.empty());
      if (hourMetrics.RetentionPolicy.Enabled)
      {
        EXPECT_TRUE(hourMetrics.RetentionPolicy.Days.HasValue());
      }
    }
    auto minuteMetrics = properties.HourMetrics;
    if (minuteMetrics.Enabled)
    {
      EXPECT_FALSE(minuteMetrics.Version.empty());
      if (minuteMetrics.RetentionPolicy.Enabled)
      {
        EXPECT_TRUE(minuteMetrics.RetentionPolicy.Days.HasValue());
      }
    }
    auto deleteRetentionPolicy = properties.DeleteRetentionPolicy;
    if (deleteRetentionPolicy.Enabled)
    {
      EXPECT_TRUE(deleteRetentionPolicy.Days.HasValue());
    }
  }

  TEST_F(BlobServiceClientTest, DISABLED_SetProperties)
  {
    auto getServicePropertiesResult = *m_blobServiceClient.GetProperties();
    Blobs::BlobServiceProperties properties;
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
    properties.Logging.RetentionPolicy.Enabled = true;
    properties.Logging.RetentionPolicy.Days = 3;

    properties.HourMetrics.Enabled = true;
    properties.HourMetrics.RetentionPolicy.Enabled = true;
    properties.HourMetrics.RetentionPolicy.Days = 4;
    properties.HourMetrics.IncludeApis = true;

    properties.MinuteMetrics.Enabled = true;
    properties.MinuteMetrics.RetentionPolicy.Enabled = true;
    properties.MinuteMetrics.RetentionPolicy.Days = 4;
    properties.MinuteMetrics.IncludeApis = true;

    properties.DefaultServiceVersion = Blobs::c_ApiVersion;

    properties.StaticWebsite.Enabled = true;
    properties.StaticWebsite.IndexDocument = "index.html";
    properties.StaticWebsite.ErrorDocument404Path = "404.html";
    properties.StaticWebsite.DefaultIndexDocumentPath.Reset();

    Blobs::BlobCorsRule corsRule;
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

    properties.DeleteRetentionPolicy.Enabled = true;
    properties.DeleteRetentionPolicy.Days = 5;

    EXPECT_NO_THROW(m_blobServiceClient.SetProperties(properties));
    // It takes some time before the new properties comes into effect.
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(10s);
    auto downloadedProperties = *m_blobServiceClient.GetProperties();
    EXPECT_EQ(downloadedProperties.Logging.Version, properties.Logging.Version);
    EXPECT_EQ(downloadedProperties.Logging.Delete, properties.Logging.Delete);
    EXPECT_EQ(downloadedProperties.Logging.Read, properties.Logging.Read);
    EXPECT_EQ(downloadedProperties.Logging.Write, properties.Logging.Write);
    EXPECT_EQ(downloadedProperties.Logging.RetentionPolicy, properties.Logging.RetentionPolicy);

    EXPECT_EQ(downloadedProperties.HourMetrics.Version, properties.HourMetrics.Version);
    EXPECT_EQ(downloadedProperties.HourMetrics.Enabled, properties.HourMetrics.Enabled);
    EXPECT_EQ(
        downloadedProperties.HourMetrics.IncludeApis.HasValue(),
        properties.HourMetrics.IncludeApis.HasValue());
    if (downloadedProperties.HourMetrics.IncludeApis.HasValue()
        == properties.HourMetrics.IncludeApis.HasValue())
    {
      EXPECT_EQ(
          downloadedProperties.HourMetrics.IncludeApis.GetValue(),
          properties.HourMetrics.IncludeApis.GetValue());
    }
    EXPECT_EQ(
        downloadedProperties.HourMetrics.RetentionPolicy, properties.HourMetrics.RetentionPolicy);

    EXPECT_EQ(downloadedProperties.MinuteMetrics.Version, properties.MinuteMetrics.Version);
    EXPECT_EQ(downloadedProperties.MinuteMetrics.Enabled, properties.MinuteMetrics.Enabled);
    EXPECT_EQ(
        downloadedProperties.MinuteMetrics.IncludeApis.HasValue(),
        properties.MinuteMetrics.IncludeApis.HasValue());
    if (downloadedProperties.MinuteMetrics.IncludeApis.HasValue()
        == properties.MinuteMetrics.IncludeApis.HasValue())
    {
      EXPECT_EQ(
          downloadedProperties.MinuteMetrics.IncludeApis.GetValue(),
          properties.MinuteMetrics.IncludeApis.GetValue());
    }
    EXPECT_EQ(
        downloadedProperties.MinuteMetrics.RetentionPolicy,
        properties.MinuteMetrics.RetentionPolicy);

    EXPECT_EQ(downloadedProperties.DefaultServiceVersion, properties.DefaultServiceVersion);
    EXPECT_EQ(downloadedProperties.Cors, properties.Cors);

    EXPECT_EQ(downloadedProperties.StaticWebsite, properties.StaticWebsite);

    EXPECT_EQ(downloadedProperties.DeleteRetentionPolicy, properties.DeleteRetentionPolicy);

    m_blobServiceClient.SetProperties(originalProperties);
  }

  TEST_F(BlobServiceClientTest, AccountInfo)
  {
    auto accountInfo = *m_blobServiceClient.GetAccountInfo();
    EXPECT_NE(accountInfo.SkuName, Blobs::SkuName::Unknown);
    EXPECT_NE(accountInfo.AccountKind, Blobs::AccountKind::Unknown);
  }

  TEST_F(BlobServiceClientTest, Statistics)
  {
    EXPECT_THROW(m_blobServiceClient.GetStatistics(), StorageError);

    auto keyCredential
        = Details::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;
    auto secondaryServiceClient
        = Blobs::BlobServiceClient(InferSecondaryUri(m_blobServiceClient.GetUri()), keyCredential);
    auto serviceStatistics = *secondaryServiceClient.GetStatistics();
    EXPECT_NE(serviceStatistics.GeoReplication.Status, Blobs::BlobGeoReplicationStatus::Unknown);
    EXPECT_TRUE(serviceStatistics.GeoReplication.LastSyncTime.HasValue());
    EXPECT_FALSE(serviceStatistics.GeoReplication.LastSyncTime.GetValue().empty());
  }

}}} // namespace Azure::Storage::Test
