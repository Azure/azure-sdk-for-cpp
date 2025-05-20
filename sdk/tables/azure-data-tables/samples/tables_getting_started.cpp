// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/data/tables.hpp>
#include <azure/identity.hpp>

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <thread>

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
  auto credential = std::make_shared<Azure::Identity::AzureCliCredential>();
  auto tableServiceClient = Azure::Data::Tables::TableServiceClient(GetServiceUrl(), credential);

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
