// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <azure/core/amqp/connection.hpp>
#include <azure/core/amqp/connection_string_credential.hpp>
#include <azure/core/amqp/message_sender.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <chrono>
#include <iostream>
#include <limits>
#include <string>

// #define EH_HOST "<<<Replace with the eventhubs host name>>>"
// #define EH_ENTITY "<<<Replace with the eventhub name>>>"

#define EH_AUTHENTICATION_SCOPE "https://eventhubs.azure.net/.default"

// #define EH_ENTITY_URL "amqps://" EH_HOST "/" EH_ENTITY

int main()
{
  // Retrieve the eventhub connection string so we can extract the host name and entity name.
  std::string eventhubConnectionString
      = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING");
  Azure::Core::Amqp::_internal::ConnectionStringCredential connectionStringCredential(
      eventhubConnectionString, Azure::Core::Amqp::_internal::CredentialType::ServiceBusSas);
  std::string eventhubsHost = connectionStringCredential.GetHostName();
  std::string eventhubsEntity = connectionStringCredential.GetEntityPath();
  if (eventhubsEntity.empty())
  {
    eventhubsEntity = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_NAME");
  }
  std::string eventhubsEntityUrl = "amqps://" + eventhubsHost + "/" + eventhubsEntity;

  Azure::Core::Amqp::_internal::ConnectionOptions connectOptions;
  connectOptions.ContainerId = "some";
  connectOptions.EnableTrace = true;
  Azure::Core::Amqp::_internal::Connection connection(eventhubsEntityUrl, connectOptions);

  Azure::Core::Amqp::_internal::SessionOptions sessionOptions;
  sessionOptions.InitialIncomingWindowSize = std::numeric_limits<int32_t>::max();
  sessionOptions.InitialOutgoingWindowSize = std::numeric_limits<uint16_t>::max();

  Azure::Core::Amqp::_internal::Session session(connection, sessionOptions);

  constexpr int maxMessageSendCount = 1000;

  Azure::Core::Amqp::Models::AmqpMessage message;
  message.SetBody(Azure::Core::Amqp::Models::AmqpValue{"Hello"});

  auto credential{std::make_shared<Azure::Identity::ClientSecretCredential>(
      Azure::Core::_internal::Environment::GetVariable("EVENTHUBS_TENANT_ID"),
      Azure::Core::_internal::Environment::GetVariable("EVENTHUBS_CLIENT_ID"),
      Azure::Core::_internal::Environment::GetVariable("EVENTHUBS_CLIENT_SECRET"))};

  Azure::Core::Amqp::_internal::MessageSenderOptions senderOptions;
  senderOptions.AuthenticationScopes = {EH_AUTHENTICATION_SCOPE};
  senderOptions.MaxMessageSize = std::numeric_limits<uint16_t>::max();
  senderOptions.MessageSource = "ingress";
  senderOptions.Name = "sender-link";
  senderOptions.SettleMode = Azure::Core::Amqp::_internal::SenderSettleMode::Settled;
  senderOptions.EnableTrace = true;
  Azure::Core::Amqp::_internal::MessageSender sender(
      session, credential, eventhubsEntityUrl, senderOptions, nullptr);

  // Open the connection to the remote. This will authenticate the client and connect to the server.
  sender.Open();

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
                    std::chrono::duration_cast<std::chrono::milliseconds>(timeDiff).count()))
          * 1000.0
            << " msgs/sec" << std::endl;

  sender.Close();
}
