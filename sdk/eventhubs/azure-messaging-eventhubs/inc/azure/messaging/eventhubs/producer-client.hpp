// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include <azure/core/amqp.hpp>
#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>

namespace Azure { namespace Messaging { namespace EventHubs {
  // RetryOptions represent the options for retries.
  struct ProducerClientCreds
  {
    std::string ConnectionString;

    // the Event Hubs namespace name (ex: myservicebus.servicebus.windows.net)
    std::string FullyQualifiedNamespace;

    std::shared_ptr<Core::Credentials::TokenCredential const> credential;

    std::string eventHub;
  }
  // ProducerClientOptions contains options for the `NewProducerClient` and
  // `NewProducerClientFromConnectionString`
  // functions.
  struct ProducerClientOptions
  {
    // Application ID that will be passed to the namespace.
    std::string ApplicationID;

    // RetryOptions controls how often operations are retried from this client and any
    // Receivers and Senders created from this client.
    Azure::Core::Http::Policies::RetryOptions RetryOptions
  }

  // ProducerClient can be used to send events to an Event Hub.
  class ProducerClient {
    const std::string anyPartitionId = "";
    std::string EventHub;
    std::vector<Azure::Core::Amqp::Network::_internal::Transport> Links;
    Azure::Core::Http::Policies::RetryOptions RetryOptions;

  public:
    ProducerClient(
        std::string fullyQualifiedNamespace,
        std::string eventHub,
        std::shared_ptr<Azure::Core::Amqp::_internal::SaslPlainConnectionStringCredential const>
            credential,
        ProducerClientOptions options = ProducerClientOptions());

    ProducerClient(
        std::string connectionString,
        std::string eventHub,
        ProducerClientOptions options = ProducerClientOptions())
    {
      auto connection = std::make_shared<Azure::Core::Amqp::_internal::SaslPlainConnectionStringCredential>(
          connectionString);
      return this
    };
  };

}}} // namespace Azure::Messaging::EventHubs