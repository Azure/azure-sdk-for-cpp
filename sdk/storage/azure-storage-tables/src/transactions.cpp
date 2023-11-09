#include <azure/storage/tables/serializers.hpp>
#include <azure/storage/tables/transactions.hpp>
#include <azure/core/base64.hpp>
using namespace Azure::Storage::Tables;

void Transaction::CreateEntity(Models::TableEntity const& entity)
{
  m_steps.emplace_back(
      Models::TransactionStep{Models::TransactionAction::InsertEntity, std::move(entity)});
}
void Transaction::DeleteEntity(Models::TableEntity const& entity)
{
  m_steps.emplace_back(
      Models::TransactionStep{Models::TransactionAction::DeleteEntity, std::move(entity)});
}

void Transaction::MergeEntity(Models::TableEntity const& entity)
{
  m_steps.emplace_back(
      Models::TransactionStep{Models::TransactionAction::UpdateEntity, std::move(entity)});
}
void Transaction::UpdateEntity(Models::TableEntity const& entity)
{
  m_steps.emplace_back(
      Models::TransactionStep{Models::TransactionAction::UpdateEntity, std::move(entity)});
}
void Transaction::UpsertEntity(Models::TableEntity const& entity)
{
  m_steps.emplace_back(
      Models::TransactionStep{Models::TransactionAction::UpsertEntity, std::move(entity)});
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
    }
  }

  accumulator += "\n\n--" + m_changesetId + "--\n";
  accumulator += "--"+ m_batchId + "\n";
  return accumulator;
};

std::string Transaction::PrepCreateEntity(Models::TableEntity entity)
{
  std::string returnValue = "--"+ m_changesetId + "\n";
  returnValue += "Content-Type: application/http\n";
  returnValue += "Content-Transfer-Encoding: binary\n\n";

  returnValue += "POST " + m_url + "/" + m_tableName + " HTTP/1.1\n";
  returnValue += "Content-Type: application/json\n";
  returnValue += "Accept: application/json;odata=minimalmetadata\n";
  returnValue += "Prefer: return-no-content\n";
  returnValue += "DataServiceVersion: 3.0;\n\n";
  returnValue += Serializers::CreateEntity(entity);
  return returnValue;
};