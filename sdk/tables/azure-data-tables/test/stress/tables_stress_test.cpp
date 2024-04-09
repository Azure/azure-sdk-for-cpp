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

#include "azure/data/tables.hpp"

#include <iostream>

using namespace Azure::Data::Tables;
namespace Azure { namespace Data { namespace Tables { namespace StressTest {

  class TransactionStressTest final {
  public:
    std::string Transactions(int num)
    {
      TableClient client("http://localhost:7777", "table");
      std::vector<Models::TransactionStep> steps;
      for (int i = 0; i < num; i++)
      {
        Models::TableEntity entity;
        entity.SetPartitionKey("pk1");
        entity.SetRowKey("rk1");
        entity.Properties.emplace(
            "prop1", Azure::Data::Tables::Models::TableEntityProperty("value1"));
        entity.Properties.emplace(
            "prop2", Azure::Data::Tables::Models::TableEntityProperty("value2"));
        steps.emplace_back(
            Models::TransactionStep{Models::TransactionActionType::InsertMerge, entity});

        steps.emplace_back(Models::TransactionStep{Models::TransactionActionType::Delete, entity});
        steps.emplace_back(
            Models::TransactionStep{Models::TransactionActionType::UpdateReplace, entity});
        steps.emplace_back(
            Models::TransactionStep{Models::TransactionActionType::UpdateMerge, entity});
      }
      auto result = client.PreparePayload("batch", "changeset", steps);
      return result;
    }
  };
}}}} // namespace Azure::Data::Tables::StressTest

int main()
{
  Azure::Data::Tables::StressTest::TransactionStressTest test;
  std::cout << "--------------\tSTARTING TEST\t--------------" << std::endl;
  std::cout << "--------------\tPRE WARMUP\t--------------" << std::endl;
  test.Transactions(WARMUP);

  std::cout << "--------------\tPOST WARMUP\t--------------" << std::endl;

  for (int i = 0; i < ROUNDS; i++)
  {
    std::cout << "--------------\tTEST ITERATION:" << i << "\t--------------" << std::endl;
    test.Transactions(REQUESTS);

    std::cout << "--------------\tDONE ITERATION:" << i << "\t--------------" << std::endl;
  }

  return 0;
}
