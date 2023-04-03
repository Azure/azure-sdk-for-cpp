// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <azure/core/amqp/connection.hpp>
#include <azure/core/amqp/message_receiver.hpp>
#include <azure/core/amqp/network/sasl_transport.hpp>
#include <chrono>
#include <iostream>
#include <limits>
#include <string>

#define EH_CONNECTION_STRING "<<<Replace with the connection string from your eventhubs instance>>>"

int main()
{
  auto credential
      = std::make_shared<Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential>(
          EH_CONNECTION_STRING);
  std::string hostUrl = "amqps://" + credential->GetHostName() + "/" + credential->GetEntityPath()
      + "/ConsumerGroups/$Default/Partitions/0";
  Azure::Core::Amqp::_internal::ConnectionOptions connectOptions;
  connectOptions.ContainerId = "whatever";
  connectOptions.EnableTrace = true;
  connectOptions.HostName = credential->GetHostName();
  Azure::Core::Amqp::_internal::Connection connection(hostUrl, connectOptions);

  Azure::Core::Amqp::_internal::Session session(connection, nullptr);
  session.SetIncomingWindow(100);

  Azure::Core::Amqp::_internal::MessageReceiverOptions receiverOptions;
  receiverOptions.Name = "receiver-link";
  receiverOptions.TargetAddress = "ingress-rx";
  receiverOptions.SettleMode = Azure::Core::Amqp::_internal::ReceiverSettleMode::First;
  receiverOptions.MaxMessageSize = std::numeric_limits<uint16_t>::max();
  receiverOptions.EnableTrace = true;

  Azure::Core::Amqp::_internal::MessageReceiver receiver(
      session, connection, credential, hostUrl, receiverOptions);

  // Open the connection to the remote.
  receiver.Open();

  auto timeStart = std::chrono::high_resolution_clock::now();

  constexpr int maxMessageReceiveCount = 1000;

  int messageReceiveCount = 0;
  while (messageReceiveCount < maxMessageReceiveCount)
  {
    auto message = receiver.WaitForIncomingMessage(connection);
    std::cout << "Received message: " << message << std::endl;
    messageReceiveCount += 1;
  }

  auto timeEnd = std::chrono::high_resolution_clock::now();
  std::chrono::nanoseconds timeDiff = timeEnd - timeStart;

  std::cout << "Received " << messageReceiveCount << " in "
            << std::chrono::duration_cast<std::chrono::milliseconds>(timeDiff).count()
            << " milliseconds. "
            << (static_cast<float>(messageReceiveCount)
                / static_cast<float>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(timeDiff).count()))
          * 1000
            << " msgs/sec" << std::endl;

  receiver.Close();
}
