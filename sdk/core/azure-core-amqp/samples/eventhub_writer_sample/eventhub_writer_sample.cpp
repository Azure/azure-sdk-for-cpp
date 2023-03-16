// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#include <azure/core/amqp/connection.hpp>
#include <azure/core/amqp/connection_string_credential.hpp>
#include <azure/core/amqp/message_sender.hpp>
#include <chrono>
#include <iostream>
#include <limits>
#include <string>

#define EH_CONNECTION_STRING "<<<Replace with the connection string from your eventhubs instance>>>"

int main()
{
  Azure::Core::_internal::Amqp::ConnectionOptions connectOptions;
  connectOptions.ContainerId = "some";
  connectOptions.EnableTrace = true;
  connectOptions.SaslCredentials
      = std::make_shared<Azure::Core::_internal::Amqp::SaslPlainConnectionStringCredential>(
          EH_CONNECTION_STRING);

  Azure::Core::_internal::Amqp::Connection connection(nullptr, connectOptions);

  Azure::Core::_internal::Amqp::Session session(connection, nullptr);
  session.SetIncomingWindow(std::numeric_limits<int32_t>::max());
  session.SetOutgoingWindow(std::numeric_limits<uint16_t>::max());

  auto timeStart = std::chrono::high_resolution_clock::now();

  constexpr int maxMessageSendCount = 5;

  uint8_t messageBody[] = "Hello";

  Azure::Core::Amqp::Models::Message message;
  message.AddBodyAmqpData({messageBody, sizeof(messageBody)});

  Azure::Core::_internal::Amqp::MessageSenderOptions senderOptions;
  senderOptions.Name = "sender-link";
  senderOptions.SourceAddress = "ingress";
  senderOptions.SenderSettleMode = Azure::Core::_internal::Amqp::SenderSettleMode::Unsettled;
  senderOptions.MaxMessageSize = std::numeric_limits<uint16_t>::max();
  Azure::Core::_internal::Amqp::MessageSender sender(
      session,
      "amqps://" + connectOptions.SaslCredentials->GetHostName() + "/"
          + connectOptions.SaslCredentials->GetEntityPath(),
      connection,
      senderOptions);

  // Open the connection to the remote.
  sender.Open();

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
          * 1000
            << " msgs/sec" << std::endl;

  sender.Close();
}
