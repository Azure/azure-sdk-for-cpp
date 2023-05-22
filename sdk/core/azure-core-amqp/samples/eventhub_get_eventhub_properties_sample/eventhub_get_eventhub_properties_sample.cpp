// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <azure/core/amqp/connection.hpp>
#include <azure/core/amqp/connection_string_credential.hpp>
#include <azure/core/amqp/management.hpp>
#include <azure/core/amqp/message_sender.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <chrono>
#include <iostream>
#include <limits>
#include <string>

#define EH_AUTHENTICATION_SCOPE "https://eventhubs.azure.net/.default"

int main()
{
  // Retrieve the eventhub connection string so we can extract the host name and entity name. We are
  // NOT using the connection string to authenticate with the eventhub, only to retrieve the host
  // name and entity (if present).
  std::string eventhubConnectionString
      = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING");

  Azure::Core::Amqp::_internal::ConnectionStringParser connectionParser(eventhubConnectionString);
  std::string eventhubsHost = connectionParser.GetHostName();
  std::string eventhubsEntity = connectionParser.GetEntityPath();
  if (eventhubsEntity.empty())
  {
    eventhubsEntity = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_NAME");
  }

  // Establish the connection to the eventhub.
  Azure::Core::Amqp::_internal::ConnectionOptions connectOptions;
  connectOptions.ContainerId = "some";
  connectOptions.EnableTrace = true;
  connectOptions.Port = connectionParser.GetPort();
  Azure::Core::Amqp::_internal::Connection connection(
      connectionParser.GetHostName(), connectOptions);

  auto credential{std::make_shared<Azure::Identity::ClientSecretCredential>(
      Azure::Core::_internal::Environment::GetVariable("EVENTHUBS_TENANT_ID"),
      Azure::Core::_internal::Environment::GetVariable("EVENTHUBS_CLIENT_ID"),
      Azure::Core::_internal::Environment::GetVariable("EVENTHUBS_CLIENT_SECRET"))};

  // Establish a session to the eventhub.
  Azure::Core::Amqp::_internal::SessionOptions sessionOptions;
  sessionOptions.InitialIncomingWindowSize = std::numeric_limits<int32_t>::max();
  sessionOptions.InitialOutgoingWindowSize = std::numeric_limits<uint16_t>::max();
  sessionOptions.AuthenticationScopes = {EH_AUTHENTICATION_SCOPE};
  Azure::Core::Amqp::_internal::Session session(connection, credential, sessionOptions);

  // Create a management client off the session.
  // Eventhubs management APIs return a status code in the "status-code" application properties.
  Azure::Core::Amqp::_internal::ManagementOptions managementClientOptions;
  managementClientOptions.EnableTrace = true;
  managementClientOptions.ExpectedStatusCodeKeyName = "status-code";
  Azure::Core::Amqp::_internal::Management managementClient(
      session, eventhubsEntity, managementClientOptions);

  managementClient.Open();

  // Send a message to the management endpoint to retrieve the properties of the eventhub.
  Azure::Core::Amqp::Models::AmqpMessage message;
  message.ApplicationProperties["name"] = Azure::Core::Amqp::Models::AmqpValue{eventhubsEntity};
  message.SetBody(Azure::Core::Amqp::Models::AmqpValue{});
  auto result = managementClient.ExecuteOperation(
      "READ" /* operation */,
      "com.microsoft:eventhub" /* type of operation */,
      "" /* locales */,
      message);

  managementClient.Close();

  std::cout << "Management endpoint properties: " << result.Message;
}
