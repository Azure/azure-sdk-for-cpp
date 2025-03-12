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

}}}} // namespace Azure::Core::Amqp::_internal
