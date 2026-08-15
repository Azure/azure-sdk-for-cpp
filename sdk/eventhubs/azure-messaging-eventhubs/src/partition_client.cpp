// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/messaging/eventhubs/partition_client.hpp"

#include "azure/messaging/eventhubs/eventhubs_exception.hpp"
#include "private/eventhubs_constants.hpp"
#include "private/eventhubs_utilities.hpp"
#include "private/retry_operation.hpp"

#include <azure/core/amqp.hpp>
#include <azure/core/amqp/internal/models/messaging_values.hpp>

#include <chrono>
#include <exception>
#include <thread>

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

    return PartitionClient(
        std::move(messageReceiver),
        session,
        partitionUrl,
        receiverName,
        std::move(options),
        std::move(retryOptions));
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
      Azure::Core::Amqp::_internal::Session const& session,
      std::string partitionUrl,
      std::string receiverName,
      PartitionClientOptions options,
      Core::Http::Policies::RetryOptions retryOptions)
      : m_receiver{messageReceiver}, m_session{session}, m_partitionUrl{std::move(partitionUrl)},
        m_receiverName{std::move(receiverName)}, m_partitionOptions{options}, m_retryOptions{
                                                                                  retryOptions}
  {
  }

  void PartitionClient::Close(Core::Context const& context) { m_receiver.Close(context); }

  void PartitionClient::RebuildReceiver(Core::Context const& context)
  {
    Log::Stream(Logger::Level::Informational)
        << "Rebuild the message receiver for " << m_partitionUrl << ".";

    try
    {
      m_receiver.Close(context);
    }
    catch (Azure::Core::OperationCancelledException const&)
    {
      throw;
    }
    catch (std::exception const& ex)
    {
      Log::Stream(Logger::Level::Warning)
          << "Exception while closing a faulted message receiver: " << ex.what();
    }

    PartitionClientOptions options{m_partitionOptions};
    options.StartPosition
        = _detail::ResumeStartPosition(m_partitionOptions.StartPosition, m_lastReceivedOffset);

    Azure::Core::Amqp::_internal::MessageReceiver receiver{
        CreateMessageReceiver(m_session, m_partitionUrl, m_receiverName, options)};
    receiver.Open(context);
    m_receiver = std::move(receiver);

    Log::Stream(Logger::Level::Informational)
        << "The message receiver for " << m_partitionUrl << " is attached again.";
  }

  PartitionClient::~PartitionClient()
  {
    try
    {
      Log::Stream(Logger::Level::Verbose) << "~PartitionClient() "
                                          << "Close Receiver.";
      m_receiver.Close();
    }
    catch (std::exception const& ex)
    {
      Log::Stream(Logger::Level::Warning)
          << "Exception in PartitionClient::~PartitionClient(): " << ex.what();
    }
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

    // RetryOperation::Execute's budget never resets, so this loop keeps its own counter.
    Azure::Core::Http::Policies::RetryOptions retryOptions{m_retryOptions};
    _detail::RetryOperation retryOperation{retryOptions};
    int32_t rebuildAttempt = 0;

    // Keep the event, and record the offset a rebuild must start after.
    auto keepMessage
        = [&](std::shared_ptr<const Azure::Core::Amqp::Models::AmqpMessage> const& message) {
            auto eventData = std::make_shared<const Models::ReceivedEventData>(message);
            if (eventData->Offset.HasValue())
            {
              m_lastReceivedOffset = eventData->Offset.Value();
            }
            rebuildAttempt = 0;
            messages.push_back(eventData);
          };

    // True: the receiver works again. False: return the events held. Throws if none are held.
    auto recover = [&](Azure::Core::Amqp::Models::_internal::AmqpError const& error) -> bool {
      EventHubsException exception{
          _detail::EventHubsExceptionFactory::CreateEventHubsException(error)};
      Azure::Core::Amqp::Models::_internal::AmqpError currentError{error};
      std::exception_ptr originalFailure{};

      for (;;)
      {
        std::chrono::milliseconds retryAfter{};
        if (!_detail::ShouldRebuildReceiver(exception)
            || !retryOperation.ShouldRetry(false, rebuildAttempt, retryAfter))
        {
          if (!messages.empty())
          {
            // The service will not send these again. The next call gets a new budget.
            Log::Stream(Logger::Level::Warning)
                << "Cannot rebuild the message receiver now. Return " << messages.size()
                << " events and keep the error for the next call: " << exception.what();
            m_pendingError = currentError;
            return false;
          }
          if (originalFailure)
          {
            std::rethrow_exception(originalFailure);
          }
          throw exception;
        }

        rebuildAttempt++;
        std::this_thread::sleep_for(retryAfter);
        context.ThrowIfCancelled();

        try
        {
          RebuildReceiver(context);
          return true;
        }
        catch (Azure::Core::OperationCancelledException const&)
        {
          throw;
        }
        catch (EventHubsException const& rebuildFailure)
        {
          Log::Stream(Logger::Level::Warning)
              << "Rebuild attempt " << rebuildAttempt << " failed: " << rebuildFailure.what();
          exception = rebuildFailure;
          originalFailure = nullptr;
          currentError.Condition
              = Azure::Core::Amqp::Models::_internal::AmqpErrorCondition{exception.ErrorCondition};
          currentError.Description = exception.ErrorDescription;
        }
        catch (Azure::Core::Credentials::AuthenticationException const& rebuildFailure)
        {
          Log::Stream(Logger::Level::Warning)
              << "Rebuild attempt " << rebuildAttempt << " failed: " << rebuildFailure.what();
          exception = _detail::TranslateAuthenticationFailure(rebuildFailure);
          originalFailure = std::current_exception();
          currentError.Condition = Azure::Core::Amqp::Models::_internal::AmqpErrorCondition{};
          currentError.Description = exception.ErrorDescription;
        }
        catch (std::exception const& rebuildFailure)
        {
          Log::Stream(Logger::Level::Warning)
              << "Rebuild attempt " << rebuildAttempt << " failed: " << rebuildFailure.what();
          EventHubsException translated{rebuildFailure.what()};
          translated.IsTransient = true;
          exception = translated;
          originalFailure = nullptr;
          currentError.Condition = Azure::Core::Amqp::Models::_internal::AmqpErrorCondition{};
          currentError.Description = translated.ErrorDescription;
        }
      }
    };

    // No event is held yet, so this recover either works or throws.
    if (m_pendingError.HasValue())
    {
      auto pendingError = m_pendingError.Value();
      m_pendingError.Reset();
      recover(pendingError);
    }

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
        keepMessage(result.first);
      }
      else if (result.second)
      {
        if (!recover(result.second))
        {
          break;
        }
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
          keepMessage(result.first);
        }
        else if (!recover(result.second))
        {
          break;
        }
      }
    }
    Log::Stream(Logger::Level::Verbose)
        << "Receive Events. Return " << messages.size() << " messages.";

    return messages;
  }
}}} // namespace Azure::Messaging::EventHubs
