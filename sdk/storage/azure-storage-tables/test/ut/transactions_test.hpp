#pragma once
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "test/ut/test_base.hpp"

#include <azure/storage/tables/rest_client.hpp>
#include <azure/storage/tables/transactions.hpp>

namespace Azure { namespace Storage { namespace Test {

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
        Azure::Storage::Tables::Models::TransactionAction action);
    void CheckTransactionBody(
        std::string const& body,
        Azure::Storage::Tables::Models::TransactionAction action);
  };
}}} // namespace Azure::Storage::Test
