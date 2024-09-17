// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/connection_string_credential.hpp"

#include "azure/core/amqp/internal/connection.hpp"
#include "azure/core/amqp/internal/network/socket_transport.hpp"

#include <azure/core/base64.hpp>
#include <azure/core/url.hpp>

#if ENABLE_UAMQP
#include <azure_c_shared_utility/sastoken.h>
#include <azure_c_shared_utility/strings.h>
#include <azure_c_shared_utility/urlencode.h>
#endif

#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <vector>

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  // Split a source string into separate substrings via a delimiter.
  std::vector<std::string> SplitString(const std::string& s, char separator)
  {
    std::vector<std::string> result;

    const auto len = s.size();
    size_t start = 0;
    while (start < len)
    {
      auto end = s.find(separator, start);
      if (end == std::string::npos)
      {
        end = len;
      }

      result.push_back(s.substr(start, end - start));

      start = end + 1;
    }

    return result;
  }

  //
  // A ServiceBus connection string has the following format:
  // "Endpoint=sb://<namespace>.servicebus.windows.net/;SharedAccessKeyName=<KeyName>;SharedAccessKey=<KeyValue>;EntityPath=<entity>"
  //
  // The connection string is a series of key=value pairs separated by semicolons. The endpoint
  // is required and the shared access key name and shared access key are optional. The shared
  // access key name and shared access key are required if the endpoint is not a local endpoint.
  // The endpoint is formatted as a URL, extract the host and port from the endpoint and use them
  // when constructing the SaslAnonymous transport.
  void ConnectionStringParser::ParseConnectionString(const std::string& connectionString)
  {
    std::unordered_map<std::string, std::string> elements;
    // Split the connection string into separate components.
    auto connectionElements{SplitString(connectionString, ';')};
    if (connectionElements.empty())
    {
      throw std::runtime_error("Connection string elements cannot be empty.");
    }
    for (const auto& element : connectionElements)
    {
      auto startDelimiter = element.find('=');
      if (startDelimiter == std::string::npos)
      {
        throw std::runtime_error("Could not find = delimiter in string " + element);
      }
      auto key = element.substr(0, startDelimiter);
      auto value = element.substr(startDelimiter + 1);

      elements.emplace(std::make_pair(key, value));
    }

    // Now that we've parsed the connection string, we can extract the elements we care about.
    {
      auto ep = elements.find("Endpoint");
      if (ep != elements.end())
      {
        m_endpoint = ep->second;

        // The m_endpoint should be a URL, extract the host and optionally port from the URL.
        Azure::Core::Url endpointUrl{m_endpoint};
        m_hostName = endpointUrl.GetHost();
        m_port = endpointUrl.GetPort() != 0 ? endpointUrl.GetPort()
                                            : Azure::Core::Amqp::_internal::AmqpTlsPort;
      }
      else
      {
        throw std::runtime_error("Connection string must contain an endpoint.");
      }
    }
    {
      auto useDevelopmentEmulator = elements.find("UseDevelopmentEmulator");
      if (useDevelopmentEmulator != elements.end())
      {
        m_useDevelopmentEmulator = useDevelopmentEmulator->second == "true";
      }
    }
    {
      auto sak = elements.find("SharedAccessKeyName");
      if (sak != elements.end())
      {
        m_sharedAccessKeyName = sak->second;
      }
    }
    {
      auto sak = elements.find("SharedAccessKey");
      if (sak != elements.end())
      {
        m_sharedAccessKey = sak->second;
      }
    }
    {
      auto ep = elements.find("EntityPath");
      if (ep != elements.end())
      {
        m_entityPath = ep->second;
      }
    }
  }

#if ENABLE_UAMQP
  std::shared_ptr<Network::_internal::Transport>
  ServiceBusSasConnectionStringCredential::GetTransport() const
  {
    //    // Construct a SASL Anonymous transport
    //    return std::make_shared<Network::SaslTransport>(GetHostName(), GetPort());
    return std::make_shared<Network::_internal::Transport>(
        Network::_internal::SocketTransportFactory::Create(GetHostName(), GetPort()));
  }
#endif

  Credentials::AccessToken ServiceBusSasConnectionStringCredential::GetToken(
      Credentials::TokenRequestContext const& tokenRequestContext,
      Context const& context) const
  {
    Credentials::AccessToken rv;
    rv.ExpiresOn = Azure::DateTime::clock::now() + std::chrono::minutes(60);
    rv.Token = GenerateSasToken(static_cast<std::chrono::system_clock::time_point>(rv.ExpiresOn));
    (void)tokenRequestContext;
    (void)context;
    return rv;
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
#if ENABLE_UAMQP
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
#else
    (void)expirationTime;
    return std::string();
#endif // ENABLE_UAMQP
  }
}}}} // namespace Azure::Core::Amqp::_internal
