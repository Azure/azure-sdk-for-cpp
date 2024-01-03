// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/response.hpp>
#include <azure/core/xml.hpp>
#include <azure/data/tables/models.hpp>
#include <azure/data/tables/transactions.hpp>
#include <azure/core/http/policies/service_version_policy.hpp>
#include <azure/core/http/policies/shared_key_lite_policy.hpp>
#include <azure/storage/common/internal/storage_bearer_token_auth.hpp>
#include <azure/storage/common/internal/storage_switch_to_secondary_policy.hpp>
#include <azure/storage/common/storage_common.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Azure { namespace Data { namespace Tables {

  namespace _detail {
    /**
     * The version used for the operations to Tables services.
     */
    constexpr static const char* ApiVersion = "2019-02-02";
    /**
     * The package name of the SDK.
     */
    constexpr static const char* TablesServicePackageName = "data-tables";
  } // namespace _detail

  /**
   * @brief API version for Storage Tables service.
   */
  class ServiceVersion final {
  public:
    /**
     * @brief Construct a new Service Version object
     *
     * @param version The string version for Storage Tables Service.
     */
    explicit ServiceVersion(std::string version) : m_version(std::move(version)) {}

    /**
     * @brief Enable comparing between two versions.
     *
     * @param other Another service version to be compared.
     */
    bool operator==(const ServiceVersion& other) const { return m_version == other.m_version; }

    /**
     * @brief Enable comparing between two versions.
     *
     * @param other Another service version to be compared.
     */
    bool operator!=(const ServiceVersion& other) const { return !(*this == other); }

    /**
     * @brief Returns string representation.
     *
     */
    std::string const& ToString() const { return m_version; }

    /**
     * @brief API version 2019-12-12.
     *
     */
    AZ_DATA_TABLES_DLLEXPORT const static ServiceVersion V2019_02_02;

  private:
    std::string m_version;
  };

  /**
   * @brief Audiences available for Blobs
   *
   */
  class TablesAudience final
      : public Azure::Core::_internal::ExtendableEnumeration<TablesAudience> {
  public:
    /**
     * @brief Construct a new TablesAudience object
     *
     * @param tablesAudience The Azure Active Directory audience to use when forming authorization
     * scopes. For the Language service, this value corresponds to a URL that identifies the Azure
     * cloud where the resource is located. For more information: See
     * https://learn.microsoft.com/en-us/azure/storage/blobs/authorize-access-azure-active-directory
     */
    explicit TablesAudience(std::string tablesAudience)
        : ExtendableEnumeration(std::move(tablesAudience))
    {
    }
  };

  /**
   * @brief Optional parameters for constructing a new TableClient.
   */
  struct TableClientOptions final : Azure::Core::_internal::ClientOptions
  {
    /**
     * SecondaryHostForRetryReads specifies whether the retry policy should retry a read
     * operation against another host. If SecondaryHostForRetryReads is "" (the default) then
     * operations are not retried against another host. NOTE: Before setting this field, make sure
     * you understand the issues around reading stale & potentially-inconsistent data at this
     * webpage: https://docs.microsoft.com/azure/storage/common/geo-redundant-design.
     */
    std::string SecondaryHostForRetryReads;

    /**
     * API version used by this client.
     */
    ServiceVersion ApiVersion{_detail::ApiVersion};

    /**
     * Enables tenant discovery through the authorization challenge when the client is configured to
     * use a TokenCredential. When enabled, the client will attempt an initial un-authorized request
     * to prompt a challenge in order to discover the correct tenant for the resource.
     */
    bool EnableTenantDiscovery = false;

    /**
     * The Audience to use for authentication with Azure Active Directory (AAD).
     *
     */
    Azure::Nullable<TablesAudience> Audience;
  };

  /**
   * @brief Table Client
   */
  class TableClient final {
  public:
    /**
     * @brief Initializes a new instance of tableClient.
     *
     * @param serviceUrl The URL of the service account that is the target of the desired operation.
     * The URL may contain SAS query parameters.
     * @param tableName The name of the table.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit TableClient(
        std::string const& serviceUrl,
        std::string const& tableName,
        const TableClientOptions& options = {});
    /**
     * @brief Initializes a new instance of tableClient.
     *
     * @param serviceUrl The URL of the service account that is the target of the desired operation.
     * The URL may contain SAS query parameters.
     * @param tableName The name of the table.
     * @param credential The shared key credential used to sign requests.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit TableClient(
        const std::string& serviceUrl,
        const std::string& tableName,
        std::shared_ptr<Core::Credentials::TokenCredential> credential,
        const TableClientOptions& options = {});

    /**
     * @brief Initializes a new instance of tableClient.
     *
     * @param tableName The name of the table.
     * @param credential The shared key credential used to sign requests.
     * @param url A url referencing the table that includes the name of the account and the name of
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit TableClient(
        const std::string& tableName,
        std::shared_ptr<Azure::Core::Credentials::SharedKeyCredential> credential,
        std::string url,
        const TableClientOptions& options = {});

    /**
     * @brief Initializes a new instance of tableClient.
     *
     * @param connectionString the connection string used to initialize.
     * @param tableName The name of the table.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     * @return TableClient.
     */
    static TableClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& tableName,
        const TableClientOptions& options = {});

    /**
     * @brief Create the table indicated in the tableName field of the client.
     *
     * @param context for canceling long running operations.
     * @return Create table result.
     */
    Response<Models::Table> Create(Core::Context const& context = {});

    /**
     * @brief Delete the table indicated in the tableName field of the client.
     *
     * @param context for canceling long running operations.
     * @return Delete table result.
     */
    Response<Models::DeleteResult> Delete(Core::Context const& context = {});

    /**
     * @brief Get table access policy.
     *
     * @param options Optional parameters to execute this function.
     * @param context for canceling long running operations.
     * @return Get access policy result.
     */
    Response<Models::TableAccessPolicy> GetAccessPolicy(
        Models::GetTableAccessPolicyOptions const& options = {},
        Core::Context const& context = {});

    /**
     * @brief Set table access policy.
     *
     * @param tableAccessPolicy The TableAccessPolicy to set.
     * @param options Optional parameters to execute this function.
     * @param context for canceling long running operations.
     * @return Set access policy result.
     */
    Response<Models::SetTableAccessPolicyResult> SetAccessPolicy(
        Models::TableAccessPolicy const& tableAccessPolicy,
        Models::SetTableAccessPolicyOptions const& options = {},
        Core::Context const& context = {});

    /**
     * @brief Create table entity.
     *
     * @param tableEntity The TableEntity to set.
     * @param options Optional parameters to execute this function.
     * @param context for canceling long running operations.
     * @return Create entity result.
     */
    Response<Models::CreateEntityResult> CreateEntity(
        Models::TableEntity const& tableEntity,
        Models::CreateEntityOptions const& options = {},
        Core::Context const& context = {});

    /**
     * @brief Update table entity.
     *
     * @param tableEntity The TableEntity to set.
     * @param options Optional parameters to execute this function.
     * @param context for canceling long running operations.
     * @return Update entity result.
     */
    Response<Models::UpdateEntityResult> UpdateEntity(
        Models::TableEntity const& tableEntity,
        Models::UpdateEntityOptions const& options = {},
        Core::Context const& context = {});

    /**
     * @brief Merge table entity.
     *
     * @param tableEntity The TableEntity to merge.
     * @param options Optional parameters to execute this function.
     * @param context for canceling long running operations.
     * @return Merge entity result.
     */
    Response<Models::MergeEntityResult> MergeEntity(
        Models::TableEntity const& tableEntity,
        Models::MergeEntityOptions const& options = {},
        Core::Context const& context = {});

    /**
     * @brief Delete table entity.
     *
     * @param tableEntity The TableEntity to delete.
     * @param options Optional parameters to execute this function.
     * @param context for canceling long running operations.
     * @return Delete entity result.
     */
    Response<Models::DeleteEntityResult> DeleteEntity(
        Models::TableEntity const& tableEntity,
        Models::DeleteEntityOptions const& options = {},
        Core::Context const& context = {});

    /**
     * @brief Upsert table entity.
     *
     * @param tableEntity The TableEntity to upsert.
     * @param options Optional parameters to execute this function.
     * @param context for canceling long running operations.
     * @return Upsert entity result.
     */
    Response<Models::UpsertEntityResult> UpsertEntity(
        Models::TableEntity const& tableEntity,
        Models::UpsertEntityOptions const& options = {},
        Core::Context const& context = {});

    /**
     * @brief Query table entities.
     *
     * @param options Optional parameters to execute this function.
     * @param context for canceling long running operations.
     * @return Entity list paged response.
     */
    Models::QueryEntitiesPagedResponse QueryEntities(
        Models::QueryEntitiesOptions const& options = {},
        Core::Context const& context = {});
    /**
     * @brief Creates a new transaction.
     *
     * @param partitionKey The partition key of the transaction.
     * @return New transaction.
     */
    Transaction CreateTransaction(std::string const& partitionKey);

    /**
     * @brief Submits a transaction.
     *
     * @param transaction The transaction to submit.
     * @param context for canceling long running operations.
     * @return Submit transaction result.
     */
    Response<Models::SubmitTransactionResult> SubmitTransaction(
        Transaction& transaction,
        Core::Context const& context = {});

  private:
    std::shared_ptr<Core::Http::_internal::HttpPipeline> m_pipeline;
    Core::Url m_url;
    std::string m_tableName;
    Models::TableEntity DeserializeEntity(Azure::Core::Json::_internal::json json);
  };

  /**
   * @brief Table Services Client
   */
  class TableServicesClient final {
  public:
    /**
     * @brief Initializes a new instance of tableServicesClient.
     *
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit TableServicesClient(const TableClientOptions& options = {});

    /**
     * @brief Initializes a new instance of tableClient.
     *
     * @param serviceUrl A url referencing the table that includes the name of the account and the
     * name of the table.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit TableServicesClient(
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
    explicit TableServicesClient(
        const std::string& serviceUrl,
        std::shared_ptr<Core::Credentials::TokenCredential> credential,
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
    explicit TableServicesClient(
        const std::string& serviceUrl,
        std::shared_ptr<Azure::Core::Credentials::SharedKeyCredential> credential,
        const TableClientOptions& options = {});
    /**
     * @brief Initializes a new instance of tableClient.
     *
     * @param connectionString the connection string used to initialize.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     * @return TableServicesClient.
     */
    static TableServicesClient CreateFromConnectionString(
        const std::string& connectionString,
        const TableClientOptions& options = {});

    /**
     * @brief List tables.
     *
     * @param options Optional parameters to execute this function.
     * @param context for canceling long running operations.
     * @return List tables paged response.
     */
    Models::ListTablesPagedResponse ListTables(
        const Models::ListTablesOptions& options = {},
        const Azure::Core::Context& context = {}) const;

    /**
     * @brief Set service properties
     *
     * @param options Optional parameters to execute this function.
     * @param context for canceling long running operations.
     */
    Response<Models::SetServicePropertiesResult> SetServiceProperties(
        Models::SetServicePropertiesOptions const& options = {},
        Core::Context const& context = {});

    /**
     * @brief Get service properties
     *
     * @param options Optional parameters to execute this function.
     * @param context for canceling long running operations.
     * @return Get service properties result.
     */
    Response<Models::TableServiceProperties> GetServiceProperties(
        Models::GetServicePropertiesOptions const& options = {},
        Core::Context const& context = {});

    /**
     * @brief Get service statistics
     *
     * @param options Optional parameters to execute this function.
     * @param context for canceling long running operations.
     * @return Get service statistics result.
     */
    Response<Models::ServiceStatistics> GetStatistics(
        Models::GetServiceStatisticsOptions const& options = {},
        Core::Context const& context = {});

    /**
     * @brief Pre flight check
     *
     * @param options Optional parameters to execute this function.
     * @param context for canceling long running operations.
     * @return Get service statistics result.
     */
    Response<Models::PreflightCheckResult> PreflightCheck(
        Models::PreflightCheckOptions const& options,
        Core::Context const& context = {});

  private:
    std::shared_ptr<Core::Http::_internal::HttpPipeline> m_pipeline;
    Core::Url m_url;
  };
}}} // namespace Azure::Data::Tables
