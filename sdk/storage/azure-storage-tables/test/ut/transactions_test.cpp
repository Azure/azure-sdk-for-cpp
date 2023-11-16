#include "table_client_test.hpp"

#include <azure/storage/tables/models.hpp>
#include <azure/storage/tables/transactions.hpp>

#include <chrono>
#include <sstream>
#include <string>
#include <thread>
using namespace Azure::Storage::Tables;

namespace Azure { namespace Storage { namespace Test {
  const std::string url("someUrl");
  const std::string tableName("someTableName");
  const std::string partitionKey("somePartitionKey");
  std::string batch;
  std::string changeset;
  TEST_F(TablesClientTest, TransactionCreate)
  {
    Tables::Transaction transaction(url, tableName, partitionKey);
    EXPECT_EQ(transaction.GetPartitionKey(), partitionKey);
    EXPECT_EQ(transaction.GetBatchId().substr(0, 6), "batch_");
    EXPECT_EQ(transaction.GetChangesetId().substr(0, 9), "changeset");
  }
  void CheckContentLines(std::vector<std::string> const& lines, Models::TransactionAction action)
  {
    EXPECT_EQ(lines[0], "--" + changeset);
    EXPECT_EQ(lines[1], "Content-Type: application/http");
    EXPECT_EQ(lines[2], "Content-Transfer-Encoding: binary");
    switch (action)
    {
      case Models::TransactionAction::InsertEntity:
        EXPECT_EQ(lines[4], "POST " + url + "/" + tableName + " HTTP/1.1");
        break;
      case Models::TransactionAction::DeleteEntity:
        EXPECT_EQ(
            lines[4],
            "DELETE " + url + "/" + tableName + "(PartitionKey='" + partitionKey
                + "',RowKey='row1') HTTP/1.1");
        break;
      case Models::TransactionAction::MergeEntity:
        EXPECT_EQ(
            lines[4],
            "MERGE " + url + "/" + tableName + "(PartitionKey='" + partitionKey
                + "',RowKey='row1') HTTP/1.1");
        break;
      case Models::TransactionAction::UpdateEntity:
        EXPECT_EQ(
            lines[4],
            "PUT " + url + "/" + tableName + "(PartitionKey='" + partitionKey
                + "',RowKey='row1') HTTP/1.1");
        break;
      case Models::TransactionAction::InsertMergeEntity:
        EXPECT_EQ(
            lines[4],
            "MERGE " + url + "/" + tableName + "(PartitionKey='" + partitionKey
                + "',RowKey='row1') HTTP/1.1");
        break;
      case Models::TransactionAction::InsertReplaceEntity:
        EXPECT_EQ(
            lines[4],
            "PUT " + url + "/" + tableName + "(PartitionKey='" + partitionKey
                + "',RowKey='row1') HTTP/1.1");
        break;
    }
    EXPECT_EQ(lines[lines.size() - 1], "--" + changeset + "--");
  }
  void CheckTransactionBody(std::string const& body, Models::TransactionAction action)
  {
    (void)action;
    std::stringstream ss(body);
    std::string line;
    std::getline(ss, line, '\n');

    // line1
    EXPECT_EQ(line.substr(0, 8), "--batch_");
    EXPECT_EQ(line.size(), 44);
    batch = line.substr(2, line.length() - 1);
    // line2
    std::getline(ss, line, '\n');
    EXPECT_EQ(line.substr(0, 50), "Content-Type: multipart/mixed; boundary=changeset_");
    EXPECT_EQ(line.size(), 86);
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

  TEST_F(TablesClientTest, TransactionBodyInsertOp)
  {
    Tables::Transaction transaction(url, tableName, partitionKey);
    Azure::Storage::Tables::Models::TableEntity entity;
    entity.RowKey = "row1";
    transaction.CreateEntity(entity);
    EXPECT_EQ(transaction.GetSteps().size(), 1);
    EXPECT_EQ(transaction.GetSteps()[0].Action, Models::TransactionAction::InsertEntity);
    EXPECT_EQ(transaction.GetSteps()[0].Entity.RowKey, "row1");
    EXPECT_EQ(transaction.GetSteps()[0].Entity.PartitionKey, partitionKey);
    auto serialized = transaction.PreparePayload();
    CheckTransactionBody(serialized, Models::TransactionAction::InsertEntity);
  }

  TEST_F(TablesClientTest, TransactionBodyDeleteOp)
  {
    Tables::Transaction transaction(url, tableName, partitionKey);

    Azure::Storage::Tables::Models::TableEntity entity;
    entity.RowKey = "row1";
    transaction.DeleteEntity(entity);
    EXPECT_EQ(transaction.GetSteps().size(), 1);
    EXPECT_EQ(transaction.GetSteps()[0].Action, Models::TransactionAction::DeleteEntity);
    EXPECT_EQ(transaction.GetSteps()[0].Entity.RowKey, "row1");
    EXPECT_EQ(transaction.GetSteps()[0].Entity.PartitionKey, partitionKey);
    auto serialized = transaction.PreparePayload();
    CheckTransactionBody(serialized, Models::TransactionAction::DeleteEntity);
  }

  TEST_F(TablesClientTest, TransactionBodyMergeOp)
  {
    Tables::Transaction transaction(url, tableName, partitionKey);

    Azure::Storage::Tables::Models::TableEntity entity;
    entity.RowKey = "row1";
    transaction.MergeEntity(entity);
    EXPECT_EQ(transaction.GetSteps().size(), 1);
    EXPECT_EQ(transaction.GetSteps()[0].Action, Models::TransactionAction::MergeEntity);
    EXPECT_EQ(transaction.GetSteps()[0].Entity.RowKey, "row1");
    EXPECT_EQ(transaction.GetSteps()[0].Entity.PartitionKey, partitionKey);
    auto serialized = transaction.PreparePayload();
    CheckTransactionBody(serialized, Models::TransactionAction::MergeEntity);
  }

  TEST_F(TablesClientTest, TransactionBodyUpdateOp)
  {
    Tables::Transaction transaction(url, tableName, partitionKey);
    Azure::Storage::Tables::Models::TableEntity entity;
    entity.RowKey = "row1";
    transaction.UpdateEntity(entity);
    EXPECT_EQ(transaction.GetSteps().size(), 1);
    EXPECT_EQ(transaction.GetSteps()[0].Action, Models::TransactionAction::UpdateEntity);
    EXPECT_EQ(transaction.GetSteps()[0].Entity.RowKey, "row1");
    EXPECT_EQ(transaction.GetSteps()[0].Entity.PartitionKey, partitionKey);
    auto serialized = transaction.PreparePayload();
    CheckTransactionBody(serialized, Models::TransactionAction::UpdateEntity);
  }

  TEST_F(TablesClientTest, TransactionBodyInsertMergeOp)
  {
    Tables::Transaction transaction(url, tableName, partitionKey);
    Azure::Storage::Tables::Models::TableEntity entity;
    entity.RowKey = "row1";
    transaction.InsertMergeEntity(entity);
    EXPECT_EQ(transaction.GetSteps().size(), 1);
    EXPECT_EQ(transaction.GetSteps()[0].Action, Models::TransactionAction::MergeEntity);
    EXPECT_EQ(transaction.GetSteps()[0].Entity.RowKey, "row1");
    EXPECT_EQ(transaction.GetSteps()[0].Entity.PartitionKey, partitionKey);
    auto serialized = transaction.PreparePayload();
    CheckTransactionBody(serialized, Models::TransactionAction::InsertMergeEntity);
  }

  TEST_F(TablesClientTest, TransactionBodyInsertReplaceOp)
  {
    Tables::Transaction transaction(url, tableName, partitionKey);
    Azure::Storage::Tables::Models::TableEntity entity;
    entity.RowKey = "row1";
    transaction.InsertReplaceEntity(entity);
    EXPECT_EQ(transaction.GetSteps().size(), 1);
    EXPECT_EQ(transaction.GetSteps()[0].Action, Models::TransactionAction::UpdateEntity);
    EXPECT_EQ(transaction.GetSteps()[0].Entity.RowKey, "row1");
    EXPECT_EQ(transaction.GetSteps()[0].Entity.PartitionKey, partitionKey);
    auto serialized = transaction.PreparePayload();
    CheckTransactionBody(serialized, Models::TransactionAction::InsertReplaceEntity);
  }
}}} // namespace Azure::Storage::Test