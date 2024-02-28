// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/data/tables.hpp>

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <thread>

std::string GetConnectionString()
{
  const static std::string ConnectionString = "";

  if (!ConnectionString.empty())
  {
    return ConnectionString;
  }
  const static std::string envConnectionString = std::getenv("STANDARD_STORAGE_CONNECTION_STRING");
  if (!envConnectionString.empty())
  {
    return envConnectionString;
  }
  throw std::runtime_error("Cannot find connection string.");
}

using namespace Azure::Data::Tables;
const std::string TableName = "sample1";

int main()
{
  auto tableServiceClient = TableServicesClient::CreateFromConnectionString(GetConnectionString());

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
