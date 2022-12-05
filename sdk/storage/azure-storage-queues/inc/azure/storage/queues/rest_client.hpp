// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// Code generated by Microsoft (R) AutoRest C++ Code Generator.
// Changes may cause incorrect behavior and will be lost if the code is regenerated.
#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

#include <azure/core/context.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/internal/strings.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/response.hpp>
#include <azure/core/url.hpp>
#include <azure/storage/queues/dll_import_export.hpp>

namespace Azure { namespace Storage { namespace Queues {
  namespace _detail {
    /**
     * The version used for the operations to Azure storage services.
     */
    constexpr static const char* ApiVersion = "2018-03-28";
  } // namespace _detail
  namespace Models {
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
      int32_t MaxAgeInSeconds = int32_t();
    };
    /**
     * @brief Response type for #Azure::Storage::Queues::ServiceClient::SetProperties.
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
    class GeoReplicationStatus final {
    public:
      GeoReplicationStatus() = default;
      explicit GeoReplicationStatus(std::string value) : m_value(std::move(value)) {}
      bool operator==(const GeoReplicationStatus& other) const { return m_value == other.m_value; }
      bool operator!=(const GeoReplicationStatus& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      AZ_STORAGE_QUEUES_DLLEXPORT const static GeoReplicationStatus Live;
      AZ_STORAGE_QUEUES_DLLEXPORT const static GeoReplicationStatus Bootstrap;
      AZ_STORAGE_QUEUES_DLLEXPORT const static GeoReplicationStatus Unavailable;

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
      std::map<
          std::string,
          std::string,
          Core::_internal::StringExtensions::CaseInsensitiveComparator>
          Metadata;
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
    inline ListQueuesIncludeFlags& operator|=(
        ListQueuesIncludeFlags& lhs,
        ListQueuesIncludeFlags rhs)
    {
      lhs = lhs | rhs;
      return lhs;
    }
    inline ListQueuesIncludeFlags operator&(ListQueuesIncludeFlags lhs, ListQueuesIncludeFlags rhs)
    {
      using type = std::underlying_type_t<ListQueuesIncludeFlags>;
      return static_cast<ListQueuesIncludeFlags>(static_cast<type>(lhs) & static_cast<type>(rhs));
    }
    inline ListQueuesIncludeFlags& operator&=(
        ListQueuesIncludeFlags& lhs,
        ListQueuesIncludeFlags rhs)
    {
      lhs = lhs & rhs;
      return lhs;
    }
    namespace _detail {
      /**
       * @brief The object returned when calling List Queues on a Queue Service.
       */
      struct ListQueuesResult final
      {
        std::string ServiceEndpoint;
        std::string Prefix;
        /**
         * Array of QueueItem.
         */
        std::vector<QueueItem> Items;
        Nullable<std::string> ContinuationToken;
      };
    } // namespace _detail
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
     * @brief Response type for #Azure::Storage::Queues::QueueClient::GetProperties.
     */
    struct QueueProperties final
    {
      std::map<
          std::string,
          std::string,
          Core::_internal::StringExtensions::CaseInsensitiveComparator>
          Metadata;
      /**
       * The approximate number of messages in the queue. This number is not lower than the actual
       * number of messages in the queue, but could be higher.
       */
      int32_t ApproximateMessageCount = int32_t();
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
      int64_t DequeueCount = int64_t();
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
    namespace _detail {
      /**
       * @brief A Message object which can be stored in a Queue.
       */
      struct QueueMessageInternal final
      {
        /**
         * The content of the message.
         */
        std::string MessageText;
      };
    } // namespace _detail
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
      int64_t DequeueCount = int64_t();
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
  } // namespace Models
  namespace _detail {
    class ServiceClient final {
    public:
      struct SetServicePropertiesOptions final
      {
        Models::QueueServiceProperties QueueServiceProperties;
      };
      static Response<Models::SetServicePropertiesResult> SetProperties(
          Core::Http::_internal::HttpPipeline& pipeline,
          const Core::Url& url,
          const SetServicePropertiesOptions& options,
          const Core::Context& context);
      struct GetServicePropertiesOptions final
      {
      };
      static Response<Models::QueueServiceProperties> GetProperties(
          Core::Http::_internal::HttpPipeline& pipeline,
          const Core::Url& url,
          const GetServicePropertiesOptions& options,
          const Core::Context& context);
      struct GetServiceStatisticsOptions final
      {
      };
      static Response<Models::ServiceStatistics> GetStatistics(
          Core::Http::_internal::HttpPipeline& pipeline,
          const Core::Url& url,
          const GetServiceStatisticsOptions& options,
          const Core::Context& context);
      struct ListServiceQueuesSegmentOptions final
      {
        Nullable<std::string> Prefix;
        Nullable<std::string> Marker;
        Nullable<int32_t> MaxResults;
        Nullable<Models::ListQueuesIncludeFlags> Include;
      };
      static Response<Models::_detail::ListQueuesResult> ListQueuesSegment(
          Core::Http::_internal::HttpPipeline& pipeline,
          const Core::Url& url,
          const ListServiceQueuesSegmentOptions& options,
          const Core::Context& context);
    };
    class QueueClient final {
    public:
      struct CreateQueueOptions final
      {
        std::map<std::string, std::string> Metadata;
      };
      static Response<Models::CreateQueueResult> Create(
          Core::Http::_internal::HttpPipeline& pipeline,
          const Core::Url& url,
          const CreateQueueOptions& options,
          const Core::Context& context);
      struct DeleteQueueOptions final
      {
      };
      static Response<Models::DeleteQueueResult> Delete(
          Core::Http::_internal::HttpPipeline& pipeline,
          const Core::Url& url,
          const DeleteQueueOptions& options,
          const Core::Context& context);
      struct GetQueuePropertiesOptions final
      {
      };
      static Response<Models::QueueProperties> GetProperties(
          Core::Http::_internal::HttpPipeline& pipeline,
          const Core::Url& url,
          const GetQueuePropertiesOptions& options,
          const Core::Context& context);
      struct SetQueueMetadataOptions final
      {
        std::map<std::string, std::string> Metadata;
      };
      static Response<Models::SetQueueMetadataResult> SetMetadata(
          Core::Http::_internal::HttpPipeline& pipeline,
          const Core::Url& url,
          const SetQueueMetadataOptions& options,
          const Core::Context& context);
      struct GetQueueAccessPolicyOptions final
      {
      };
      static Response<Models::QueueAccessPolicy> GetAccessPolicy(
          Core::Http::_internal::HttpPipeline& pipeline,
          const Core::Url& url,
          const GetQueueAccessPolicyOptions& options,
          const Core::Context& context);
      struct SetQueueAccessPolicyOptions final
      {
        std::vector<Models::SignedIdentifier> QueueAcl;
      };
      static Response<Models::SetQueueAccessPolicyResult> SetAccessPolicy(
          Core::Http::_internal::HttpPipeline& pipeline,
          const Core::Url& url,
          const SetQueueAccessPolicyOptions& options,
          const Core::Context& context);
      struct ReceiveQueueMessagesOptions final
      {
        Nullable<int64_t> NumberOfMessages;
        Nullable<int32_t> Visibilitytimeout;
      };
      static Response<Models::ReceivedMessages> ReceiveMessages(
          Core::Http::_internal::HttpPipeline& pipeline,
          const Core::Url& url,
          const ReceiveQueueMessagesOptions& options,
          const Core::Context& context);
      struct ClearQueueMessagesOptions final
      {
      };
      static Response<Models::ClearMessagesResult> ClearMessages(
          Core::Http::_internal::HttpPipeline& pipeline,
          const Core::Url& url,
          const ClearQueueMessagesOptions& options,
          const Core::Context& context);
      struct EnqueueQueueMessageOptions final
      {
        Models::_detail::QueueMessageInternal QueueMessage;
        Nullable<int32_t> Visibilitytimeout;
        Nullable<int32_t> MessageTimeToLive;
      };
      static Response<Models::EnqueueMessageResult> EnqueueMessage(
          Core::Http::_internal::HttpPipeline& pipeline,
          const Core::Url& url,
          const EnqueueQueueMessageOptions& options,
          const Core::Context& context);
      struct PeekQueueMessagesOptions final
      {
        Nullable<int64_t> NumberOfMessages;
      };
      static Response<Models::PeekedMessages> PeekMessages(
          Core::Http::_internal::HttpPipeline& pipeline,
          const Core::Url& url,
          const PeekQueueMessagesOptions& options,
          const Core::Context& context);
      struct UpdateQueueMessageOptions final
      {
        Models::_detail::QueueMessageInternal QueueMessage;
        std::string PopReceipt;
        int32_t Visibilitytimeout = int32_t();
      };
      static Response<Models::UpdateMessageResult> UpdateMessage(
          Core::Http::_internal::HttpPipeline& pipeline,
          const Core::Url& url,
          const UpdateQueueMessageOptions& options,
          const Core::Context& context);
      struct DeleteQueueMessageOptions final
      {
        std::string PopReceipt;
      };
      static Response<Models::DeleteMessageResult> DeleteMessage(
          Core::Http::_internal::HttpPipeline& pipeline,
          const Core::Url& url,
          const DeleteQueueMessageOptions& options,
          const Core::Context& context);
      struct UpdateQueueMessageVisibilityOptions final
      {
        std::string PopReceipt;
        int32_t Visibilitytimeout = int32_t();
      };
      static Response<Models::UpdateMessageResult> UpdateMessageVisibility(
          Core::Http::_internal::HttpPipeline& pipeline,
          const Core::Url& url,
          const UpdateQueueMessageVisibilityOptions& options,
          const Core::Context& context);
    };
  } // namespace _detail
}}} // namespace Azure::Storage::Queues