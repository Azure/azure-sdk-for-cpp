// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/connection_string_credential.hpp"

#include "azure/core/amqp/internal/network/socket_transport.hpp"

#include <azure/core/base64.hpp>
#include <azure/core/url.hpp>

#include <azure_c_shared_utility/sastoken.h>
#include <azure_c_shared_utility/strings.h>
#include <azure_c_shared_utility/urlencode.h>

#include <algorithm>
#include <iterator>
#include <vector>

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  std::shared_ptr<Network::_internal::Transport>
  ServiceBusSasConnectionStringCredential::GetTransport() const
  {
    //    // Construct a SASL Anonymous transport
    //    return std::make_shared<Network::SaslTransport>(GetHostName(), GetPort());
    return std::make_shared<Network::_internal::Transport>(
        Network::_internal::SocketTransportFactory::Create(GetHostName(), GetPort()));
  }


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
    // For now, create the SAS token using the azure-c-shared-utility functions for SAS token
    // creations. In the future (when we've integrated into Azure Core), move this to the Azure Core
    // APIs.

    /* Construct a SAS token */
    /* Base64 encode the SharedAccessKey - this needs to be double encoded because of <reasons> */
    std::vector<uint8_t> dataToEncode;
    dataToEncode.reserve(GetSharedAccessKey().size());
    std::transform(
        std::begin(GetSharedAccessKey()),
        std::end(GetSharedAccessKey()),
        std::back_inserter(dataToEncode),
        [](char ch) { return static_cast<uint8_t>(ch); });

    std::string encodedKeyValue{Azure::Core::Convert::Base64Encode(dataToEncode)};
    Azure::Core::Url resourceUri{GetEndpoint()};
    resourceUri.AppendPath(GetEntityPath());

    auto sasKeyValue{STRING_construct(encodedKeyValue.c_str())};
    auto encodedResourceUri = URL_EncodeString(resourceUri.GetAbsoluteUrl().c_str());
    auto sasKeyName = STRING_construct(GetSharedAccessKeyName().c_str());
    auto sasToken = SASToken_Create(
        sasKeyValue,
        encodedResourceUri,
        sasKeyName,
        std::chrono::duration_cast<std::chrono::seconds>(expirationTime.time_since_epoch())
            .count());
    if (sasToken == nullptr)
    {
      throw std::runtime_error("Could not create SAS token.");
    }
    std::string rv(STRING_c_str(sasToken));
    STRING_delete(sasToken);
    STRING_delete(sasKeyValue);
    STRING_delete(sasKeyName);
    STRING_delete(encodedResourceUri);
    return rv;
  }
}}}} // namespace Azure::Core::Amqp::_internal
