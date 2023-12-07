// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

#include <azure/core/datetime.hpp>
#include <azure/core/internal/extendable_enumeration.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/paged_response.hpp>
#include <azure/storage/tables/dll_import_export.hpp>

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Azure { namespace Storage { namespace Tables {
  class TableServicesClient;
  class TableClient;

  namespace Models {

    struct Table final
    {
      std::string TableName;
      std::string Metadata;
      std::string EditLink;
      std::string Type;
      std::string Id;
    };

    /**
     * @brief Include this parameter to specify that the queues' metadata be returned as part of
     * the response body.
     */
    enum class ListTablesIncludeFlags
    {
      None = 0,
      Metadata = 1,
    };
    inline ListTablesIncludeFlags operator|(ListTablesIncludeFlags lhs, ListTablesIncludeFlags rhs)
    {
      using type = std::underlying_type_t<ListTablesIncludeFlags>;
      return static_cast<ListTablesIncludeFlags>(static_cast<type>(lhs) | static_cast<type>(rhs));
    }
    inline ListTablesIncludeFlags& operator|=(
        ListTablesIncludeFlags& lhs,
        ListTablesIncludeFlags rhs)
    {
      lhs = lhs | rhs;
      return lhs;
    }
    inline ListTablesIncludeFlags operator&(ListTablesIncludeFlags lhs, ListTablesIncludeFlags rhs)
    {
      using type = std::underlying_type_t<ListTablesIncludeFlags>;
      return static_cast<ListTablesIncludeFlags>(static_cast<type>(lhs) & static_cast<type>(rhs));
    }
    inline ListTablesIncludeFlags& operator&=(
        ListTablesIncludeFlags& lhs,
        ListTablesIncludeFlags rhs)
    {
      lhs = lhs & rhs;
      return lhs;
    }

    struct ListTablesOptions final
    {
      /**
       * @brief Specifies a string that filters the results to return only queues whose name
       * begins with the specified prefix.
       */
      Azure::Nullable<std::string> Prefix;

      /**
       * @brief A string value that identifies the portion of the list of queues to be returned
       * with the next listing operation. The operation returns a non-empty continuation token if
       * the listing operation did not return all queues remaining to be listed with the current
       * segment. The ContinuationToken value can be used as the value for the ContinuationToken
       * parameter in a subsequent call to request the next segment of list items.
       */
      Azure::Nullable<std::string> ContinuationToken;

      /**
       * @brief Specifies the maximum number of queues to return.
       */
      Azure::Nullable<int32_t> PageSizeHint;

      /**
       * @brief Specifies that the queue's metadata be returned.
       */
      Models::ListTablesIncludeFlags Include = Models::ListTablesIncludeFlags::None;
    };

    /**
     * @brief Response type for #Azure::Storage::Queues::QueueServiceClient::ListQueues.
     */
    class ListTablesPagedResponse final
        : public Azure::Core::PagedResponse<ListTablesPagedResponse> {

      friend class TableServicesClient;
      friend class Azure::Core::PagedResponse<ListTablesPagedResponse>;

    public:
      /**
       * Service endpoint.
       */
      std::string ServiceEndpoint;

      /**
       * Container name prefix that's used to filter the result.
       */
      Azure::Nullable<std::string> Prefix;

      /**
       * Queue items.
       */
      std::vector<Models::Table> Tables;

    public:
      std::shared_ptr<TableServicesClient> m_tableServiceClient;
      ListTablesOptions m_operationOptions;

    private:
      void OnNextPage(const Azure::Core::Context& context);
    };
    /**
     * @brief The retention policy.
     */
    struct RetentionPolicy final
    {
      /**
       * Indicates whether a retention policy is enabled for the storage service.
       */
      bool IsEnabled = bool();
      /**
       * Indicates the number of days that metrics or logging or soft-deleted data should be
       * retained. All data older than this value will be deleted.
       */
      Nullable<int32_t> Days;
    };
    /**
     * @brief A summary of request statistics grouped by API in hour or minute aggregates for
     * queues.
     */
    struct Metrics final
    {
      /**
       * The version of Storage Analytics to configure.
       */
      std::string Version;
      /**
       * Indicates whether metrics are enabled for the Queue service.
       */
      bool IsEnabled = bool();
      /**
       * Indicates whether metrics should generate summary statistics for called API operations.
       */
      Nullable<bool> IncludeApis;
      /**
       * The retention policy.
       */
      RetentionPolicy RetentionPolicyDefinition;
    };
    /**
     * @brief Azure Analytics Logging settings.
     */
    struct AnalyticsLogging final
    {
      /**
       * The version of Storage Analytics to configure.
       */
      std::string Version;
      /**
       * Indicates whether all delete requests should be logged.
       */
      bool Delete = bool();
      /**
       * Indicates whether all read requests should be logged.
       */
      bool Read = bool();
      /**
       * Indicates whether all write requests should be logged.
       */
      bool Write = bool();
      /**
       * The retention policy.
       */
      RetentionPolicy RetentionPolicyDefinition;
    };
    /**
     * @brief CORS is an HTTP feature that enables a web application running under one domain to
     * access resources in another domain. Web browsers implement a security restriction known as
     * same-origin policy that prevents a web page from calling APIs in a different domain; CORS
     * provides a secure way to allow one domain (the origin domain) to call APIs in another domain.
     */
    struct CorsRule final
    {
      /**
       * The origin domains that are permitted to make a request against the storage service via
       * CORS. The origin domain is the domain from which the request originates. Note that the
       * origin must be an exact case-sensitive match with the origin that the user age sends to the
       * service. You can also use the wildcard character '*' to allow all origin domains to make
       * requests via CORS.
       */
      std::string AllowedOrigins;
      /**
       * The methods (HTTP request verbs) that the origin domain may use for a CORS request. (comma
       * separated).
       */
      std::string AllowedMethods;
      /**
       * The request headers that the origin domain may specify on the CORS request.
       */
      std::string AllowedHeaders;
      /**
       * The response headers that may be sent in the response to the CORS request and exposed by
       * the browser to the request issuer.
       */
      std::string ExposedHeaders;
      /**
       * The maximum amount time that a browser should cache the preflight OPTIONS request.
       */
      int32_t MaxAgeInSeconds = int32_t();
    };
    /**
     * @brief Storage Service Properties.
     */
    struct TableServiceProperties final
    {
      /**
       * Azure Analytics Logging settings.
       */
      AnalyticsLogging Logging;
      /**
       * A summary of request statistics grouped by API in hourly aggregates for queues.
       */
      Metrics HourMetrics;
      /**
       * A summary of request statistics grouped by API in minute aggregates for queues.
       */
      Metrics MinuteMetrics;
      /**
       * The set of CORS rules.
       */
      std::vector<CorsRule> Cors;
    };
    struct GetServicePropertiesOptions final
    {
    };

    struct SetServicePropertiesOptions final
    {
      TableServiceProperties ServiceProperties;
    };

    /**
     * @brief Response type for #Azure::Storage::Tables::TablesServiceClient::SetProperties.
     */
    struct SetServicePropertiesResult final
    {
    };

    /**
     * @brief Response type for #Azure::Storage::Tables::TablesServiceClient::PreflightCheck.
     */
    struct PreflightCheckResult final
    {
    };

    struct PreflightCheckOptions final
    {
      std::string Origin;
      std::string TableName;
    };
    /**
     * @brief The status of the secondary location.
     */
    class GeoReplicationStatus final {
    public:
      /** Constructs a new GeoReplicationStatus instance */
      GeoReplicationStatus() = default;
      /** Constructs a new GeoReplicationStatus from a string. */
      explicit GeoReplicationStatus(std::string value) : m_value(std::move(value)) {}
      /** Compares with another GeoReplicationStatus. */
      bool operator==(const GeoReplicationStatus& other) const { return m_value == other.m_value; }
      /** Compares with another GeoReplicationStatus. */
      bool operator!=(const GeoReplicationStatus& other) const { return !(*this == other); }
      /** Converts the value to a string. */
      const std::string& ToString() const { return m_value; }
      /** Constant value of type GeoReplicationStatus: Live */
      AZ_STORAGE_TABLES_DLLEXPORT const static GeoReplicationStatus Live;
      /** Constant value of type GeoReplicationStatus: Bootstrap */
      AZ_STORAGE_TABLES_DLLEXPORT const static GeoReplicationStatus Bootstrap;
      /** Constant value of type GeoReplicationStatus: Unavailable */
      AZ_STORAGE_TABLES_DLLEXPORT const static GeoReplicationStatus Unavailable;

    private:
      std::string m_value;
    };
    /**
     * @brief Geo-Replication information for the Secondary Storage Service.
     */
    struct GeoReplication final
    {
      /**
       * The status of the secondary location.
       */
      GeoReplicationStatus Status;
      /**
       * A GMT date/time value, to the second. All primary writes preceding this value are
       * guaranteed to be available for read operations at the secondary. Primary writes after this
       * point in time may or may not be available for reads.
       */
      Nullable<DateTime> LastSyncedOn;
    };
    /**
     * @brief Stats for the storage service.
     */
    struct ServiceStatistics final
    {
      /**
       * Geo-Replication information for the Secondary Storage Service.
       */
      Models::GeoReplication GeoReplication;
    };

    struct GetServiceStatisticsOptions final
    {
    };

    struct DeleteResult final
    {
    };

    struct SignedIdentifier final
    {
      /**
       * A unique id.
       */
      std::string Id;
      /**
       * The date-time the policy is active.
       */
      Nullable<DateTime> StartsOn;
      /**
       * The date-time the policy expires.
       */
      Nullable<DateTime> ExpiresOn;
      /**
       * The permissions for the acl policy.
       */
      std::string Permissions;
    };
    struct TableAccessPolicy final
    {
      /**
       * A collection of signed identifiers.
       */
      std::vector<SignedIdentifier> SignedIdentifiers;
    };

    struct GetTableAccessPolicyOptions final
    {
    };

    struct SetTableAccessPolicyOptions final
    {
    };

    struct SetTableAccessPolicyResult final
    {
    };

    struct TableEntity final
    {
      std::string PartitionKey;
      std::string RowKey;
      std::map<std::string, std::string> Properties;
      Azure::Nullable<std::string> ETag;
    };
    enum UpsertKind
    {
      Update,
      Merge,
    };
    struct UpsertEntityOptions
    {
      UpsertKind UpsertType = UpsertKind::Update;
    };
    struct CreateEntityOptions : public UpsertEntityOptions
    {
      CreateEntityOptions() = default;
      CreateEntityOptions(UpsertEntityOptions const& other) { (void)other; }
    };

    struct CreateEntityResult
    {
      std::string ETag;
    };

    struct UpdateEntityOptions final
    {
      UpdateEntityOptions() = default;
      UpdateEntityOptions(UpsertEntityOptions const& other) { (void)other; }
    };

    struct UpdateEntityResult
    {
      std::string ETag;
    };

    struct MergeEntityOptions final
    {
      MergeEntityOptions() = default;
      MergeEntityOptions(UpsertEntityOptions const& other) { (void)other; }
    };

    struct MergeEntityResult
    {
      std::string ETag;
    };
    struct DeleteEntityOptions final
    {
    };

    struct DeleteEntityResult final
    {
    };

    struct UpsertEntityResult final : public MergeEntityResult,
                                      UpdateEntityResult,
                                      CreateEntityResult
    {
      std::string ETag;

      UpsertEntityResult() = default;
      UpsertEntityResult(MergeEntityResult const& other)
          : MergeEntityResult(other), ETag(std::move(other.ETag))
      {
      }
      UpsertEntityResult(UpdateEntityResult const& other)
          : UpdateEntityResult(other), ETag(std::move(other.ETag))
      {
      }
      UpsertEntityResult(CreateEntityResult const& other)
          : CreateEntityResult(other), ETag(std::move(other.ETag))
      {
      }
    };

    struct QueryEntitiesOptions final
    {
      std::string PartitionKey;
      std::string RowKey;
      std::string SelectColumns;
      Azure::Nullable<std::string> Filter;
    };

    class QueryEntitiesPagedResponse final
        : public Azure::Core::PagedResponse<QueryEntitiesPagedResponse> {
    public:
      std::string NextPartitionKey;
      std::string NextRowKey;
      std::vector<Models::TableEntity> TableEntities;

    private:
      std::shared_ptr<TableClient> m_tableClient;
      QueryEntitiesOptions m_operationOptions;
      friend class TableServicesClient;
      friend class Azure::Core::PagedResponse<QueryEntitiesPagedResponse>;

      void OnNextPage(const Azure::Core::Context& context);
    };

    enum TransactionAction
    {
      InsertEntity = 32,
      DeleteEntity,
      MergeEntity,
      UpdateEntity,
      InsertMergeEntity,
      InsertReplaceEntity
    };

    struct TransactionStep final
    {
      TransactionAction Action;
      Models::TableEntity Entity;
    };

    struct TransactionError final
    {
      std::string Message;
      std::string Code;
    };

    struct SubmitTransactionResult final
    {
      std::string StatusCode;
      Azure::Nullable<TransactionError> Error;
    };
  } // namespace Models
}}} // namespace Azure::Storage::Tables
