// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

#include <azure/core/url.hpp>
#include <azure/core/uuid.hpp>
#include <azure/data/tables/models.hpp>

namespace Azure { namespace Data { namespace Tables {
  /**
   * @brief Transaction Definition
   *
   */
  class Transaction final {
  public:
    /**
     * @brief Construct a new Transaction object
     *
     * @param url The url of the service
     * @param tableName the name of the table
     * @param partitionKey the partition key
     */
    Transaction(
        std::string const& url,
        std::string const& tableName,
        std::string const& partitionKey)
        : m_partitionKey{std::move(partitionKey)}, m_tableName{tableName}
    {
      Azure::Core::Url _url(url);
      _url.SetQueryParameters({});
      m_url = _url.GetAbsoluteUrl();
      m_batchId = "batch_" + Azure::Core::Uuid::CreateUuid().ToString();
      m_changesetId = "changeset_" + Azure::Core::Uuid::CreateUuid().ToString();
    }

    /**
     * @brief Get the Partition Key
     *
     * @return Partition key
     */
    std::string const& GetPartitionKey() const { return m_partitionKey; }

    /**
     * @brief Get the Batch Id
     *
     * @return Batch id
     */
    std::string const& GetBatchId() const { return m_batchId; }

    /**
     * @brief Get the Changeset Id
     *
     * @return Changeset id
     */
    std::string const& GetChangesetId() const { return m_changesetId; }

    /**
     * @brief Get the Steps
     *
     * @return vector of steps
     */
    std::vector<Models::TransactionStep> const& GetSteps() const { return m_steps; }

    /**
     * @brief Add a Create Entity Step
     *
     * @param entity The entity to create
     */
    void CreateEntity(Models::TableEntity const& entity);

    /**
     * @brief Add a Delete Entity Step
     *
     * @param entity The entity to delete
     */
    void DeleteEntity(Models::TableEntity const& entity);

    /**
     * @brief Add a Merge Entity Step
     *
     * @param entity The entity to merge
     */
    void MergeEntity(Models::TableEntity const& entity);

    /**
     * @brief Add a Update Entity Step
     *
     * @param entity The entity to update
     */
    void UpdateEntity(Models::TableEntity const& entity);

    /**
     * @brief Add a Upsert Entity Step
     *
     * @param entity The entity to upsert
     */
    void UpsertEntity(Models::TableEntity const& entity);

    /**
     * @brief Add a Insert Or Replace Entity Step
     *
     * @param entity The entity to insert or replace
     */
    void InsertReplaceEntity(Models::TableEntity const& entity);

    /**
     * @brief Add a Insert Or Merge Entity Step
     *
     * @param entity The entity to insert or merge
     */
    void InsertMergeEntity(Models::TableEntity const& entity);

    /**
     * @brief Prepare the payload
     *
     * @return The payload
     */
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
