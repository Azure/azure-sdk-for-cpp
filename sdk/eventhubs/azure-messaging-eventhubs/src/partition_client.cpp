// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/messaging/eventhubs/partition_client.hpp"

#include "azure/messaging/eventhubs/eventhubs_exception.hpp"
#include "private/eventhubs_constants.hpp"
#include "private/eventhubs_utilities.hpp"
#include "private/retry_operation.hpp"

#include <azure/core/amqp.hpp>
#include <azure/core/amqp/internal/models/messaging_values.hpp>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

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
      sourceOptions.Filter.emplace(AmqpSymbol{description.Name}, value.AsAmqpValue());
    }

    FilterDescription SelectorFilter{"apache.org:selector-filter:string", 0x0000468c00000004};

    std::string GetStartExpression(Models::StartPosition const& startPosition)
    {
      Log::Stream(Logger::Level::Verbose)
          << "Get Start Expression for StartPosition: " << startPosition;
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
        returnValue = "amqp.annotation.x-opt-enqueued-time " + greaterThan + "'"
            + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                                 static_cast<std::chrono::system_clock::time_point>(
                                     startPosition.EnqueuedTime.Value())
                                     .time_since_epoch())
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
            + startPosition.Offset.Value() + "'";
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
        Log::Stream(Logger::Level::Verbose) << "No start position set, use default.";
        return "amqp.annotation.x-opt-offset > '@latest'";
      }
      else
      {
        Log::Stream(Logger::Level::Verbose) << "Get Start Expression, returnValue: " << returnValue;
        return returnValue;
      }
    }
#if ENABLE_UAMQP
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

      receiverOptions.EnableTrace = _detail::EnableAmqpTrace;
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
#elif ENABLE_RUST_AMQP
    // Helper function to create a message receiver.
    Azure::Core::Amqp::_internal::MessageReceiver CreateMessageReceiver(
        Azure::Core::Amqp::_internal::Session const& session,
        std::string const& partitionUrl,
        std::string const& receiverName,
        PartitionClientOptions const& options)
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

      receiverOptions.EnableTrace = _detail::EnableAmqpTrace;
      // Set the link credit to the prefetch count. If the user has not set a prefetch count, then
      // we will use the default value.
      if (options.Prefetch >= 0)
      {
        receiverOptions.MaxLinkCredit = options.Prefetch;
      }
      receiverOptions.Name = receiverName;
      receiverOptions.Properties.emplace(AmqpSymbol{"com.microsoft:receiver-name"}, receiverName);
      if (options.OwnerLevel.HasValue())
      {
        receiverOptions.Properties.emplace(
            AmqpSymbol{"com.microsoft:epoch"}, options.OwnerLevel.Value());
      }
      return session.CreateMessageReceiver(messageSource, receiverOptions);
    }
#endif

  } // namespace

  PartitionClient _detail::PartitionClientFactory::CreatePartitionClient(
      Azure::Core::Amqp::_internal::Session const& session,
      std::string const& partitionUrl,
      std::string const& receiverName,
      PartitionClientOptions options,
      Azure::Core::Http::Policies::RetryOptions retryOptions,
      Azure::Core::Context const& context)
  {
    Azure::Core::Amqp::_internal::MessageReceiver messageReceiver{
        CreateMessageReceiver(session, partitionUrl, receiverName, options)};
    messageReceiver.Open(context);

    return PartitionClient(std::move(messageReceiver), std::move(options), std::move(retryOptions));
  }

  /** Creates a new PartitionClient
   *
   * @param messageReceiver Message Receiver for the partition client.
   * @param options options used to create the PartitionClient.
   * @param retryOptions controls how many times we should retry an operation in response to being
   * throttled or encountering a transient error.
   */
  PartitionClient::PartitionClient(
      Azure::Core::Amqp::_internal::MessageReceiver const& messageReceiver,
      PartitionClientOptions options,
      Core::Http::Policies::RetryOptions retryOptions)
      : m_receiver{messageReceiver}, m_partitionOptions{options}, m_retryOptions{retryOptions}
  {
  }

  PartitionClient::~PartitionClient()
  {
    Log::Stream(Logger::Level::Verbose) << "~PartitionClient() "
                                        << "Close Receiver.";
    m_receiver.Close();
  }

  /** Receive events from the partition.
   *
   * @param maxMessages The maximum number of messages to receive.
   * @param context A context to control the request lifetime.
   * @return A vector of received events.
   *
   */
  std::vector<std::shared_ptr<const Models::ReceivedEventData>> PartitionClient::ReceiveEvents(
      uint32_t maxMessages,
      Core::Context const& context)
  {
    std::vector<std::shared_ptr<const Models::ReceivedEventData>> messages;

    while (messages.size() < maxMessages && !context.IsCancelled())
    {
      std::pair<
          std::shared_ptr<const Azure::Core::Amqp::Models::AmqpMessage>,
          Azure::Core::Amqp::Models::_internal::AmqpError>
          result;

      // TryPeekForIncomingMessage will return two empty values if there is no data available.
      result = m_receiver.TryWaitForIncomingMessage();
      if (result.first)
      {
        messages.push_back(std::make_shared<Models::ReceivedEventData>(result.first));
      }
      else if (result.second)
      {
        throw _detail::EventHubsExceptionFactory::CreateEventHubsException(result.second);
      }
      // If we haven't gotten *any* messages, we're done. Otherwise, we'll wait for more.
      else if (!messages.empty())
      {
        break;
      }
      else
      {
        result = m_receiver.WaitForIncomingMessage(context);
        if (result.first)
        {
          Log::Stream(Logger::Level::Verbose)
              << "Received message. Message count now " << messages.size();
          messages.push_back(std::make_shared<const Models::ReceivedEventData>(result.first));
        }
        else
        {
          throw _detail::EventHubsExceptionFactory::CreateEventHubsException(result.second);
        }
      }
    }
    Log::Stream(Logger::Level::Verbose)
        << "Receive Events. Return " << messages.size() << " messages.";

    return messages;
  }
}}} // namespace Azure::Messaging::EventHubs
