// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <cassert>
#include <iostream>

#include <azure/core/base64.hpp>
#include <azure/storage/queues.hpp>

#include "samples_common.hpp"

SAMPLE(QueuesEncodeMessage, QueuesEncodeMessage)
void QueuesEncodeMessage()
{
  using namespace Azure::Storage::Queues;
  std::string QueueName = "sample-queue";

  auto queueClient = QueueClient::CreateFromConnectionString(GetConnectionString(), QueueName);
  queueClient.Create();

  // Binary message cannot be enqueued directly, we encode the message with Base64.
  std::vector<uint8_t> binaryMessage{0x00, 0x01, 0x02, 0x03};
  std::string encodedMessage = Azure::Core::Convert::Base64Encode(binaryMessage);
  queueClient.EnqueueMessage(encodedMessage);

  auto receiveMessagesResult = queueClient.ReceiveMessages().Value;
  auto& receivedMessage = receiveMessagesResult.Messages[0];

  std::cout << receivedMessage.MessageText << std::endl;

  // Sometimes messages are Base64 encoded. Some other queue clients automatically encode the
  // message even it's in plaintext. We need to decode the message in that case.
  auto decodedMessage = Azure::Core::Convert::Base64Decode(receivedMessage.MessageText);
  assert(decodedMessage == binaryMessage);
}
