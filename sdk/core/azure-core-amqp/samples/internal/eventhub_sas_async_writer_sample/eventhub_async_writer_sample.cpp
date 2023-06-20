// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <azure/core/amqp/connection.hpp>
#include <azure/core/amqp/message_sender.hpp>

#include <chrono>
#include <iostream>
#include <limits>
#include <string>

int main()
{
  std::string eventhubConnectionString = std::getenv("EVENTHUB_CONNECTION_STRING");

  auto credential{
      std::make_shared<Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential>(
          eventhubConnectionString)};

  std::string targetEntity = credential->GetEntityPath();
  if (targetEntity.empty())
  {
    targetEntity = std::getenv("EVENTHUB_NAME");
  }
  Azure::Core::Amqp::_internal::ConnectionOptions connectionOptions;
  connectionOptions.ContainerId = "some";
  connectionOptions.Port = credential->GetPort();

  Azure::Core::Amqp::_internal::Connection connection(
      credential->GetHostName(), credential, connectionOptions);

  Azure::Core::Amqp::_internal::SessionOptions sessionOptions;
  sessionOptions.InitialIncomingWindowSize = std::numeric_limits<int32_t>::max();
  sessionOptions.InitialOutgoingWindowSize = std::numeric_limits<uint16_t>::max();

  Azure::Core::Amqp::_internal::Session session{connection.CreateSession(sessionOptions)};

  Azure::Core::Amqp::_internal::MessageSenderOptions senderOptions;
  senderOptions.Name = "sender-link";
  senderOptions.MessageSource = "ingress";
  senderOptions.SettleMode = Azure::Core::Amqp::_internal::SenderSettleMode::Unsettled;
  senderOptions.MaxMessageSize = std::numeric_limits<uint16_t>::max();

  Azure::Core::Amqp::_internal::MessageSender sender{
      session.CreateMessageSender(targetEntity, senderOptions, nullptr)};

  // Open the connection to the remote.
  sender.Open();

  constexpr int maxMessageSendCount = 1000;

  Azure::Core::Amqp::Models::AmqpMessage message;
  message.SetBody(Azure::Core::Amqp::Models::AmqpBinaryData{'H', 'e', 'l', 'l', 'o'});

  std::vector<Azure::Core::Amqp::Common::_internal::QueuedOperation<
      Azure::Core::Amqp::_internal::MessageSender::SendResult>>
      results;

  int messageSendCount = 0;
  auto timeStart = std::chrono::high_resolution_clock::now();

  while (messageSendCount < maxMessageSendCount)
  {
    results.push_back(sender.QueueSend(message));
    messageSendCount += 1;
  }

  auto timeEnd = std::chrono::high_resolution_clock::now();
  std::chrono::nanoseconds timeDiff = timeEnd - timeStart;

  std::cout << "Sent " << messageSendCount << " in "
            << std::chrono::duration_cast<std::chrono::milliseconds>(timeDiff).count()
            << " milliseconds. "
            << (static_cast<float>(messageSendCount)
                / static_cast<float>(
                    std::chrono::duration_cast<std::chrono::seconds>(timeDiff).count()))
            << " msg/sec" << std::endl;


  timeStart = std::chrono::high_resolution_clock::now();

  for (auto& result : results)
  {
    auto sendResult = result.WaitForOperationResult({}, connection);
    if (std::get<0>(sendResult) != Azure::Core::Amqp::_internal::MessageSendStatus::Ok)
    {
      std::cout << "Error sending message: " << static_cast<int>(std::get<0>(sendResult))
                << std::endl;
    }
  }

  timeEnd = std::chrono::high_resolution_clock::now();
  timeDiff = timeEnd - timeStart;

  std::cout << "Waiting for sends to complete took "
            << std::chrono::duration_cast<std::chrono::milliseconds>(timeDiff).count()
            << " milliseconds" << std::endl;
  sender.Close();
}
