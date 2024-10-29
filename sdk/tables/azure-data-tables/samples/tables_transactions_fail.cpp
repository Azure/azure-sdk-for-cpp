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
using namespace Azure::Data::Tables::Models;

const std::string TableName = "transactions2";

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

  // create table
  tableServiceClient.CreateTable(TableName);
  // get table client from table service client
  auto tableClient = tableServiceClient.GetTableClient(TableName);

  // Create two table entities
  TableEntity entity;
  TableEntity entity2;
  entity.SetPartitionKey("P1");
  entity.SetRowKey("R1");
  entity.Properties["Name"] = TableEntityProperty("Azure");
  entity.Properties["Product"] = TableEntityProperty("Tables");
  entity2.SetPartitionKey("P1");
  entity2.SetRowKey("R1");
  entity2.Properties["Name"] = TableEntityProperty("Azure2");
  entity2.Properties["Product"] = TableEntityProperty("Tables2");

  // Create a transaction with two steps
  std::vector<TransactionStep> steps;
  steps.emplace_back(TransactionStep{TransactionActionType::Add, entity});
  steps.emplace_back(TransactionStep{TransactionActionType::Add, entity2});

  // Submit the transaction
  auto response = tableClient.SubmitTransaction(steps);

  // Check the response
  if (!response.Value.Error.HasValue())
  {
    std::cout << "Transaction completed successfully." << std::endl;
  }
  else
  {
    std::cout << "Transaction failed with error: " << response.Value.Error.Value().Message
              << std::endl;
  }
  // delete existing table
  tableServiceClient.DeleteTable(TableName);
  return 0;
}
