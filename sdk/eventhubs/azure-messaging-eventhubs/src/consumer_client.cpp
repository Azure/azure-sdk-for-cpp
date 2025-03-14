// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "private/eventhubs_constants.hpp"
#include "private/eventhubs_utilities.hpp"
#include "private/package_version.hpp"

#include <azure/core/amqp/internal/message_receiver.hpp>
#include <azure/messaging/eventhubs.hpp>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;
using namespace Azure::Messaging::EventHubs::Models;
using namespace Azure::Core::Amqp::_internal;

namespace Azure { namespace Messaging { namespace EventHubs {

  ConsumerClient::ConsumerClient(
      std::string const& fullyQualifiedNamespace,
      std::string const& eventHub,
      std::shared_ptr<const Azure::Core::Credentials::TokenCredential> credential,
      std::string const& consumerGroup,
      ConsumerClientOptions const& options)
      : m_fullyQualifiedNamespace{fullyQualifiedNamespace}, m_eventHub{eventHub},
        m_consumerGroup{consumerGroup}, m_credential{credential}, m_consumerClientOptions(options)
  {
    m_hostUrl = _detail::EventHubsServiceScheme + m_fullyQualifiedNamespace + "/" + m_eventHub
        + _detail::EventHubsConsumerGroupsPath + m_consumerGroup;
  }

  ConsumerClient::~ConsumerClient()
  {
    Log::Stream(Logger::Level::Informational) << "Destroy consumer client.";

    Close({});
  }

  void ConsumerClient::Close(Azure::Core::Context const& context)
  {
    Log::Stream(Logger::Level::Verbose) << "Close producer client.";
    {
      std::unique_lock<std::mutex> lock(m_propertiesClientLock);
      if (m_propertiesClient)
      {
        m_propertiesClient->Close(context);
        m_propertiesClient.reset();
      }
    }
    Log::Stream(Logger::Level::Verbose) << "Closing message senders.";
    // Tear down the sessions and then the connections, in that order.
    for (auto& receiver : m_receivers)
    {
      receiver.second.Close(context);
    }

#if ENABLE_RUST_AMQP
    Log::Stream(Logger::Level::Verbose) << "Closing sessions.";
    for (auto& session : m_sessions)
    {
      session.second.End(context);
    }
    Log::Stream(Logger::Level::Verbose) << "Closing connections.";
    for (auto& connection : m_connections)
    {
      connection.second.Close(context);
    }
#endif

    while (!m_sessions.empty())
    {
      m_sessions.erase(m_sessions.begin());
    }

    while (!m_connections.empty())
    {
      m_connections.erase(m_connections.begin());
    };
    m_receivers.clear();
  }

  Azure::Core::Amqp::_internal::Connection ConsumerClient::CreateConnection(
      std::string const& partitionId,
      Azure::Core::Context const& context) const
  {
    ConnectionOptions connectOptions;
    connectOptions.ContainerId
        = "Consumer for " + m_consumerClientOptions.ApplicationID + " on " + partitionId;
    connectOptions.EnableTrace = _detail::EnableAmqpTrace;
    connectOptions.AuthenticationScopes = {"https://eventhubs.azure.net/.default"};
    connectOptions.Port = m_targetPort;

    // Set the user agent related properties in the connectOptions based on the package
    // information and application ID.
    _detail::EventHubsUtilities::SetUserAgent(
        connectOptions,
        m_consumerClientOptions.ApplicationID,
        m_consumerClientOptions.CppStandardVersion);

    auto connection{Azure::Core::Amqp::_internal::Connection{
        m_fullyQualifiedNamespace, m_credential, connectOptions}};
#if ENABLE_RUST_AMQP
    connection.Open(context);
#endif
    return connection;
    (void)context;
  }

  void ConsumerClient::EnsureConnection(
      std::string const& partitionId,
      Azure::Core::Context const& context)
  {
    std::unique_lock<std::recursive_mutex> lock(m_sessionsLock);
    if (m_connections.find(partitionId) == m_connections.end())
    {
      m_connections.emplace(partitionId, CreateConnection(partitionId, context));
    }
  }

  Azure::Core::Amqp::_internal::Session ConsumerClient::CreateSession(
      std::string const& partitionId,
      Azure::Core::Context const& context) const
  {
    SessionOptions sessionOptions;
    sessionOptions.InitialIncomingWindowSize
        = static_cast<uint32_t>((std::numeric_limits<int32_t>::max)());

    auto session{m_connections.at(partitionId).CreateSession(sessionOptions)};
#if ENABLE_RUST_AMQP
    session.Begin(context);
#endif
    return session;
    (void)context;
  }

  void ConsumerClient::EnsureSession(
      std::string const& partitionId,
      Azure::Core::Context const& context)
  {
    EnsureConnection(partitionId, context);
    std::unique_lock<std::recursive_mutex> lock(m_sessionsLock);
    if (m_sessions.find(partitionId) == m_sessions.end())
    {
      m_sessions.emplace(partitionId, CreateSession(partitionId, context));
    }
  }

  Azure::Core::Amqp::_internal::Session ConsumerClient::GetSession(
      std::string const& partitionId = {})
  {
    std::unique_lock<std::recursive_mutex> lock(m_sessionsLock);
    return m_sessions.at(partitionId);
  }

  std::shared_ptr<_detail::EventHubsPropertiesClient> ConsumerClient::GetPropertiesClient(
      Azure::Core::Context const& context)
  {
    std::lock_guard<std::mutex> lock(m_propertiesClientLock);
    EnsureConnection({}, context);
    if (!m_propertiesClient)
    {
      m_propertiesClient
          = std::make_shared<_detail::EventHubsPropertiesClient>(m_connections.at(""), m_eventHub);
    }
    return m_propertiesClient;
  }

  PartitionClient ConsumerClient::CreatePartitionClient(
      std::string const& partitionId,
      PartitionClientOptions const& options,
      Azure::Core::Context const& context)
  {
    std::string suffix = !partitionId.empty() ? "/Partitions/" + partitionId : "";
    std::string hostUrl = m_hostUrl + suffix;

    EnsureSession(partitionId, context);

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
    return GetPropertiesClient(context)->GetEventHubsProperties(m_eventHub, context);
  }

  Models::EventHubPartitionProperties ConsumerClient::GetPartitionProperties(
      std::string const& partitionId,
      Core::Context const& context)
  {
    return GetPropertiesClient(context)->GetEventHubsPartitionProperties(
        m_eventHub, partitionId, context);
  }
}}} // namespace Azure::Messaging::EventHubs
