// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/data/tables/transactions.hpp"

#include "azure/data/tables/internal/serializers.hpp"

using namespace Azure::Data::Tables;
using namespace Azure::Data::Tables::_detail;
void Transaction::CreateEntity(Models::TableEntity const& entity)
{
  Models::TableEntity _entity = entity;
  _entity.PartitionKey = m_partitionKey;
  m_steps.emplace_back(
      Models::TransactionStep{Models::TransactionAction::InsertEntity, std::move(_entity)});
}
void Transaction::DeleteEntity(Models::TableEntity const& entity)
{
  Models::TableEntity _entity = entity;
  _entity.PartitionKey = m_partitionKey;
  m_steps.emplace_back(
      Models::TransactionStep{Models::TransactionAction::DeleteEntity, std::move(_entity)});
}

void Transaction::MergeEntity(Models::TableEntity const& entity)
{
  Models::TableEntity _entity = entity;
  _entity.PartitionKey = m_partitionKey;
  m_steps.emplace_back(
      Models::TransactionStep{Models::TransactionAction::MergeEntity, std::move(_entity)});
}
void Transaction::InsertMergeEntity(Models::TableEntity const& entity)
{
  Models::TableEntity _entity = entity;
  _entity.PartitionKey = m_partitionKey;
  m_steps.emplace_back(
      Models::TransactionStep{Models::TransactionAction::MergeEntity, std::move(_entity)});
}
void Transaction::UpdateEntity(Models::TableEntity const& entity)
{
  Models::TableEntity _entity = entity;
  _entity.PartitionKey = m_partitionKey;
  m_steps.emplace_back(
      Models::TransactionStep{Models::TransactionAction::UpdateEntity, std::move(_entity)});
}
void Transaction::InsertReplaceEntity(Models::TableEntity const& entity)
{
  Models::TableEntity _entity = entity;
  _entity.PartitionKey = m_partitionKey;
  m_steps.emplace_back(
      Models::TransactionStep{Models::TransactionAction::UpdateEntity, std::move(_entity)});
}
std::string Transaction::PreparePayload()
{
  std::string accumulator
      = "--" + m_batchId + "\nContent-Type: multipart/mixed; boundary=" + m_changesetId + "\n\n";

  for (auto step : m_steps)
  {
    switch (step.Action)
    {
      case Models::TransactionAction::InsertEntity:
        accumulator += PrepCreateEntity(step.Entity);
        break;
      case Models::TransactionAction::DeleteEntity:
        accumulator += PrepDeleteEntity(step.Entity);
        break;
      case Models::TransactionAction::InsertMergeEntity:
      case Models::TransactionAction::MergeEntity:
        accumulator += PrepMergeEntity(step.Entity);
        break;
      case Models::TransactionAction::InsertReplaceEntity:
      case Models::TransactionAction::UpdateEntity:
        accumulator += PrepUpdateEntity(step.Entity);
        break;
    }
  }

  accumulator += "\n\n--" + m_changesetId + "--\n";
  accumulator += "--" + m_batchId + "\n";
  return accumulator;
}

std::string Transaction::PrepCreateEntity(Models::TableEntity entity)
{
  std::string returnValue = "--" + m_changesetId + "\n";
  returnValue += "Content-Type: application/http\n";
  returnValue += "Content-Transfer-Encoding: binary\n\n";

  returnValue += "POST " + m_url + "/" + m_tableName + " HTTP/1.1\n";
  returnValue += "Content-Type: application/json\n";
  returnValue += "Accept: application/json;odata=minimalmetadata\n";
  returnValue += "Prefer: return-no-content\n";
  returnValue += "DataServiceVersion: 3.0;\n\n";
  returnValue += Serializers::CreateEntity(entity);
  return returnValue;
}

std::string Transaction::PrepDeleteEntity(Models::TableEntity entity)
{
  std::string returnValue = "--" + m_changesetId + "\n";
  returnValue += "Content-Type: application/http\n";
  returnValue += "Content-Transfer-Encoding: binary\n\n";

  returnValue += "DELETE " + m_url + "/" + m_tableName + "(PartitionKey='" + entity.PartitionKey
      + "',RowKey='" + entity.RowKey + "')" + " HTTP/1.1\n";
  returnValue += "Accept: application/json;odata=minimalmetadata\n";
  // returnValue += "Prefer: return-no-content\n";
  returnValue += "DataServiceVersion: 3.0;\n";
  if (entity.ETag.HasValue())
  {
    returnValue += "If-Match: " + entity.ETag.Value();
  }
  else
  {
    returnValue += "If-Match: *";
  }
  returnValue += "\n";
  return returnValue;
}

std::string Transaction::PrepMergeEntity(Models::TableEntity entity)
{
  std::string returnValue = "--" + m_changesetId + "\n";
  returnValue += "Content-Type: application/http\n";
  returnValue += "Content-Transfer-Encoding: binary\n\n";

  returnValue += "MERGE " + m_url + "/" + m_tableName + "(PartitionKey='" + entity.PartitionKey
      + "',RowKey='" + entity.RowKey + "')" + " HTTP/1.1\n";
  returnValue += "Content-Type: application/json\n";
  returnValue += "Accept: application/json;odata=minimalmetadata\n";
  returnValue += "DataServiceVersion: 3.0;\n\n";
  returnValue += Serializers::MergeEntity(entity);

  return returnValue;
}

std::string Transaction::PrepUpdateEntity(Models::TableEntity entity)
{
  std::string returnValue = "--" + m_changesetId + "\n";
  returnValue += "Content-Type: application/http\n";
  returnValue += "Content-Transfer-Encoding: binary\n\n";

  returnValue += "PUT " + m_url + "/" + m_tableName + "(PartitionKey='" + entity.PartitionKey
      + "',RowKey='" + entity.RowKey + "')" + " HTTP/1.1\n";
  returnValue += "Content-Type: application/json\n";
  returnValue += "Accept: application/json;odata=minimalmetadata\n";
  returnValue += "Prefer: return-no-content\n";
  returnValue += "DataServiceVersion: 3.0;\n";
  if (entity.ETag.HasValue())
  {
    returnValue += "If-Match: " + entity.ETag.Value();
  }
  else
  {
    returnValue += "If-Match: *";
  }
  returnValue += "\n\n";
  returnValue += Serializers::UpdateEntity(entity);
  return returnValue;
}
