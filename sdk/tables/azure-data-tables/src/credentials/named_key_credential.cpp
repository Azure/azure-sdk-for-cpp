// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/data/tables/credentials/named_key_credential.hpp"

#include <algorithm>

namespace Azure { namespace Data { namespace Tables { namespace Credentials { namespace _detail {

  ConnectionStringParts ParseConnectionString(const std::string& connectionString)
  {
    std::map<std::string, std::string> connectionStringMap;

    std::string::const_iterator cur = connectionString.begin();

    while (cur != connectionString.end())
    {
      auto key_begin = cur;
      auto key_end = std::find(cur, connectionString.end(), '=');
      std::string key = std::string(key_begin, key_end);
      cur = key_end;
      if (cur != connectionString.end())
      {
        ++cur;
      }

      auto value_begin = cur;
      auto value_end = std::find(cur, connectionString.end(), ';');
      std::string value = std::string(value_begin, value_end);
      cur = value_end;
      if (cur != connectionString.end())
      {
        ++cur;
      }

      if (!key.empty() || !value.empty())
      {
        connectionStringMap[std::move(key)] = std::move(value);
      }
    }

    auto getWithDefault = [](const std::map<std::string, std::string>& m,
                             const std::string& key,
                             const std::string& defaultValue = std::string()) {
      auto ite = m.find(key);
      return ite == m.end() ? defaultValue : ite->second;
    };

    ConnectionStringParts connectionStringParts;

    std::string defaultEndpointsProtocol
        = getWithDefault(connectionStringMap, "DefaultEndpointsProtocol", "https");
    std::string EndpointSuffix
        = getWithDefault(connectionStringMap, "EndpointSuffix", "core.windows.net");
    std::string accountName = getWithDefault(connectionStringMap, "AccountName");
    connectionStringParts.AccountName = accountName;

    std::string endpoint = getWithDefault(connectionStringMap, "TableEndpoint");
    if (endpoint.empty() && !accountName.empty())
    {
      endpoint = defaultEndpointsProtocol + "://" + accountName + ".table." + EndpointSuffix;
    }
    connectionStringParts.TableServiceUrl = Azure::Core::Url(std::move(endpoint));

    std::string accountKey = getWithDefault(connectionStringMap, "AccountKey");
    connectionStringParts.AccountKey = accountKey;
    if (!accountKey.empty())
    {
      if (accountName.empty())
      {
        throw std::runtime_error("Cannot find account name in connection string.");
      }
      connectionStringParts.KeyCredential
          = std::make_shared<NamedKeyCredential>(accountName, accountKey);
    }

    std::string sas = getWithDefault(connectionStringMap, "SharedAccessSignature");
    if (!sas.empty())
    {
      if (sas[0] != '?')
      {
        sas = '?' + sas;
      }

      connectionStringParts.TableServiceUrl
          = Azure::Core::Url(connectionStringParts.TableServiceUrl.GetAbsoluteUrl() + sas);
    }

    return connectionStringParts;
  }

  std::string GetDefaultScopeForAudience(const std::string& audience)
  {
    if (!audience.empty() && audience.back() == '/')
    {
      return audience + ".default";
    }
    return audience + "/.default";
  }
}}}}} // namespace Azure::Data::Tables::Credentials::_detail
