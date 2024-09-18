// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/connection_string_credential.hpp"

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  // Generate a Shared Access Signature token for a ServiceBus client.
  //
  // The spec for a SharedAccessSignature is here:
  // https://learn.microsoft.com/en-us/azure/service-bus-messaging/service-bus-sas#generate-a-shared-access-signature-token
  // Samples for SAS generation are here:
  // https://learn.microsoft.com/en-us/rest/api/eventhub/generate-sas-token
  //
  std::string ServiceBusSasConnectionStringCredential::GenerateSasToken(
      std::chrono::system_clock::time_point const& expirationTime) const
  {
    (void)expirationTime;
    return std::string();
  }
}}}} // namespace Azure::Core::Amqp::_internal
