// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/amqp/connection.hpp>
#include <azure/core/amqp/message_sender.hpp>

#include <chrono>
#include <iostream>
#include <limits>
#include <string>

#define EH_CONNECTION_STRING "<<<Replace with the connection string from your eventhubs instance>>>"

int main()
{
  auto credentials{
      std::make_shared<Azure::Core::Amqp::_internal::SaslPlainConnectionStringCredential>(
          EH_CONNECTION_STRING)};
  std::string targetEntity = credentials->GetEntityPath();
  if (targetEntity.empty())
  {
    targetEntity = std::getenv("EVENTHUB_NAME");
  }
  Azure::Core::Amqp::_internal::ConnectionOptions connectionOptions;
  connectionOptions.ContainerId = "some";
  connectionOptions.SaslCredentials = credentials;

  Azure::Core::Amqp::_internal::Connection connection(
      credentials->GetHostName(), credentials->GetPort(), credentials, connectionOptions);

  Azure::Core::Amqp::_internal::SessionOptions sessionOptions;
  sessionOptions.InitialIncomingWindowSize = std::numeric_limits<int32_t>::max();
  sessionOptions.InitialOutgoingWindowSize = std::numeric_limits<uint16_t>::max();
  Azure::Core::Amqp::_internal::Session session(connection);

  // @begin_snippet: CreateSender
  Azure::Core::Amqp::_internal::MessageSenderOptions senderOptions;
  senderOptions.Name = "sender-link";
  senderOptions.MessageSource = "source";
  senderOptions.SettleMode = Azure::Core::Amqp::_internal::SenderSettleMode::Unsettled;
  senderOptions.MaxMessageSize = std::numeric_limits<uint16_t>::max();

  Azure::Core::Amqp::_internal::MessageSender sender(
      session, credentials->GetEntityPath(), senderOptions, nullptr);
  // @end_snippet

  // Open the connection to the remote.
  sender.Open();

  auto timeStart = std::chrono::high_resolution_clock::now();

  constexpr int maxMessageSendCount = 1000;

  Azure::Core::Amqp::Models::AmqpMessage message;
  message.SetBody(Azure::Core::Amqp::Models::AmqpBinaryData{'H', 'e', 'l', 'l', 'o'});

  int messageSendCount = 0;
  while (messageSendCount < maxMessageSendCount)
  {
    auto result = sender.Send(message);
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

  sender.Close();
}
