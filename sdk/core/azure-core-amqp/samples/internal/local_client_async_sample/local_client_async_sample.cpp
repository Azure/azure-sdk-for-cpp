// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/amqp/internal/connection.hpp>
#include <azure/core/amqp/internal/message_sender.hpp>
#include <azure/core/amqp/internal/models/messaging_values.hpp>

#include <chrono>
#include <iostream>
#include <limits>
#include <string>

int main()
{
  Azure::Core::Amqp::_internal ::ConnectionOptions connectionOptions;
  connectionOptions.EnableTrace = true;
  connectionOptions.ContainerId = "some";
  Azure::Core::Amqp::_internal::Connection connection("localhost", nullptr, connectionOptions);

  Azure::Core::Amqp::_internal::SessionOptions sessionOptions;
  sessionOptions.InitialIncomingWindowSize = (std::numeric_limits<int32_t>::max)();
  sessionOptions.InitialOutgoingWindowSize = (std::numeric_limits<uint16_t>::max)();

  Azure::Core::Amqp::_internal::Session session = connection.CreateSession(sessionOptions);

  Azure::Core::Amqp::_internal::MessageSenderOptions senderOptions;
  senderOptions.Name = "sender-link";
  senderOptions.MessageSource = "ingress";
  senderOptions.MaxMessageSize = (std::numeric_limits<uint16_t>::max)();
  Azure::Core::Amqp::_internal::MessageSender sender{
      session.CreateMessageSender("localhost/ingress", senderOptions)};

  // Open the connection to the remote.
  if (sender.Open())
  {

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
}
