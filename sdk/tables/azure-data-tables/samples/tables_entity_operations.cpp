// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/data/tables.hpp>
#include <azure/identity.hpp>

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <thread>

using namespace Azure::Data::Tables;
using namespace Azure::Data::Tables::Models;
const std::string TableName = "table";

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
  auto tableClient = Azure::Data::Tables::TableClient(GetServiceUrl(), TableName, credential);

  // create new table
  tableServiceClient.CreateTable(TableName);

  // list tables
  auto tables = tableServiceClient.QueryTables();
  for (auto table : tables.Tables)
  {
    std::cout << table.TableName << std::endl;
  }
  // init new entity
  Azure::Data::Tables::Models::TableEntity entity;
  entity.SetPartitionKey("P1");
  entity.SetRowKey("R1");
  entity.Properties["Name"] = TableEntityProperty("Azure");
  entity.Properties["Product"] = TableEntityProperty("Tables");
  // create new entity
  auto response = tableClient.AddEntity(entity);

  // update entity
  std::cout << response.Value.ETag << std::endl;
  entity.Properties["Product"] = TableEntityProperty("Tables2");
  auto updateResponse = tableClient.UpdateEntity(entity);
  std::cout << updateResponse.Value.ETag << std::endl;

  // merge entity
  entity.Properties["Product"] = TableEntityProperty("Tables3");
  entity.SetETag(updateResponse.Value.ETag);
  auto updateResponse2 = tableClient.MergeEntity(entity);

  // delete entity
  std::cout << updateResponse2.Value.ETag << std::endl;
  entity.SetETag(updateResponse2.Value.ETag);
  auto deleteResponse = tableClient.DeleteEntity(entity);

  // delete existing table
  tableServiceClient.DeleteTable(TableName);
  return 0;
}
