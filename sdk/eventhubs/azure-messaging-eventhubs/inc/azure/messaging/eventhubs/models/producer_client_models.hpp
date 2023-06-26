// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

// cspell: words myservicebus

#pragma once
#include <azure/core/amqp.hpp>
#include <azure/core/amqp/management.hpp>
#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/policies/policy.hpp>

#include <iostream>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Models {

  /**@brief Credentials data bag used internally by the producer
   */
  struct ProducerClientCreds
  {
    std::string ConnectionString;

    // the Event Hubs namespace name (ex: myservicebus.servicebus.windows.net)
    std::string FullyQualifiedNamespace;

    std::string EventHub;

    std::string TargetUrl;

    std::shared_ptr<Core::Credentials::TokenCredential> Credential;
    std::shared_ptr<Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential>
        SasCredential;
  };

  /**@brief Contains options for the ProducerClient creation
   */
  struct ProducerClientOptions
  {
    /**@brief  Application ID that will be passed to the namespace.
     */
    std::string ApplicationID = "";

    /**@brief  RetryOptions controls how often operations are retried from this client and any
     * Receivers and Senders created from this client.
     */
    Azure::Core::Http::Policies::RetryOptions RetryOptions{};

    /**@brief  Message sender options.
     */
    Azure::Core::Amqp::_internal::MessageSenderOptions SenderOptions{};
  };

}}}} // namespace Azure::Messaging::EventHubs::Models
