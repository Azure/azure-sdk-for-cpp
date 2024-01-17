#pragma once
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/data/tables/tables_clients.hpp"
#include "azure/data/tables/transactions.hpp"
#include "test/ut/test_base.hpp"

namespace Azure { namespace Data { namespace Test {

  class TransactionsBodyTest : public Azure::Storage::Test::StorageTest {

  protected:
    const std::string url = "someUrl";
    const std::string tableName = "someTableName";
    const std::string partitionKey = "somePartitionKey";
    const std::string rowKey = "someRowKey";
    std::string batch;
    std::string changeset;
    void CheckContentLines(
        std::vector<std::string> const& lines,
        Azure::Data::Tables::Models::TransactionAction action);
    void CheckTransactionBody(
        std::string const& body,
        Azure::Data::Tables::Models::TransactionAction action);
  };
}}} // namespace Azure::Data::Test
