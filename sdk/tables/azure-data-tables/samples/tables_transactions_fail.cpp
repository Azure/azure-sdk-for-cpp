// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/data/tables.hpp>
#include <azure/identity.hpp>

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <thread>

const std::string TableName = "transactions2";

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

  // create table
  tableServiceClient.CreateTable(TableName);
  // get table client from table service client
  auto tableClient = tableServiceClient.GetTableClient(TableName);

  // Create two table entities
  Azure::Data::Tables::Models::TableEntity entity;
  Azure::Data::Tables::Models::TableEntity entity2;
  entity.SetPartitionKey("P1");
  entity.SetRowKey("R1");
  entity.Properties["Name"] = Azure::Data::Tables::Models::TableEntityProperty("Azure");
  entity.Properties["Product"] = Azure::Data::Tables::Models::TableEntityProperty("Tables");
  entity2.SetPartitionKey("P1");
  entity2.SetRowKey("R1");
  entity2.Properties["Name"] = Azure::Data::Tables::Models::TableEntityProperty("Azure2");
  entity2.Properties["Product"] = Azure::Data::Tables::Models::TableEntityProperty("Tables2");

  // Create a transaction with two steps
  std::vector<Azure::Data::Tables::Models::TransactionStep> steps;
  steps.emplace_back(Azure::Data::Tables::Models::TransactionStep{
      Azure::Data::Tables::Models::TransactionActionType::Add, entity});
  steps.emplace_back(Azure::Data::Tables::Models::TransactionStep{
      Azure::Data::Tables::Models::TransactionActionType::Add, entity2});

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
