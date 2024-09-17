// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/amqp/internal/connection.hpp>
#include <azure/core/amqp/internal/connection_string_credential.hpp>
#include <azure/core/amqp/internal/message_sender.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/identity.hpp>

#include <chrono>
#include <iostream>
#include <limits>
#include <string>

constexpr const char* EH_AUTHENTICATION_SCOPE = "https://eventhubs.azure.net/.default";

int main()
{
  // Retrieve the eventhub connection string so we can extract the host name and entity name. We are
  // NOT using the connection string to connect to the eventhub.
  std::string eventhubConnectionString = std::getenv("EVENTHUB_CONNECTION_STRING");
  Azure::Core::Amqp::_internal::ConnectionStringParser connectionStringCredential(
      eventhubConnectionString);
  std::string eventhubsHost = connectionStringCredential.GetHostName();
  std::string eventhubsEntity = connectionStringCredential.GetEntityPath();

  // If the connection string does not specify an entity path, then look for the eventhub name in an
  // environment variable.
  if (eventhubsEntity.empty())
  {
    eventhubsEntity = std::getenv("EVENTHUB_NAME");
  }

  // Establish credentials for the eventhub client.
  auto credential{std::make_shared<Azure::Identity::DefaultAzureCredential>()};

  Azure::Core::Amqp::_internal::ConnectionOptions connectionOptions;
  connectionOptions.ContainerId = "some";
  connectionOptions.EnableTrace = true;
  connectionOptions.AuthenticationScopes = {EH_AUTHENTICATION_SCOPE};
  Azure::Core::Amqp::_internal::Connection connection(eventhubsHost, credential, connectionOptions);

  Azure::Core::Amqp::_internal::SessionOptions sessionOptions;
  sessionOptions.InitialIncomingWindowSize = (std::numeric_limits<int32_t>::max)();
  sessionOptions.InitialOutgoingWindowSize = (std::numeric_limits<uint16_t>::max)();

  Azure::Core::Amqp::_internal::Session session(connection.CreateSession(sessionOptions));

  constexpr int maxMessageSendCount = 1000;

  Azure::Core::Amqp::Models::AmqpMessage message;
  message.SetBody(Azure::Core::Amqp::Models::AmqpValue{"Hello"});

  Azure::Core::Amqp::_internal::MessageSenderOptions senderOptions;
  senderOptions.MaxMessageSize = (std::numeric_limits<uint16_t>::max)();
  senderOptions.MessageSource = "ingress";
  senderOptions.Name = "sender-link";
  senderOptions.SettleMode = Azure::Core::Amqp::_internal::SenderSettleMode::Settled;
  senderOptions.EnableTrace = true;
  Azure::Core::Amqp::_internal::MessageSender sender(
      session.CreateMessageSender(eventhubsEntity, senderOptions));

  // Open the connection to the remote. This will authenticate the client and connect to the server.
  if (auto err = sender.Open())
  {
    std::cout << "Failed to open the message sender: " << err << std::endl;
    return 1;
  }

  auto timeStart = std::chrono::high_resolution_clock::now();

  int messageSendCount = 0;
  while (messageSendCount < maxMessageSendCount)
  {
    auto result = sender.Send(message);
    if (std::get<0>(result) != Azure::Core::Amqp::_internal::MessageSendStatus::Ok)
    {
      std::cout << "Failed to send message " << messageSendCount << std::endl;
      std::cout << "Message error information: " << std::get<1>(result) << std::endl;
      break;
    }
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
