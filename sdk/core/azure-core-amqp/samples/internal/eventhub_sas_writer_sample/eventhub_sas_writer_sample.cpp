// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/amqp/internal/connection.hpp>
#include <azure/core/amqp/internal/connection_string_credential.hpp>
#include <azure/core/amqp/internal/message_sender.hpp>

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
  std::string entityPath = credential->GetEntityPath();
  if (entityPath.empty())
  {
    entityPath = std::getenv("EVENTHUB_NAME");
  }
  Azure::Core::Amqp::_internal::ConnectionOptions connectionOptions;
  connectionOptions.ContainerId = "some";
  connectionOptions.EnableTrace = true;
  connectionOptions.Port = credential->GetPort();
  Azure::Core::Amqp::_internal::Connection connection(
      credential->GetHostName(), credential, connectionOptions);

  Azure::Core::Amqp::_internal::SessionOptions sessionOptions;
  sessionOptions.InitialIncomingWindowSize = (std::numeric_limits<int32_t>::max)();
  sessionOptions.InitialOutgoingWindowSize = (std::numeric_limits<uint16_t>::max)();

  Azure::Core::Amqp::_internal::Session session(connection.CreateSession(sessionOptions));

  constexpr int maxMessageSendCount = 1000;
  Azure::Core::Amqp::Models::AmqpMessage message;
  message.SetBody(Azure::Core::Amqp::Models::AmqpBinaryData{'H', 'e', 'l', 'l', 'o'});

  Azure::Core::Amqp::_internal::MessageSenderOptions senderOptions;
  senderOptions.EnableTrace = true;
  senderOptions.Name = "sender-link";
  senderOptions.MessageSource = "ingress";
  senderOptions.SettleMode = Azure::Core::Amqp::_internal::SenderSettleMode::Settled;
  senderOptions.MaxMessageSize = (std::numeric_limits<uint16_t>::max)();
  Azure::Core::Amqp::_internal::MessageSender sender(session.CreateMessageSender(
      entityPath,
      senderOptions
#if ENABLE_UAMQP
      ,
      nullptr
#endif
      ));

  // Open the connection to the remote.
  if (auto err = sender.Open())
  {
    std::cout << "Sender is open" << std::endl;
  }
  else
  {
    std::cout << "Sender failed to open" << err << std::endl;
    return 1;
  }

  auto timeStart = std::chrono::high_resolution_clock::now();

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
            << static_cast<float>(messageSendCount)
          / static_cast<float>(
                   std::chrono::duration_cast<std::chrono::milliseconds>(timeDiff).count())
            << " messages/millisecond. "
            << (static_cast<float>(messageSendCount)
                / static_cast<float>(
                    std::chrono::duration_cast<std::chrono::seconds>(timeDiff).count()))
            << " msg/sec" << std::endl;

  sender.Close();
}
