// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

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
    QueueServiceClientTest::SetUp();

    if (shouldSkipTest())
    {
      return;
    }

    m_queueName = GetLowercaseIdentifier();
    m_queueClient
        = std::make_shared<Queues::QueueClient>(m_queueServiceClient->GetQueueClient(m_queueName));

    while (true)
    {
      try
      {
        m_queueClient->Create();
        break;
      }
      catch (StorageException& e)
      {
        if (e.ErrorCode != "QueueBeingDeleted")
        {
          throw;
        }
        SUCCEED() << "Queue is being deleted. Will try again after 3 seconds.";
        std::this_thread::sleep_for(std::chrono::seconds(3));
      }
    }

    m_resourceCleanupFunctions.push_back(
        [queueClient = *m_queueClient]() { queueClient.Delete(); });
  }

  Queues::QueueClient QueueClientTest::GetQueueClientForTest(
      const std::string& queueName,
      Queues::QueueClientOptions clientOptions)
  {
    InitStorageClientOptions(clientOptions);
    const std::string queueUrl = GetQueueUrl(queueName);
    auto queueClient = m_useTokenCredentialByDefault
        ? Queues::QueueClient(queueUrl, GetTestCredential(), clientOptions)
        : Queues::QueueClient::CreateFromConnectionString(
            StandardStorageConnectionString(), queueName, clientOptions);
    m_resourceCleanupFunctions.push_back([queueClient]() { queueClient.Delete(); });

    return queueClient;
  }

  TEST_F(QueueClientTest, Constructors_LIVEONLY_)
  {
    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto getSas = [&]() {
      auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
      auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

      Sas::AccountSasBuilder accountSasBuilder;
      accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
      accountSasBuilder.StartsOn = sasStartsOn;
      accountSasBuilder.ExpiresOn = sasExpiresOn;
      accountSasBuilder.Services = Sas::AccountSasServices::Queue;
      accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;
      accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::Read);
      auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
      return sasToken;
    };

    auto clientOptions = InitStorageClientOptions<Queues::QueueClientOptions>();
    {
      auto queueClient = Queues::QueueClient::CreateFromConnectionString(
          StandardStorageConnectionString(), m_queueName, clientOptions);
      EXPECT_NO_THROW(queueClient.GetProperties());
    }

    {
      auto queueClient = Queues::QueueClient(m_queueClient->GetUrl(), keyCredential, clientOptions);
      EXPECT_NO_THROW(queueClient.GetProperties());
    }

    {
      auto queueClient = Queues::QueueClient(m_queueClient->GetUrl() + getSas(), clientOptions);
      EXPECT_NO_THROW(queueClient.GetProperties());
    }
  }

  TEST_F(QueueClientTest, CreateDelete)
  {
    auto queueClient = GetQueueClientForTest(LowercaseRandomString());
    Azure::Storage::Queues::CreateQueueOptions options;
    options.Metadata = RandomMetadata();
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

    queueClient = GetQueueClientForTest(LowercaseRandomString() + "UPPERCASE");
    EXPECT_THROW(queueClient.Create(), StorageException);

    queueClient = GetQueueClientForTest(LowercaseRandomString());
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
    auto clientOptions = InitStorageClientOptions<Queues::QueueClientOptions>();
    auto queueClient = Queues::QueueClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_queueName, clientOptions);

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
    if (m_testContext.IsLiveMode())
    {
      EXPECT_EQ(ret.Value.SignedIdentifiers, signedIdentifiers);
    }
    queueClient.Delete();
  }

  TEST_F(QueueClientTest, AclOAuth_PLAYBACKONLY_)
  {
    auto credential = GetTestCredential();
    auto clientOptions = InitStorageClientOptions<Queues::QueueClientOptions>();
    auto queueClient = Queues::QueueClient(m_queueClient->GetUrl(), credential, clientOptions);

    Queues::Models::QueueAccessPolicy accessPolicy;
    EXPECT_NO_THROW(queueClient.SetAccessPolicy(accessPolicy));

    EXPECT_NO_THROW(queueClient.GetAccessPolicy());
  }

  TEST_F(QueueClientTest, Audience)
  {
    auto credential = GetTestCredential();
    auto clientOptions = InitStorageClientOptions<Queues::QueueClientOptions>();

    // audience by default
    auto queueClient = Queues::QueueClient(m_queueClient->GetUrl(), credential, clientOptions);
    EXPECT_NO_THROW(queueClient.GetProperties());

    // default audience
    clientOptions.Audience = Queues::QueueAudience::DefaultAudience;
    queueClient = Queues::QueueClient(m_queueClient->GetUrl(), credential, clientOptions);
    EXPECT_NO_THROW(queueClient.GetProperties());

    // service audience
    const auto accountName = StandardStorageAccountName();
    clientOptions.Audience = Queues::QueueAudience::CreateQueueServiceAccountAudience(accountName);
    queueClient = Queues::QueueClient(m_queueClient->GetUrl(), credential, clientOptions);
    EXPECT_NO_THROW(queueClient.GetProperties());

    // custom audience
    auto queueUrl = Azure::Core::Url(queueClient.GetUrl());
    clientOptions.Audience
        = Queues::QueueAudience(queueUrl.GetScheme() + "://" + queueUrl.GetHost());
    queueClient = Queues::QueueClient(m_queueClient->GetUrl(), credential, clientOptions);
    EXPECT_NO_THROW(queueClient.GetProperties());

    queueClient
        = Queues::QueueServiceClient(m_queueServiceClient->GetUrl(), credential, clientOptions)
              .GetQueueClient(m_queueName);
    EXPECT_NO_THROW(queueClient.GetProperties());

    // error audience
    clientOptions.Audience = Queues::QueueAudience("https://disk.compute.azure.com");
    queueClient = Queues::QueueClient(m_queueClient->GetUrl(), credential, clientOptions);
    EXPECT_THROW(queueClient.GetProperties(), StorageException);

    queueClient
        = Queues::QueueServiceClient(m_queueServiceClient->GetUrl(), credential, clientOptions)
              .GetQueueClient(m_queueName);
    EXPECT_THROW(queueClient.GetProperties(), StorageException);
  }
}}} // namespace Azure::Storage::Test
