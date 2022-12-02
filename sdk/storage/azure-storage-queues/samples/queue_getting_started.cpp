// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "get_env.hpp"

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <thread>

#include <azure/storage/queues.hpp>

std::string GetConnectionString()
{
  const static std::string ConnectionString = "";

  if (!ConnectionString.empty())
  {
    return ConnectionString;
  }
  const static std::string envConnectionString = std::getenv("AZURE_STORAGE_CONNECTION_STRING");
  if (!envConnectionString.empty())
  {
    return envConnectionString;
  }
  throw std::runtime_error("Cannot find connection string.");
}

using namespace Azure::Storage::Queues;
const std::string QueueName = "sample-queue";

void ProducerFunc()
{
  auto queueClient = QueueClient::CreateFromConnectionString(GetConnectionString(), QueueName);

  for (int i = 0; i < 5; ++i)
  {
    std::string msg = "Message " + std::to_string(i);
    queueClient.EnqueueMessage(msg);
  }

  for (int i = 5; i < 10; ++i)
  {
    std::string msg = "Message " + std::to_string(i);
    EnqueueMessageOptions options;
    options.TimeToLive = std::chrono::seconds(60 * 60 * 24);
    options.VisibilityTimeout = std::chrono::seconds(1);
    queueClient.EnqueueMessage(msg, options);
  }
}

void ConsumerFunc()
{
  auto queueClient = QueueClient::CreateFromConnectionString(GetConnectionString(), QueueName);

  int counter = 0;
  while (counter < 5)
  {
    auto receiveMessagesResult = queueClient.ReceiveMessages().Value;
    if (!receiveMessagesResult.Messages.empty())
    {
      auto& msg = receiveMessagesResult.Messages[0];

      std::cout << msg.MessageText << std::endl;
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
    auto receiveMessagesResult = queueClient.ReceiveMessages(receiveOptions).Value;
    for (auto& msg : receiveMessagesResult.Messages)
    {
      std::cout << msg.MessageText << std::endl;
      ++counter;

      auto updateResponse
          = queueClient.UpdateMessage(msg.MessageId, msg.PopReceipt, std::chrono::seconds(30));

      queueClient.DeleteMessage(msg.MessageId, updateResponse.Value.PopReceipt);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

int main()
{
  auto queueClient = QueueClient::CreateFromConnectionString(GetConnectionString(), QueueName);
  queueClient.Create();

  ProducerFunc();
  ConsumerFunc();
  ConsumerFunc2();

  return 0;
}