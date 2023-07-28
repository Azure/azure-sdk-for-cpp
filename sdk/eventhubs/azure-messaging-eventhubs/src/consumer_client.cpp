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
    m_hostName = sasCredential->GetHostName();
    m_hostUrl = "amqps://" + m_hostName + "/" + m_eventHub + "/ConsumerGroups/" + m_consumerGroup;
  }

  ConsumerClient::ConsumerClient(
      std::string const& hostName,
      std::string const& eventHub,
      std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
      std::string const& consumerGroup,
      ConsumerClientOptions const& options)
      : m_hostName{hostName}, m_eventHub{eventHub}, m_consumerGroup{consumerGroup},
        m_credential{credential}, m_consumerClientOptions(options)
  {
    m_hostUrl = "amqps://" + m_hostName + "/" + m_eventHub + "/ConsumerGroups/" + m_consumerGroup;
  }

  namespace {
    struct FilterDescription
    {
      std::string Name;
      std::uint64_t Code;
    };
    void AddFilterElementToSourceOptions(
        Azure::Core::Amqp::Models::_internal::MessageSourceOptions& sourceOptions,
        FilterDescription description,
        Azure::Core::Amqp::Models::AmqpValue const& filterValue)
    {
      Azure::Core::Amqp::Models::AmqpDescribed value{description.Code, filterValue};
      sourceOptions.Filter.emplace(description.Name, value);
    }

    FilterDescription SelectorFilter{"apache.org:selector-filter:string", 0x0000468c00000004};
  } // namespace

  PartitionClient ConsumerClient::CreatePartitionClient(
      std::string partitionId,
      PartitionClientOptions const& options)
  {
    PartitionClient partitionClient(options, m_consumerClientOptions.RetryOptions);

    std::string suffix = !partitionId.empty() ? "/Partitions/" + partitionId : "";
    std::string hostUrl = m_hostUrl + suffix;

    ConnectionOptions connectOptions;
    connectOptions.ContainerId = m_consumerClientOptions.ApplicationID;
    connectOptions.EnableTrace = true;
    connectOptions.AuthenticationScopes = {"https://eventhubs.azure.net/.default"};

    // Set the user agent related properties in the connectOptions based on the package information
    // and application ID.
    _detail::EventHubUtilities::SetUserAgent(connectOptions, m_consumerClientOptions.ApplicationID);

    Connection connection(m_hostName, m_credential, connectOptions);
    SessionOptions sessionOptions;
    sessionOptions.InitialIncomingWindowSize = static_cast<uint32_t>(
        m_consumerClientOptions.MaxMessageSize.ValueOr(std::numeric_limits<int32_t>::max()));

    Session session{connection.CreateSession(sessionOptions)};

    Azure::Core::Amqp::Models::_internal::MessageSourceOptions sourceOptions;
    sourceOptions.Address = static_cast<Azure::Core::Amqp::Models::AmqpValue>(hostUrl);
    AddFilterElementToSourceOptions(
        sourceOptions,
        SelectorFilter,
        static_cast<Azure::Core::Amqp::Models::AmqpValue>(
            GetStartExpression(options.StartPosition)));

    Azure::Core::Amqp::Models::_internal::MessageSource messageSource(sourceOptions);
    Azure::Core::Amqp::_internal::MessageReceiverOptions receiverOptions;
    if (m_consumerClientOptions.MaxMessageSize)
    {
      receiverOptions.MaxMessageSize = m_consumerClientOptions.MaxMessageSize.Value();
    }
    receiverOptions.EnableTrace = true;
    receiverOptions.MessageTarget = m_consumerClientOptions.MessageTarget;
    receiverOptions.Name = m_consumerClientOptions.Name;
    receiverOptions.Properties.emplace("com.microsoft:receiver-name", m_consumerClientOptions.Name);
    if (options.OwnerLevel.HasValue())
    {
      receiverOptions.Properties.emplace("com.microsoft:epoch", options.OwnerLevel.Value());
    }

    MessageReceiver receiver = session.CreateMessageReceiver(messageSource, receiverOptions);

    // Open the connection to the remote.
    receiver.Open();
    m_sessions.emplace(partitionId, session);
    partitionClient.PushBackReceiver(receiver);
    return partitionClient;
  }

  std::string ConsumerClient::GetStartExpression(Models::StartPosition const& startPosition)
  {
    std::string greaterThan = ">";

    if (startPosition.Inclusive)
    {
      greaterThan = ">=";
    }

    constexpr const char* expressionErrorText
        = "Only a single start point can be set: Earliest, EnqueuedTime, "
          "Latest, Offset, or SequenceNumber";

    std::string returnValue;
    if (startPosition.EnqueuedTime.HasValue())
    {
      returnValue = "amqp.annotation.x--opt-enqueued-time " + greaterThan + "'"
          + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                               startPosition.EnqueuedTime.Value().time_since_epoch())
                               .count())
          + "'";
    }
    if (startPosition.Offset.HasValue())
    {
      if (!returnValue.empty())
      {
        throw std::runtime_error(expressionErrorText);
      }
      returnValue = "amqp.annotation.x-opt-offset " + greaterThan + "'"
          + std::to_string(startPosition.Offset.Value()) + "'";
    }
    if (startPosition.SequenceNumber.HasValue())
    {
      if (!returnValue.empty())
      {
        throw std::runtime_error(expressionErrorText);
      }
      returnValue = "amqp.annotation.x-opt-sequence-number " + greaterThan + "'"
          + std::to_string(startPosition.SequenceNumber.Value()) + "'";
    }
    if (startPosition.Latest.HasValue())
    {
      if (!returnValue.empty())
      {
        throw std::runtime_error(expressionErrorText);
      }
      returnValue = "amqp.annotation.x-opt-offset > '@latest'";
    }
    if (startPosition.Earliest.HasValue())
    {
      if (!returnValue.empty())
      {
        throw std::runtime_error(expressionErrorText);
      }
      returnValue = "amqp.annotation.x-opt-offset > '-1'";
    }
    // If we don't have a filter value, then default to the start.
    if (returnValue.empty())
    {
      return "amqp.annotation.x-opt-offset > '@latest'";
    }
    else
    {
      return returnValue;
    }
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

    return _detail::EventHubUtilities::GetEventHubsProperties(
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

    return _detail::EventHubUtilities::GetEventHubsPartitionProperties(
        m_sessions.at(partitionId), m_eventHub, partitionId, context);
  }
}}} // namespace Azure::Messaging::EventHubs
