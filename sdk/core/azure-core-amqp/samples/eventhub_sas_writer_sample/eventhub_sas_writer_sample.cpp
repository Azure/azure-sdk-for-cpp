// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <azure/core/amqp/connection.hpp>
#include <azure/core/amqp/connection_string_credential.hpp>
#include <azure/core/amqp/message_sender.hpp>
#include <chrono>
#include <iostream>
#include <limits>
#include <string>

// Note: The connection string provided must either have an "EntityPath" entry or the constructor
// for the SasConnectionStringCredential has to have an entity path provided.
#define EH_CONNECTION_STRING "<<<Replace with the connection string from your eventhubs instance>>>"

int main()
{
  auto credential{
      std::make_shared<Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential>(
          EH_CONNECTION_STRING)};
  std::string eventUrl = "amqps://" + credential->GetHostName() + "/" + credential->GetEntityPath();
  Azure::Core::Amqp::_internal::ConnectionOptions connectOptions;
  connectOptions.ContainerId = "some";
  connectOptions.EnableTrace = true;
  connectOptions.HostName = credential->GetHostName();
  Azure::Core::Amqp::_internal::Connection connection(eventUrl, connectOptions);

  Azure::Core::Amqp::_internal::Session session(connection, nullptr);
  session.SetIncomingWindow(std::numeric_limits<int32_t>::max());
  session.SetOutgoingWindow(std::numeric_limits<uint16_t>::max());

  constexpr int maxMessageSendCount = 1000;
  uint8_t messageBody[] = "Hello";
  Azure::Core::Amqp::Models::Message message;
  message.AddBodyAmqpData({messageBody, sizeof(messageBody)});

  Azure::Core::Amqp::_internal::MessageSenderOptions senderOptions;
  senderOptions.EnableTrace = true;
  senderOptions.Name = "sender-link";
  senderOptions.SourceAddress = "ingress";
  senderOptions.SettleMode = Azure::Core::Amqp::_internal::SenderSettleMode::Settled;
  senderOptions.MaxMessageSize = std::numeric_limits<uint16_t>::max();

  Azure::Core::Amqp::_internal::MessageSender sender(
      session, credential, eventUrl, connection, senderOptions, nullptr);

  // Open the connection to the remote.
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
