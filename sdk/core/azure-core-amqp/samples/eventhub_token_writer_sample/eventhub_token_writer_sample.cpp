// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <azure/core/amqp/connection.hpp>
#include <azure/core/amqp/connection_string_credential.hpp>
#include <azure/core/amqp/message_sender.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <azure/identity/environment_credential.hpp>
#include <chrono>
#include <iostream>
#include <limits>
#include <string>

#define EH_HOST "<<<Replace with the eventhubs host name>>>"
#define EH_ENTITY "<<<Replace with the eventhub name>>>"

#define EH_AUTHENTICATION_SCOPE "https://eventhubs.azure.net/.default"

#define EH_ENTITY_URL "amqps://" EH_HOST "/" EH_ENTITY

int main()
{

  Azure::Core::_internal::Amqp::ConnectionOptions connectOptions;
  connectOptions.ContainerId = "some";
  connectOptions.EnableTrace = true;
  connectOptions.HostName = EH_HOST;
  Azure::Core::_internal::Amqp::Connection connection(nullptr, connectOptions);

  Azure::Core::_internal::Amqp::Session session(connection, nullptr);
  session.SetIncomingWindow(std::numeric_limits<int32_t>::max());
  session.SetOutgoingWindow(std::numeric_limits<uint16_t>::max());

  constexpr int maxMessageSendCount = 1000;

  uint8_t messageBody[] = "Hello";

  Azure::Core::Amqp::Models::Message message;
  message.AddBodyAmqpData({messageBody, sizeof(messageBody)});

  auto credential{std::make_shared<Azure::Identity::ClientSecretCredential>(
      Azure::Core::_internal::Environment::GetVariable("EVENTHUB_TENANT_ID"),
      Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CLIENT_ID"),
      Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CLIENT_SECRET"))};

  Azure::Core::_internal::Amqp::MessageSenderOptions senderOptions;
  senderOptions.AuthenticationScopes = {EH_AUTHENTICATION_SCOPE};
  senderOptions.MaxMessageSize = std::numeric_limits<uint16_t>::max();
  senderOptions.SourceAddress = "ingress";
  senderOptions.Name = "sender-link";
  senderOptions.SettleMode = Azure::Core::_internal::Amqp::SenderSettleMode::Settled;
  senderOptions.EnableTrace = true;
  Azure::Core::_internal::Amqp::MessageSender sender(
      session, credential, EH_ENTITY_URL, connection, senderOptions);

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
