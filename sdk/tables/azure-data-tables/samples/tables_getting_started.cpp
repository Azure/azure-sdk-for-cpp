// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/data/tables.hpp>

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <thread>

using namespace Azure::Data::Tables;
const std::string TableName = "sample1";

int main()
{
  // create table service client with the specified url containing the account name.
  const std::string serviceUrl = "https://account-name.table.core.windows.net";
  auto tableServiceClient = TableServiceClient(serviceUrl);

  // create new table
  tableServiceClient.CreateTable(TableName);

  // query tables
  auto tables = tableServiceClient.QueryTables();

  // print table names
  for (auto table : tables.Tables)
  {
    std::cout << table.TableName << std::endl;
  }
  // delete existing table
  tableServiceClient.DeleteTable(TableName);
  return 0;
}
