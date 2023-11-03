// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// Code generated by Microsoft (R) AutoRest Code Generator.
// Changes may cause incorrect behavior and will be lost if the code is regenerated.

#pragma once

#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/extendable_enumeration.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/nullable.hpp>
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
#include <azure/storage/tables/dll_import_export.hpp>
#include <azure/storage/tables/rest_client.hpp>
#include <azure/storage/tables/rtti.hpp>
#include <azure/storage/tables/models.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Azure { namespace Storage { namespace Tables {
  namespace _detail {
    /**
     * The version used for the operations to Azure storage services.
     */
    constexpr static const char* ApiVersion = "2019-02-02";
  } // namespace _detail

  class AllowedMethodsType final
      : public Core::_internal::ExtendableEnumeration<AllowedMethodsType> {
  public:
    AllowedMethodsType() = default;

    explicit AllowedMethodsType(std::string allowedMethods)
        : ExtendableEnumeration(std::move(allowedMethods))
    {
    }

    static AllowedMethodsType const Delete;
    static AllowedMethodsType const Get;
    static AllowedMethodsType const Head;
    static AllowedMethodsType const Merge;
    static AllowedMethodsType const Post;
    static AllowedMethodsType const Options;
    static AllowedMethodsType const Put;
    static AllowedMethodsType const Patch;
    static AllowedMethodsType const Connect;
    static AllowedMethodsType const Trace;
  };

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
    AZ_STORAGE_TABLES_DLLEXPORT const static ServiceVersion V2023_01_01;

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

    /**
     * @brief Default Audience. Use to acquire a token for authorizing requests to any Azure
     * Storage account.
     */
    AZ_STORAGE_TABLES_DLLEXPORT const static TablesAudience PublicAudience;
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
     * #Azure::Storage::Tables::Models::TablesAudience::PublicAudience will be assumed if
     * Audience is not set.
     */
    Azure::Nullable<TablesAudience> Audience;

    std::string SubscriptionId;
  };
  
  class TableClient final {
  public:
    explicit TableClient(std::string subscriptionId);

    explicit TableClient(const std::string& subscritionId, const TableClientOptions& options)
        : TableClient(subscritionId)
    {
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
      perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
          m_url.GetHost(), options.SecondaryHostForRetryReads));
      perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
      perOperationPolicies.emplace_back(
          std::make_unique<_internal::StorageServiceVersionPolicy>(options.ApiVersion.ToString()));
      m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
          options,
          _internal::TablesServicePackageName,
          _detail::ApiVersion,
          std::move(perRetryPolicies),
          std::move(perOperationPolicies));
    }

    explicit TableClient(
        const std::string& subscritionId,
        std::shared_ptr<Core::Credentials::TokenCredential> credential,
        const TableClientOptions& options)
        : TableClient(subscritionId, options)
    {
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
      perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
          m_url.GetHost(), options.SecondaryHostForRetryReads));
      perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
      {
        Azure::Core::Credentials::TokenRequestContext tokenContext;
        tokenContext.Scopes.emplace_back(
            options.Audience.HasValue()
                ? _internal::GetDefaultScopeForAudience(options.Audience.Value().ToString())
                : TablesAudience::PublicAudience.ToString());
        perRetryPolicies.emplace_back(
            std::make_unique<_internal::StorageBearerTokenAuthenticationPolicy>(
                credential, tokenContext, options.EnableTenantDiscovery));
      }
      perOperationPolicies.emplace_back(
          std::make_unique<_internal::StorageServiceVersionPolicy>(options.ApiVersion.ToString()));
      m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
          options,
          _internal::TablesServicePackageName,
          _detail::ApiVersion,
          std::move(perRetryPolicies),
          std::move(perOperationPolicies));
    }

    explicit TableClient(
        const std::string& tableName,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        std::string url,
        const TableClientOptions& options)
        : m_url(std::move(url)), m_tableName(tableName)
        
    {
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
      perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
          m_url.GetHost(), options.SecondaryHostForRetryReads));
      perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
      perOperationPolicies.emplace_back(
          std::make_unique<_internal::StorageServiceVersionPolicy>(options.ApiVersion.ToString()));
      m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
          options,
          _internal::TablesServicePackageName,
          _detail::ApiVersion,
          std::move(perRetryPolicies),
          std::move(perOperationPolicies));

      TableClientOptions newOptions = options;
      newOptions.PerRetryPolicies.emplace_back(
          std::make_unique<_internal::SharedKeyPolicyLite>(credential));

      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies2;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies2;
      perRetryPolicies2.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
          m_url.GetHost(), newOptions.SecondaryHostForRetryReads));
      perRetryPolicies2.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
      perOperationPolicies2.emplace_back(std::make_unique<_internal::StorageServiceVersionPolicy>(
          newOptions.ApiVersion.ToString()));
      m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
          newOptions,
          _internal::TablesServicePackageName,
          _detail::ApiVersion,
          std::move(perRetryPolicies2),
          std::move(perOperationPolicies2));
    }

    static TableClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& tableName,
        const TableClientOptions& options = TableClientOptions())
    {
      auto parsedConnectionString = _internal::ParseConnectionString(connectionString);
      auto tablesUrl = std::move(parsedConnectionString.TableServiceUrl);

      if (parsedConnectionString.KeyCredential)
      {
        return TableClient(
            tableName,
            std::move(parsedConnectionString.KeyCredential),
            tablesUrl.GetAbsoluteUrl().empty()
                ? Azure::Storage::_internal::TablesManagementPublicEndpoint
                : tablesUrl.GetAbsoluteUrl(),
            options);
      }
      else
      {
        return TableClient(tableName, options);
      }
    }

    Response<Models::Table> Create(Core::Context const& context = {});

    Response<Models::DeleteResult> Delete(
        Core::Context const& context = {});

    Response<Models::TableAccessPolicy> GetAccessPolicy(
        Models::GetTableAccessPolicyOptions const& options = {},
		Core::Context const& context = {});

    Response<Models::SetTableAccessPolicyResult> SetAccessPolicy(
        Models::TableAccessPolicy const& tableAccessPolicy,
        Models::SetTableAccessPolicyOptions const& options = {},
        Core::Context const& context = {});

  private:
    std::shared_ptr<Core::Http::_internal::HttpPipeline> m_pipeline;
    Core::Url m_url;
    std::string m_subscriptionId;
    std::string m_tableName;
  };  

  class TableServicesClient final {
  public:
    explicit TableServicesClient(std::string subscriptionId, const TableClientOptions& options = {})
        : m_subscriptionId(std::move(subscriptionId))
    {
      TableClientOptions newOptions = options;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
      perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
          m_url.GetHost(), newOptions.SecondaryHostForRetryReads));
      perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
      perOperationPolicies.emplace_back(std::make_unique<_internal::StorageServiceVersionPolicy>(
          newOptions.ApiVersion.ToString()));
      m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
          newOptions,
          "storage-tables",
          _detail::ApiVersion,
          std::move(perRetryPolicies),
          std::move(perOperationPolicies));
    };

    explicit TableServicesClient(
        std::string subscriptionId,
        std::shared_ptr<Core::Credentials::TokenCredential> credential,
        const std::string& serviceUrl = Azure::Storage::_internal::TablesManagementPublicEndpoint,
        const TableClientOptions& options = {})
        : TableServicesClient(subscriptionId, options)

    {
      TableClientOptions newOptions = options;
      m_url = Azure::Core::Url(serviceUrl);
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
      perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
          m_url.GetHost(), newOptions.SecondaryHostForRetryReads));
      perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
      {
        Azure::Core::Credentials::TokenRequestContext tokenContext;
        tokenContext.Scopes.emplace_back(
            newOptions.Audience.HasValue() ? newOptions.Audience.Value().ToString()
                                           : TablesAudience::PublicAudience.ToString());
        perRetryPolicies.emplace_back(
            std::make_unique<_internal::StorageBearerTokenAuthenticationPolicy>(
                credential, tokenContext, newOptions.EnableTenantDiscovery));
      }
      perOperationPolicies.emplace_back(std::make_unique<_internal::StorageServiceVersionPolicy>(
          newOptions.ApiVersion.ToString()));
      m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
          newOptions,
          _internal::TablesServicePackageName,
          _detail::ApiVersion,
          std::move(perRetryPolicies),
          std::move(perOperationPolicies));
    };

    explicit TableServicesClient(
        const std::string& subscriptionId,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        const std::string& serviceUrl = Azure::Storage::_internal::TablesManagementPublicEndpoint,
        const TableClientOptions& options = TableClientOptions())
        : m_subscriptionId(std::move(subscriptionId)), m_url(Azure::Core::Url(serviceUrl))
    {
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
      perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
          m_url.GetHost(), options.SecondaryHostForRetryReads));
      perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
      perOperationPolicies.emplace_back(
          std::make_unique<_internal::StorageServiceVersionPolicy>(options.ApiVersion.ToString()));
      m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
          options,
          _internal::TablesServicePackageName,
          _detail::ApiVersion,
          std::move(perRetryPolicies),
          std::move(perOperationPolicies));

      TableClientOptions newOptions = options;
      newOptions.PerRetryPolicies.emplace_back(
          std::make_unique<_internal::SharedKeyPolicyLite>(credential));

      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies2;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies2;
      perRetryPolicies2.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
          m_url.GetHost(), newOptions.SecondaryHostForRetryReads));
      perRetryPolicies2.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
      perOperationPolicies2.emplace_back(std::make_unique<_internal::StorageServiceVersionPolicy>(
          newOptions.ApiVersion.ToString()));
      m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
          newOptions,
          _internal::TablesServicePackageName,
          _detail::ApiVersion,
          std::move(perRetryPolicies2),
          std::move(perOperationPolicies2));
    }

    static TableServicesClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& subscriptionId,
        const TableClientOptions& options = TableClientOptions())
    {
      auto parsedConnectionString = _internal::ParseConnectionString(connectionString);
      auto tablesUrl = std::move(parsedConnectionString.TableServiceUrl);

      if (parsedConnectionString.KeyCredential)
      {
        return TableServicesClient(subscriptionId, 
            std::move(parsedConnectionString.KeyCredential),
            tablesUrl.GetAbsoluteUrl().empty()
                ? Azure::Storage::_internal::TablesManagementPublicEndpoint
                : tablesUrl.GetAbsoluteUrl(),
            options);
      }
	  else
	  {
		return TableServicesClient(subscriptionId, options);
	  }
    }

    Models::ListTablesPagedResponse ListTables(
        const Models::ListTablesOptions& options={},
        const Azure::Core::Context& context={}) const;

    Response<Models::SetServicePropertiesResult> SetServiceProperties(
        Models::SetServicePropertiesOptions const& options = {},
        Core::Context const& context = {});

    Response<Models::TableServiceProperties> GetServiceProperties(
        Models::GetServicePropertiesOptions const& options = {},
        Core::Context const& context = {});

    Response<Models::ServiceStatistics> GetStatistics(
        Models::GetServiceStatisticsOptions const& options = {},
        Core::Context const& context = {});

    Response<Models::PreflightCheckResult> PreflightCheck(
        Models::PreflightCheckOptions const& options,
        Core::Context const& context = {});

    TableClient GetTableClient(std::string tableName) const {};
  private:
    std::shared_ptr<Core::Http::_internal::HttpPipeline> m_pipeline;
    Core::Url m_url;
    std::string m_subscriptionId;
  };
}}} // namespace Azure::Storage::Tables
