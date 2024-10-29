// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/data/tables.hpp>
#include <azure/identity.hpp>

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <thread>

using namespace Azure::Identity;
using namespace Azure::Data::Tables;
const std::string TableName = "sample1";

std::string GetAccountName()
{
  const static std::string envAccountName = std::getenv("ACCOUNT_NAME");
  if (!envAccountName.empty())
  {
    return envAccountName;
  }
  throw std::runtime_error("Cannot find account name.");
}

int main()
{
  // create table service client with the specified url containing the account name.
  const std::string accountName = GetAccountName();
  const std::string serviceUrl = "https://" + accountName + ".table.core.windows.net";
  auto credential = std::make_shared<DefaultAzureCredential>();
  auto tableServiceClient = TableServiceClient(serviceUrl, credential);

  // query tables
  auto tables = tableServiceClient.QueryTables();

  // print table names
  for (auto table : tables.Tables)
  {
    std::cout << table.TableName << std::endl;
  }

  // get statistics
  auto statistics = tableServiceClient.GetStatistics();

  std::cout << statistics.Value.GeoReplication.Status.ToString() << std::endl;

  // get service properties
  auto serviceProperties = tableServiceClient.GetServiceProperties();

  std::cout << serviceProperties.Value.MinuteMetrics.Version << std::endl;

  return 0;
}
