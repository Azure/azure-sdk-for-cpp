// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "private/eventhubs_utilities.hpp"
#include "private/package_version.hpp"

#include <azure/core/amqp/message_receiver.hpp>
#include <azure/core/platform.hpp>
#include <azure/messaging/eventhubs.hpp>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;
using namespace Azure::Messaging::EventHubs::Models;
using namespace Azure::Core::Amqp::_internal;

namespace Azure { namespace Messaging { namespace EventHubs {
  ConsumerClient::ConsumerClient(
      std::string const& connectionString,
      std::string const& eventHub,
      std::string const& consumerGroup,
      ConsumerClientOptions const& options)
      : m_connectionString{connectionString}, m_eventHub{eventHub}, m_consumerGroup{consumerGroup},
        m_consumerClientOptions(options)
  {
    auto sasCredential
        = std::make_shared<ServiceBusSasConnectionStringCredential>(m_connectionString);

    m_credential = sasCredential;
    if (!sasCredential->GetEntityPath().empty())
    {
      m_eventHub = sasCredential->GetEntityPath();
    }
    m_fullyQualifiedNamespace = sasCredential->GetHostName();
    m_hostUrl = "amqps://" + m_fullyQualifiedNamespace + "/" + m_eventHub + "/ConsumerGroups/"
        + m_consumerGroup;
  }

  ConsumerClient::ConsumerClient(
      std::string const& fullyQualifiedNamespace,
      std::string const& eventHub,
      std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
      std::string const& consumerGroup,
      ConsumerClientOptions const& options)
      : m_fullyQualifiedNamespace{fullyQualifiedNamespace}, m_eventHub{eventHub},
        m_consumerGroup{consumerGroup}, m_credential{credential}, m_consumerClientOptions(options)
  {
    m_hostUrl = "amqps://" + m_fullyQualifiedNamespace + "/" + m_eventHub + "/ConsumerGroups/"
        + m_consumerGroup;
  }

  PartitionClient ConsumerClient::CreatePartitionClient(
      std::string partitionId,
      PartitionClientOptions const& options,
      Azure::Core::Context const& context)
  {
    std::string suffix = !partitionId.empty() ? "/Partitions/" + partitionId : "";
    std::string hostUrl = m_hostUrl + suffix;

    ConnectionOptions connectOptions;
    connectOptions.ContainerId = m_consumerClientOptions.ApplicationID;
    connectOptions.EnableTrace = true;
    connectOptions.AuthenticationScopes = {"https://eventhubs.azure.net/.default"};

    // Set the user agent related properties in the connectOptions based on the package information
    // and application ID.
    _detail::EventHubsUtilities::SetUserAgent(
        connectOptions, m_consumerClientOptions.ApplicationID);

    Connection connection(m_fullyQualifiedNamespace, m_credential, connectOptions);
    SessionOptions sessionOptions;
    sessionOptions.InitialIncomingWindowSize
        = static_cast<uint32_t>(std::numeric_limits<int32_t>::max());

    Session session{connection.CreateSession(sessionOptions)};
    m_sessions.emplace(partitionId, session);
    

    return _detail::PartitionClientFactory::CreatePartitionClient(
        session,
        hostUrl,
        m_consumerClientOptions.Name,
        options,
        m_consumerClientOptions.RetryOptions,
        context);
  }

  Models::EventHubProperties ConsumerClient::GetEventHubProperties(Core::Context const& context)
  {
    // We need to capture the partition client here, because we need to keep it alive across the
    // call to GetEventHubsProperties.
    //
    // If we don't keep the PartitionClient alive, the message receiver inside the partition client
    // will be disconnected AFTER the outgoing ATTACH frame is sent. When the response for the
    // ATTACH frame is received, it creates a new link_endpoint which is in the half attached state.
    // This runs into a uAMQP bug where an incoming link detach frame will cause a crash if the
    // corresponding link_endpoint is in the half attached state.
    std::shared_ptr<PartitionClient> client;
    if (m_sessions.find("0") == m_sessions.end())
    {
      client = std::make_shared<PartitionClient>(CreatePartitionClient("0"));
    }

    return _detail::EventHubsUtilities::GetEventHubsProperties(
        m_sessions.at("0"), m_eventHub, context);
  }

  Models::EventHubPartitionProperties ConsumerClient::GetPartitionProperties(
      std::string const& partitionId,
      Core::Context const& context)
  {
    if (m_sessions.find(partitionId) == m_sessions.end())
    {
      CreatePartitionClient(partitionId);
    }

    return _detail::EventHubsUtilities::GetEventHubsPartitionProperties(
        m_sessions.at(partitionId), m_eventHub, partitionId, context);
  }
}}} // namespace Azure::Messaging::EventHubs
