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

namespace Azure { namespace Storage { namespace Tables { namespace Models {
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
    RetentionPolicy RetentionPolicy;
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
    RetentionPolicy RetentionPolicy;
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
      TableServiceProperties TableServiceProperties;
    };

    /**
     * @brief Response type for #Azure::Storage::Queues::QueueServiceClient::SetProperties.
     */
    struct SetServicePropertiesResult final
    {
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
      AZ_STORAGE_TABLES_DLLEXPORT const static GeoReplicationStatus
          Unavailable;

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


}}}} // namespace Azure::Storage::Tables::Models