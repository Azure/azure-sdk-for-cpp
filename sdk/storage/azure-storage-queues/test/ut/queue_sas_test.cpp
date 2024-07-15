// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "queue_client_test.hpp"

#include <azure/storage/queues/queue_sas_builder.hpp>

#include <chrono>

namespace Azure { namespace Storage { namespace Test {

  class QueueSasTest : public QueueClientTest {
  public:
    template <class T>
    T GetSasAuthenticatedClient(const T& queueClient, const std::string& sasToken)
    {
      T queueClient1(
          AppendQueryParameters(Azure::Core::Url(queueClient.GetUrl()), sasToken),
          InitStorageClientOptions<Queues::QueueClientOptions>());
      return queueClient1;
    }

    void VerifyQueueSasRead(const Queues::QueueClient& queueClient, const std::string& sasToken)
    {
      auto queueClient1 = GetSasAuthenticatedClient(queueClient, sasToken);
      EXPECT_NO_THROW(queueClient1.GetProperties());
    }

    void VerifyQueueSasNonRead(const Queues::QueueClient& queueClient, const std::string& sasToken)
    {
      auto queueClient1 = GetSasAuthenticatedClient(queueClient, sasToken);
      EXPECT_THROW(queueClient1.GetProperties(), StorageException);
    }

    void VerifyQueueSasAdd(const Queues::QueueClient& queueClient, const std::string& sasToken)
    {
      auto queueClient1 = GetSasAuthenticatedClient(queueClient, sasToken);
      EXPECT_NO_THROW(queueClient1.EnqueueMessage("message1"));
    }

    void VerifyQueueSasUpdate(const Queues::QueueClient& queueClient, const std::string& sasToken)
    {
      auto sendReceipt = queueClient.EnqueueMessage("message0").Value;
      auto queueClient1 = GetSasAuthenticatedClient(queueClient, sasToken);
      auto updateReceipt
          = queueClient1
                .UpdateMessage(
                    sendReceipt.MessageId, sendReceipt.PopReceipt, std::chrono::seconds(0))
                .Value;
      queueClient.DeleteMessage(sendReceipt.MessageId, updateReceipt.PopReceipt);
    }

    void VerifyQueueSasProcess(const Queues::QueueClient& queueClient, const std::string& sasToken)
    {
      auto sendReceipt = queueClient.EnqueueMessage("message0").Value;
      auto queueClient1 = GetSasAuthenticatedClient(queueClient, sasToken);
      // Message deletion requires "p" permission
      queueClient1.DeleteMessage(sendReceipt.MessageId, sendReceipt.PopReceipt);
    }

    void VerifyQueueSasWrite(const Queues::QueueClient& queueClient, const std::string& sasToken)
    {
      auto queueClient1 = GetSasAuthenticatedClient(queueClient, sasToken);
      Metadata m;
      m["key1"] = "meta1";
      EXPECT_NO_THROW(queueClient1.SetMetadata(m));
    }

    void VerifyQueueSasList(
        const Queues::QueueServiceClient& queueServiceClient,
        const std::string& sasToken)
    {
      auto queueServiceClient1 = GetSasAuthenticatedClient(queueServiceClient, sasToken);
      EXPECT_NO_THROW(queueServiceClient.ListQueues());
    }

    void VerifyQueueSasCreate(
        const Queues::QueueServiceClient& queueServiceClient,
        const std::string& newQueueName,
        const std::string& sasToken)
    {
      auto queueServiceClient1 = GetSasAuthenticatedClient(queueServiceClient, sasToken);
      EXPECT_NO_THROW(queueServiceClient1.CreateQueue(newQueueName));
      queueServiceClient.DeleteQueue(newQueueName);
    }

    void VerifyQueueSasDelete(
        const Queues::QueueServiceClient& queueServiceClient,
        const std::string& newQueueName,
        const std::string& sasToken)

    {
      queueServiceClient.CreateQueue(newQueueName);
      auto queueServiceClient1 = GetSasAuthenticatedClient(queueServiceClient, sasToken);
      EXPECT_NO_THROW(queueServiceClient1.DeleteQueue(newQueueName));
      try
      {
        queueServiceClient.DeleteQueue(newQueueName);
      }
      catch (Azure::Storage::StorageException&)
      {
      }
    }
  };

  TEST_F(QueueSasTest, AccountSasPermissions_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.StartsOn = sasStartsOn;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Queue;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto queueClient = *m_queueClient;
    auto queueServiceClient = *m_queueServiceClient;

    for (auto permissions : {
             Sas::AccountSasPermissions::All,
             Sas::AccountSasPermissions::Read,
             Sas::AccountSasPermissions::Write,
             Sas::AccountSasPermissions::List,
             Sas::AccountSasPermissions::Create,
             Sas::AccountSasPermissions::Delete,
             Sas::AccountSasPermissions::Add,
             Sas::AccountSasPermissions::Process,
             Sas::AccountSasPermissions::Update,
         })
    {
      accountSasBuilder.SetPermissions(permissions);
      auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);

      if ((permissions & Sas::AccountSasPermissions::Read) == Sas::AccountSasPermissions::Read)
      {
        VerifyQueueSasRead(queueClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Write) == Sas::AccountSasPermissions::Write)
      {
        VerifyQueueSasWrite(queueClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::List) == Sas::AccountSasPermissions::List)
      {
        VerifyQueueSasList(queueServiceClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Create) == Sas::AccountSasPermissions::Create)
      {
        VerifyQueueSasCreate(queueServiceClient, LowercaseRandomString(), sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Delete) == Sas::AccountSasPermissions::Delete)
      {
        VerifyQueueSasDelete(queueServiceClient, LowercaseRandomString(), sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Add) == Sas::AccountSasPermissions::Add)
      {
        VerifyQueueSasAdd(queueClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Process)
          == Sas::AccountSasPermissions::Process)
      {
        VerifyQueueSasProcess(queueClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Update) == Sas::AccountSasPermissions::Update)
      {
        VerifyQueueSasUpdate(queueClient, sasToken);
      }
    }
  }

  TEST_F(QueueSasTest, ServiceSasPermissions_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    Sas::QueueSasBuilder queueSasBuilder;
    queueSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    queueSasBuilder.StartsOn = sasStartsOn;
    queueSasBuilder.ExpiresOn = sasExpiresOn;
    queueSasBuilder.QueueName = m_queueName;

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto queueClient = *m_queueClient;

    for (auto permissions :
         {Sas::QueueSasPermissions::Read,
          Sas::QueueSasPermissions::Add,
          Sas::QueueSasPermissions::Update,
          Sas::QueueSasPermissions::Process,
          Sas::QueueSasPermissions::All})
    {
      queueSasBuilder.SetPermissions(permissions);
      auto sasToken = queueSasBuilder.GenerateSasToken(*keyCredential);

      if ((permissions & Sas::QueueSasPermissions::Read) == Sas::QueueSasPermissions::Read)
      {
        VerifyQueueSasRead(queueClient, sasToken);
      }
      if ((permissions & Sas::QueueSasPermissions::Add) == Sas::QueueSasPermissions::Add)
      {
        VerifyQueueSasAdd(queueClient, sasToken);
      }
      if ((permissions & Sas::QueueSasPermissions::Update) == Sas::QueueSasPermissions::Update)
      {
        VerifyQueueSasUpdate(queueClient, sasToken);
      }
      if ((permissions & Sas::QueueSasPermissions::Process) == Sas::QueueSasPermissions::Process)
      {
        VerifyQueueSasProcess(queueClient, sasToken);
      }
    }
  }

  TEST_F(QueueSasTest, QueueSasExpired_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiredOn = std::chrono::system_clock::now() - std::chrono::minutes(1);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    Sas::QueueSasBuilder queueSasBuilder;
    queueSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    queueSasBuilder.StartsOn = sasStartsOn;
    queueSasBuilder.ExpiresOn = sasExpiredOn;
    queueSasBuilder.QueueName = m_queueName;
    queueSasBuilder.SetPermissions(Sas::QueueSasPermissions::All);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto queueClient = *m_queueClient;

    auto sasToken = queueSasBuilder.GenerateSasToken(*keyCredential);
    VerifyQueueSasNonRead(queueClient, sasToken);

    queueSasBuilder.ExpiresOn = sasExpiresOn;
    sasToken = queueSasBuilder.GenerateSasToken(*keyCredential);
    VerifyQueueSasRead(queueClient, sasToken);
  }

  TEST_F(QueueSasTest, QueueSasWithoutStartTime_LIVEONLY_)
  {
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    Sas::QueueSasBuilder queueSasBuilder;
    queueSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    queueSasBuilder.ExpiresOn = sasExpiresOn;
    queueSasBuilder.QueueName = m_queueName;
    queueSasBuilder.SetPermissions(Sas::QueueSasPermissions::All);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;
    auto sasToken = queueSasBuilder.GenerateSasToken(*keyCredential);

    auto queueClient = *m_queueClient;
    VerifyQueueSasRead(queueClient, sasToken);
  }

  TEST_F(QueueSasTest, QueueSasWithIP_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiredOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto queueClient = *m_queueClient;

    Sas::QueueSasBuilder queueSasBuilder;
    queueSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    queueSasBuilder.StartsOn = sasStartsOn;
    queueSasBuilder.ExpiresOn = sasExpiredOn;
    queueSasBuilder.SetPermissions(Sas::QueueSasPermissions::All);
    queueSasBuilder.QueueName = m_queueName;

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;
    auto sasToken = queueSasBuilder.GenerateSasToken(*keyCredential);

    VerifyQueueSasRead(queueClient, sasToken);

    queueSasBuilder.IPRange = "0.0.0.0-0.0.0.1";
    sasToken = queueSasBuilder.GenerateSasToken(*keyCredential);
    VerifyQueueSasNonRead(queueClient, sasToken);
  }

  TEST_F(QueueSasTest, QueueSasWithIdentifier_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto clientOptions = InitStorageClientOptions<Queues::QueueClientOptions>();
    auto queueClient = Queues::QueueClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_queueName, clientOptions);
    Queues::Models::SignedIdentifier identifier;
    identifier.Id = RandomString(64);
    identifier.StartsOn = sasStartsOn;
    identifier.ExpiresOn = sasExpiresOn;
    identifier.Permissions = "r";
    Queues::Models::QueueAccessPolicy accessPolicy;
    accessPolicy.SignedIdentifiers.push_back(identifier);
    queueClient.SetAccessPolicy(accessPolicy);

    Sas::QueueSasBuilder queueSasBuilder;
    queueSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    queueSasBuilder.ExpiresOn = sasExpiresOn;
    queueSasBuilder.SetPermissions(static_cast<Sas::QueueSasPermissions>(0));
    queueSasBuilder.Identifier = identifier.Id;
    queueSasBuilder.QueueName = m_queueName;

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;
    auto sasToken = queueSasBuilder.GenerateSasToken(*keyCredential);

    TestSleep(std::chrono::seconds(30));

    VerifyQueueSasRead(queueClient, sasToken);
  }

  TEST_F(QueueSasTest, AccountSasAuthorizationErrorDetail_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.StartsOn = sasStartsOn;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Queue;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::Object;
    accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::All);
    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto queueServiceClient = *m_queueServiceClient;

    auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    auto unauthorizedQueueServiceClient = GetSasAuthenticatedClient(queueServiceClient, sasToken);
    try
    {
      unauthorizedQueueServiceClient.ListQueues();
    }
    catch (StorageException& e)
    {
      EXPECT_EQ("AuthorizationResourceTypeMismatch", e.ErrorCode);
      EXPECT_TRUE(e.AdditionalInformation.count("ExtendedErrorDetail") != 0);
    }
  }

}}} // namespace Azure::Storage::Test
