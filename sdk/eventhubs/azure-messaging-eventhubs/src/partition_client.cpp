// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/messaging/eventhubs/partition_client.hpp"

#include "azure/messaging/eventhubs/eventhubs_exception.hpp"
#include "private/eventhubs_utilities.hpp"
#include "private/retry_operation.hpp"

#include <azure/core/amqp.hpp>

namespace Azure { namespace Messaging { namespace EventHubs {
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

    std::string GetStartExpression(Models::StartPosition const& startPosition)
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

    // Helper function to create a message receiver.
    Azure::Core::Amqp::_internal::MessageReceiver CreateMessageReceiver(
        Azure::Core::Amqp::_internal::Session const& session,
        std::string const& partitionUrl,
        std::string const& receiverName,
        PartitionClientOptions const& options,
        Azure::Core::Amqp::_internal::MessageReceiverEvents* events = nullptr)
    {
      Azure::Core::Amqp::Models::_internal::MessageSourceOptions sourceOptions;
      sourceOptions.Address = static_cast<Azure::Core::Amqp::Models::AmqpValue>(partitionUrl);
      AddFilterElementToSourceOptions(
          sourceOptions,
          SelectorFilter,
          static_cast<Azure::Core::Amqp::Models::AmqpValue>(
              GetStartExpression(options.StartPosition)));

      Azure::Core::Amqp::Models::_internal::MessageSource messageSource(sourceOptions);
      Azure::Core::Amqp::_internal::MessageReceiverOptions receiverOptions;

      receiverOptions.EnableTrace = true;
      // Set the link credit to the prefetch count. If the user has not set a prefetch count, then
      // we will use the default value.
      if (options.Prefetch >= 0)
      {
        receiverOptions.MaxLinkCredit = options.Prefetch;
      }
      receiverOptions.Name = receiverName;
      receiverOptions.Properties.emplace("com.microsoft:receiver-name", receiverName);
      if (options.OwnerLevel.HasValue())
      {
        receiverOptions.Properties.emplace("com.microsoft:epoch", options.OwnerLevel.Value());
      }
      return session.CreateMessageReceiver(messageSource, receiverOptions, events);
    }
  } // namespace

  PartitionClient::PartitionClient(
      Azure::Core::Amqp::_internal::Session const& session,
      std::string const& partitionUrl,
      std::string const& receiverName,
      PartitionClientOptions options,
      Azure::Core::Http::Policies::RetryOptions retryOptions)
      : m_receiver{CreateMessageReceiver(session, partitionUrl, receiverName, options)},
        m_partitionOptions{options}, m_retryOptions{retryOptions}

  {
    // Open the connection to the remote.
    m_receiver.Open();
  }

  /** Receive events from the partition.
   *
   * @param maxMessages The maximum number of messages to receive.
   * @param context A context to control the request lifetime.
   * @return A vector of received events.
   *
   */
  std::vector<Models::ReceivedEventData> PartitionClient::ReceiveEvents(
      uint32_t maxMessages,
      Core::Context const& context)
  {
    std::vector<Models::ReceivedEventData> messages;
    // bool prefetchDisabled = m_prefetchCount < 0;

    while (messages.size() < maxMessages && !context.IsCancelled())
    {
      auto message = m_receiver.WaitForIncomingMessage(context);
      if (message.first.HasValue())
      {
        messages.push_back(Models::ReceivedEventData{message.first.Value()});
      }
      else
      {
        throw _detail::EventHubsExceptionFactory::CreateEventHubsException(message.second);
      }
    }
    return messages;
  }
  void PartitionClient::OnMessageReceiverStateChanged(
      Azure::Core::Amqp::_internal::MessageReceiver const& receiver,
      Azure::Core::Amqp::_internal::MessageReceiverState newState,
      Azure::Core::Amqp::_internal::MessageReceiverState oldState)
  {
    (void)receiver;
    (void)newState;
    (void)oldState;
  }
  Azure::Core::Amqp::Models::AmqpValue PartitionClient::OnMessageReceived(
      Azure::Core::Amqp::_internal::MessageReceiver const& receiver,
      Azure::Core::Amqp::Models::AmqpMessage const& message)
  {
    (void)receiver;
    (void)message;
    // Queue the incoming message to the received message queue.
    //    m_receivedMessageQueue.CompleteOperation(message, {});
    return Azure::Core::Amqp::Models::_internal::Messaging::DeliveryAccepted();
  }
  void PartitionClient::OnMessageReceiverDisconnected(
      Azure::Core::Amqp::Models::_internal::AmqpError const& error)
  {
    // Queue the error to the received message queue along with a null message.
    //    m_receivedMessageQueue.CompleteOperation(
    //        Azure::Core::Amqp::Models::AmqpMessage{nullptr}, error);
    (void)error;
  }
}}} // namespace Azure::Messaging::EventHubs
