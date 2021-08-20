// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <iostream>
#include <thread>

#include <azure/storage/queues.hpp>

#include "samples_common.hpp"

using namespace Azure::Storage::Queues;
std::string QueueName = "sample-queue";

void ProducerFunc()
{
  auto queueClient = QueueClient::CreateFromConnectionString(GetConnectionString(), QueueName);

  for (int i = 0; i < 5; ++i)
  {
    std::string msg = "Message " + std::to_string(i);
    queueClient.EnqueueMessage(msg);
  }
}

void ConsumerFunc()
{
  auto queueClient = QueueClient::CreateFromConnectionString(GetConnectionString(), QueueName);

  int counter = 0;
  while (counter < 5)
  {
    auto msgResponse = queueClient.ReceiveMessages();
    if (!msgResponse.Value.Messages.empty())
    {
      auto& msg = msgResponse.Value.Messages[0];

      std::cout << msg.Body << std::endl;
      ++counter;

      queueClient.DeleteMessage(msg.MessageId, msg.PopReceipt);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void ConsumerFunc2()
{
  auto queueClient = QueueClient::CreateFromConnectionString(GetConnectionString(), QueueName);

  int counter = 0;
  while (counter < 5)
  {
    ReceiveMessagesOptions receiveOptions;
    receiveOptions.MaxMessages = 3;
    auto msgResponse = queueClient.ReceiveMessages(receiveOptions);
    for (auto& msg : msgResponse.Value.Messages)
    {
      std::cout << msg.Body << std::endl;
      ++counter;

      auto updateResponse
          = queueClient.UpdateMessage(msg.MessageId, msg.PopReceipt, std::chrono::seconds(30));

      queueClient.DeleteMessage(msg.MessageId, updateResponse.Value.PopReceipt);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

SAMPLE(QueuesGettingStarted, QueuesGettingStarted)
void QueuesGettingStarted()
{
  auto queueClient = QueueClient::CreateFromConnectionString(GetConnectionString(), QueueName);
  queueClient.CreateIfNotExists();

  ProducerFunc();
  ConsumerFunc();
}
