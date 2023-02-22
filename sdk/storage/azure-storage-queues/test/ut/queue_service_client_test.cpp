// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <thread>

#include <azure/identity/client_secret_credential.hpp>
#include <azure/storage/queues.hpp>

#include "test/ut/test_base.hpp"

namespace Azure { namespace Storage { namespace Queues { namespace Models {

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

}}}} // namespace Azure::Storage::Queues::Models

namespace Azure { namespace Storage { namespace Test {

  class QueueServiceClientTest : public Azure::Storage::Test::StorageTest {
  protected:
    void SetUp()
    {
      StorageTest::SetUp();

      m_options = InitClientOptions<Queues::QueueClientOptions>();
      m_queueServiceClient = std::make_shared<Queues::QueueServiceClient>(
          Queues::QueueServiceClient::CreateFromConnectionString(
              StandardStorageConnectionString(), m_options));
    }
    void TearDown() { StorageTest::TearDown(); }

    std::shared_ptr<Queues::QueueServiceClient> m_queueServiceClient;
    Queues::QueueClientOptions m_options;
  };

  TEST_F(QueueServiceClientTest, ListQueues)
  {
    const std::string prefix1 = "prefix1-a-";
    const std::string prefix2 = "prefix2-b-";

    std::set<std::string> p1Queues;
    std::set<std::string> p2Queues;
    std::set<std::string> p1p2Queues;

    for (int i = 0; i < 5; ++i)
    {
      std::string queueName = prefix1 + "a" + std::to_string(i);
      auto queueClient = m_queueServiceClient->GetQueueClient(queueName);
      queueClient.Create();
      p1Queues.insert(queueName);
      p1p2Queues.insert(queueName);
    }
    for (int i = 0; i < 5; ++i)
    {
      std::string queueName = prefix2 + "b" + std::to_string(i);
      auto queueClient = m_queueServiceClient->GetQueueClient(queueName);
      queueClient.Create();
      p2Queues.insert(queueName);
      p1p2Queues.insert(queueName);
    }

    Azure::Storage::Queues::ListQueuesOptions options;
    options.PageSizeHint = 4;
    std::set<std::string> listQueues;
    for (auto pageResult = m_queueServiceClient->ListQueues(options); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
      EXPECT_FALSE(pageResult.RawResponse->GetHeaders().at(_internal::HttpHeaderRequestId).empty());
      EXPECT_FALSE(pageResult.RawResponse->GetHeaders().at(_internal::HttpHeaderDate).empty());
      EXPECT_FALSE(
          pageResult.RawResponse->GetHeaders().at(_internal::HttpHeaderXMsVersion).empty());
      EXPECT_FALSE(pageResult.ServiceEndpoint.empty());
      for (const auto& q : pageResult.Queues)
      {
        listQueues.insert(q.Name);
      }
    }
    EXPECT_TRUE(
        std::includes(listQueues.begin(), listQueues.end(), p1p2Queues.begin(), p1p2Queues.end()));

    options.Prefix = prefix1;
    listQueues.clear();
    for (auto pageResult = m_queueServiceClient->ListQueues(options); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
      EXPECT_FALSE(pageResult.RawResponse->GetHeaders().at(_internal::HttpHeaderRequestId).empty());
      EXPECT_FALSE(pageResult.RawResponse->GetHeaders().at(_internal::HttpHeaderDate).empty());
      EXPECT_FALSE(
          pageResult.RawResponse->GetHeaders().at(_internal::HttpHeaderXMsVersion).empty());
      EXPECT_FALSE(pageResult.ServiceEndpoint.empty());
      for (const auto& q : pageResult.Queues)
      {
        EXPECT_FALSE(q.Name.empty());
        listQueues.insert(q.Name);
      }
    }
    EXPECT_TRUE(
        std::includes(listQueues.begin(), listQueues.end(), p1Queues.begin(), p1Queues.end()));

    for (const auto& q : p1p2Queues)
    {
      auto queueClient = m_queueServiceClient->GetQueueClient(q);
      queueClient.Delete();
    }
  }

  TEST_F(QueueServiceClientTest, GetProperties)
  {
    auto ret = m_queueServiceClient->GetProperties();
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
  }

  TEST_F(QueueServiceClientTest, SetProperties)
  {
    auto getServicePropertiesResult = m_queueServiceClient->GetProperties().Value;
    Queues::Models::QueueServiceProperties properties;
    properties.Logging = getServicePropertiesResult.Logging;
    properties.HourMetrics = getServicePropertiesResult.HourMetrics;
    properties.MinuteMetrics = getServicePropertiesResult.MinuteMetrics;
    properties.Cors = getServicePropertiesResult.Cors;

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

    Queues::Models::CorsRule corsRule;
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

    EXPECT_NO_THROW(m_queueServiceClient->SetProperties(properties));

    // It takes some time before the new properties comes into effect.
    using namespace std::chrono_literals;
    TestSleep(10s);
    auto downloadedProperties = m_queueServiceClient->GetProperties().Value;
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

    EXPECT_EQ(downloadedProperties.Cors, properties.Cors);

    auto res = m_queueServiceClient->SetProperties(originalProperties);
  }

  TEST_F(QueueServiceClientTest, Statistics)
  {
    EXPECT_THROW(m_queueServiceClient->GetStatistics(), StorageException);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;
    auto secondaryServiceClient = Queues::QueueServiceClient(
        InferSecondaryUrl(m_queueServiceClient->GetUrl()), keyCredential, m_options);
    auto serviceStatistics = secondaryServiceClient.GetStatistics().Value;
    EXPECT_FALSE(serviceStatistics.GeoReplication.Status.ToString().empty());
    if (serviceStatistics.GeoReplication.LastSyncedOn.HasValue())
    {
      EXPECT_TRUE(IsValidTime(serviceStatistics.GeoReplication.LastSyncedOn.Value()));
    }
  }

  TEST_F(QueueServiceClientTest, CreateDeleteQueue)
  {
    std::string queueName = LowercaseRandomString();
    auto queueClient = m_queueServiceClient->CreateQueue(queueName);
    EXPECT_NO_THROW(queueClient.Value.GetProperties());

    m_queueServiceClient->DeleteQueue(queueName);
    EXPECT_THROW(queueClient.Value.GetProperties(), StorageException);
  }

}}} // namespace Azure::Storage::Test
