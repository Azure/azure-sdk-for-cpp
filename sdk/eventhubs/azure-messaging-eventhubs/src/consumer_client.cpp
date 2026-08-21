// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "private/best_effort_cleanup.hpp"
#include "private/eventhubs_constants.hpp"
#include "private/eventhubs_diagnostics.hpp"
#include "private/eventhubs_tracing.hpp"
#include "private/eventhubs_utilities.hpp"
#include "private/package_version.hpp"

#include <azure/core/amqp/internal/message_receiver.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <stdexcept>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;
using namespace Azure::Messaging::EventHubs::Models;
using namespace Azure::Core::Amqp::_internal;

namespace {
Azure::Messaging::EventHubs::_detail::AmqpDiagnosticsContext CreateDiagnosticsContext(
    std::string const& clientId,
    std::string const& partitionId,
    std::string componentType,
    std::string componentName)
{
  return Azure::Messaging::EventHubs::_detail::AmqpDiagnosticsContext{
      clientId, partitionId, std::move(componentType), std::move(componentName), 1};
}

std::string CreateConnectionName(
    std::string const& clientId,
    std::string const& applicationId,
    std::string const& partitionId)
{
  return clientId + (applicationId.empty() ? std::string{} : "/application/" + applicationId)
      + "/partition/" + (partitionId.empty() ? std::string("<gateway>") : partitionId)
      + "/generation/1";
}
} // namespace

namespace Azure { namespace Messaging { namespace EventHubs {

  ConsumerClient::ConsumerClient(
      std::string const& connectionString,
      std::string const& eventHub,
      std::string const& consumerGroup,
      ConsumerClientOptions const& options)
      : m_connectionString{connectionString}, m_eventHub{eventHub}, m_consumerGroup{consumerGroup},
        m_consumerClientOptions(options),
        m_clientIdentifier{_detail::CreateClientIdentifier("consumer", options.Name)},
        m_tracingFactory{_detail::CreateTracingContextFactory(options.TracingProvider)}
  {
    auto details
        = _detail::EventHubsUtilities::CreateConnectionStringDetails(connectionString, eventHub);
    m_credential = std::move(details.Credential);
    m_eventHub = std::move(details.EventHub);
    m_fullyQualifiedNamespace = std::move(details.FullyQualifiedNamespace);
    m_targetPort = details.Port;
    m_hostUrl = details.ServiceScheme + m_fullyQualifiedNamespace + "/" + m_eventHub
        + _detail::EventHubsConsumerGroupsPath + m_consumerGroup;
    if (m_consumerClientOptions.Name.empty())
    {
      m_consumerClientOptions.Name = m_clientIdentifier;
    }
  }

  ConsumerClient::ConsumerClient(
      std::string const& fullyQualifiedNamespace,
      std::string const& eventHub,
      std::shared_ptr<const Azure::Core::Credentials::TokenCredential> credential,
      std::string const& consumerGroup,
      ConsumerClientOptions const& options)
      : m_fullyQualifiedNamespace{fullyQualifiedNamespace}, m_eventHub{eventHub},
        m_consumerGroup{consumerGroup}, m_credential{credential}, m_consumerClientOptions(options),
        m_clientIdentifier{_detail::CreateClientIdentifier("consumer", options.Name)},
        m_tracingFactory{_detail::CreateTracingContextFactory(options.TracingProvider)}
  {
    m_hostUrl = _detail::EventHubsServiceScheme + m_fullyQualifiedNamespace + "/" + m_eventHub
        + _detail::EventHubsConsumerGroupsPath + m_consumerGroup;
    if (m_consumerClientOptions.Name.empty())
    {
      m_consumerClientOptions.Name = m_clientIdentifier;
    }
  }

  ConsumerClient::~ConsumerClient()
  {
    try
    {
      Log::Stream(Logger::Level::Informational) << "Destroy consumer client.";

      Close({});
    }
    catch (std::exception const& ex)
    {
      Log::Stream(Logger::Level::Warning)
          << "Exception in ConsumerClient::~ConsumerClient(): " << ex.what();
    }
  }

  void ConsumerClient::Close(Azure::Core::Context const& context)
  {
    Log::Stream(Logger::Level::Verbose) << "Close consumer client.";
    {
      std::unique_lock<std::mutex> lock(m_propertiesClientLock);
      if (m_propertiesClient)
      {
        try
        {
          m_propertiesClient->Close(context);
        }
        catch (std::exception const& ex)
        {
          Log::Stream(Logger::Level::Warning)
              << "Exception while closing the consumer properties client: " << ex.what();
        }
        m_propertiesClient.reset();
      }
    }
    Log::Stream(Logger::Level::Verbose) << "Closing message receivers.";
    // Tear down the sessions and then the connections, in that order.
    _detail::ForEachBestEffort(
        m_receivers.begin(),
        m_receivers.end(),
        [&context](decltype(m_receivers)::value_type& receiver) { receiver.second.Close(context); },
        [](decltype(m_receivers)::value_type& receiver, std::exception const& ex) {
          Log::Stream(Logger::Level::Warning)
              << "Exception while closing the message receiver for partition " << receiver.first
              << ": " << ex.what();
        });

#if ENABLE_RUST_AMQP
    Log::Stream(Logger::Level::Verbose) << "Closing sessions.";
    _detail::ForEachBestEffort(
        m_sessions.begin(),
        m_sessions.end(),
        [&context](decltype(m_sessions)::value_type& session) { session.second.End(context); },
        [](decltype(m_sessions)::value_type& session, std::exception const& ex) {
          Log::Stream(Logger::Level::Warning) << "Exception while ending the session for partition "
                                              << session.first << ": " << ex.what();
        });
    Log::Stream(Logger::Level::Verbose) << "Closing connections.";
    _detail::ForEachBestEffort(
        m_connections.begin(),
        m_connections.end(),
        [&context](decltype(m_connections)::value_type& connection) {
          connection.second.Close(context);
        },
        [](decltype(m_connections)::value_type& connection, std::exception const& ex) {
          Log::Stream(Logger::Level::Warning)
              << "Exception while closing the connection for partition " << connection.first << ": "
              << ex.what();
        });
#endif

    m_sessions.clear();
    m_connections.clear();
    m_receivers.clear();
  }

  Azure::Core::Amqp::_internal::Connection ConsumerClient::CreateConnection(
      std::string const& partitionId,
      Azure::Core::Context const& context) const
  {
    ConnectionOptions connectOptions;
    connectOptions.ContainerId = CreateConnectionName(
        m_clientIdentifier, m_consumerClientOptions.ApplicationID, partitionId);
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
      auto diagnosticsContext = CreateDiagnosticsContext(
          m_clientIdentifier,
          partitionId,
          "connection",
          CreateConnectionName(
              m_clientIdentifier, m_consumerClientOptions.ApplicationID, partitionId));
      _detail::LogAmqpLifecycle(Logger::Level::Verbose, diagnosticsContext, "creating");
      try
      {
        m_connections.emplace(partitionId, CreateConnection(partitionId, context));
        _detail::LogAmqpLifecycle(Logger::Level::Verbose, diagnosticsContext, "created");
      }
      catch (std::exception const& ex)
      {
        _detail::LogAmqpLifecycle(
            Logger::Level::Warning, diagnosticsContext, "create_failed", ex.what());
        throw;
      }
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
      auto diagnosticsContext
          = CreateDiagnosticsContext(m_clientIdentifier, partitionId, "session", partitionId);
      _detail::LogAmqpLifecycle(Logger::Level::Verbose, diagnosticsContext, "creating");
      try
      {
        m_sessions.emplace(partitionId, CreateSession(partitionId, context));
        _detail::LogAmqpLifecycle(Logger::Level::Verbose, diagnosticsContext, "created");
      }
      catch (std::exception const& ex)
      {
        _detail::LogAmqpLifecycle(
            Logger::Level::Warning, diagnosticsContext, "create_failed", ex.what());
        throw;
      }
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
        m_clientIdentifier,
        partitionId,
        options,
        m_consumerClientOptions.RetryOptions,
        m_tracingFactory,
        m_eventHub,
        m_fullyQualifiedNamespace,
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
