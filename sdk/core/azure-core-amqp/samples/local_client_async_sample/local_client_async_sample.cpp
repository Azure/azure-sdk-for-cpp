// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <azure/core/amqp/connection.hpp>
#include <azure/core/amqp/message_sender.hpp>
#include <azure/core/amqp/models/messaging_values.hpp>
#include <chrono>
#include <iostream>
#include <limits>
#include <string>

int main()
{
  Azure::Core::Amqp::_internal ::ConnectionOptions connectOptions;
  connectOptions.EnableTrace = true;
  connectOptions.ContainerId = "some";
  Azure::Core::Amqp::_internal::Connection connection("amqp://localhost:5672", connectOptions);

  Azure::Core::Amqp::_internal::SessionOptions sessionOptions;
  sessionOptions.InitialIncomingWindowSize = std::numeric_limits<int32_t>::max();
  sessionOptions.InitialOutgoingWindowSize = std::numeric_limits<uint16_t>::max();

  Azure::Core::Amqp::_internal::Session session(connection, sessionOptions);

  Azure::Core::Amqp::_internal::MessageSenderOptions senderOptions;
  senderOptions.Name = "sender-link";
  senderOptions.MessageSource = "ingress";
  senderOptions.MaxMessageSize = std::numeric_limits<uint16_t>::max();
  Azure::Core::Amqp::_internal::MessageSender sender(
      session, "localhost/ingress", senderOptions, nullptr);

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
            << " milliseconds" << std::endl;

  sender.Close();
}
