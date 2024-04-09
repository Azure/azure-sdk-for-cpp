// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

#include "azure/data/tables/credentials/azure_sas_credential.hpp"
#include "azure/data/tables/credentials/named_key_credential.hpp"
#include "azure/data/tables/models.hpp"

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

#ifdef _azure_TABLES_TESTING_BUILD
namespace Azure { namespace Data { namespace Tables { namespace StressTest {
  class TransactionStressTest;
}}}} // namespace Azure::Data::Tables::StressTest
namespace Azure { namespace Data { namespace Test {
  class TransactionsBodyTest_TransactionCreate_Test;
  class TransactionsBodyTest_TransactionBodyInsertMergeOp_Test;
  class TransactionsBodyTest_TransactionBodyInsertReplaceOp_Test;
  class TransactionsBodyTest_TransactionBodyDeleteOp_Test;
  class TransactionsBodyTest_TransactionBodyUpdateMergeOp_Test;
  class TransactionsBodyTest_TransactionBodyUpdateReplaceOp_Test;
  class TransactionsBodyTest_TransactionBodyAddOp_Test;
}}} // namespace Azure::Data::Test
#endif

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
    // various strings used in the library
    constexpr static const char* OriginHeader = "Origin";
    constexpr static const char* AccessControlRequestMethodHeader = "Access-Control-Request-Method";
    constexpr static const char* ResrouceTypeService = "service";
    constexpr static const char* ComponentProperties = "properties";
    constexpr static const char* ContentTypeXml = "application/xml";
    constexpr static const char* ContentTypeJson = "application/json";
    constexpr static const char* ResourceTypeHeader = "restype";
    constexpr static const char* CompHeader = "comp";
    constexpr static const char* ContentTypeHeader = "Content-Type";
    constexpr static const char* ContentLengthHeader = "Content-Length";
    constexpr static const char* AcceptHeader = "Accept";
    constexpr static const char* PreferHeader = "Prefer";
    constexpr static const char* PreferNoContent = "return-no-content";
    constexpr static const char* AcceptFullMeta = "application/json;odata=fullmetadata";
    constexpr static const char* IfMatch = "If-Match";
    constexpr static const char* PartitionKeyFragment = "(PartitionKey='";
    constexpr static const char* RowKeyFragment = "',RowKey='";
    constexpr static const char* ClosingFragment = "')";
    constexpr static const char* Value = "value";
    constexpr static const char* TableName = "TableName";
    constexpr static const char* ODataEditLink = "odata.editLink";
    constexpr static const char* ODataId = "odata.id";
    constexpr static const char* ODataType = "odata.type";
    constexpr static const char* ODataMeta = "odata.metadata";
    constexpr static const char* ODataError = "odata.error";
  } // namespace _detail

  /**
   * @brief API version for Tables service.
   */
  class ServiceVersion final {
  public:
    /**
     * @brief Construct a new Service Version object
     *
     * @param version The string version for Tables Service.
     */
    explicit ServiceVersion(std::string version) : m_version{std::move(version)} {}

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
     * @param credential The named key credential used to sign requests.
     * @param url A url referencing the table that includes the name of the account and the name of
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit TableClient(
        const std::string& tableName,
        std::shared_ptr<Azure::Data::Tables::Credentials::NamedKeyCredential> credential,
        std::string url,
        const TableClientOptions& options = {});

    /**
     * @brief Initializes a new instance of tableClient.
     *
     * @param serviceUrl The service Url
     * @param credential The SAS credential used to sign requests.
     * @param tableName The name of the table.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit TableClient(
        const std::string& serviceUrl,
        std::shared_ptr<Azure::Data::Tables::Credentials::AzureSasCredential> credential,
        const std::string& tableName,
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
     * @brief Get table access policy.
     *
     * @param context for canceling long running operations.
     * @return Get access policy result.
     */
    Response<Models::TableAccessPolicy> GetAccessPolicy(Core::Context const& context = {});

    /**
     * @brief Set table access policy.
     *
     * @param tableAccessPolicy The TableAccessPolicy to set.
     * @param context for canceling long running operations.
     * @return Set access policy result.
     */
    Response<Models::SetTableAccessPolicyResult> SetAccessPolicy(
        Models::TableAccessPolicy const& tableAccessPolicy,
        Core::Context const& context = {});

    /**
     * @brief Add table entity.
     *
     * @param tableEntity The TableEntity to set.
     * @param options Optional parameters to execute this function.
     * @param context for canceling long running operations.
     * @return Add entity result.
     */
    Response<Models::AddEntityResult> AddEntity(
        Models::TableEntity const& tableEntity,
        Models::AddEntityOptions const& options = {},
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
     * @param context for canceling long running operations.
     * @return Delete entity result.
     */
    Response<Models::DeleteEntityResult> DeleteEntity(
        Models::TableEntity const& tableEntity,
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
     * @brief Get one table entity.
     *
     * @param partitionKey The partition key of the entity.
     * @param rowKey The row key of the entity.
     * @param context for canceling long running operations.
     * @return Table entity.
     */
    Response<Models::TableEntity> GetEntity(
        const std::string& partitionKey,
        const std::string& rowKey,
        Core::Context const& context = {});

    /**
     * @brief Submits a transaction.
     *
     * @param steps the transaction steps to execute.
     * @param context for canceling long running operations.
     * @return Submit transaction result.
     */
    Response<Models::SubmitTransactionResult> SubmitTransaction(
        std::vector<Models::TransactionStep> const& steps,
        Core::Context const& context = {});

  private:
#ifdef _azure_TABLES_TESTING_BUILD
    friend class Azure::Data::Tables::StressTest::TransactionStressTest;
    friend class Azure::Data::Test::TransactionsBodyTest_TransactionCreate_Test;
    friend class Azure::Data::Test::TransactionsBodyTest_TransactionBodyInsertMergeOp_Test;
    friend class Azure::Data::Test::TransactionsBodyTest_TransactionBodyInsertReplaceOp_Test;
    friend class Azure::Data::Test::TransactionsBodyTest_TransactionBodyDeleteOp_Test;
    friend class Azure::Data::Test::TransactionsBodyTest_TransactionBodyUpdateMergeOp_Test;
    friend class Azure::Data::Test::TransactionsBodyTest_TransactionBodyUpdateReplaceOp_Test;
    friend class Azure::Data::Test::TransactionsBodyTest_TransactionBodyAddOp_Test;
#endif

    std::string PreparePayload(
        std::string const& batchId,
        std::string const& changesetId,
        std::vector<Models::TransactionStep> const& steps);
    std::string PrepAddEntity(std::string const& changesetId, Models::TableEntity entity);
    std::string PrepDeleteEntity(std::string const& changesetId, Models::TableEntity entity);
    std::string PrepMergeEntity(std::string const& changesetId, Models::TableEntity entity);
    std::string PrepUpdateEntity(std::string const& changesetId, Models::TableEntity entity);
    std::shared_ptr<Core::Http::_internal::HttpPipeline> m_pipeline;
    Core::Url m_url;
    std::string m_tableName;
  };

  /**
   * @brief Table Service Client
   */
  class TableServiceClient final {
  public:
    /**
     * @brief Initializes a new instance of tableServiceClient.
     *
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit TableServiceClient(const TableClientOptions& options = {});

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
        std::shared_ptr<Core::Credentials::TokenCredential> credential,
        const TableClientOptions& options = {});

    /**
     * @brief Initializes a new instance of tableClient.
     *
     * @param serviceUrl A url referencing the table that includes the name of the account and the
     * name of the table.
     * @param credential The named key credential used to sign requests.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit TableServiceClient(
        const std::string& serviceUrl,
        std::shared_ptr<Azure::Data::Tables::Credentials::NamedKeyCredential> credential,
        const TableClientOptions& options = {});

    /**
     * @brief Initializes a new instance of tableClient.
     *
     * @param serviceUrl A url referencing the table that includes the name of the account and the
     * name of the table.
     * @param credential The SAS credential used to sign requests.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit TableServiceClient(
        const std::string& serviceUrl,
        std::shared_ptr<Azure::Data::Tables::Credentials::AzureSasCredential> credential,
        const TableClientOptions& options = {});

    /**
     * @brief Initializes a new instance of tableClient.
     *
     * @param connectionString the connection string used to initialize.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     * @return TableServiceClient.
     */
    static TableServiceClient CreateFromConnectionString(
        const std::string& connectionString,
        const TableClientOptions& options = {});

    /**
     * @brief Create the table indicated in the tableName field of the client.
     *
     * @param context for canceling long running operations.
     * @param tableName The name of the table to be created.
     * @return Create table result.
     */
    Response<Models::Table> CreateTable(
        std::string const& tableName,
        Core::Context const& context = {});

    /**
     * @brief Delete the table indicated in the tableName field of the client.
     *
     * @param context for canceling long running operations.
     * @param tableName The name of the table to be deleted.
     * @return Delete table result.
     */
    Response<Models::DeleteTableResult> DeleteTable(
        std::string const& tableName,
        Core::Context const& context = {});

    /**
     * @brief Query tables.
     *
     * @param options Optional parameters to execute this function.
     * @param context for canceling long running operations.
     * @return List tables paged response.
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
        Core::Context const& context = {});

    /**
     * @brief Get service properties
     *
     * @param context for canceling long running operations.
     * @return Get service properties result.
     */
    Response<Models::TableServiceProperties> GetServiceProperties(
        Core::Context const& context = {});

    /**
     * @brief Get service statistics
     *
     * @param context for canceling long running operations.
     * @return Get service statistics result.
     */
    Response<Models::ServiceStatistics> GetStatistics(Core::Context const& context = {});

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
    std::shared_ptr<Core::Credentials::TokenCredential> m_tokenCredential;
    std::shared_ptr<Azure::Data::Tables::Credentials::NamedKeyCredential> m_namedKeyCredential;
    Core::Url m_url;
  };
}}} // namespace Azure::Data::Tables
