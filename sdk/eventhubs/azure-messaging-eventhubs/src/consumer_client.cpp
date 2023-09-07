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

  void ConsumerClient::EnsureSession(std::string const& partitionId)
  {
    if (m_sessions.find(partitionId) == m_sessions.end())
    {
      ConnectionOptions connectOptions;
      connectOptions.ContainerId = m_consumerClientOptions.ApplicationID;
      connectOptions.EnableTrace = true;
      connectOptions.AuthenticationScopes = {"https://eventhubs.azure.net/.default"};

      // Set the user agent related properties in the connectOptions based on the package
      // information and application ID.
      _detail::EventHubsUtilities::SetUserAgent(
          connectOptions, m_consumerClientOptions.ApplicationID);

      Connection connection(m_fullyQualifiedNamespace, m_credential, connectOptions);
      SessionOptions sessionOptions;
      sessionOptions.InitialIncomingWindowSize
          = static_cast<uint32_t>(std::numeric_limits<int32_t>::max());

      Session session{connection.CreateSession(sessionOptions)};
      m_sessions.emplace(partitionId, session);
    }
  }

  Azure::Core::Amqp::_internal::Session ConsumerClient::GetSession(std::string const& partitionId)
  {
    return m_sessions.at(partitionId);
  }

  PartitionClient ConsumerClient::CreatePartitionClient(
      std::string const& partitionId,
      PartitionClientOptions const& options,
      Azure::Core::Context const& context)
  {
    std::string suffix = !partitionId.empty() ? "/Partitions/" + partitionId : "";
    std::string hostUrl = m_hostUrl + suffix;

    EnsureSession(partitionId);

    return _detail::PartitionClientFactory::CreatePartitionClient(
        GetSession(partitionId),
        hostUrl,
        m_consumerClientOptions.Name,
        options,
        m_consumerClientOptions.RetryOptions,
        context);
  }

  Models::EventHubProperties ConsumerClient::GetEventHubProperties(Core::Context const& context)
  {
    // Since EventHub properties are not tied to a partition, we don't specify a partition ID.
    EnsureSession();

    return _detail::EventHubsUtilities::GetEventHubsProperties(GetSession(), m_eventHub, context);
  }

  Models::EventHubPartitionProperties ConsumerClient::GetPartitionProperties(
      std::string const& partitionId,
      Core::Context const& context)
  {
    EnsureSession(partitionId);

    return _detail::EventHubsUtilities::GetEventHubsPartitionProperties(
        GetSession(partitionId), m_eventHub, partitionId, context);
  }
}}} // namespace Azure::Messaging::EventHubs
