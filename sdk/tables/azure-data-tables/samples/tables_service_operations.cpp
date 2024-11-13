// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/data/tables.hpp>
#include <azure/identity.hpp>

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <thread>

using namespace Azure::Data::Tables;
const std::string TableName = "sample1";

// The following environment variables must be set before running the sample.
// * ACCOUNT_NAME: The name of the storage account.
std::string GetAccountName() { return std::getenv("ACCOUNT_NAME"); }
std::string const GetServiceUrl()
{
  return std::string{"https://" + GetAccountName() + ".table.core.windows.net/"};
}

int main()
{
  auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();
  auto tableServiceClient = Azure::Data::Tables::TableServiceClient(GetServiceUrl(), credential);
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
