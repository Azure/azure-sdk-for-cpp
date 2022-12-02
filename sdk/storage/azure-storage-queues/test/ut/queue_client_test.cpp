// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "queue_client_test.hpp"

#include <chrono>
#include <thread>

namespace Azure { namespace Storage { namespace Queues { namespace Models {

  bool operator==(const SignedIdentifier& lhs, const SignedIdentifier& rhs)
  {
    return lhs.Id == rhs.Id && lhs.StartsOn.HasValue() == rhs.StartsOn.HasValue()
        && (!lhs.StartsOn.HasValue() || lhs.StartsOn.Value() == rhs.StartsOn.Value())
        && lhs.ExpiresOn.HasValue() == rhs.ExpiresOn.HasValue()
        && (!lhs.ExpiresOn.HasValue() || lhs.ExpiresOn.Value() == rhs.ExpiresOn.Value())
        && lhs.Permissions == rhs.Permissions;
  }

}}}} // namespace Azure::Storage::Queues::Models

namespace Azure { namespace Storage { namespace Test {

  void QueueClientTest::SetUp()
  {
    StorageTest::SetUp();
    CHECK_SKIP_TEST();

    m_options = InitClientOptions<Queues::QueueClientOptions>();
    m_queueServiceClient = std::make_shared<Queues::QueueServiceClient>(
        Queues::QueueServiceClient::CreateFromConnectionString(
            StandardStorageConnectionString(), m_options));
    m_testName = GetTestName();
    m_testNameLowercase = GetTestNameLowerCase();

    m_queueName = m_testNameLowercase + "base";
    m_queueClient
        = std::make_shared<Queues::QueueClient>(m_queueServiceClient->GetQueueClient(m_queueName));
    m_queueClient->Create();
  }

  void QueueClientTest::TearDown()
  {
    CHECK_SKIP_TEST();
    m_queueClient->Delete();
    StorageTest::TearDown();
  }

  TEST_F(QueueClientTest, CreateDelete)
  {
    auto queueClient = Azure::Storage::Queues::QueueClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_testNameLowercase, m_options);
    Azure::Storage::Queues::CreateQueueOptions options;
    Azure::Storage::Metadata metadata;
    metadata["key1"] = "one";
    metadata["key2"] = "TWO";
    options.Metadata = metadata;
    auto res = queueClient.Create(options);
    EXPECT_TRUE(res.Value.Created);
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderDate).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderXMsVersion).empty());
    res = queueClient.Create(options);
    EXPECT_FALSE(res.Value.Created);
    res = queueClient.Create();
    EXPECT_FALSE(res.Value.Created);

    auto res2 = queueClient.Delete();
    EXPECT_FALSE(res2.RawResponse->GetHeaders().at(_internal::HttpHeaderRequestId).empty());
    EXPECT_FALSE(res2.RawResponse->GetHeaders().at(_internal::HttpHeaderDate).empty());
    EXPECT_FALSE(res2.RawResponse->GetHeaders().at(_internal::HttpHeaderXMsVersion).empty());

    queueClient = Azure::Storage::Queues::QueueClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_testNameLowercase + "UPPERCASE", m_options);
    EXPECT_THROW(queueClient.Create(), StorageException);
    queueClient = Azure::Storage::Queues::QueueClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_testNameLowercase + "2", m_options);
    {
      auto response = queueClient.Delete();
      EXPECT_FALSE(response.Value.Deleted);
    }
    {
      auto response = queueClient.Create();
      EXPECT_TRUE(response.Value.Created);
    }
    {
      auto response = queueClient.Delete();
      EXPECT_TRUE(response.Value.Deleted);
    }
  }

  TEST_F(QueueClientTest, Metadata)
  {
    Azure::Storage::Metadata metadata;
    metadata["key1"] = "one";
    metadata["key2"] = "TWO";
    auto res = m_queueClient->SetMetadata(metadata);
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderDate).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderXMsVersion).empty());

    auto res2 = m_queueClient->GetProperties();
    EXPECT_FALSE(res2.RawResponse->GetHeaders().at(_internal::HttpHeaderRequestId).empty());
    EXPECT_FALSE(res2.RawResponse->GetHeaders().at(_internal::HttpHeaderDate).empty());
    EXPECT_FALSE(res2.RawResponse->GetHeaders().at(_internal::HttpHeaderXMsVersion).empty());
    auto properties = res2.Value;
    EXPECT_EQ(properties.Metadata, metadata);

    Queues::ListQueuesOptions listOptions;
    listOptions.Prefix = m_queueName;
    listOptions.Include = Queues::Models::ListQueuesIncludeFlags::Metadata;
    for (auto page = m_queueServiceClient->ListQueues(listOptions); page.HasPage();
         page.MoveToNextPage())
    {
      for (auto& q : page.Queues)
      {
        if (q.Name == m_queueName)
        {
          EXPECT_EQ(q.Metadata, metadata);
        }
      }
    }

    metadata.clear();
    m_queueClient->SetMetadata(metadata);
    properties = m_queueClient->GetProperties().Value;
    EXPECT_TRUE(properties.Metadata.empty());
  }

  TEST_F(QueueClientTest, AccessControlList_LIVEONLY_)
  {
    auto queueClient = Azure::Storage::Queues::QueueClient::CreateFromConnectionString(
        StandardStorageConnectionString(), LowercaseRandomString());
    queueClient.Create();

    std::vector<Queues::Models::SignedIdentifier> signedIdentifiers;
    {
      Queues::Models::SignedIdentifier identifier;
      identifier.Id = RandomString(64);
      identifier.StartsOn = std::chrono::system_clock::now() - std::chrono::minutes(1);
      identifier.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(1);
      identifier.Permissions = "r";
      signedIdentifiers.emplace_back(identifier);
    }
    {
      Queues::Models::SignedIdentifier identifier;
      identifier.Id = RandomString(64);
      identifier.StartsOn = std::chrono::system_clock::now() - std::chrono::minutes(2);
      identifier.ExpiresOn.Reset();
      /* cspell:disable-next-line */
      identifier.Permissions = "raup";
      signedIdentifiers.emplace_back(identifier);
    }
    {
      Queues::Models::SignedIdentifier identifier;
      identifier.Id = RandomString(64);
      identifier.Permissions = "r";
      signedIdentifiers.emplace_back(identifier);
    }
    {
      Queues::Models::SignedIdentifier identifier;
      identifier.Id = RandomString(64);
      identifier.StartsOn = std::chrono::system_clock::now() - std::chrono::minutes(1);
      identifier.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(1);
      signedIdentifiers.emplace_back(identifier);
    }

    Queues::Models::QueueAccessPolicy accessPolicy;
    accessPolicy.SignedIdentifiers = signedIdentifiers;
    EXPECT_NO_THROW(queueClient.SetAccessPolicy(accessPolicy));

    auto ret = queueClient.GetAccessPolicy();
    EXPECT_EQ(ret.Value.SignedIdentifiers, signedIdentifiers);

    queueClient.Delete();
  }

}}} // namespace Azure::Storage::Test