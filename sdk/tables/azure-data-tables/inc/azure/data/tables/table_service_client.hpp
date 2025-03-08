// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

#include "azure/data/tables/table_client.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/response.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Azure { namespace Data { namespace Tables {
  /**
   * @brief Table Service Client
   */
  class TableServiceClient final {
  public:
    /**
     * @brief Initializes a new instance of tableClient.
     *
     * @param serviceUrl A url referencing the table that includes the name of the account and the
     * name of the table.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit TableServiceClient(
        const std::string& serviceUrl,
        const TableClientOptions& options = {});

    /**
     * @brief Initializes a new instance of tableClient.
     *
     * @param serviceUrl A url referencing the table that includes the name of the account and the
     * name of the table.
     * @param credential The shared key credential used to sign requests.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit TableServiceClient(
        const std::string& serviceUrl,
        std::shared_ptr<Core::Credentials::TokenCredential const> credential,
        const TableClientOptions& options = {});

    /**
     * @brief Creates a new table under the given account.
     *
     * @param context for canceling long running operations.
     * @param tableName The name of the table to be created.
     * @return Create table result.
     */
    Response<Models::Table> CreateTable(
        std::string const& tableName,
        Core::Context const& context = {}) const;

    /**
     * @brief Operation permanently deletes the specified table.
     *
     * @param context for canceling long running operations.
     * @param tableName The name of the table to be deleted.
     * @return Delete table result.
     */
    Response<Models::DeleteTableResult> DeleteTable(
        std::string const& tableName,
        Core::Context const& context = {}) const;

    /**
     * @brief Queries tables under the given account.
     *
     * @param options Optional parameters to execute this function.
     * @param context for canceling long running operations.
     * @return Query tables paged response.
     */
    Models::QueryTablesPagedResponse QueryTables(
        const Models::QueryTablesOptions& options = {},
        const Azure::Core::Context& context = {}) const;

    /**
     * @brief Set service properties
     *
     * @param options Optional parameters to execute this function.
     * @param context for canceling long running operations.
     */
    Response<Models::SetServicePropertiesResult> SetServiceProperties(
        Models::SetServicePropertiesOptions const& options = {},
        Core::Context const& context = {}) const;

    /**
     * @brief Get service properties
     *
     * @param context for canceling long running operations.
     * @return Get service properties result.
     */
    Response<Models::TableServiceProperties> GetServiceProperties(
        Core::Context const& context = {}) const;

    /**
     * @brief Get service statistics
     *
     * @param context for canceling long running operations.
     * @return Get service statistics result.
     */
    Response<Models::ServiceStatistics> GetStatistics(Core::Context const& context = {}) const;

    /**
     * @brief Pre flight check
     *
     * @param options Optional parameters to execute this function.
     * @param context for canceling long running operations.
     * @return Get service statistics result.
     */
    Response<Models::PreflightCheckResult> PreflightCheck(
        Models::PreflightCheckOptions const& options,
        Core::Context const& context = {}) const;
    /**
     * @brief Get table client.
     *
     * @param tableName The name of the table.
     * @param options Optional parameters for the table client.
     * @return TableClient.
     * @remark The TableClient can be used to perform operations on the table. This method will
     * attempt to create a table client with the same credentials as the service client except the
     * SAS token credential as the SAS token permissions varies from the service level permissions.
     */
    TableClient GetTableClient(const std::string& tableName, TableClientOptions const& options = {})
        const;

  private:
    std::shared_ptr<Core::Http::_internal::HttpPipeline> m_pipeline;
    std::shared_ptr<Core::Credentials::TokenCredential const> m_tokenCredential;
    Core::Url m_url;
  };
}}} // namespace Azure::Data::Tables
