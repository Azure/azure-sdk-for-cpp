// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <azure/core/amqp/connection.hpp>
#include <azure/core/amqp/connection_string_credential.hpp>
#include <azure/core/amqp/management.hpp>
#include <azure/core/amqp/message_sender.hpp>
#include <azure/core/internal/environment.hpp>
#include <chrono>
#include <iostream>
#include <limits>
#include <string>

int main()
{
  std::string eventhubConnectionString
      = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING");

  auto credential{
      std::make_shared<Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential>(
          eventhubConnectionString)};
  std::string entityPath = credential->GetEntityPath();
  if (entityPath.empty())
  {
    entityPath = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_NAME");
  }
  std::string eventUrl = "amqps://" + credential->GetHostName() + "/" + entityPath;
  Azure::Core::Amqp::_internal::ConnectionOptions connectOptions;
  connectOptions.ContainerId = "some";
  connectOptions.EnableTrace = true;
  Azure::Core::Amqp::_internal::Connection connection(eventUrl, connectOptions);

  Azure::Core::Amqp::_internal::SessionOptions sessionOptions;
  sessionOptions.InitialIncomingWindowSize = std::numeric_limits<int32_t>::max();
  sessionOptions.InitialOutgoingWindowSize = std::numeric_limits<uint16_t>::max();

  Azure::Core::Amqp::_internal::Session session(connection, sessionOptions);

  Azure::Core::Amqp::_internal::ManagementOptions managementClientOptions;
  managementClientOptions.EnableTrace = true;
  Azure::Core::Amqp::_internal::Management managementClient(
      session, "$management", managementClientOptions);

  managementClient.Open();

  Azure::Core::Amqp::Models::AmqpMessage message;
  message.ApplicationProperties["name"] = Azure::Core::Amqp::Models::AmqpValue{entityPath};

  auto result
      = managementClient.ExecuteOperation("READ", "com.microsoft:eventhub", "", message);

  managementClient.Close();

  std::cout << "Management endpoint properties: " << result.Message;

}
