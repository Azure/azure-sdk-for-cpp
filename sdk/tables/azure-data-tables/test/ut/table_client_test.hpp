#pragma once
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/data/tables/models.hpp"
#include "azure/data/tables/tables_clients.hpp"
#include "test/ut/test_base.hpp"

namespace Azure { namespace Data { namespace Test {

  struct ConnectionStringParts
  {
    std::string AccountName;
    std::string AccountKey;
    Azure::Core::Url TableServiceUrl;
  };

  enum class AuthType
  {
    Key = 0x0,
  };

  class TablesClientTest : public Azure::Storage::Test::StorageTest,
                           public ::testing::WithParamInterface<AuthType> {
  protected:
    void SetUp() override;
    void TearDown() override;
    Azure::Data::Tables::TableClient CreateTableClientForTest(
        Tables::TableClientOptions& clientOptions);

    Azure::Data::Tables::TableClient CreateKeyTableClientForTest(
        Tables::TableClientOptions& clientOptions);
    std::string GetConnectionString()
    {
      const static std::string ConnectionString = "";

      if (!ConnectionString.empty())
      {
        return ConnectionString;
      }
      const static std::string envConnectionString = GetEnv("STANDARD_STORAGE_CONNECTION_STRING");
      if (!envConnectionString.empty())
      {
        return envConnectionString;
      }
      throw std::runtime_error("Cannot find connection string.");
    }

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

    std::string GetAccountName()
    {
      return ParseConnectionString(GetConnectionString()).AccountName;
    }

    std::string GetAccountKey() { return ParseConnectionString(GetConnectionString()).AccountKey; }

  protected:
    std::string m_tableName;
    std::shared_ptr<Tables::TableServiceClient> m_tableServiceClient;
    std::shared_ptr<Tables::TableClient> m_tableClient;
  };
}}} // namespace Azure::Data::Test
