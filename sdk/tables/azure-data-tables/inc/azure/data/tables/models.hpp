// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

#include "azure/data/tables/dll_import_export.hpp"
#include "azure/data/tables/enum_operators.hpp"

#include <azure/core/datetime.hpp>
#include <azure/core/internal/extendable_enumeration.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/paged_response.hpp>

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Azure { namespace Data { namespace Tables {
  class TableServiceClient;
  class TableClient;

  namespace Models {
    /**
     * @brief Table definition struct.
     */
    struct Table final
    {
      /**
       * Table Name.
       */
      std::string TableName;
      /**
       * Meta data.
       */
      std::string Metadata;
      /**
       * Edit Link.
       */
      std::string EditLink;
      /**
       * Type.
       */
      std::string Type;
      /**
       * Id.
       */
      std::string Id;
    };

    /**
     * @brief Include this parameter to specify that the tables' metadata be returned as part of
     * the response body.
     */
    enum class QueryTablesIncludeFlags
    {
      None = 0,
      Metadata = 1,
    };

    /**
     * @brief Query Tables options.
     *
     */
    struct QueryTablesOptions final
    {
      /**
       * @brief Specifies a string that filters the results to return only tables whose name
       * begins with the specified prefix.
       */
      Azure::Nullable<std::string> Prefix;

      /**
       * @brief A string value that identifies the portion of the list of tables to be returned
       * with the next listing operation. The operation returns a non-empty continuation token if
       * the listing operation did not return all tables remaining to be listed with the current
       * segment. The ContinuationToken value can be used as the value for the ContinuationToken
       * parameter in a subsequent call to request the next segment of list items.
       */
      Azure::Nullable<std::string> ContinuationToken;

      /**
       * @brief Specifies the maximum number of tables to return.
       */
      Azure::Nullable<std::int32_t> PageSizeHint;

      /**
       * @brief Specifies that the table's metadata be returned.
       */
      Models::QueryTablesIncludeFlags Include = Models::QueryTablesIncludeFlags::None;
    };

    /**
     * @brief Query tables paged response.
     */
    class QueryTablesPagedResponse final
        : public Azure::Core::PagedResponse<QueryTablesPagedResponse> {

      friend class Azure::Data::Tables::TableServiceClient;
      friend class Azure::Core::PagedResponse<QueryTablesPagedResponse>;

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
       * Table items.
       */
      std::vector<Models::Table> Tables;

    public:
      /**
       * Table Service Client.
       */
      std::shared_ptr<TableServiceClient> m_tableServiceClient;
      /** Operation options */
      QueryTablesOptions m_operationOptions;

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
      Nullable<std::int32_t> DataRetentionInDays;
    };

    /**
     * @brief A summary of request statistics grouped by API in hour or minute aggregates for
     * tables.
     */
    struct Metrics final
    {
      /**
       * The version of Storage Analytics to configure.
       */
      std::string Version;
      /**
       * Indicates whether metrics are enabled for the Table service.
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
       * service. You can also use the wild card character '*' to allow all origin domains to make
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
      std::int32_t MaxAgeInSeconds = int32_t();
    };

    /**
     * @brief Table Service Properties.
     */
    struct TableServiceProperties final
    {
      /**
       * Azure Analytics Logging settings.
       */
      AnalyticsLogging Logging;
      /**
       * A summary of request statistics grouped by API in hourly aggregates for tables.
       */
      Metrics HourMetrics;
      /**
       * A summary of request statistics grouped by API in minute aggregates for tables.
       */
      Metrics MinuteMetrics;
      /**
       * The set of CORS rules.
       */
      std::vector<CorsRule> Cors;
    };

    /**
     * @brief Set Service Properties options.
     *
     */
    struct SetServicePropertiesOptions final
    {
      /**
       * Service properties.
       */
      TableServiceProperties ServiceProperties;
    };

    /**
     * @brief Set service properties response
     */
    struct SetServicePropertiesResult final
    {
    };

    /**
     * @brief Preflight check response.
     */
    struct PreflightCheckResult final
    {
    };

    /**
     * @brief Preflight check options.
     *
     */
    struct PreflightCheckOptions final
    {
      /**
       * Origin.
       */
      std::string Origin;
      /**
       * Table Name.
       */
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
      AZ_DATA_TABLES_DLLEXPORT const static GeoReplicationStatus Live;
      /** Constant value of type GeoReplicationStatus: Bootstrap */
      AZ_DATA_TABLES_DLLEXPORT const static GeoReplicationStatus Bootstrap;
      /** Constant value of type GeoReplicationStatus: Unavailable */
      AZ_DATA_TABLES_DLLEXPORT const static GeoReplicationStatus Unavailable;

    private:
      std::string m_value;
    };

    /**
     * @brief Table Entity Data Type.
     */
    class TableEntityDataType final
        : public Azure::Core::_internal::ExtendableEnumeration<TableEntityDataType> {
    public:
      /**
       * @brief Construct a new TableEntityDataType object
       */
      TableEntityDataType() = default;
      /**
       * @brief Construct a new TableEntityDataType object
       *
       * @param tableEntityDataType entity data type string.
       */
      explicit TableEntityDataType(std::string tableEntityDataType)
          : ExtendableEnumeration(std::move(tableEntityDataType))
      {
      }
      /** Constant value of type TableEntityDataType:EdmBinary */
      AZ_DATA_TABLES_DLLEXPORT const static TableEntityDataType EdmBinary;
      /** Constant value of type TableEntityDataType:EdmBoolean */
      AZ_DATA_TABLES_DLLEXPORT const static TableEntityDataType EdmBoolean;
      /** Constant value of type TableEntityDataType:EdmDateTime */
      AZ_DATA_TABLES_DLLEXPORT const static TableEntityDataType EdmDateTime;
      /** Constant value of type TableEntityDataType:EdmDouble */
      AZ_DATA_TABLES_DLLEXPORT const static TableEntityDataType EdmDouble;
      /** Constant value of type TableEntityDataType:EdmGuid */
      AZ_DATA_TABLES_DLLEXPORT const static TableEntityDataType EdmGuid;
      /** Constant value of type TableEntityDataType:EdmInt32 */
      AZ_DATA_TABLES_DLLEXPORT const static TableEntityDataType EdmInt32;
      /** Constant value of type TableEntityDataType:EdmInt64 */
      AZ_DATA_TABLES_DLLEXPORT const static TableEntityDataType EdmInt64;
      /** Constant value of type TableEntityDataType:EdmString */
      AZ_DATA_TABLES_DLLEXPORT const static TableEntityDataType EdmString;
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

    /**
     * @brief Delete result.
     *
     */
    struct DeleteTableResult final
    {
    };

    /**
     * @brief Signed identifier.
     *
     */
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

    /**
     * @brief Table Access Policy.
     *
     */
    struct TableAccessPolicy final
    {
      /**
       * A collection of signed identifiers.
       */
      std::vector<SignedIdentifier> SignedIdentifiers;
    };

    /**
     * @brief Set Table Access Policy result.
     *
     */
    struct SetTableAccessPolicyResult final
    {
    };

    /**
     * @brief Table entity property.
     *
     */
    class TableEntityProperty final {
    public:
      /** Default constructor. */
      TableEntityProperty() = default;

      /**
       * @brief Construct a new TableEntityProperty object.
       *
       * @param value Property value.
       */
      TableEntityProperty(std::string const& value) : Value(std::move(value)) {}
      /**
       * @brief Construct a new TableEntityProperty object.
       * @param value Property value.
       * @param type Property type.
       */
      TableEntityProperty(std::string const& value, TableEntityDataType type)
          : Value(std::move(value)), Type(type)
      {
      }
      /**
       * Property value.
       */
      std::string Value;
      /**
       * Property type.
       */
      Azure::Nullable<TableEntityDataType> Type;
    };

    /**
     * @brief Table Entity
     *
     */
    class TableEntity final {
      constexpr static const char* PartitionKeyPropertyName = "PartitionKey";
      constexpr static const char* RowKeyPropertyName = "RowKey";
      constexpr static const char* ETagPropertyName = "odata.etag";
      constexpr static const char* TimestampPropertyName = "Timestamp";

    public:
      /**
       * Properties
       */
      std::map<std::string, TableEntityProperty> Properties;

      /**
       * @brief Get partition key.
       *
       * @return Partition key
       */
      TableEntityProperty GetPartitionKey() const { return GetProperty(PartitionKeyPropertyName); }

      /**
       * @brief Set Partition key.
       *
       * @param partitionKey Partition key.
       */
      void SetPartitionKey(const std::string& partitionKey)
      {
        Properties[PartitionKeyPropertyName] = TableEntityProperty(partitionKey);
      }

      /**
       * @brief Get row key.
       *
       * @return Row key
       */
      TableEntityProperty GetRowKey() const { return GetProperty(RowKeyPropertyName); }
      /**
       * @brief Set Row key.
       *
       * @param rowKey Row key.
       */
      void SetRowKey(const std::string& rowKey)
      {
        Properties[RowKeyPropertyName] = TableEntityProperty(rowKey);
      }

      /**
       * @brief Get ETag.
       *
       * @return ETag
       */
      TableEntityProperty GetETag() const { return GetProperty(ETagPropertyName); }
      /**
       * @brief Set ETag.
       *
       * @param eTag ETag.
       */
      void SetETag(const std::string& eTag)
      {
        Properties[ETagPropertyName] = TableEntityProperty(eTag);
      }

      /**
       * @brief Get time stamp.
       *
       * @return timestamp
       */
      TableEntityProperty GetTimestamp() const { return GetProperty(TimestampPropertyName); }
      /**
       * @brief Set time stamp.
       *
       * @param timestamp time stamp.
       */
      void SetTimestamp(const std::string& timestamp)
      {
        Properties[TimestampPropertyName] = TableEntityProperty(timestamp);
      }

    private:
      TableEntityProperty GetProperty(std::string const& name) const
      {
        return Properties.find(name) == Properties.end() ? TableEntityProperty()
                                                         : Properties.at(name);
      }
    };

    /**
     * @brief Upsert Kind
     *
     */
    enum class UpsertKind
    {
      Update,
      Merge,
    };

    /**
     * @brief Upsert Entity options.
     *
     */
    struct UpsertEntityOptions
    {
      /**
       * @brief Upsert type.
       *
       */
      UpsertKind UpsertType = UpsertKind::Update;
    };
    /**
     * @brief Add Entity options.
     *
     */
    struct AddEntityOptions : public UpsertEntityOptions
    {
      AddEntityOptions() = default;
      /**
       * @brief Create Entity options constructor.
       *
       * @param other Upsert Entity options.
       */
      explicit AddEntityOptions(UpsertEntityOptions const& other) { (void)other; }
    };

    /**
     * @brief Add Entity result.
     *
     */
    struct AddEntityResult
    {
      /**
       * ETag
       */
      std::string ETag;
    };

    /**
     * @brief Update Entity options.
     *
     */
    struct UpdateEntityOptions final
    {
      /**
       * @brief Update Entity options default constructor.
       *
       */
      UpdateEntityOptions() = default;
      /**
       * @brief Update Entity options constructor.
       *
       * @param other Update Entity options.
       */
      UpdateEntityOptions(UpsertEntityOptions const& other) { (void)other; }
    };

    /**
     * @brief Update Entity result.
     *
     */
    struct UpdateEntityResult
    {
      /**
       * ETag
       */
      std::string ETag;
    };

    /**
     * @brief Merge Entity options.
     *
     */
    struct MergeEntityOptions final
    {
      /**
       * @brief Merge Entity options default constructor.
       *
       */
      MergeEntityOptions() = default;
      /**
       * @brief Merge Entity options constructor.
       *
       * @param other Upsert Entity options.
       */
      MergeEntityOptions(UpsertEntityOptions const& other) { (void)other; }
    };

    /**
     * @brief Merge Entity result.
     *
     */
    struct MergeEntityResult
    {
      /**
       * ETag
       */
      std::string ETag;
    };

    /**
     * @brief Delete Entity result.
     *
     */
    struct DeleteEntityResult final
    {
    };

    /**
     * @brief Upsert Entity result.
     *
     */
    struct UpsertEntityResult final : public MergeEntityResult, UpdateEntityResult, AddEntityResult
    {
      /**
       * ETag
       */
      std::string ETag;
      /**
       * @brief Upsert Entity result default constructor.
       *
       */
      UpsertEntityResult() = default;
      /**
       * @brief Upsert Entity result constructor.
       *
       * @param other Merge Entity result.
       */
      UpsertEntityResult(MergeEntityResult const& other)
          : MergeEntityResult(other), ETag(std::move(other.ETag))
      {
      }
      /**
       * @brief Upsert Entity result constructor.
       *
       * @param other Update Entity result.
       */
      UpsertEntityResult(UpdateEntityResult const& other)
          : UpdateEntityResult(other), ETag(std::move(other.ETag))
      {
      }
      /**
       * @brief Upsert Entity result constructor.
       *
       * @param other Add Entity result.
       */
      UpsertEntityResult(AddEntityResult const& other)
          : AddEntityResult(other), ETag(std::move(other.ETag))
      {
      }
    };

    /**
     * @brief Query Entities options.
     *
     */
    struct QueryEntitiesOptions final
    {
      /**
       * @brief The Partition key.
       *
       */
      std::string PartitionKey;
      /**
       * @brief The row key.
       *
       */
      std::string RowKey;
      /**
       * @brief The select query.
       *
       */
      std::string SelectColumns;
      /**
       * @brief The filter expression.
       *
       */
      Azure::Nullable<std::string> Filter;
    };

    /**
     * @brief Query Entities result.
     *
     */
    class QueryEntitiesPagedResponse final
        : public Azure::Core::PagedResponse<QueryEntitiesPagedResponse> {
    public:
      /**
       * Next partition key.
       */
      std::string NextPartitionKey;
      /**
       * Next row key.
       */
      std::string NextRowKey;
      /**
       * Table entities.
       */
      std::vector<Models::TableEntity> TableEntities;

    private:
      std::shared_ptr<TableClient> m_tableClient;
      QueryEntitiesOptions m_operationOptions;
      friend class Azure::Data::Tables::TableServiceClient;
      friend class Azure::Core::PagedResponse<QueryEntitiesPagedResponse>;

      void OnNextPage(const Azure::Core::Context& context);
    };

    /**
     * @brief Transaction Action
     *
     */
    enum class TransactionActionType
    {
      Add,
      UpdateMerge,
      UpdateReplace,
      Delete,
      InsertMerge,
      InsertReplace
    };

    /**
     * @brief Transaction Step
     *
     */
    struct TransactionStep final
    {
      /**
       * Action.
       */
      TransactionActionType Action;
      /**
       * Entity.
       */
      Models::TableEntity Entity;
    };

    /**
     * @brief Transaction Error
     *
     */
    struct TransactionError final
    {
      /**
       * Error Message.
       */
      std::string Message;
      /**
       * Error Code.
       */
      std::string Code;
    };

    /**
     * @brief Submit Transaction options.
     *
     */
    struct SubmitTransactionResult final
    {
      /**
       * Status Code.
       */
      std::string StatusCode;
      /**
       * Error.
       */
      Azure::Nullable<TransactionError> Error;
    };
  } // namespace Models
}}} // namespace Azure::Data::Tables
