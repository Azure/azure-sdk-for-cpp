// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @brief Validates the Azure Core transport adapters with fault responses from server.
 *
 * @note This test requires the Http-fault-injector
 * (https://github.com/Azure/azure-sdk-tools/tree/main/tools/http-fault-injector) running. Follow
 * the instructions to install and run the server before running this test.
 *
 */

#define REQUESTS 250
#define WARMUP 100
#define ROUNDS 1000

#include <azure/data/tables.hpp>

#include <iostream>

using namespace Azure::Data::Tables;

std::string Transactions(int num)
{
  TableClient client("http://localhost:7777", "table");
  auto transaction = client.CreateTransaction("pk1");
  for (int i = 0; i < num; i++)
  {
    Models::TableEntity entity;
    entity.PartitionKey = "pk1";
    entity.RowKey = "rk1";
    entity.Properties.emplace("prop1", "value1");
    entity.Properties.emplace("prop2", "value2");
    transaction.CreateEntity(entity);
    transaction.DeleteEntity(entity);
    transaction.UpdateEntity(entity);
    transaction.MergeEntity(entity);
  }
  auto result = transaction.PreparePayload();
  return result;
}

int main()
{
  std::cout << "--------------\tSTARTING TEST\t--------------" << std::endl;
  std::cout << "--------------\tPRE WARMUP\t--------------" << std::endl;
  Transactions(WARMUP);

  std::cout << "--------------\tPOST WARMUP\t--------------" << std::endl;

  for (int i = 0; i < ROUNDS; i++)
  {
    std::cout << "--------------\tTEST ITERATION:" << i << "\t--------------" << std::endl;
    Transactions(REQUESTS);

    std::cout << "--------------\tDONE ITERATION:" << i << "\t--------------" << std::endl;
  }

  return 0;
}
