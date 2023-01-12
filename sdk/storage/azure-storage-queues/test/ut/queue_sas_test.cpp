// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/queues/queue_sas_builder.hpp>

#include "queue_client_test.hpp"

#include <chrono>

namespace Azure { namespace Storage { namespace Test {

  TEST_F(QueueClientTest, QueueSasTest_LIVEONLY_)
  {
    CHECK_SKIP_TEST();

    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiredOn = std::chrono::system_clock::now() - std::chrono::minutes(1);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    std::string queueName = LowercaseRandomString();

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.StartsOn = sasStartsOn;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Queue;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;

    Sas::QueueSasBuilder queueSasBuilder;
    queueSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    queueSasBuilder.StartsOn = sasStartsOn;
    queueSasBuilder.ExpiresOn = sasExpiresOn;
    queueSasBuilder.QueueName = queueName;

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;
    auto accountName = keyCredential->AccountName;
    auto queueServiceClient0
        = Queues::QueueServiceClient::CreateFromConnectionString(StandardStorageConnectionString());
    auto queueClient0 = queueServiceClient0.GetQueueClient(queueName);
    queueClient0.Create();

    std::string queueServiceUrl = queueServiceClient0.GetUrl();
    std::string queueUrl = queueClient0.GetUrl();

    auto verifyQueueRead = [&](const std::string& sas) {
      auto queueClient = Queues::QueueClient(queueUrl + sas);
      queueClient.GetProperties();
    };

    auto verifyQueueAdd = [&](const std::string& sas) {
      auto queueClient = Queues::QueueClient(queueUrl + sas);
      queueClient.EnqueueMessage("message1");
    };

    auto verifyQueueUpdate = [&](const std::string& sas) {
      auto sendReceipt = queueClient0.EnqueueMessage("message 0").Value;
      auto queueClient = Queues::QueueClient(queueUrl + sas);
      auto updateReceipt
          = queueClient
                .UpdateMessage(
                    sendReceipt.MessageId, sendReceipt.PopReceipt, std::chrono::seconds(0))
                .Value;
      queueClient0.DeleteMessage(sendReceipt.MessageId, updateReceipt.PopReceipt);
    };

    auto verifyQueueProcess = [&](const std::string& sas) {
      auto sendReceipt = queueClient0.EnqueueMessage("message 0").Value;
      auto queueClient = Queues::QueueClient(queueUrl + sas);
      queueClient.DeleteMessage(sendReceipt.MessageId, sendReceipt.PopReceipt);
    };

    auto verifyQueueWrite = [&](const std::string& sas) {
      auto queueClient = Queues::QueueClient(queueUrl + sas);
      Metadata m;
      m["key1"] = RandomString();
      EXPECT_NO_THROW(queueClient.SetMetadata(m));
    };

    auto verifyQueueList = [&](const std::string& sas) {
      auto queueServiceClient = Queues::QueueServiceClient(queueServiceUrl + sas);
      EXPECT_NO_THROW(queueServiceClient.ListQueues());
    };

    auto verifyQueueCreate = [&](const std::string& sas) {
      auto queueServiceClient = Queues::QueueServiceClient(queueServiceUrl + sas);
      const std::string newQueueName = LowercaseRandomString();
      EXPECT_NO_THROW(queueServiceClient.CreateQueue(newQueueName));
      queueServiceClient0.GetQueueClient(newQueueName).Delete();
    };

    auto verifyQueueDelete = [&](const std::string& sas) {
      const std::string newQueueName = LowercaseRandomString();
      queueServiceClient0.CreateQueue(newQueueName);
      auto queueServiceClient = Queues::QueueServiceClient(queueServiceUrl + sas);
      EXPECT_NO_THROW(queueServiceClient.DeleteQueue(newQueueName));
    };

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
        verifyQueueRead(sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Write) == Sas::AccountSasPermissions::Write)
      {
        verifyQueueWrite(sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::List) == Sas::AccountSasPermissions::List)
      {
        verifyQueueList(sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Create) == Sas::AccountSasPermissions::Create)
      {
        verifyQueueCreate(sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Delete) == Sas::AccountSasPermissions::Delete)
      {
        verifyQueueDelete(sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Add) == Sas::AccountSasPermissions::Add)
      {
        verifyQueueAdd(sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Process)
          == Sas::AccountSasPermissions::Process)
      {
        verifyQueueProcess(sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Update) == Sas::AccountSasPermissions::Update)
      {
        verifyQueueUpdate(sasToken);
      }
    }

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
        verifyQueueRead(sasToken);
      }
      if ((permissions & Sas::QueueSasPermissions::Add) == Sas::QueueSasPermissions::Add)
      {
        verifyQueueAdd(sasToken);
      }
      if ((permissions & Sas::QueueSasPermissions::Update) == Sas::QueueSasPermissions::Update)
      {
        verifyQueueUpdate(sasToken);
      }
      if ((permissions & Sas::QueueSasPermissions::Process) == Sas::QueueSasPermissions::Process)
      {
        verifyQueueProcess(sasToken);
      }
    }

    queueSasBuilder.SetPermissions(Sas::QueueSasPermissions::All);
    // Expires
    {
      Sas::QueueSasBuilder builder2 = queueSasBuilder;
      builder2.StartsOn = sasStartsOn;
      builder2.ExpiresOn = sasExpiredOn;
      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      EXPECT_THROW(verifyQueueRead(sasToken), StorageException);
    }

    // Without start time
    {
      Sas::QueueSasBuilder builder2 = queueSasBuilder;
      builder2.StartsOn.Reset();
      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      EXPECT_NO_THROW(verifyQueueRead(sasToken));
    }

    // IP
    {
      Sas::QueueSasBuilder builder2 = queueSasBuilder;
      builder2.IPRange = "0.0.0.0-0.0.0.1";
      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      EXPECT_THROW(verifyQueueRead(sasToken), StorageException);

      // TODO: Add this test case back with support to contain IPv6 ranges when service is ready.
      // builder2.IPRange = "0.0.0.0-255.255.255.255";
      // sasToken = builder2.GenerateSasToken(*keyCredential);
      // EXPECT_NO_THROW(verifyQueueRead(sasToken));
    }

    // Identifier
    {
      Queues::Models::SignedIdentifier identifier;
      identifier.Id = RandomString(64);
      identifier.StartsOn = sasStartsOn;
      identifier.ExpiresOn = sasExpiresOn;
      identifier.Permissions = "r";
      Queues::Models::QueueAccessPolicy accessPolicy;
      accessPolicy.SignedIdentifiers.push_back(identifier);
      queueClient0.SetAccessPolicy(accessPolicy);

      Sas::QueueSasBuilder builder2 = queueSasBuilder;
      builder2.StartsOn.Reset();
      builder2.ExpiresOn = Azure::DateTime();
      builder2.SetPermissions(static_cast<Sas::QueueSasPermissions>(0));
      builder2.Identifier = identifier.Id;

      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      // TODO: looks like a server bug, the identifier doesn't work sometimes.
      // EXPECT_NO_THROW(verifyQueueRead(sasToken));
    }
    queueClient0.Delete();
  }

}}} // namespace Azure::Storage::Test
