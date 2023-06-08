// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once

#include <azure/core/amqp.hpp>
#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/messaging/eventhubs.hpp>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Models {

  /**@brief Contains options for the ConsumerClient creation
   */
  struct ConsumerClientOptions
  {
    /**@brief ApplicationID is used as the identifier when setting the User-Agent property.
     */
    std::string ApplicationID = "";

    /**@brief  RetryOptions controls how often operations are retried from this client and any
     * Receivers and Senders created from this client.
     */
    Azure::Core::Http::Policies::RetryOptions RetryOptions{};

    /**@brief  Message sender options.
     */
    Azure::Core::Amqp::_internal::MessageReceiverOptions ReceiverOptions{};
  };

  struct ConsumerClientCreds
  {
    std::string ConnectionString;

    // the Event Hubs namespace name (ex: myservicebus.servicebus.windows.net)
    std::string FullyQualifiedNamespace;

    std::string EventHub;

    std::string ConsumerGroup;

    std::shared_ptr<Core::Credentials::TokenCredential> Credential;
    std::shared_ptr<Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential>
        SasCredential;

    std::string HostUrl;
  };

}}}} // namespace Azure::Messaging::EventHubs::Models
