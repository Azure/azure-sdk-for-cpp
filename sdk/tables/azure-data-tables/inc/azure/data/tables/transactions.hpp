// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/response.hpp>
#include <azure/core/url.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/internal/shared_key_policy_lite.hpp>
#include <azure/storage/common/internal/storage_bearer_token_auth.hpp>
#include <azure/storage/common/internal/storage_per_retry_policy.hpp>
#include <azure/storage/common/internal/storage_service_version_policy.hpp>
#include <azure/storage/common/internal/storage_switch_to_secondary_policy.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_credential.hpp>
#include <azure/data/tables/dll_import_export.hpp>
#include <azure/data/tables/models.hpp>
#include <azure/data/tables/rtti.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Azure { namespace Data { namespace Tables {

  class Transaction final {
  public:
    Transaction(
        std::string const& url,
        std::string const& tableName,
        std::string const& partitionKey)
        : m_partitionKey(std::move(partitionKey)), m_tableName(tableName)
    {
      Azure::Core::Url _url(url);
      _url.SetQueryParameters({});
      m_url = _url.GetAbsoluteUrl();
      m_batchId = "batch_" + Azure::Core::Uuid::CreateUuid().ToString();
      m_changesetId = "changeset_" + Azure::Core::Uuid::CreateUuid().ToString();
    }

    std::string const& GetPartitionKey() const { return m_partitionKey; }
    std::string const& GetBatchId() const { return m_batchId; }
    std::string const& GetChangesetId() const { return m_changesetId; }

    std::vector<Models::TransactionStep> const& GetSteps() const { return m_steps; }

    void CreateEntity(Models::TableEntity const& entity);
    void DeleteEntity(Models::TableEntity const& entity);
    void MergeEntity(Models::TableEntity const& entity);
    void UpdateEntity(Models::TableEntity const& entity);
    void UpsertEntity(Models::TableEntity const& entity);
    void InsertReplaceEntity(Models::TableEntity const& entity);
    void InsertMergeEntity(Models::TableEntity const& entity);
    std::string PreparePayload();

  private:
    std::string PrepCreateEntity(Models::TableEntity entity);
    std::string PrepDeleteEntity(Models::TableEntity entity);
    std::string PrepMergeEntity(Models::TableEntity entity);
    std::string PrepUpdateEntity(Models::TableEntity entity);
    std::string m_partitionKey;
    std::string m_url;
    std::string m_tableName;
    std::vector<Models::TransactionStep> m_steps;
    std::string m_batchId;
    std::string m_changesetId;
  };
}}} // namespace Azure::Data::Tables
