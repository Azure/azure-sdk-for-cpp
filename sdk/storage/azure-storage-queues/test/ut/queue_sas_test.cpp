// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/queues/queue_sas_builder.hpp>

#include "queue_client_test.hpp"

#include <chrono>

namespace Azure { namespace Storage { namespace Test {

  TEST_F(QueueClientTest, QueueSasTest)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiredOn = std::chrono::system_clock::now() - std::chrono::minutes(1);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    std::string queueName = LowercaseRandomString();
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

    std::string queueUrl = queueClient0.GetUrl();

    auto verifyQueueRead = [&](const std::string& sas) {
      auto queueClient = Queues::QueueClient(queueUrl + sas);
      queueClient.GetProperties();
    };

    auto verifyQueueAdd = [&](const std::string& sas) {
      auto queueClient = Queues::QueueClient(queueUrl + sas);
      queueClient.SendMessage("message1");
    };

    auto verifyQueueUpdate = [&](const std::string& sas) {
      auto sendReceipt = queueClient0.SendMessage("message 0").Value;
      auto queueClient = Queues::QueueClient(queueUrl + sas);
      auto updateReceipt
          = queueClient.UpdateMessage(sendReceipt.MessageId, sendReceipt.PopReceipt, 0).Value;
      queueClient0.DeleteMessage(sendReceipt.MessageId, updateReceipt.PopReceipt);
    };

    auto verifyQueueProcess = [&](const std::string& sas) {
      auto sendReceipt = queueClient0.SendMessage("message 0").Value;
      auto queueClient = Queues::QueueClient(queueUrl + sas);
      queueClient.DeleteMessage(sendReceipt.MessageId, sendReceipt.PopReceipt);
    };

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
      queueClient0.SetAccessPolicy({identifier});

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
