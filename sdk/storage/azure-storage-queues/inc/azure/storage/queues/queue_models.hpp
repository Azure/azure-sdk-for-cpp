// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
/**
 * @file
 * @brief Defines Queue service models and operation results.
 */

#pragma once

#include <azure/core/case_insensitive_containers.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/internal/extendable_enumeration.hpp>
#include <azure/core/nullable.hpp>
#include <azure/storage/queues/dll_import_export.hpp>

#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace Azure { namespace Storage { namespace Queues { namespace Models {
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
    Nullable<std::int32_t> Days;
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
    Models::RetentionPolicy RetentionPolicy;
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
    Models::RetentionPolicy RetentionPolicy;
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
    std::int32_t MaxAgeInSeconds = std::int32_t();
  };
  /**
   * @brief Response type for #Azure::Storage::Queues::QueueServiceClient::SetProperties.
   */
  struct SetServicePropertiesResult final
  {
  };
  /**
   * @brief Storage Service Properties.
   */
  struct QueueServiceProperties final
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
  /**
   * @brief The status of the secondary location.
   */
  class GeoReplicationStatus final
      : public Core::_internal::ExtendableEnumeration<GeoReplicationStatus> {
  public:
    /** Constructs a new GeoReplicationStatus instance */
    GeoReplicationStatus() = default;
    /** Constructs a new GeoReplicationStatus from a string. */
    explicit GeoReplicationStatus(std::string value) : ExtendableEnumeration(std::move(value)) {}

    /** Constant value of type GeoReplicationStatus: Live */
    AZ_STORAGE_QUEUES_DLLEXPORT const static GeoReplicationStatus Live;
    /** Constant value of type GeoReplicationStatus: Bootstrap */
    AZ_STORAGE_QUEUES_DLLEXPORT const static GeoReplicationStatus Bootstrap;
    /** Constant value of type GeoReplicationStatus: Unavailable */
    AZ_STORAGE_QUEUES_DLLEXPORT const static GeoReplicationStatus Unavailable;
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
   * @brief A user delegation key.
   */
  struct UserDelegationKey final
  {
    /**
     * The Azure Active Directory object ID in GUID format.
     */
    std::string SignedObjectId;
    /**
     * The Azure Active Directory tenant ID in GUID format.
     */
    std::string SignedTenantId;
    /**
     * The date-time the key is active.
     */
    DateTime SignedStartsOn;
    /**
     * The date-time the key expires.
     */
    DateTime SignedExpiresOn;
    /**
     * Abbreviation of the Azure Storage service that accepts the key.
     */
    std::string SignedService;
    /**
     * The service version that created the key.
     */
    std::string SignedVersion;
    /**
     * The delegated user tenant id in Azure AD. Return if DelegatedUserTid is specified.
     */
    Nullable<std::string> SignedDelegatedUserTid;
    /**
     * The key as a base64 string.
     */
    std::string Value;
  };
  /**
   * @brief An Azure Storage Queue.
   */
  struct QueueItem final
  {
    /**
     * The name of the Queue.
     */
    std::string Name;
    /**
     * A set of name-value pairs associated with this queue.
     */
    Core::CaseInsensitiveMap Metadata;
  };
  /**
   * @brief Include this parameter to specify that the queues' metadata be returned as part of the
   * response body.
   */
  enum class ListQueuesIncludeFlags
  {
    None = 0,
    Metadata = 1,
  };
  inline ListQueuesIncludeFlags operator|(ListQueuesIncludeFlags lhs, ListQueuesIncludeFlags rhs)
  {
    using type = std::underlying_type_t<ListQueuesIncludeFlags>;
    return static_cast<ListQueuesIncludeFlags>(static_cast<type>(lhs) | static_cast<type>(rhs));
  }
  inline ListQueuesIncludeFlags& operator|=(ListQueuesIncludeFlags& lhs, ListQueuesIncludeFlags rhs)
  {
    lhs = lhs | rhs;
    return lhs;
  }
  inline ListQueuesIncludeFlags operator&(ListQueuesIncludeFlags lhs, ListQueuesIncludeFlags rhs)
  {
    using type = std::underlying_type_t<ListQueuesIncludeFlags>;
    return static_cast<ListQueuesIncludeFlags>(static_cast<type>(lhs) & static_cast<type>(rhs));
  }
  inline ListQueuesIncludeFlags& operator&=(ListQueuesIncludeFlags& lhs, ListQueuesIncludeFlags rhs)
  {
    lhs = lhs & rhs;
    return lhs;
  }
  /**
   * @brief Response type for #Azure::Storage::Queues::QueueClient::Create.
   */
  struct CreateQueueResult final
  {
    /**
     * Indicates if the queue was successfully created by this operation.
     */
    bool Created = bool();
  };
  /**
   * @brief Response type for #Azure::Storage::Queues::QueueClient::Delete.
   */
  struct DeleteQueueResult final
  {
    /**
     * Indicates if the queue was successfully deleted by this operation.
     */
    bool Deleted = true;
  };
  /**
   * @brief Response type for #Azure::Storage::Queues::QueueClient::SetMetadata.
   */
  struct SetQueueMetadataResult final
  {
  };
  /**
   * @brief Signed identifier.
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
   * @brief Response type for #Azure::Storage::Queues::QueueClient::GetAccessPolicy.
   */
  struct QueueAccessPolicy final
  {
    /**
     * A collection of signed identifiers.
     */
    std::vector<SignedIdentifier> SignedIdentifiers;
  };
  /**
   * @brief Response type for #Azure::Storage::Queues::QueueClient::SetAccessPolicy.
   */
  struct SetQueueAccessPolicyResult final
  {
  };
  /**
   * @brief The object returned in the QueueMessageList array when calling Get Messages on a
   * Queue.
   */
  struct QueueMessage final
  {
    /**
     * The Id of the Message.
     */
    std::string MessageId;
    /**
     * The time the Message was inserted into the Queue.
     */
    DateTime InsertedOn;
    /**
     * The time that the Message will expire and be automatically deleted.
     */
    DateTime ExpiresOn;
    /**
     * This value is required to delete the Message. If deletion fails using this popreceipt then
     * the message has been dequeued by another client.
     */
    std::string PopReceipt;
    /**
     * The time that the message will again become visible in the Queue.
     */
    DateTime NextVisibleOn;
    /**
     * The number of times the message has been dequeued.
     */
    std::int64_t DequeueCount = std::int64_t();
    /**
     * The content of the Message.
     */
    std::string MessageText;
  };
  /**
   * @brief Response type for #Azure::Storage::Queues::QueueClient::ReceiveMessages.
   */
  struct ReceivedMessages final
  {
    /**
     * The object returned when calling Get Messages on a Queue.
     */
    std::vector<QueueMessage> Messages;
  };
  /**
   * @brief Response type for #Azure::Storage::Queues::QueueClient::ClearMessages.
   */
  struct ClearMessagesResult final
  {
  };
  /**
   * @brief The object returned in the QueueMessageList array when calling Put Message on a Queue.
   */
  struct EnqueueMessageResult final
  {
    /**
     * The Id of the Message.
     */
    std::string MessageId;
    /**
     * The time the Message was inserted into the Queue.
     */
    DateTime InsertedOn;
    /**
     * The time that the Message will expire and be automatically deleted.
     */
    DateTime ExpiresOn;
    /**
     * This value is required to delete the Message. If deletion fails using this popreceipt then
     * the message has been dequeued by another client.
     */
    std::string PopReceipt;
    /**
     * The time that the message will again become visible in the Queue.
     */
    DateTime NextVisibleOn;
  };
  /**
   * @brief The object returned in the QueueMessageList array when calling Peek Messages on a
   * Queue.
   */
  struct PeekedQueueMessage final
  {
    /**
     * The Id of the Message.
     */
    std::string MessageId;
    /**
     * The time the Message was inserted into the Queue.
     */
    DateTime InsertedOn;
    /**
     * The time that the Message will expire and be automatically deleted.
     */
    DateTime ExpiresOn;
    /**
     * The number of times the message has been dequeued.
     */
    std::int64_t DequeueCount = std::int64_t();
    /**
     * The content of the Message.
     */
    std::string MessageText;
  };
  /**
   * @brief Response type for #Azure::Storage::Queues::QueueClient::PeekMessages.
   */
  struct PeekedMessages final
  {
    /**
     * The object returned when calling Peek Messages on a Queue.
     */
    std::vector<PeekedQueueMessage> Messages;
  };
  /**
   * @brief Response type for #Azure::Storage::Queues::QueueClient::DeleteMessage.
   */
  struct DeleteMessageResult final
  {
  };
  /**
   * @brief Response type for #Azure::Storage::Queues::QueueClient::UpdateMessage.
   */
  struct UpdateMessageResult final
  {
    /**
     * The pop receipt of the queue message.
     */
    std::string PopReceipt;
    /**
     * A UTC date/time value that represents when the message will be visible on the queue.
     */
    DateTime NextVisibleOn;
  };
}}}} // namespace Azure::Storage::Queues::Models