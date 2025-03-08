// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words myeventhub

#pragma once
#include "event_data_batch.hpp"
#include "models/management_models.hpp"

#include <azure/core/amqp.hpp>
#include <azure/core/amqp/internal/message_sender.hpp>
#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/policies/policy.hpp>

#include <iostream>

namespace Azure { namespace Messaging { namespace EventHubs {
  namespace _detail {
    class EventHubsPropertiesClient;
  } // namespace _detail

  class ProducerClient;

  /**@brief Contains options for the ProducerClient creation
   */
  struct ProducerClientOptions final
  {
    /**@brief  Application ID that will be passed to the namespace.
     */
    std::string ApplicationID = "";

    /**@brief  RetryOptions controls how often operations are retried from this client and any
     * Receivers and Senders created from this client.
     */
    Azure::Core::Http::Policies::RetryOptions RetryOptions{};

    /** @brief The name of the producer client link, used in diagnostics.
     */
    std::string Name{};

    /**@brief  The maximum size of the message that can be sent.
     */
    Azure::Nullable<std::uint64_t> MaxMessageSize{};

  private:
    // The friend declaration is needed so that ProducerClient could access CppStandardVersion,
    // and it is not a struct's public field like the ones above to be set non-programmatically.
    // When building the SDK, tests, or samples, the value of __cplusplus is ultimately controlled
    // by the cmake files in this repo (i.e. C++14), therefore we set distinct values of 0, -1, etc
    // when it is the case.
    friend class ProducerClient;
    long CppStandardVersion =
#if defined(_azure_BUILDING_SDK)
        -2L
#elif defined(_azure_BUILDING_TESTS)
        -1L
#elif defined(_azure_BUILDING_SAMPLES)
        0L
#else
    // https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
#if defined(_MSVC_LANG) && __cplusplus == 199711L
        _MSVC_LANG
#else
        __cplusplus
#endif
#endif
        ;
  };

  /**@brief  ProducerClient can be used to send events to an Event Hub.
   */
  class ProducerClient final {

  public:
    /** Get the fully qualified namespace from the connection string */
    std::string const& GetEventHubName() { return m_eventHub; }

    /** Get Retry options for this ProducerClient */
    Azure::Core::Http::Policies::RetryOptions const& GetRetryOptions() const
    {
      return m_producerClientOptions.RetryOptions;
    }

    /** Create a ProducerClient from another ProducerClient. */
    ProducerClient(ProducerClient const& other) = delete;

    /** Assign a ProducerClient another ProducerClient. */
    ProducerClient& operator=(ProducerClient const& other) = delete;

    ProducerClient(ProducerClient&& other) = delete;
    ProducerClient& operator=(ProducerClient&& other) = delete;

    /** Default Constructor for a ProducerClient */
    ProducerClient() = default;

    /**@brief Constructs a new ProducerClient instance
     *
     * @param connectionString Event hubs connection string
     * @param eventHub Event hub name
     * @param options Additional options for creating the client
     */
    ProducerClient(
        std::string const& connectionString,
        std::string const& eventHub,
        ProducerClientOptions options = {});

    /**@brief Constructs a new ProducerClient instance
     *
     * @param fullyQualifiedNamespace Fully qualified namespace name
     * @param eventHub Event hub name
     * @param credential Credential to use for authentication
     * @param options Additional options for creating the client
     */
    ProducerClient(
        std::string const& fullyQualifiedNamespace,
        std::string const& eventHub,
        std::shared_ptr<Azure::Core::Credentials::TokenCredential const> credential,
        ProducerClientOptions options = {});

    ~ProducerClient() { Close(); }

    /** @brief Close all the connections and sessions.
     *
     * @param context Context for the operation can be used for request cancellation.
     */
    void Close(Azure::Core::Context const& context = {})
    {
      for (auto& sender : m_senders)
      {
        sender.second.Close(context);
      }
      m_senders.clear();

      // Close needs to tear down all outstanding sessions and connections, but the functionality to
      // tear these down isn't complete yet.
      //    for (auto& session : m_sessions)
      //    {
      // session.second.Close(context);
      // }
      //    for (auto& connection : m_connections)
      //    {
      // connection.second.Close(context);
      // }
    }

    /** @brief Create a new EventDataBatch to be sent to the Event Hub.
     *
     * @param options Optional batch options
     * @param context Context for the operation can be used for request cancellation.
     *
     * @return newly created EventDataBatch object.
     */
    EventDataBatch CreateBatch(
        EventDataBatchOptions const& options = {},
        Azure::Core::Context const& context = {});

    /**@brief Send an EventDataBatch to the remote Event Hub.
     *
     * @param eventDataBatch Batch to send
     * @param context Request context
     */
    void Send(EventDataBatch const& eventDataBatch, Core::Context const& context = {});

    /**@brief Send an EventData to the remote Event Hub.
     *
     * @remark This method will create a new EventDataBatch and add the event to it. If the event
     * exceeds the maximum size allowed by the Event Hubs service, an exception will be thrown.
     *
     * @param eventData event to send
     * @param context Request context
     */
    void Send(Models::EventData const& eventData, Core::Context const& context = {});

    /**@brief Send a vector of EventData items to the remote Event Hub.
     *
     * @remark This method will create a new EventDataBatch and add the events to it. If the events
     * exceeds the maximum size allowed by the Event Hubs service, an exception will be thrown.
     *
     * @param eventData events to send
     * @param context Request context
     */
    void Send(std::vector<Models::EventData> const& eventData, Core::Context const& context = {});

    /**@brief  GetEventHubProperties gets properties of an eventHub. This includes data
     * like name, and partitions.
     *
     * @param context Context for the operation can be used for request cancellation.
     */
    Models::EventHubProperties GetEventHubProperties(Core::Context const& context = {});

    /**@brief  GetPartitionProperties gets properties for a specific partition. This includes data
     * like the last enqueued sequence number, the first sequence number and when an event was last
     * enqueued to the partition.
     *
     * @param partitionID partition ID to detail.
     * @param context Context for the operation can be used for request cancellation.
     */
    Models::EventHubPartitionProperties GetPartitionProperties(
        std::string const& partitionID,
        Core::Context const& context = {});

  private:
    /// The connection string for the Event Hubs namespace
    std::string m_connectionString;

    /// the Event Hubs namespace name (ex: myeventhub.servicebus.windows.net)
    std::string m_fullyQualifiedNamespace;

    /// The name of the Event Hub
    std::string m_eventHub{};

    /// The URL to the Event Hubs namespace
    std::string m_targetUrl{};

    uint16_t m_targetPort = Azure::Core::Amqp::_internal::AmqpTlsPort;

    /// Credentials to be used to authenticate the client.
    std::shared_ptr<Core::Credentials::TokenCredential const> m_credential{};

    ProducerClientOptions m_producerClientOptions{};

    // Protects m_senders and m_connection.
    std::mutex m_sendersLock;
    std::map<std::string, Azure::Core::Amqp::_internal::Connection> m_connections{};
    std::map<std::string, Azure::Core::Amqp::_internal::MessageSender> m_senders{};

    std::recursive_mutex m_sessionsLock;
    std::map<std::string, Azure::Core::Amqp::_internal::Session> m_sessions{};

    std::mutex m_propertiesClientLock;
    std::shared_ptr<_detail::EventHubsPropertiesClient> m_propertiesClient;

    Azure::Core::Amqp::_internal::Connection CreateConnection() const;
    Azure::Core::Amqp::_internal::Session CreateSession(std::string const& partitionId);

    // Ensure that the connection for this producer has been established.
    void EnsureConnection(const std::string& partitionId);

    // Ensure that a session for the specified partition ID has been established.
    void EnsureSession(std::string const& partitionId);

    // Ensure that a message sender for the specified partition has been created.
    void EnsureSender(std::string const& partitionId, Azure::Core::Context const& context = {});

    std::shared_ptr<_detail::EventHubsPropertiesClient> GetPropertiesClient();

    Azure::Core::Amqp::_internal::MessageSender GetSender(std::string const& partitionId);
    Azure::Core::Amqp::_internal::Session GetSession(std::string const& partitionId);
  };
}}} // namespace Azure::Messaging::EventHubs
