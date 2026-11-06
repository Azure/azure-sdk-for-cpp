// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "transactions_test.hpp"

#include <chrono>
#include <sstream>
#include <string>
#include <thread>
using namespace Azure::Data::Tables;

namespace Azure { namespace Data { namespace Test {
  const std::string url("someUrl");
  const std::string tableName("someTableName");
  const std::string partitionKey("somePartitionKey");
  const std::string rowKey("someRowKey");

  TEST_F(TransactionsBodyTest, TransactionCreate)
  {
    std::vector<Models::TransactionStep> steps;
    TableClient client("http://localhost:7777", "table");

    auto ser = client.PreparePayload("batch", "changeset", steps);

    EXPECT_EQ(
        ser,
        "--batch\r\nContent-Type: multipart/mixed; "
        "boundary=changeset\r\n\r\n--changeset--\r\n--batch--\r\n");
  }

  TEST_F(TransactionsBodyTest, TransactionBodyAddOp)
  {
    std::vector<Models::TransactionStep> steps;
    TableClient client(url, tableName);

    Azure::Data::Tables::Models::TableEntity entity;
    entity.SetRowKey(rowKey);
    entity.SetPartitionKey(partitionKey);
    steps.emplace_back(Models::TransactionStep{Models::TransactionActionType::Add, entity});
    auto serialized = client.PreparePayload("batch_", "changeset_1", steps);

    CheckTransactionBody(serialized, Models::TransactionActionType::Add);
  }

  TEST_F(TransactionsBodyTest, TransactionBodyInsertMergeOp)
  {
    std::vector<Models::TransactionStep> steps;
    TableClient client(url, tableName);

    Azure::Data::Tables::Models::TableEntity entity;
    entity.SetRowKey(rowKey);
    entity.SetPartitionKey(partitionKey);
    steps.emplace_back(Models::TransactionStep{Models::TransactionActionType::InsertMerge, entity});
    auto serialized = client.PreparePayload("batch_", "changeset_1", steps);

    CheckTransactionBody(serialized, Models::TransactionActionType::InsertMerge);
  }

  TEST_F(TransactionsBodyTest, TransactionBodyInsertReplaceOp)
  {
    std::vector<Models::TransactionStep> steps;
    TableClient client(url, tableName);

    Azure::Data::Tables::Models::TableEntity entity;
    entity.SetRowKey(rowKey);
    entity.SetPartitionKey(partitionKey);
    steps.emplace_back(
        Models::TransactionStep{Models::TransactionActionType::InsertReplace, entity});
    auto serialized = client.PreparePayload("batch_", "changeset_1", steps);

    CheckTransactionBody(serialized, Models::TransactionActionType::InsertReplace);
  }

  TEST_F(TransactionsBodyTest, TransactionBodyDeleteOp)
  {
    std::vector<Models::TransactionStep> steps;
    TableClient client(url, tableName);

    Azure::Data::Tables::Models::TableEntity entity;
    entity.SetRowKey(rowKey);
    entity.SetPartitionKey(partitionKey);
    steps.emplace_back(Models::TransactionStep{Models::TransactionActionType::Delete, entity});
    auto serialized = client.PreparePayload("batch_", "changeset_1", steps);
    CheckTransactionBody(serialized, Models::TransactionActionType::Delete);
  }

  TEST_F(TransactionsBodyTest, TransactionBodyUpdateMergeOp)
  {
    std::vector<Models::TransactionStep> steps;
    TableClient client(url, tableName);

    Azure::Data::Tables::Models::TableEntity entity;
    entity.SetRowKey(rowKey);
    entity.SetPartitionKey(partitionKey);
    steps.emplace_back(Models::TransactionStep{Models::TransactionActionType::UpdateMerge, entity});
    auto serialized = client.PreparePayload("batch_", "changeset_1", steps);
    CheckTransactionBody(serialized, Models::TransactionActionType::UpdateMerge);
  }

  TEST_F(TransactionsBodyTest, TransactionBodyUpdateReplaceOp)
  {
    std::vector<Models::TransactionStep> steps;
    TableClient client(url, tableName);

    Azure::Data::Tables::Models::TableEntity entity;
    entity.SetRowKey(rowKey);
    entity.SetPartitionKey(partitionKey);
    steps.emplace_back(
        Models::TransactionStep{Models::TransactionActionType::UpdateReplace, entity});
    auto serialized = client.PreparePayload("batch_", "changeset_1", steps);
    CheckTransactionBody(serialized, Models::TransactionActionType::UpdateReplace);
  }

  void TransactionsBodyTest::CheckContentLines(
      std::vector<std::string> const& lines,
      Models::TransactionActionType action)
  {
    EXPECT_EQ(lines[0], "--" + changeset);
    EXPECT_EQ(lines[1], "Content-Type: application/http");
    EXPECT_EQ(lines[2], "Content-Transfer-Encoding: binary");
    switch (action)
    {
      case Models::TransactionActionType::Add:
        EXPECT_EQ(lines[4], "POST " + url + "/" + tableName + " HTTP/1.1");
        break;
      case Models::TransactionActionType::Delete:
        EXPECT_EQ(
            Core::Url::Decode(lines[4]),
            "DELETE " + url + "/" + tableName + "(PartitionKey='" + partitionKey + "',RowKey='"
                + rowKey + "') HTTP/1.1");
        break;
      case Models::TransactionActionType::UpdateMerge:
        EXPECT_EQ(
            Core::Url::Decode(lines[4]),
            "MERGE " + url + "/" + tableName + "(PartitionKey='" + partitionKey + "',RowKey='"
                + rowKey + "') HTTP/1.1");
        break;
      case Models::TransactionActionType::UpdateReplace:
        EXPECT_EQ(
            Core::Url::Decode(lines[4]),
            "PUT " + url + "/" + tableName + "(PartitionKey='" + partitionKey + "',RowKey='"
                + rowKey + "') HTTP/1.1");
        break;
      case Models::TransactionActionType::InsertMerge:
        EXPECT_EQ(
            Core::Url::Decode(lines[4]),
            "MERGE " + url + "/" + tableName + "(PartitionKey='" + partitionKey + "',RowKey='"
                + rowKey + "') HTTP/1.1");
        break;
      case Models::TransactionActionType::InsertReplace:
        EXPECT_EQ(
            Core::Url::Decode(lines[4]),
            "PUT " + url + "/" + tableName + "(PartitionKey='" + partitionKey + "',RowKey='"
                + rowKey + "') HTTP/1.1");
        break;
    }
    EXPECT_EQ(lines[lines.size() - 1], "--" + changeset + "--");
  }
  void TransactionsBodyTest::CheckTransactionBody(
      std::string const& body,
      Models::TransactionActionType action)
  {
    std::stringstream ss(body);
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(ss, line, '\n'))
    {
      ASSERT_FALSE(line.empty());
      ASSERT_EQ(line.back(), '\r');
      line.pop_back();
      lines.emplace_back(std::move(line));
    }
    ASSERT_GE(lines.size(), 9U);
    EXPECT_EQ(lines[0], "--batch_");
    batch = lines[0].substr(2);
    EXPECT_EQ(lines[1], "Content-Type: multipart/mixed; boundary=changeset_1");
    changeset = "changeset_1";
    EXPECT_EQ(lines[2], "");
    std::vector<std::string> contentLines(lines.begin() + 3, lines.end() - 1);
    CheckContentLines(contentLines, action);
    EXPECT_EQ(lines.back(), "--" + batch + "--");
  }
}}} // namespace Azure::Data::Test
