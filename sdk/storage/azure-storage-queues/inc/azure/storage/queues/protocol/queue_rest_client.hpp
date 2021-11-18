// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <string>
#include <type_traits>
#include <vector>

#include <azure/core/context.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/response.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/xml_wrapper.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_exception.hpp>

#include "azure/storage/queues/dll_import_export.hpp"

/* cspell:ignore numofmessages */

namespace Azure { namespace Storage { namespace Queues {
  namespace Models {

    /**
     * @brief Extensible enum used to identify the status of secondary storage endpoint.
     */
    class GeoReplicationStatus final {
    public:
      GeoReplicationStatus() = default;
      explicit GeoReplicationStatus(std::string value) : m_value(std::move(value)) {}
      bool operator==(const GeoReplicationStatus& other) const { return m_value == other.m_value; }
      bool operator!=(const GeoReplicationStatus& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }
      /**
       * The secondary location is active and operational.
       */
      AZ_STORAGE_QUEUES_DLLEXPORT const static GeoReplicationStatus Live;
      /**
       * Initial synchronization from the primary location to the secondary location is in progress.
       */
      AZ_STORAGE_QUEUES_DLLEXPORT const static GeoReplicationStatus Bootstrap;
      /**
       * The secondary location is temporarily unavailable.
       */
      AZ_STORAGE_QUEUES_DLLEXPORT const static GeoReplicationStatus Unavailable;

    private:
      std::string m_value;
    }; // extensible enum GeoReplicationStatus

    /**
     * @brief A peeked message object stored in the queue.
     */
    struct PeekedQueueMessage final
    {
      /**
       * The content of the message.
       */
      std::string MessageText;
      /**
       * A GUID value that identifies the message in the queue.
       */
      std::string MessageId;
      /**
       * The time the message was inserted into the queue.
       */
      Azure::DateTime InsertedOn;
      /**
       * The time that the message will expire and be automatically deleted from the queue.
       */
      Azure::DateTime ExpiresOn;
      /**
       * The number of times the message has been dequeued.
       */
      int64_t DequeueCount = 0;
    }; // struct PeekedQueueMessage

    /**
     * @brief A queue item from the result of
     * #Azure::Storage::Queues::QueueServiceClient::ListQueues.
     */
    struct QueueItem final
    {
      /**
       * Queue name.
       */
      std::string Name;
      /**
       * A set of name-value pairs associated with a queue as user-defined metadata.
       */
      Storage::Metadata Metadata;
    }; // struct QueueItem

    /**
     * @brief A message object stored in the queue.
     */
    struct QueueMessage final
    {
      /**
       * The content of the message.
       */
      std::string MessageText;
      /**
       * A GUID value that identifies the message in the queue.
       */
      std::string MessageId;
      /**
       * The time the message was inserted into the queue.
       */
      Azure::DateTime InsertedOn;
      /**
       * The time that the message will expire and be automatically deleted from the queue.
       */
      Azure::DateTime ExpiresOn;
      /**
       * An opaque string that is required to delete or update a message.
       */
      std::string PopReceipt;
      /**
       * The time that the message will again become visible in the queue.
       */
      Azure::DateTime NextVisibleOn;
      /**
       * The number of times the message has been dequeued.
       */
      int64_t DequeueCount = 0;
    }; // struct QueueMessage

    /**
     * @brief Determines how long the associated data should persist.
     */
    struct RetentionPolicy final
    {
      /**
       * Indicates whether this retention policy is enabled.
       */
      bool IsEnabled = false;
      /**
       * Indicates the number of days that metrics or logging or soft-deleted data should be
       * retained.
       */
      Azure::Nullable<int32_t> Days;
    }; // struct RetentionPolicy

    /**
     * @brief Describes how you reference an ACL in a queue.
     */
    struct SignedIdentifier final
    {
      /**
       * A unique ID for this signed identifier.
       */
      std::string Id;
      /**
       * Date and time since when this policy is active.
       */
      Azure::Nullable<Azure::DateTime> StartsOn;
      /**
       * Date and time the policy expires.
       */
      Azure::Nullable<Azure::DateTime> ExpiresOn;
      /**
       * The permissions for this ACL policy.
       */
      std::string Permissions;
    }; // struct SignedIdentifier

    /**
     * @brief Azure analytics logging settings.
     */
    struct AnalyticsLogging final
    {
      /**
       * The version of storage analytics to configure.
       */
      std::string Version;
      /**
       * Whether all delete requests should be logged.
       */
      bool Delete = false;
      /**
       * Whether all read requests should be logged.
       */
      bool Read = false;
      /**
       * Whether all write requests should be logged.
       */
      bool Write = false;
      /**
       * Determines how long the data should be persist.
       */
      Models::RetentionPolicy RetentionPolicy;
    }; // struct AnalyticsLogging

    /**
     * @brief Settings for a CORS rule.
     */
    struct CorsRule final
    {
      /**
       * A comma-separated list of origin domains that are allowed via CORS, or "*" if all domains
       * are allowed.
       */
      std::string AllowedOrigins;
      /**
       * A comma-separated list of HTTP methods that are allowed to be executed by the origin. For
       * Azure Storage, permitted methods are DELETE, GET, HEAD, MERGE, POST, OPTIONS or PUT.
       */
      std::string AllowedMethods;
      /**
       * A comma-separated list of headers allowed to be part of the cross-origin request.
       */
      std::string AllowedHeaders;
      /**
       * A comma-separated list of response headers to expose to CORS clients.
       */
      std::string ExposedHeaders;
      /**
       * The number of seconds that the client/browser should cache a preflight response.
       */
      int32_t MaxAgeInSeconds = 0;
    }; // struct CorsRule

    /**
     * @brief Geo-replication information for the secondary storage endpoint.
     */
    struct GeoReplication final
    {
      /**
       * Status of the secondary storage endpoint.
       */
      GeoReplicationStatus Status;
      /**
       * All primary writes preceding this value are guaranteed to be available for read operations
       * at the secondary. Primary writes after this point in time may or may not be available for
       * reads. This value may be null if replication status is bootstrap or unavailable.
       */
      Azure::Nullable<Azure::DateTime> LastSyncedOn;
    }; // struct GeoReplication

    /**
     * @brief Summary of request statistics grouped by API in hour or minute aggregates for queues.
     */
    struct Metrics final
    {
      /**
       * The version of storage analytics to configure.
       */
      std::string Version;
      /**
       * Indicates whether metrics are enabled for queue service.
       */
      bool IsEnabled = false;
      /**
       * Determines how long the metrics data should persist.
       */
      Models::RetentionPolicy RetentionPolicy;
      /**
       * Indicates whether metrics should generate summary statistics for called API operations.
       */
      Azure::Nullable<bool> IncludeApis;
    }; // struct Metrics

    /**
     * @brief Response type for #Azure::Storage::Queues::QueueClient::ClearMessages.
     */
    struct ClearMessagesResult final
    {
    }; // struct ClearMessagesResult

    /**
     * @brief Response type for #Azure::Storage::Queues::QueueClient::Create.
     */
    struct CreateQueueResult final
    {
      /**
       * Indicates if the queue was successfully created in this operation.
       */
      bool Created = false;
    }; // struct CreateQueueResult

    struct DeleteMessageResult final
    {
    }; // struct DeleteMessageResult

    /**
     * @brief Response type for #Azure::Storage::Queues::QueueClientClient::Delete.
     */
    struct DeleteQueueResult final
    {
      /**
       * Indicates if the queue was successfully deleted in this operation.
       */
      bool Deleted = false;
    }; // struct DeleteQueueResult

    /**
     * @brief Response type for #Azure::Storage::Queues::QueueClient::EnqueueMessage.
     */
    struct EnqueueMessageResult final
    {
      /**
       * A GUID value that identifies the message in the queue.
       */
      std::string MessageId;
      /**
       * The time the message was inserted into the queue.
       */
      Azure::DateTime InsertedOn;
      /**
       * The time that the message will expire and be automatically deleted from the queue.
       */
      Azure::DateTime ExpiresOn;
      /**
       * An opaque string that is required to delete or update a message.
       */
      std::string PopReceipt;
      /**
       * The time that the message will again become visible in the queue.
       */
      Azure::DateTime NextVisibleOn;
    }; // struct EnqueueMessageResult

    enum class ListQueuesIncludeFlags
    {
      /**
       * No extra data should be included.
       */
      None = 0,
      /**
       * Metadata should be included.
       */
      Metadata = 1,
    }; // bitwise enum ListQueuesIncludeFlags

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

    struct PeekedMessages final
    {
      /**
       * A vector of peeked messages.
       */
      std::vector<PeekedQueueMessage> Messages;
    }; // struct PeekedMessages

    /**
     * @brief Response type for #Azure::Storage::Queues::QueueClient::GetAccessPolicy.
     */
    struct QueueAccessPolicy final
    {
      /**
       * A collection of signed identifiers.
       */
      std::vector<SignedIdentifier> SignedIdentifiers;
    }; // struct QueueAccessPolicy

    /**
     * @brief Response type for #Azure::Storage::Queues::QueueClient::GetProperties.
     */
    struct QueueProperties final
    {
      /**
       * A set of name-value pairs associated with a queue as user-defined metadata.
       */
      Storage::Metadata Metadata;
      /**
       * The approximate number of messages in the queue. This number is not lower than the actual
       * number of messages in the queue, but could be higher.
       */
      int64_t ApproximateMessageCount = 0;
    }; // struct QueueProperties

    /**
     * @brief Properties of queue service.
     */
    struct QueueServiceProperties final
    {
      /**
       * Azure analytics logging settings.
       */
      AnalyticsLogging Logging;
      /**
       * Summary of request statistics grouped by API in hour aggregates for queues.
       */
      Metrics HourMetrics;
      /**
       * Summary of request statistics grouped by API in minute aggregates for queues.
       */
      Metrics MinuteMetrics;
      /**
       * CORS rules set.
       */
      std::vector<CorsRule> Cors;
    }; // struct QueueServiceProperties

    struct ReceivedMessages final
    {
      /**
       * A vector of received messages.
       */
      std::vector<QueueMessage> Messages;
    }; // struct ReceivedMessages

    /**
     * @brief Response type for #Azure::Storage::Queues::QueueServiceClient::GetStatistics.
     */
    struct ServiceStatistics final
    {
      /**
       * Geo-replication information for the secondary storage endpoint.
       */
      Models::GeoReplication GeoReplication;
    }; // struct ServiceStatistics

    /**
     * @brief Response type for #Azure::Storage::Queues::QueueClient::SetAccessPolicy.
     */
    struct SetQueueAccessPolicyResult final
    {
    }; // struct SetQueueAccessPolicyResult

    /**
     * @brief Response type for #Azure::Storage::Queues::QueueClient::SetMetadata.
     */
    struct SetQueueMetadataResult final
    {
    }; // struct SetQueueMetadataResult

    /**
     * @brief Response type for #Azure::Storage::Queues::QueueServiceClient::SetProperties.
     */
    struct SetServicePropertiesResult final
    {
    }; // struct SetServicePropertiesResult

    /**
     * @brief Response type for #Azure::Storage::Queues::QueueClient::UpdateMessage.
     */
    struct UpdateMessageResult final
    {
      /**
       * An opaque string that is required to delete or update a message.
       */
      std::string PopReceipt;
      /**
       * The time that the message will again become visible in the queue.
       */
      Azure::DateTime NextVisibleOn;
    }; // struct UpdateMessageResult

    namespace _detail {
      struct ListQueuesResult final
      {
        std::string ServiceEndpoint;
        std::string Prefix;
        Azure::Nullable<std::string> ContinuationToken;
        std::vector<QueueItem> Items;
      }; // struct ListQueuesResult
    } // namespace _detail

  } // namespace Models

  namespace _detail {
    constexpr static const char* ApiVersion = "2018-03-28";
  } // namespace _detail

  namespace _detail {

    using namespace Models;

    inline std::string ListQueuesIncludeFlagsToString(const ListQueuesIncludeFlags& val)
    {
      ListQueuesIncludeFlags value_list[] = {
          ListQueuesIncludeFlags::Metadata,
      };
      const char* string_list[] = {
          "metadata",
      };
      std::string ret;
      for (size_t i = 0; i < sizeof(value_list) / sizeof(ListQueuesIncludeFlags); ++i)
      {
        if ((val & value_list[i]) == value_list[i])
        {
          if (!ret.empty())
          {
            ret += ",";
          }
          ret += string_list[i];
        }
      }
      return ret;
    }

    class QueueRestClient final {
    public:
      class Service final {
      public:
        struct ListQueuesOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::string> Prefix;
          Azure::Nullable<std::string> ContinuationToken;
          Azure::Nullable<int32_t> MaxResults;
          ListQueuesIncludeFlags Include = ListQueuesIncludeFlags::None;
        }; // struct ListQueuesOptions

        static Azure::Response<Models::_detail::ListQueuesResult> ListQueues(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const ListQueuesOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.SetHeader("x-ms-version", "2018-03-28");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("comp", "list");
          if (options.Prefix.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "prefix", _internal::UrlEncodeQueryParameter(options.Prefix.Value()));
          }
          if (options.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "marker", _internal::UrlEncodeQueryParameter(options.ContinuationToken.Value()));
          }
          if (options.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "maxresults", std::to_string(options.MaxResults.Value()));
          }
          std::string list_queues_include_flags = ListQueuesIncludeFlagsToString(options.Include);
          if (!list_queues_include_flags.empty())
          {
            request.GetUrl().AppendQueryParameter(
                "include", _internal::UrlEncodeQueryParameter(list_queues_include_flags));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          Models::_detail::ListQueuesResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            _internal::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = ListQueuesResultInternalFromXml(reader);
          }
          return Azure::Response<Models::_detail::ListQueuesResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct GetServicePropertiesOptions final
        {
          Azure::Nullable<int32_t> Timeout;
        }; // struct GetServicePropertiesOptions

        static Azure::Response<QueueServiceProperties> GetProperties(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const GetServicePropertiesOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter("restype", "service");
          request.GetUrl().AppendQueryParameter("comp", "properties");
          request.SetHeader("x-ms-version", "2018-03-28");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          QueueServiceProperties response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            _internal::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = QueueServicePropertiesFromXml(reader);
          }
          return Azure::Response<QueueServiceProperties>(
              std::move(response), std::move(pHttpResponse));
        }

        struct SetServicePropertiesOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          QueueServiceProperties Properties;
        }; // struct SetServicePropertiesOptions

        static Azure::Response<SetServicePropertiesResult> SetProperties(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const SetServicePropertiesOptions& options,
            const Azure::Core::Context& context)
        {
          std::string xml_body;
          {
            _internal::XmlWriter writer;
            SetServicePropertiesOptionsToXml(writer, options);
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::End});
            xml_body = writer.GetDocument();
          }
          Azure::Core::IO::MemoryBodyStream xml_body_stream(
              reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
          auto request = Azure::Core::Http::Request(
              Azure::Core::Http::HttpMethod::Put, url, &xml_body_stream);
          request.SetHeader("Content-Length", std::to_string(xml_body_stream.Length()));
          request.GetUrl().AppendQueryParameter("restype", "service");
          request.GetUrl().AppendQueryParameter("comp", "properties");
          request.SetHeader("x-ms-version", "2018-03-28");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          SetServicePropertiesResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Accepted)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          return Azure::Response<SetServicePropertiesResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct GetServiceStatisticsOptions final
        {
          Azure::Nullable<int32_t> Timeout;
        }; // struct GetServiceStatisticsOptions

        static Azure::Response<ServiceStatistics> GetStatistics(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const GetServiceStatisticsOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter("restype", "service");
          request.GetUrl().AppendQueryParameter("comp", "stats");
          request.SetHeader("x-ms-version", "2018-03-28");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          ServiceStatistics response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            _internal::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = ServiceStatisticsFromXml(reader);
          }
          return Azure::Response<ServiceStatistics>(std::move(response), std::move(pHttpResponse));
        }

      private:
        static Models::_detail::ListQueuesResult ListQueuesResultInternalFromXml(
            _internal::XmlReader& reader)
        {
          Models::_detail::ListQueuesResult ret;
          enum class XmlTagName
          {
            k_EnumerationResults,
            k_Prefix,
            k_NextMarker,
            k_Queues,
            k_Queue,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (path.size() > 0)
              {
                path.pop_back();
              }
              else
              {
                break;
              }
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "EnumerationResults")
              {
                path.emplace_back(XmlTagName::k_EnumerationResults);
              }
              else if (node.Name == "Prefix")
              {
                path.emplace_back(XmlTagName::k_Prefix);
              }
              else if (node.Name == "NextMarker")
              {
                path.emplace_back(XmlTagName::k_NextMarker);
              }
              else if (node.Name == "Queues")
              {
                path.emplace_back(XmlTagName::k_Queues);
              }
              else if (node.Name == "Queue")
              {
                path.emplace_back(XmlTagName::k_Queue);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
              if (path.size() == 3 && path[0] == XmlTagName::k_EnumerationResults
                  && path[1] == XmlTagName::k_Queues && path[2] == XmlTagName::k_Queue)
              {
                ret.Items.emplace_back(QueueItemFromXml(reader));
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                  && path[1] == XmlTagName::k_Prefix)
              {
                ret.Prefix = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_EnumerationResults
                  && path[1] == XmlTagName::k_NextMarker)
              {
                ret.ContinuationToken = node.Value;
              }
            }
            else if (node.Type == _internal::XmlNodeType::Attribute)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_EnumerationResults
                  && node.Name == "ServiceEndpoint")
              {
                ret.ServiceEndpoint = node.Value;
              }
            }
          }
          return ret;
        }

        static QueueServiceProperties QueueServicePropertiesFromXml(_internal::XmlReader& reader)
        {
          QueueServiceProperties ret;
          enum class XmlTagName
          {
            k_StorageServiceProperties,
            k_Logging,
            k_HourMetrics,
            k_MinuteMetrics,
            k_Cors,
            k_CorsRule,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (path.size() > 0)
              {
                path.pop_back();
              }
              else
              {
                break;
              }
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "StorageServiceProperties")
              {
                path.emplace_back(XmlTagName::k_StorageServiceProperties);
              }
              else if (node.Name == "Logging")
              {
                path.emplace_back(XmlTagName::k_Logging);
              }
              else if (node.Name == "HourMetrics")
              {
                path.emplace_back(XmlTagName::k_HourMetrics);
              }
              else if (node.Name == "MinuteMetrics")
              {
                path.emplace_back(XmlTagName::k_MinuteMetrics);
              }
              else if (node.Name == "Cors")
              {
                path.emplace_back(XmlTagName::k_Cors);
              }
              else if (node.Name == "CorsRule")
              {
                path.emplace_back(XmlTagName::k_CorsRule);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
              if (path.size() == 2 && path[0] == XmlTagName::k_StorageServiceProperties
                  && path[1] == XmlTagName::k_Logging)
              {
                ret.Logging = AnalyticsLoggingFromXml(reader);
                path.pop_back();
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_StorageServiceProperties
                  && path[1] == XmlTagName::k_HourMetrics)
              {
                ret.HourMetrics = MetricsFromXml(reader);
                path.pop_back();
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_StorageServiceProperties
                  && path[1] == XmlTagName::k_MinuteMetrics)
              {
                ret.MinuteMetrics = MetricsFromXml(reader);
                path.pop_back();
              }
              else if (
                  path.size() == 3 && path[0] == XmlTagName::k_StorageServiceProperties
                  && path[1] == XmlTagName::k_Cors && path[2] == XmlTagName::k_CorsRule)
              {
                ret.Cors.emplace_back(CorsRuleFromXml(reader));
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
            }
          }
          return ret;
        }

        static ServiceStatistics ServiceStatisticsFromXml(_internal::XmlReader& reader)
        {
          ServiceStatistics ret;
          enum class XmlTagName
          {
            k_StorageServiceStats,
            k_GeoReplication,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (path.size() > 0)
              {
                path.pop_back();
              }
              else
              {
                break;
              }
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "StorageServiceStats")
              {
                path.emplace_back(XmlTagName::k_StorageServiceStats);
              }
              else if (node.Name == "GeoReplication")
              {
                path.emplace_back(XmlTagName::k_GeoReplication);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
              if (path.size() == 2 && path[0] == XmlTagName::k_StorageServiceStats
                  && path[1] == XmlTagName::k_GeoReplication)
              {
                ret.GeoReplication = GeoReplicationFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
            }
          }
          return ret;
        }

        static AnalyticsLogging AnalyticsLoggingFromXml(_internal::XmlReader& reader)
        {
          AnalyticsLogging ret;
          enum class XmlTagName
          {
            k_Version,
            k_Delete,
            k_Read,
            k_Write,
            k_RetentionPolicy,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (path.size() > 0)
              {
                path.pop_back();
              }
              else
              {
                break;
              }
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "Version")
              {
                path.emplace_back(XmlTagName::k_Version);
              }
              else if (node.Name == "Delete")
              {
                path.emplace_back(XmlTagName::k_Delete);
              }
              else if (node.Name == "Read")
              {
                path.emplace_back(XmlTagName::k_Read);
              }
              else if (node.Name == "Write")
              {
                path.emplace_back(XmlTagName::k_Write);
              }
              else if (node.Name == "RetentionPolicy")
              {
                path.emplace_back(XmlTagName::k_RetentionPolicy);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
              if (path.size() == 1 && path[0] == XmlTagName::k_RetentionPolicy)
              {
                ret.RetentionPolicy = RetentionPolicyFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_Version)
              {
                ret.Version = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_Delete)
              {
                ret.Delete = node.Value == "true";
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_Read)
              {
                ret.Read = node.Value == "true";
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_Write)
              {
                ret.Write = node.Value == "true";
              }
            }
          }
          return ret;
        }

        static CorsRule CorsRuleFromXml(_internal::XmlReader& reader)
        {
          CorsRule ret;
          enum class XmlTagName
          {
            k_AllowedOrigins,
            k_AllowedMethods,
            k_MaxAgeInSeconds,
            k_ExposedHeaders,
            k_AllowedHeaders,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (path.size() > 0)
              {
                path.pop_back();
              }
              else
              {
                break;
              }
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "AllowedOrigins")
              {
                path.emplace_back(XmlTagName::k_AllowedOrigins);
              }
              else if (node.Name == "AllowedMethods")
              {
                path.emplace_back(XmlTagName::k_AllowedMethods);
              }
              else if (node.Name == "MaxAgeInSeconds")
              {
                path.emplace_back(XmlTagName::k_MaxAgeInSeconds);
              }
              else if (node.Name == "ExposedHeaders")
              {
                path.emplace_back(XmlTagName::k_ExposedHeaders);
              }
              else if (node.Name == "AllowedHeaders")
              {
                path.emplace_back(XmlTagName::k_AllowedHeaders);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_AllowedOrigins)
              {
                ret.AllowedOrigins = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_AllowedMethods)
              {
                ret.AllowedMethods = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_MaxAgeInSeconds)
              {
                ret.MaxAgeInSeconds = std::stoi(node.Value);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_ExposedHeaders)
              {
                ret.ExposedHeaders = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_AllowedHeaders)
              {
                ret.AllowedHeaders = node.Value;
              }
            }
          }
          return ret;
        }

        static GeoReplication GeoReplicationFromXml(_internal::XmlReader& reader)
        {
          GeoReplication ret;
          enum class XmlTagName
          {
            k_Status,
            k_LastSyncTime,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (path.size() > 0)
              {
                path.pop_back();
              }
              else
              {
                break;
              }
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "Status")
              {
                path.emplace_back(XmlTagName::k_Status);
              }
              else if (node.Name == "LastSyncTime")
              {
                path.emplace_back(XmlTagName::k_LastSyncTime);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_Status)
              {
                ret.Status = GeoReplicationStatus(node.Value);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_LastSyncTime)
              {
                ret.LastSyncedOn
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
              }
            }
          }
          return ret;
        }

        static Metrics MetricsFromXml(_internal::XmlReader& reader)
        {
          Metrics ret;
          enum class XmlTagName
          {
            k_Version,
            k_Enabled,
            k_IncludeAPIs,
            k_RetentionPolicy,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (path.size() > 0)
              {
                path.pop_back();
              }
              else
              {
                break;
              }
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "Version")
              {
                path.emplace_back(XmlTagName::k_Version);
              }
              else if (node.Name == "Enabled")
              {
                path.emplace_back(XmlTagName::k_Enabled);
              }
              else if (node.Name == "IncludeAPIs")
              {
                path.emplace_back(XmlTagName::k_IncludeAPIs);
              }
              else if (node.Name == "RetentionPolicy")
              {
                path.emplace_back(XmlTagName::k_RetentionPolicy);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
              if (path.size() == 1 && path[0] == XmlTagName::k_RetentionPolicy)
              {
                ret.RetentionPolicy = RetentionPolicyFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_Version)
              {
                ret.Version = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_Enabled)
              {
                ret.IsEnabled = node.Value == "true";
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_IncludeAPIs)
              {
                ret.IncludeApis = node.Value == "true";
              }
            }
          }
          return ret;
        }

        static QueueItem QueueItemFromXml(_internal::XmlReader& reader)
        {
          QueueItem ret;
          enum class XmlTagName
          {
            k_Name,
            k_Metadata,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (path.size() > 0)
              {
                path.pop_back();
              }
              else
              {
                break;
              }
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "Name")
              {
                path.emplace_back(XmlTagName::k_Name);
              }
              else if (node.Name == "Metadata")
              {
                path.emplace_back(XmlTagName::k_Metadata);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
              if (path.size() == 1 && path[0] == XmlTagName::k_Metadata)
              {
                ret.Metadata = MetadataFromXml(reader);
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_Name)
              {
                ret.Name = node.Value;
              }
            }
          }
          return ret;
        }

        static Metadata MetadataFromXml(_internal::XmlReader& reader)
        {
          Metadata ret;
          int depth = 0;
          std::string key;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (depth++ == 0)
              {
                key = node.Name;
              }
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (depth-- == 0)
              {
                break;
              }
            }
            else if (depth == 1 && node.Type == _internal::XmlNodeType::Text)
            {
              ret.emplace(std::move(key), node.Value);
            }
          }
          return ret;
        }

        static RetentionPolicy RetentionPolicyFromXml(_internal::XmlReader& reader)
        {
          RetentionPolicy ret;
          enum class XmlTagName
          {
            k_Enabled,
            k_Days,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (path.size() > 0)
              {
                path.pop_back();
              }
              else
              {
                break;
              }
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "Enabled")
              {
                path.emplace_back(XmlTagName::k_Enabled);
              }
              else if (node.Name == "Days")
              {
                path.emplace_back(XmlTagName::k_Days);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_Enabled)
              {
                ret.IsEnabled = node.Value == "true";
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_Days)
              {
                ret.Days = std::stoi(node.Value);
              }
            }
          }
          return ret;
        }

        static void SetServicePropertiesOptionsToXml(
            _internal::XmlWriter& writer,
            const SetServicePropertiesOptions& options)
        {
          writer.Write(
              _internal::XmlNode{_internal::XmlNodeType::StartTag, "StorageServiceProperties"});
          QueueServicePropertiesToXml(writer, options.Properties);
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

        static void QueueServicePropertiesToXml(
            _internal::XmlWriter& writer,
            const QueueServiceProperties& options)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Logging"});
          AnalyticsLoggingToXml(writer, options.Logging);
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "HourMetrics"});
          MetricsToXml(writer, options.HourMetrics);
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "MinuteMetrics"});
          MetricsToXml(writer, options.MinuteMetrics);
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Cors"});
          for (const auto& i : options.Cors)
          {
            CorsRuleToXml(writer, i);
          }
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

        static void AnalyticsLoggingToXml(
            _internal::XmlWriter& writer,
            const AnalyticsLogging& options)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Version"});
          writer.Write(
              _internal::XmlNode{_internal::XmlNodeType::Text, std::string(), options.Version});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Delete"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, std::string(), options.Delete ? "true" : "false"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Read"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, std::string(), options.Read ? "true" : "false"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Write"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, std::string(), options.Write ? "true" : "false"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "RetentionPolicy"});
          RetentionPolicyToXml(writer, options.RetentionPolicy);
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

        static void CorsRuleToXml(_internal::XmlWriter& writer, const CorsRule& options)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "CorsRule"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "AllowedOrigins"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, std::string(), options.AllowedOrigins});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "AllowedMethods"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, std::string(), options.AllowedMethods});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "AllowedHeaders"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, std::string(), options.AllowedHeaders});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "ExposedHeaders"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, std::string(), options.ExposedHeaders});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "MaxAgeInSeconds"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text,
              std::string(),
              std::to_string(options.MaxAgeInSeconds)});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

        static void MetricsToXml(_internal::XmlWriter& writer, const Metrics& options)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Version"});
          writer.Write(
              _internal::XmlNode{_internal::XmlNodeType::Text, std::string(), options.Version});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Enabled"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, std::string(), options.IsEnabled ? "true" : "false"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          if (options.IncludeApis.HasValue())
          {
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "IncludeAPIs"});
            writer.Write(_internal::XmlNode{
                _internal::XmlNodeType::Text,
                std::string(),
                options.IncludeApis.Value() ? "true" : "false"});
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          }
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "RetentionPolicy"});
          RetentionPolicyToXml(writer, options.RetentionPolicy);
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

        static void RetentionPolicyToXml(
            _internal::XmlWriter& writer,
            const RetentionPolicy& options)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Enabled"});
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::Text, std::string(), options.IsEnabled ? "true" : "false"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          if (options.Days.HasValue())
          {
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Days"});
            writer.Write(_internal::XmlNode{
                _internal::XmlNodeType::Text, std::string(), std::to_string(options.Days.Value())});
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          }
        }

      }; // class Service

      class Queue final {
      public:
        struct CreateQueueOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          Storage::Metadata Metadata;
        }; // struct CreateQueueOptions

        static Azure::Response<CreateQueueResult> Create(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const CreateQueueOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2018-03-28");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          for (const auto& pair : options.Metadata)
          {
            request.SetHeader("x-ms-meta-" + pair.first, pair.second);
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          CreateQueueResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (!(http_status_code == Azure::Core::Http::HttpStatusCode::Created
                || http_status_code == Azure::Core::Http::HttpStatusCode::NoContent))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          if (http_status_code == Azure::Core::Http::HttpStatusCode::Created)
          {
            response.Created = true;
          }
          return Azure::Response<CreateQueueResult>(std::move(response), std::move(pHttpResponse));
        }

        struct DeleteQueueOptions final
        {
          Azure::Nullable<int32_t> Timeout;
        }; // struct DeleteQueueOptions

        static Azure::Response<DeleteQueueResult> Delete(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const DeleteQueueOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Delete, url);
          request.SetHeader("x-ms-version", "2018-03-28");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          DeleteQueueResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (!(http_status_code == Azure::Core::Http::HttpStatusCode::NoContent
                || http_status_code == Azure::Core::Http::HttpStatusCode::NotFound))
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          if (http_status_code == Azure::Core::Http::HttpStatusCode::NoContent)
          {
            response.Deleted = true;
          }
          return Azure::Response<DeleteQueueResult>(std::move(response), std::move(pHttpResponse));
        }

        struct SetQueueMetadataOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          Storage::Metadata Metadata;
        }; // struct SetQueueMetadataOptions

        static Azure::Response<SetQueueMetadataResult> SetMetadata(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const SetQueueMetadataOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.GetUrl().AppendQueryParameter("comp", "metadata");
          request.SetHeader("x-ms-version", "2018-03-28");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          for (const auto& pair : options.Metadata)
          {
            request.SetHeader("x-ms-meta-" + pair.first, pair.second);
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          SetQueueMetadataResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::NoContent)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          return Azure::Response<SetQueueMetadataResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct GetQueuePropertiesOptions final
        {
          Azure::Nullable<int32_t> Timeout;
        }; // struct GetQueuePropertiesOptions

        static Azure::Response<QueueProperties> GetProperties(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const GetQueuePropertiesOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, url);
          request.GetUrl().AppendQueryParameter("comp", "metadata");
          request.SetHeader("x-ms-version", "2018-03-28");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          QueueProperties response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          for (auto i = httpResponse.GetHeaders().lower_bound("x-ms-meta-");
               i != httpResponse.GetHeaders().end() && i->first.substr(0, 10) == "x-ms-meta-";
               ++i)
          {
            response.Metadata.emplace(i->first.substr(10), i->second);
          }
          response.ApproximateMessageCount
              = std::stoll(httpResponse.GetHeaders().at("x-ms-approximate-messages-count"));
          return Azure::Response<QueueProperties>(std::move(response), std::move(pHttpResponse));
        }

        struct GetQueueAccessPolicyOptions final
        {
          Azure::Nullable<int32_t> Timeout;
        }; // struct GetQueueAccessPolicyOptions

        static Azure::Response<QueueAccessPolicy> GetAccessPolicy(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const GetQueueAccessPolicyOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.SetHeader("x-ms-version", "2018-03-28");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("comp", "acl");
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          QueueAccessPolicy response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            _internal::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = QueueAccessPolicyFromXml(reader);
          }
          return Azure::Response<QueueAccessPolicy>(std::move(response), std::move(pHttpResponse));
        }

        struct SetQueueAccessPolicyOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          std::vector<SignedIdentifier> SignedIdentifiers;
        }; // struct SetQueueAccessPolicyOptions

        static Azure::Response<SetQueueAccessPolicyResult> SetAccessPolicy(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const SetQueueAccessPolicyOptions& options,
            const Azure::Core::Context& context)
        {
          std::string xml_body;
          {
            _internal::XmlWriter writer;
            SetQueueAccessPolicyOptionsToXml(writer, options);
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::End});
            xml_body = writer.GetDocument();
          }
          Azure::Core::IO::MemoryBodyStream xml_body_stream(
              reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
          auto request = Azure::Core::Http::Request(
              Azure::Core::Http::HttpMethod::Put, url, &xml_body_stream);
          request.SetHeader("Content-Length", std::to_string(xml_body_stream.Length()));
          request.SetHeader("x-ms-version", "2018-03-28");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("comp", "acl");
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          SetQueueAccessPolicyResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::NoContent)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          return Azure::Response<SetQueueAccessPolicyResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct EnqueueMessageOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          std::string MessageText;
          Azure::Nullable<std::chrono::seconds> VisibilityTimeout;
          Azure::Nullable<std::chrono::seconds> TimeToLive;
        }; // struct EnqueueMessageOptions

        static Azure::Response<EnqueueMessageResult> EnqueueMessage(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const EnqueueMessageOptions& options,
            const Azure::Core::Context& context)
        {
          std::string xml_body;
          {
            _internal::XmlWriter writer;
            EnqueueMessageOptionsToXml(writer, options);
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::End});
            xml_body = writer.GetDocument();
          }
          Azure::Core::IO::MemoryBodyStream xml_body_stream(
              reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
          auto request = Azure::Core::Http::Request(
              Azure::Core::Http::HttpMethod::Post, url, &xml_body_stream);
          request.SetHeader("Content-Length", std::to_string(xml_body_stream.Length()));
          request.SetHeader("x-ms-version", "2018-03-28");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          if (options.VisibilityTimeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "visibilitytimeout", std::to_string(options.VisibilityTimeout.Value().count()));
          }
          if (options.TimeToLive.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "messagettl", std::to_string(options.TimeToLive.Value().count()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          EnqueueMessageResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Created)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            _internal::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = EnqueueMessageResultFromXml(reader);
          }
          return Azure::Response<EnqueueMessageResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct ReceiveMessagesOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<int64_t> MaxMessages;
          Azure::Nullable<std::chrono::seconds> VisibilityTimeout;
        }; // struct ReceiveMessagesOptions

        static Azure::Response<ReceivedMessages> ReceiveMessages(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const ReceiveMessagesOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.SetHeader("x-ms-version", "2018-03-28");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          if (options.MaxMessages.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "numofmessages", std::to_string(options.MaxMessages.Value()));
          }
          if (options.VisibilityTimeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "visibilitytimeout", std::to_string(options.VisibilityTimeout.Value().count()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          ReceivedMessages response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            _internal::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = ReceivedMessagesFromXml(reader);
          }
          return Azure::Response<ReceivedMessages>(std::move(response), std::move(pHttpResponse));
        }

        struct PeekMessagesOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<int64_t> MaxMessages;
        }; // struct PeekMessagesOptions

        static Azure::Response<PeekedMessages> PeekMessages(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const PeekMessagesOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
          request.SetHeader("x-ms-version", "2018-03-28");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter("peekonly", "true");
          if (options.MaxMessages.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "numofmessages", std::to_string(options.MaxMessages.Value()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          PeekedMessages response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::Ok)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          {
            const auto& httpResponseBody = httpResponse.GetBody();
            _internal::XmlReader reader(
                reinterpret_cast<const char*>(httpResponseBody.data()), httpResponseBody.size());
            response = PeekedMessagesFromXml(reader);
          }
          return Azure::Response<PeekedMessages>(std::move(response), std::move(pHttpResponse));
        }

        struct DeleteMessageOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          std::string PopReceipt;
        }; // struct DeleteMessageOptions

        static Azure::Response<DeleteMessageResult> DeleteMessage(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const DeleteMessageOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Delete, url);
          request.SetHeader("x-ms-version", "2018-03-28");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter(
              "popreceipt", _internal::UrlEncodeQueryParameter(options.PopReceipt));
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          DeleteMessageResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::NoContent)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          return Azure::Response<DeleteMessageResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct ClearMessagesOptions final
        {
          Azure::Nullable<int32_t> Timeout;
        }; // struct ClearMessagesOptions

        static Azure::Response<ClearMessagesResult> ClearMessages(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const ClearMessagesOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Delete, url);
          request.SetHeader("x-ms-version", "2018-03-28");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          ClearMessagesResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::NoContent)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          return Azure::Response<ClearMessagesResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct UpdateMessageVisibilityOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          std::string PopReceipt;
          std::chrono::seconds VisibilityTimeout;
        }; // struct UpdateMessageVisibilityOptions

        static Azure::Response<UpdateMessageResult> UpdateMessageVisibility(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const UpdateMessageVisibilityOptions& options,
            const Azure::Core::Context& context)
        {
          auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader("Content-Length", "0");
          request.SetHeader("x-ms-version", "2018-03-28");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter(
              "popreceipt", _internal::UrlEncodeQueryParameter(options.PopReceipt));
          request.GetUrl().AppendQueryParameter(
              "visibilitytimeout", std::to_string(options.VisibilityTimeout.count()));
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          UpdateMessageResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::NoContent)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.PopReceipt = httpResponse.GetHeaders().at("x-ms-popreceipt");
          response.NextVisibleOn = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("x-ms-time-next-visible"),
              Azure::DateTime::DateFormat::Rfc1123);
          return Azure::Response<UpdateMessageResult>(
              std::move(response), std::move(pHttpResponse));
        }

        struct UpdateMessageOptions final
        {
          std::string MessageText;
          Azure::Nullable<int32_t> Timeout;
          std::string PopReceipt;
          std::chrono::seconds VisibilityTimeout;
        }; // struct UpdateMessageOptions

        static Azure::Response<UpdateMessageResult> UpdateMessage(
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            const Azure::Core::Url& url,
            const UpdateMessageOptions& options,
            const Azure::Core::Context& context)
        {
          std::string xml_body;
          {
            _internal::XmlWriter writer;
            UpdateMessageOptionsToXml(writer, options);
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::End});
            xml_body = writer.GetDocument();
          }
          Azure::Core::IO::MemoryBodyStream xml_body_stream(
              reinterpret_cast<const uint8_t*>(xml_body.data()), xml_body.length());
          auto request = Azure::Core::Http::Request(
              Azure::Core::Http::HttpMethod::Put, url, &xml_body_stream);
          request.SetHeader("Content-Length", std::to_string(xml_body_stream.Length()));
          request.SetHeader("x-ms-version", "2018-03-28");
          if (options.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                "timeout", std::to_string(options.Timeout.Value()));
          }
          request.GetUrl().AppendQueryParameter(
              "popreceipt", _internal::UrlEncodeQueryParameter(options.PopReceipt));
          request.GetUrl().AppendQueryParameter(
              "visibilitytimeout", std::to_string(options.VisibilityTimeout.count()));
          auto pHttpResponse = pipeline.Send(request, context);
          Azure::Core::Http::RawResponse& httpResponse = *pHttpResponse;
          UpdateMessageResult response;
          auto http_status_code = httpResponse.GetStatusCode();
          if (http_status_code != Azure::Core::Http::HttpStatusCode::NoContent)
          {
            throw StorageException::CreateFromResponse(std::move(pHttpResponse));
          }
          response.PopReceipt = httpResponse.GetHeaders().at("x-ms-popreceipt");
          response.NextVisibleOn = Azure::DateTime::Parse(
              httpResponse.GetHeaders().at("x-ms-time-next-visible"),
              Azure::DateTime::DateFormat::Rfc1123);
          return Azure::Response<UpdateMessageResult>(
              std::move(response), std::move(pHttpResponse));
        }

      private:
        static EnqueueMessageResult EnqueueMessageResultFromXml(_internal::XmlReader& reader)
        {
          EnqueueMessageResult ret;
          enum class XmlTagName
          {
            k_QueueMessagesList,
            k_QueueMessage,
            k_MessageId,
            k_InsertionTime,
            k_ExpirationTime,
            k_PopReceipt,
            k_TimeNextVisible,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (path.size() > 0)
              {
                path.pop_back();
              }
              else
              {
                break;
              }
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "QueueMessagesList")
              {
                path.emplace_back(XmlTagName::k_QueueMessagesList);
              }
              else if (node.Name == "QueueMessage")
              {
                path.emplace_back(XmlTagName::k_QueueMessage);
              }
              else if (node.Name == "MessageId")
              {
                path.emplace_back(XmlTagName::k_MessageId);
              }
              else if (node.Name == "InsertionTime")
              {
                path.emplace_back(XmlTagName::k_InsertionTime);
              }
              else if (node.Name == "ExpirationTime")
              {
                path.emplace_back(XmlTagName::k_ExpirationTime);
              }
              else if (node.Name == "PopReceipt")
              {
                path.emplace_back(XmlTagName::k_PopReceipt);
              }
              else if (node.Name == "TimeNextVisible")
              {
                path.emplace_back(XmlTagName::k_TimeNextVisible);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 3 && path[0] == XmlTagName::k_QueueMessagesList
                  && path[1] == XmlTagName::k_QueueMessage && path[2] == XmlTagName::k_MessageId)
              {
                ret.MessageId = node.Value;
              }
              else if (
                  path.size() == 3 && path[0] == XmlTagName::k_QueueMessagesList
                  && path[1] == XmlTagName::k_QueueMessage
                  && path[2] == XmlTagName::k_InsertionTime)
              {
                ret.InsertedOn
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
              }
              else if (
                  path.size() == 3 && path[0] == XmlTagName::k_QueueMessagesList
                  && path[1] == XmlTagName::k_QueueMessage
                  && path[2] == XmlTagName::k_ExpirationTime)
              {
                ret.ExpiresOn
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
              }
              else if (
                  path.size() == 3 && path[0] == XmlTagName::k_QueueMessagesList
                  && path[1] == XmlTagName::k_QueueMessage && path[2] == XmlTagName::k_PopReceipt)
              {
                ret.PopReceipt = node.Value;
              }
              else if (
                  path.size() == 3 && path[0] == XmlTagName::k_QueueMessagesList
                  && path[1] == XmlTagName::k_QueueMessage
                  && path[2] == XmlTagName::k_TimeNextVisible)
              {
                ret.NextVisibleOn
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
              }
            }
          }
          return ret;
        }

        static PeekedMessages PeekedMessagesFromXml(_internal::XmlReader& reader)
        {
          PeekedMessages ret;
          enum class XmlTagName
          {
            k_QueueMessagesList,
            k_QueueMessage,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (path.size() > 0)
              {
                path.pop_back();
              }
              else
              {
                break;
              }
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "QueueMessagesList")
              {
                path.emplace_back(XmlTagName::k_QueueMessagesList);
              }
              else if (node.Name == "QueueMessage")
              {
                path.emplace_back(XmlTagName::k_QueueMessage);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
              if (path.size() == 2 && path[0] == XmlTagName::k_QueueMessagesList
                  && path[1] == XmlTagName::k_QueueMessage)
              {
                ret.Messages.emplace_back(PeekedQueueMessageFromXml(reader));
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
            }
          }
          return ret;
        }

        static QueueAccessPolicy QueueAccessPolicyFromXml(_internal::XmlReader& reader)
        {
          QueueAccessPolicy ret;
          enum class XmlTagName
          {
            k_SignedIdentifiers,
            k_SignedIdentifier,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (path.size() > 0)
              {
                path.pop_back();
              }
              else
              {
                break;
              }
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "SignedIdentifiers")
              {
                path.emplace_back(XmlTagName::k_SignedIdentifiers);
              }
              else if (node.Name == "SignedIdentifier")
              {
                path.emplace_back(XmlTagName::k_SignedIdentifier);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
              if (path.size() == 2 && path[0] == XmlTagName::k_SignedIdentifiers
                  && path[1] == XmlTagName::k_SignedIdentifier)
              {
                ret.SignedIdentifiers.emplace_back(SignedIdentifierFromXml(reader));
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
            }
          }
          return ret;
        }

        static ReceivedMessages ReceivedMessagesFromXml(_internal::XmlReader& reader)
        {
          ReceivedMessages ret;
          enum class XmlTagName
          {
            k_QueueMessagesList,
            k_QueueMessage,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (path.size() > 0)
              {
                path.pop_back();
              }
              else
              {
                break;
              }
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "QueueMessagesList")
              {
                path.emplace_back(XmlTagName::k_QueueMessagesList);
              }
              else if (node.Name == "QueueMessage")
              {
                path.emplace_back(XmlTagName::k_QueueMessage);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
              if (path.size() == 2 && path[0] == XmlTagName::k_QueueMessagesList
                  && path[1] == XmlTagName::k_QueueMessage)
              {
                ret.Messages.emplace_back(QueueMessageFromXml(reader));
                path.pop_back();
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
            }
          }
          return ret;
        }

        static PeekedQueueMessage PeekedQueueMessageFromXml(_internal::XmlReader& reader)
        {
          PeekedQueueMessage ret;
          enum class XmlTagName
          {
            k_MessageText,
            k_MessageId,
            k_InsertionTime,
            k_ExpirationTime,
            k_DequeueCount,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (path.size() > 0)
              {
                path.pop_back();
              }
              else
              {
                break;
              }
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "MessageText")
              {
                path.emplace_back(XmlTagName::k_MessageText);
              }
              else if (node.Name == "MessageId")
              {
                path.emplace_back(XmlTagName::k_MessageId);
              }
              else if (node.Name == "InsertionTime")
              {
                path.emplace_back(XmlTagName::k_InsertionTime);
              }
              else if (node.Name == "ExpirationTime")
              {
                path.emplace_back(XmlTagName::k_ExpirationTime);
              }
              else if (node.Name == "DequeueCount")
              {
                path.emplace_back(XmlTagName::k_DequeueCount);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_MessageText)
              {
                ret.MessageText = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_MessageId)
              {
                ret.MessageId = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_InsertionTime)
              {
                ret.InsertedOn
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_ExpirationTime)
              {
                ret.ExpiresOn
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_DequeueCount)
              {
                ret.DequeueCount = std::stoll(node.Value);
              }
            }
          }
          return ret;
        }

        static QueueMessage QueueMessageFromXml(_internal::XmlReader& reader)
        {
          QueueMessage ret;
          enum class XmlTagName
          {
            k_MessageText,
            k_MessageId,
            k_InsertionTime,
            k_ExpirationTime,
            k_PopReceipt,
            k_TimeNextVisible,
            k_DequeueCount,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (path.size() > 0)
              {
                path.pop_back();
              }
              else
              {
                break;
              }
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "MessageText")
              {
                path.emplace_back(XmlTagName::k_MessageText);
              }
              else if (node.Name == "MessageId")
              {
                path.emplace_back(XmlTagName::k_MessageId);
              }
              else if (node.Name == "InsertionTime")
              {
                path.emplace_back(XmlTagName::k_InsertionTime);
              }
              else if (node.Name == "ExpirationTime")
              {
                path.emplace_back(XmlTagName::k_ExpirationTime);
              }
              else if (node.Name == "PopReceipt")
              {
                path.emplace_back(XmlTagName::k_PopReceipt);
              }
              else if (node.Name == "TimeNextVisible")
              {
                path.emplace_back(XmlTagName::k_TimeNextVisible);
              }
              else if (node.Name == "DequeueCount")
              {
                path.emplace_back(XmlTagName::k_DequeueCount);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_MessageText)
              {
                ret.MessageText = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_MessageId)
              {
                ret.MessageId = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_InsertionTime)
              {
                ret.InsertedOn
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_ExpirationTime)
              {
                ret.ExpiresOn
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_PopReceipt)
              {
                ret.PopReceipt = node.Value;
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_TimeNextVisible)
              {
                ret.NextVisibleOn
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
              }
              else if (path.size() == 1 && path[0] == XmlTagName::k_DequeueCount)
              {
                ret.DequeueCount = std::stoll(node.Value);
              }
            }
          }
          return ret;
        }

        static SignedIdentifier SignedIdentifierFromXml(_internal::XmlReader& reader)
        {
          SignedIdentifier ret;
          enum class XmlTagName
          {
            k_Id,
            k_AccessPolicy,
            k_Start,
            k_Expiry,
            k_Permission,
            k_Unknown,
          };
          std::vector<XmlTagName> path;
          while (true)
          {
            auto node = reader.Read();
            if (node.Type == _internal::XmlNodeType::End)
            {
              break;
            }
            else if (node.Type == _internal::XmlNodeType::EndTag)
            {
              if (path.size() > 0)
              {
                path.pop_back();
              }
              else
              {
                break;
              }
            }
            else if (node.Type == _internal::XmlNodeType::StartTag)
            {
              if (node.Name == "Id")
              {
                path.emplace_back(XmlTagName::k_Id);
              }
              else if (node.Name == "AccessPolicy")
              {
                path.emplace_back(XmlTagName::k_AccessPolicy);
              }
              else if (node.Name == "Start")
              {
                path.emplace_back(XmlTagName::k_Start);
              }
              else if (node.Name == "Expiry")
              {
                path.emplace_back(XmlTagName::k_Expiry);
              }
              else if (node.Name == "Permission")
              {
                path.emplace_back(XmlTagName::k_Permission);
              }
              else
              {
                path.emplace_back(XmlTagName::k_Unknown);
              }
            }
            else if (node.Type == _internal::XmlNodeType::Text)
            {
              if (path.size() == 1 && path[0] == XmlTagName::k_Id)
              {
                ret.Id = node.Value;
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_AccessPolicy
                  && path[1] == XmlTagName::k_Start)
              {
                ret.StartsOn
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc3339);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_AccessPolicy
                  && path[1] == XmlTagName::k_Expiry)
              {
                ret.ExpiresOn
                    = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc3339);
              }
              else if (
                  path.size() == 2 && path[0] == XmlTagName::k_AccessPolicy
                  && path[1] == XmlTagName::k_Permission)
              {
                ret.Permissions = node.Value;
              }
            }
          }
          return ret;
        }

        static void EnqueueMessageOptionsToXml(
            _internal::XmlWriter& writer,
            const EnqueueMessageOptions& options)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "QueueMessage"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "MessageText"});
          writer.Write(
              _internal::XmlNode{_internal::XmlNodeType::Text, std::string(), options.MessageText});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

        static void SetQueueAccessPolicyOptionsToXml(
            _internal::XmlWriter& writer,
            const SetQueueAccessPolicyOptions& options)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "SignedIdentifiers"});
          for (const auto& i : options.SignedIdentifiers)
          {
            SignedIdentifierToXml(writer, i);
          }
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

        static void UpdateMessageOptionsToXml(
            _internal::XmlWriter& writer,
            const UpdateMessageOptions& options)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "QueueMessage"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "MessageText"});
          writer.Write(
              _internal::XmlNode{_internal::XmlNodeType::Text, std::string(), options.MessageText});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

        static void SignedIdentifierToXml(
            _internal::XmlWriter& writer,
            const SignedIdentifier& options)
        {
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "SignedIdentifier"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Id"});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::Text, std::string(), options.Id});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "AccessPolicy"});
          if (options.StartsOn.HasValue())
          {
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Start"});
            writer.Write(_internal::XmlNode{
                _internal::XmlNodeType::Text,
                std::string(),
                options.StartsOn.Value().ToString(
                    Azure::DateTime::DateFormat::Rfc3339,
                    Azure::DateTime::TimeFractionFormat::AllDigits)});
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          }
          if (options.ExpiresOn.HasValue())
          {
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Expiry"});
            writer.Write(_internal::XmlNode{
                _internal::XmlNodeType::Text,
                std::string(),
                options.ExpiresOn.Value().ToString(
                    Azure::DateTime::DateFormat::Rfc3339,
                    Azure::DateTime::TimeFractionFormat::AllDigits)});
            writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          }
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Permission"});
          writer.Write(
              _internal::XmlNode{_internal::XmlNodeType::Text, std::string(), options.Permissions});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }

      }; // class Queue

    }; // class QueueRestClient

  } // namespace _detail

}}} // namespace Azure::Storage::Queues
