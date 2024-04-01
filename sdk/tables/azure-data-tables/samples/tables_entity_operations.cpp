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
const std::string TableName = "table";

int main()
{
  auto tableServiceClient = TableServiceClient::CreateFromConnectionString(GetConnectionString());
  auto tableClient = TableClient::CreateFromConnectionString(GetConnectionString(), TableName);

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
  entity.SetPartitionKey ("P1");
  entity.SetRowKey ( "R1");
  entity.Properties["Name"] = "Azure";
  entity.Properties["Product"] = "Tables";
  // create new entity
  auto response = tableClient.AddEntity(entity);

  // update entity
  std::cout << response.Value.ETag << std::endl;
  entity.Properties["Product"] = "Tables2";
  auto updateResponse = tableClient.UpdateEntity(entity);
  std::cout << updateResponse.Value.ETag << std::endl;

  // merge entity
  entity.Properties["Product"] = "Tables3";
  entity.SetETag( updateResponse.Value.ETag);
  auto updateResponse2 = tableClient.MergeEntity(entity);

  // delete entity
  std::cout << updateResponse2.Value.ETag << std::endl;
  entity.SetETag(updateResponse2.Value.ETag);
  auto deleteResponse = tableClient.DeleteEntity(entity);

  // delete existing table
  tableServiceClient.DeleteTable(TableName);
  return 0;
}
