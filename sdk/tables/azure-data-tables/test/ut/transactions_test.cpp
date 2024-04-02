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
        "--batch\nContent-Type: multipart/mixed; "
        "boundary=changeset\n\n\n\n--changeset--\n--batch\n");
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
            lines[4],
            "DELETE " + url + "/" + tableName + "(PartitionKey='" + partitionKey + "',RowKey='"
                + rowKey + "') HTTP/1.1");
        break;
      case Models::TransactionActionType::UpdateMerge:
        EXPECT_EQ(
            lines[4],
            "MERGE " + url + "/" + tableName + "(PartitionKey='" + partitionKey + "',RowKey='"
                + rowKey + "') HTTP/1.1");
        break;
      case Models::TransactionActionType::UpdateReplace:
        EXPECT_EQ(
            lines[4],
            "PUT " + url + "/" + tableName + "(PartitionKey='" + partitionKey + "',RowKey='"
                + rowKey + "') HTTP/1.1");
        break;
      case Models::TransactionActionType::InsertMerge:
        EXPECT_EQ(
            lines[4],
            "MERGE " + url + "/" + tableName + "(PartitionKey='" + partitionKey + "',RowKey='"
                + rowKey + "') HTTP/1.1");
        break;
      case Models::TransactionActionType::InsertReplace:
        EXPECT_EQ(
            lines[4],
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
    (void)action;
    std::stringstream ss(body);
    std::string line;
    std::getline(ss, line, '\n');

    // line1
    EXPECT_EQ(line.substr(0, 8), "--batch_");
    EXPECT_EQ(line.size(), 8);
    batch = line.substr(2, line.length() - 1);
    // line2
    std::getline(ss, line, '\n');
    EXPECT_EQ(line, "Content-Type: multipart/mixed; boundary=changeset_1");
    changeset = line.substr(40, line.length() - 1);

    // line3
    std::getline(ss, line, '\n');
    EXPECT_EQ(line, "");
    std::vector<std::string> contentLines;
    std::getline(ss, line, '\n');
    while (line != "--" + batch)
    {
      contentLines.push_back(line);
      std::getline(ss, line, '\n');
    }
    CheckContentLines(contentLines, action);
    EXPECT_EQ(line, "--" + batch);
  }
}}} // namespace Azure::Data::Test
