// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/messaging/eventhubs/partition_client.hpp"

#include "azure/messaging/eventhubs/eventhubs_exception.hpp"
#include "private/eventhubs_constants.hpp"
#include "private/eventhubs_utilities.hpp"
#include "private/retry_operation.hpp"

#include <azure/core/amqp.hpp>
#include <azure/core/amqp/internal/claims_based_security.hpp>
#include <azure/core/amqp/internal/models/messaging_values.hpp>

#include <chrono>
#include <condition_variable>
#include <exception>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
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

#if ENABLE_UAMQP
  namespace _detail {
    enum class PendingFailureKind
    {
      None,
      Ordinary,
      Authentication,
      Permanent,
    };

    struct ReceiverStack final
    {
      ReceiverStack(
          Azure::Core::Amqp::_internal::Connection connection,
          Azure::Core::Amqp::_internal::Session session,
          Azure::Core::Amqp::_internal::MessageReceiver receiver)
          : Connection{std::move(connection)}, Session{std::move(session)}, Receiver{
                                                                                std::move(receiver)}
      {
      }

      Azure::Core::Amqp::_internal::Connection Connection;
      Azure::Core::Amqp::_internal::Session Session;
      Azure::Core::Amqp::_internal::MessageReceiver Receiver;
    };

    struct PartitionClientState final
    {
      PartitionClientState(
          std::string fullyQualifiedNamespace,
          std::shared_ptr<const Azure::Core::Credentials::TokenCredential> credential,
          std::uint16_t targetPort,
          std::string applicationId,
          long cppStandardVersion,
          std::string containerId,
          std::string partitionUrl,
          std::string receiverName,
          PartitionClientOptions options,
          Azure::Core::Http::Policies::RetryOptions retryOptions)
          : FullyQualifiedNamespace{std::move(fullyQualifiedNamespace)},
            Credential{std::move(credential)}, TargetPort{targetPort},
            ApplicationId{std::move(applicationId)}, CppStandardVersion{cppStandardVersion},
            ContainerId{std::move(containerId)}, PartitionUrl{std::move(partitionUrl)},
            ReceiverName{std::move(receiverName)}, Options{std::move(options)},
            RetryOptions{std::move(retryOptions)}
      {
      }

      std::mutex Lock;
      std::mutex ReceiveLock;
      std::condition_variable ReceiveCondition;
      std::shared_ptr<ReceiverStack> Stack;
      std::uint64_t Generation{0};
      bool Closed{false};
      bool ActiveReceive{false};
      Azure::Core::Context ActiveReceiveContext;

      std::string FullyQualifiedNamespace;
      std::shared_ptr<const Azure::Core::Credentials::TokenCredential> Credential;
      std::uint16_t TargetPort;
      std::string ApplicationId;
      long CppStandardVersion;
      std::string ContainerId;
      std::string PartitionUrl;
      std::string ReceiverName;
      PartitionClientOptions Options;
      Azure::Core::Http::Policies::RetryOptions RetryOptions;
      Azure::Nullable<std::string> LastReceivedOffset;
      Azure::Nullable<Azure::Core::Amqp::Models::_internal::AmqpError> PendingError;
      std::exception_ptr PendingFailure;
      PendingFailureKind PendingKind{PendingFailureKind::None};
    };
  } // namespace _detail

  namespace {
    void CloseReceiverStack(
        std::shared_ptr<_detail::ReceiverStack> const& stack,
        Azure::Core::Context const& context)
    {
      if (!stack)
      {
        return;
      }

      try
      {
        stack->Receiver.Close(context);
      }
      catch (std::exception const& ex)
      {
        Log::Stream(Logger::Level::Warning)
            << "Exception while closing a message receiver: " << ex.what();
      }
      try
      {
        stack->Session.End(context);
      }
      catch (std::exception const& ex)
      {
        Log::Stream(Logger::Level::Warning)
            << "Exception while ending a receiver session: " << ex.what();
      }
      // The uAMQP connection closes when the final stack object is destroyed. Connection::Close
      // is intentionally private for this backend.
    }

    std::shared_ptr<_detail::ReceiverStack> CreateReceiverStack(
        _detail::PartitionClientState const& state,
        PartitionClientOptions const& options,
        Azure::Core::Context const& context)
    {
      Azure::Core::Amqp::_internal::ConnectionOptions connectionOptions;
      connectionOptions.ContainerId = state.ContainerId;
      connectionOptions.EnableTrace = _detail::EnableAmqpTrace;
      connectionOptions.AuthenticationScopes = {"https://eventhubs.azure.net/.default"};
      connectionOptions.Port = state.TargetPort;
      _detail::EventHubsUtilities::SetUserAgent(
          connectionOptions, state.ApplicationId, state.CppStandardVersion);

      Azure::Core::Amqp::_internal::Connection connection{
          state.FullyQualifiedNamespace, state.Credential, connectionOptions};

      Azure::Core::Amqp::_internal::SessionOptions sessionOptions;
      sessionOptions.InitialIncomingWindowSize
          = static_cast<std::uint32_t>((std::numeric_limits<std::int32_t>::max)());
      Azure::Core::Amqp::_internal::Session session{connection.CreateSession(sessionOptions)};
      auto receiver
          = CreateMessageReceiver(session, state.PartitionUrl, state.ReceiverName, options);
      auto stack = std::make_shared<_detail::ReceiverStack>(
          std::move(connection), std::move(session), std::move(receiver));
      try
      {
        stack->Receiver.Open(context);
      }
      catch (...)
      {
        CloseReceiverStack(stack, context);
        throw;
      }
      return stack;
    }

    class ReceiveLease final {
    public:
      ReceiveLease(
          std::shared_ptr<_detail::PartitionClientState> state,
          Azure::Core::Context const& parentContext)
          : m_state{std::move(state)}, m_receiveLock{m_state->ReceiveLock},
            m_childContext{parentContext.WithDeadline(parentContext.GetDeadline())}
      {
        std::lock_guard<std::mutex> lock(m_state->Lock);
        if (m_state->Closed)
        {
          throw Azure::Core::OperationCancelledException("Partition client is closed.");
        }
        m_state->ActiveReceive = true;
        m_state->ActiveReceiveContext = m_childContext;
        m_stack = m_state->Stack;
      }

      ~ReceiveLease()
      {
        // Release the snapshot before notifying close callers that the backend call has ended.
        m_stack.reset();
        {
          std::lock_guard<std::mutex> lock(m_state->Lock);
          m_state->ActiveReceive = false;
          m_state->ActiveReceiveContext = Azure::Core::Context{};
        }
        m_state->ReceiveCondition.notify_all();
      }

      std::shared_ptr<_detail::ReceiverStack> GetStack() const { return m_stack; }
      Azure::Core::Context const& GetContext() const { return m_childContext; }

      void RefreshStack()
      {
        std::lock_guard<std::mutex> lock(m_state->Lock);
        if (m_state->Closed)
        {
          throw Azure::Core::OperationCancelledException("Partition client is closed.");
        }
        m_stack = m_state->Stack;
      }

    private:
      std::shared_ptr<_detail::PartitionClientState> m_state;
      std::unique_lock<std::mutex> m_receiveLock;
      Azure::Core::Context m_childContext;
      std::shared_ptr<_detail::ReceiverStack> m_stack;
    };
  } // namespace
#endif

#if ENABLE_UAMQP && defined(_azure_EVENTHUBS_TEST_HOOKS)
  namespace {
    std::mutex PartitionClientStateCloseHookLock;
    std::function<void()> PartitionClientStateCloseHook;
  } // namespace
#endif

#if ENABLE_UAMQP
  void _detail::ClosePartitionClientState(
      std::shared_ptr<_detail::PartitionClientState> const& state,
      Azure::Core::Context const& context)
  {
    if (!state)
    {
      return;
    }

#if ENABLE_UAMQP && defined(_azure_EVENTHUBS_TEST_HOOKS)
    std::function<void()> closeHook;
    {
      std::lock_guard<std::mutex> lock(PartitionClientStateCloseHookLock);
      closeHook = std::move(PartitionClientStateCloseHook);
    }
    if (closeHook)
    {
      closeHook();
    }
#endif

    std::shared_ptr<_detail::ReceiverStack> stackToClose;
    Azure::Core::Context activeReceiveContext;
    bool activeReceive = false;
    {
      std::lock_guard<std::mutex> lock(state->Lock);
      if (!state->Closed)
      {
        state->Closed = true;
        ++state->Generation;
        stackToClose = std::move(state->Stack);
      }
      activeReceive = state->ActiveReceive;
      if (activeReceive)
      {
        activeReceiveContext = state->ActiveReceiveContext;
      }
    }
    if (activeReceive)
    {
      activeReceiveContext.Cancel();
      std::unique_lock<std::mutex> lock(state->Lock);
      state->ReceiveCondition.wait(lock, [&state] { return !state->ActiveReceive; });
    }
    CloseReceiverStack(stackToClose, context);
  }
#endif

#if ENABLE_UAMQP && defined(_azure_EVENTHUBS_TEST_HOOKS)
  namespace _detail {
    void SetPartitionClientStateCloseHook(std::function<void()> hook)
    {
      std::lock_guard<std::mutex> lock(PartitionClientStateCloseHookLock);
      PartitionClientStateCloseHook = std::move(hook);
    }
  } // namespace _detail
#endif

#if ENABLE_UAMQP
  PartitionClient _detail::PartitionClientFactory::CreatePartitionClient(
      std::string fullyQualifiedNamespace,
      std::shared_ptr<const Azure::Core::Credentials::TokenCredential> credential,
      std::uint16_t targetPort,
      std::string applicationId,
      long cppStandardVersion,
      std::string containerId,
      std::string partitionUrl,
      std::string receiverName,
      PartitionClientOptions options,
      Azure::Core::Http::Policies::RetryOptions retryOptions,
      Azure::Core::Context const& context)
  {
    auto state = std::make_shared<_detail::PartitionClientState>(
        std::move(fullyQualifiedNamespace),
        std::move(credential),
        targetPort,
        std::move(applicationId),
        cppStandardVersion,
        std::move(containerId),
        std::move(partitionUrl),
        std::move(receiverName),
        std::move(options),
        std::move(retryOptions));

    _detail::RetryOperation retryOperation{state->RetryOptions};
    _detail::RetryOperation::AuthenticationRecoveryState authenticationState;
    for (;;)
    {
      std::shared_ptr<_detail::ReceiverStack> candidate;
      try
      {
        if (!retryOperation.Execute(
                [&]() -> bool {
                  candidate = CreateReceiverStack(*state, state->Options, context);
                  return true;
                },
                context))
        {
          throw std::runtime_error("Could not create the message receiver.");
        }

        {
          std::lock_guard<std::mutex> lock(state->Lock);
          state->Stack = std::move(candidate);
          state->Generation++;
        }
        return PartitionClient{std::move(state)};
      }
      catch (Azure::Core::Amqp::_detail::CbsPutTokenFailedException const& failure)
      {
        std::chrono::milliseconds retryAfter{};
        if (!retryOperation.ShouldRetryAuthentication(authenticationState, retryAfter))
        {
          failure.RethrowOriginal();
        }
        _detail::RetryOperation::WaitForRetryDelay(retryAfter, context);
      }
    }
  }
#elif ENABLE_RUST_AMQP
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
#endif

#if ENABLE_UAMQP
  PartitionClient::PartitionClient(std::shared_ptr<_detail::PartitionClientState> state)
      : m_state{std::move(state)},
        m_receiver{m_state->Stack->Receiver}, m_session{m_state->Stack->Session}
  {
  }

  void PartitionClient::Close(Core::Context const& context)
  {
    _detail::ClosePartitionClientState(m_state, context);
  }

  void PartitionClient::RebuildReceiver(Core::Context const& context)
  {
    auto state = m_state;
    PartitionClientOptions options;
    std::uint64_t expectedGeneration;
    std::shared_ptr<_detail::ReceiverStack> oldStack;
    {
      std::lock_guard<std::mutex> lock(state->Lock);
      if (state->Closed)
      {
        throw Azure::Core::OperationCancelledException("Partition client is closed.");
      }
      oldStack = std::move(state->Stack);
      expectedGeneration = ++state->Generation;
      options = state->Options;
      options.StartPosition
          = _detail::ResumeStartPosition(state->Options.StartPosition, state->LastReceivedOffset);
    }

    Log::Stream(Logger::Level::Informational)
        << "Rebuild the message receiver for " << state->PartitionUrl << ".";
    CloseReceiverStack(oldStack, context);
    auto candidate = CreateReceiverStack(*state, options, context);
    auto candidateReceiver = candidate->Receiver;
    auto candidateSession = candidate->Session;

    bool installed = false;
    {
      std::lock_guard<std::mutex> lock(state->Lock);
      if (state->Closed || state->Generation != expectedGeneration)
      {
        // Close the candidate after releasing the state lock.
      }
      else
      {
        installed = true;
        state->Stack = std::move(candidate);
        ++state->Generation;
      }
    }

    if (!installed)
    {
      CloseReceiverStack(candidate, context);
      throw Azure::Core::OperationCancelledException("Partition client was closed.");
    }

    // Drop the copies of the old stack, or its connection stays open until the client dies.
    m_receiver = std::move(candidateReceiver);
    m_session = std::move(candidateSession);

    Log::Stream(Logger::Level::Informational)
        << "The message receiver for " << state->PartitionUrl << " is attached again.";
  }
#elif ENABLE_RUST_AMQP
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
#endif

  PartitionClient& PartitionClient::operator=(PartitionClient&& other)
  {
    if (this == &other)
    {
      return *this;
    }

#if ENABLE_UAMQP
    _detail::ClosePartitionClientState(m_state, {});
#endif
    m_state = std::move(other.m_state);
    m_receiver = std::move(other.m_receiver);
    m_session = std::move(other.m_session);
    m_partitionUrl = std::move(other.m_partitionUrl);
    m_receiverName = std::move(other.m_receiverName);
    m_lastReceivedOffset = std::move(other.m_lastReceivedOffset);
    m_pendingError = std::move(other.m_pendingError);
    m_partitionOptions = std::move(other.m_partitionOptions);
    m_retryOptions = std::move(other.m_retryOptions);
    return *this;
  }

  PartitionClient::~PartitionClient()
  {
    try
    {
      Log::Stream(Logger::Level::Verbose) << "~PartitionClient() "
                                          << "Close Receiver.";
#if ENABLE_UAMQP
      _detail::ClosePartitionClientState(m_state, {});
#elif ENABLE_RUST_AMQP
      m_receiver.Close();
#endif
    }
    catch (std::exception const& ex)
    {
      Log::Stream(Logger::Level::Warning)
          << "Exception in PartitionClient::~PartitionClient(): " << ex.what();
    }
  }

#if ENABLE_UAMQP
  std::vector<std::shared_ptr<const Models::ReceivedEventData>> PartitionClient::ReceiveEvents(
      uint32_t maxMessages,
      Core::Context const& context)
  {
    std::vector<std::shared_ptr<const Models::ReceivedEventData>> messages;
    auto state = m_state;
    ReceiveLease lease{state, context};

    // RetryOperation::Execute's budget never resets, so this loop keeps its own counter.
    Azure::Core::Http::Policies::RetryOptions retryOptions;
    {
      std::lock_guard<std::mutex> lock(state->Lock);
      retryOptions = state->RetryOptions;
    }
    _detail::RetryOperation retryOperation{retryOptions};
    int32_t rebuildAttempt = 0;
    _detail::RetryOperation::AuthenticationRecoveryState authenticationState;

    // Keep the event, and record the offset a rebuild must start after.
    auto keepMessage
        = [&](std::shared_ptr<const Azure::Core::Amqp::Models::AmqpMessage> const& message) {
            auto eventData = std::make_shared<const Models::ReceivedEventData>(message);
            if (eventData->Offset.HasValue())
            {
              std::lock_guard<std::mutex> lock(state->Lock);
              state->LastReceivedOffset = eventData->Offset.Value();
            }
            rebuildAttempt = 0;
            messages.push_back(eventData);
          };

    // True: the receiver works again. False: return the events held. Throws if none are held.
    auto recover = [&](Azure::Core::Amqp::Models::_internal::AmqpError const* error,
                       bool authenticationFailure,
                       std::exception_ptr initialFailure) -> bool {
      EventHubsException exception = error
          ? _detail::EventHubsExceptionFactory::CreateEventHubsException(*error)
          : EventHubsException{"Authentication failure."};
      Azure::Core::Amqp::Models::_internal::AmqpError currentError;
      if (error)
      {
        currentError = *error;
      }
      std::exception_ptr originalFailure{std::move(initialFailure)};
      bool permanentFailure = false;

      for (;;)
      {
        std::chrono::milliseconds retryAfter{};
        bool shouldRetry = false;
        if (!permanentFailure)
        {
          shouldRetry = authenticationFailure
              ? retryOperation.ShouldRetryAuthentication(authenticationState, retryAfter)
              : _detail::ShouldRebuildReceiver(exception)
                  && retryOperation.ShouldRetry(false, rebuildAttempt, retryAfter);
        }
        if (!shouldRetry)
        {
          if (!messages.empty())
          {
            // The service will not send these again. The next call gets a new budget.
            Log::Stream(Logger::Level::Warning)
                << "Cannot rebuild the message receiver now. Return " << messages.size()
                << " events and keep the error for the next call: " << exception.what();
            std::lock_guard<std::mutex> lock(state->Lock);
            if (error || !currentError.Condition.ToString().empty()
                || !currentError.Description.empty())
            {
              state->PendingError = currentError;
            }
            else
            {
              state->PendingError.Reset();
            }
            state->PendingFailure
                = originalFailure ? originalFailure : std::make_exception_ptr(exception);
            state->PendingKind = permanentFailure
                ? _detail::PendingFailureKind::Permanent
                : (authenticationFailure ? _detail::PendingFailureKind::Authentication
                                         : _detail::PendingFailureKind::Ordinary);
            return false;
          }
          if (originalFailure)
          {
            std::rethrow_exception(originalFailure);
          }
          throw exception;
        }

        if (!authenticationFailure)
        {
          rebuildAttempt++;
        }
        _detail::RetryOperation::WaitForRetryDelay(retryAfter, lease.GetContext());

        try
        {
          RebuildReceiver(lease.GetContext());
          lease.RefreshStack();
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
          authenticationFailure = false;
          permanentFailure = false;
        }
        catch (Azure::Core::Amqp::_detail::CbsPutTokenFailedException const& rebuildFailure)
        {
          Log::Stream(Logger::Level::Warning)
              << "Authentication recovery failed: " << rebuildFailure.what();
          exception = EventHubsException{"Authentication failure."};
          currentError = Azure::Core::Amqp::Models::_internal::AmqpError{};
          originalFailure = rebuildFailure.GetOriginal();
          authenticationFailure = true;
          permanentFailure = false;
        }
        catch (Azure::Core::Credentials::AuthenticationException const& rebuildFailure)
        {
          Log::Stream(Logger::Level::Warning)
              << "Rebuild attempt " << rebuildAttempt << " failed: " << rebuildFailure.what();
          exception = _detail::TranslateAuthenticationFailure(rebuildFailure);
          originalFailure = std::current_exception();
          currentError = Azure::Core::Amqp::Models::_internal::AmqpError{};
          permanentFailure = true;
        }
        catch (std::exception const& rebuildFailure)
        {
          Log::Stream(Logger::Level::Warning)
              << "Rebuild attempt " << rebuildAttempt << " failed: " << rebuildFailure.what();
          EventHubsException translated{rebuildFailure.what()};
          translated.IsTransient = true;
          exception = translated;
          originalFailure = nullptr;
          currentError = Azure::Core::Amqp::Models::_internal::AmqpError{};
          currentError.Description = translated.ErrorDescription;
          authenticationFailure = false;
          permanentFailure = false;
        }
      }
    };

    // No event is held yet, so this recover either works or throws.
    Azure::Nullable<Azure::Core::Amqp::Models::_internal::AmqpError> pendingError;
    std::exception_ptr pendingFailure;
    _detail::PendingFailureKind pendingKind = _detail::PendingFailureKind::None;
    {
      std::lock_guard<std::mutex> lock(state->Lock);
      pendingKind = state->PendingKind;
      pendingFailure = state->PendingFailure;
      state->PendingKind = _detail::PendingFailureKind::None;
      state->PendingFailure = nullptr;
      if (state->PendingError.HasValue())
      {
        pendingError = state->PendingError.Value();
        state->PendingError.Reset();
      }
    }
    if (pendingKind == _detail::PendingFailureKind::Permanent)
    {
      std::rethrow_exception(pendingFailure);
    }
    if (pendingKind != _detail::PendingFailureKind::None)
    {
      recover(
          pendingError.HasValue() ? &pendingError.Value() : nullptr,
          pendingKind == _detail::PendingFailureKind::Authentication,
          std::move(pendingFailure));
    }

    while (messages.size() < maxMessages && !lease.GetContext().IsCancelled())
    {
      std::pair<
          std::shared_ptr<const Azure::Core::Amqp::Models::AmqpMessage>,
          Azure::Core::Amqp::Models::_internal::AmqpError>
          result;

      // TryWaitForIncomingMessage returns two empty values if there is no data available.
      auto stack = lease.GetStack();
      if (!stack)
      {
        std::lock_guard<std::mutex> lock(state->Lock);
        if (state->Closed)
        {
          throw Azure::Core::OperationCancelledException("Partition client is closed.");
        }
        throw std::runtime_error("Partition client has no receiver stack.");
      }
      result = stack->Receiver.TryWaitForIncomingMessage();
      if (result.first)
      {
        keepMessage(result.first);
      }
      else if (result.second)
      {
        bool const authenticationFailure = result.second.Condition
            == Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::UnauthorizedAccess;
        if (!recover(&result.second, authenticationFailure, nullptr))
        {
          break;
        }
      }
      // If no messages have arrived, wait for one. Otherwise return the messages already held.
      else if (!messages.empty())
      {
        break;
      }
      else
      {
        result = stack->Receiver.WaitForIncomingMessage(lease.GetContext());
        if (result.first)
        {
          Log::Stream(Logger::Level::Verbose)
              << "Received message. Message count now " << messages.size();
          keepMessage(result.first);
        }
        else if (!recover(
                     &result.second,
                     result.second.Condition
                         == Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::
                             UnauthorizedAccess,
                     nullptr))
        {
          break;
        }
      }
    }
    Log::Stream(Logger::Level::Verbose)
        << "Receive Events. Return " << messages.size() << " messages.";

    return messages;
  }
#elif ENABLE_RUST_AMQP
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
      // currentError tracks the last rebuild fault, not the first. m_pendingError takes
      // this value below, so the next call recovers from the fault that stopped this
      // loop, not from an earlier one that a later attempt already showed can be fixed.
      Azure::Core::Amqp::Models::_internal::AmqpError currentError{error};
      // Preserves a rebuild failure whose original type carries more than the
      // translated EventHubsException above, such as an AuthenticationException. The
      // loop throws this one when it stops, so the caller sees the original type.
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
#endif
}}} // namespace Azure::Messaging::EventHubs
