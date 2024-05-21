// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
/**
 * @file
 * @brief Defines Queue client options and operation options.
 *
 */

#pragma once

#include "azure/storage/queues/rest_client.hpp"

#include <azure/core/internal/client_options.hpp>
#include <azure/core/internal/extendable_enumeration.hpp>
#include <azure/storage/common/storage_common.hpp>

#include <chrono>
#include <string>

namespace Azure { namespace Storage { namespace Queues {

  /**
   * @brief Audiences available for queue service
   *
   */
  class QueueAudience final : public Azure::Core::_internal::ExtendableEnumeration<QueueAudience> {
  public:
    /**
     * @brief Construct a new QueueAudience object
     *
     * @param queueAudience The Azure Active Directory audience to use when forming authorization
     * scopes. For the Language service, this value corresponds to a URL that identifies the Azure
     * cloud where the resource is located. For more information: See
     * https://learn.microsoft.com/en-us/azure/storage/blobs/authorize-access-azure-active-directory
     */
    explicit QueueAudience(std::string queueAudience)
        : ExtendableEnumeration(std::move(queueAudience))
    {
    }

    /**
     * @brief The service endpoint for a given storage account. Use this method to acquire a token
     * for authorizing requests to that specific Azure Storage account and service only.
     *
     * @param storageAccountName he storage account name used to populate the service endpoint.
     * @return The service endpoint for a given storage account.
     */
    static QueueAudience CreateQueueServiceAccountAudience(const std::string& storageAccountName)
    {
      return QueueAudience("https://" + storageAccountName + ".queue.core.windows.net/");
    }

    /**
     * @brief Default Audience. Use to acquire a token for authorizing requests to any Azure
     * Storage account.
     */
    AZ_STORAGE_QUEUES_DLLEXPORT const static QueueAudience DefaultAudience;
  };

  /**
   * @brief API version for Storage Queue service.
   */
  class ServiceVersion final {
  public:
    /**
     * @brief Construct a new Service Version object
     *
     * @param version The string version for Storage Queue Service.
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
     * @brief API version 2018-03-28.
     *
     */
    AZ_STORAGE_QUEUES_DLLEXPORT const static ServiceVersion V2018_03_28;

    /**
     * @brief API version 2019-12-12.
     *
     */
    AZ_STORAGE_QUEUES_DLLEXPORT const static ServiceVersion V2019_12_12;

    /**
     * @brief API version 2024-08-04.
     *
     */
    AZ_STORAGE_QUEUES_DLLEXPORT const static ServiceVersion V2024_08_04;

  private:
    std::string m_version;
  };

  /**
   * @brief Optional parameters for constructing a new QueueClient.
   */
  struct QueueClientOptions final : Azure::Core::_internal::ClientOptions
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
     * #Azure::Storage::Queues::QueueAudience::DefaultAudience will be assumed if
     * Audience is not set.
     */
    Azure::Nullable<QueueAudience> Audience;
  };

  /**
   * Optional parameters for #Azure::Storage::Queues::QueueServiceClient::ListQueues.
   */
  struct ListQueuesOptions final
  {
    /**
     * @brief Specifies a string that filters the results to return only queues whose name begins
     * with the specified prefix.
     */
    Azure::Nullable<std::string> Prefix;

    /**
     * @brief A string value that identifies the portion of the list of queues to be returned with
     * the next listing operation. The operation returns a non-empty continuation token if the
     * listing operation did not return all queues remaining to be listed with the current segment.
     * The ContinuationToken value can be used as the value for the ContinuationToken parameter in a
     * subsequent call to request the next segment of list items.
     */
    Azure::Nullable<std::string> ContinuationToken;

    /**
     * @brief Specifies the maximum number of queues to return.
     */
    Azure::Nullable<int32_t> PageSizeHint;

    /**
     * @brief Specifies that the queue's metadata be returned.
     */
    Models::ListQueuesIncludeFlags Include = Models::ListQueuesIncludeFlags::None;
  };

  /**
   * Optional parameters for #Azure::Storage::Queues::QueueServiceClient::SetProperties.
   */
  struct SetServicePropertiesOptions final
  {
  };

  /**
   * Optional parameters for #Azure::Storage::Queues::QueueServiceClient::GetProperties.
   */
  struct GetServicePropertiesOptions final
  {
  };

  /**
   * Optional parameters for #Azure::Storage::Queues::QueueServiceClient::GetStatistics.
   */
  struct GetQueueServiceStatisticsOptions final
  {
  };

  /**
   * Optional parameters for #Azure::Storage::Queues::QueueClient::Create.
   */
  struct CreateQueueOptions final
  {
    /**
     * @brief Name-value pairs to associate with the container as metadata.
     */
    Storage::Metadata Metadata;
  };

  /**
   * Optional parameters for #Azure::Storage::Queues::QueueClient::Delete.
   */
  struct DeleteQueueOptions final
  {
  };

  /**
   * Optional parameters for #Azure::Storage::Queues::QueueClient::GetProperties.
   */
  struct GetQueuePropertiesOptions final
  {
  };

  /**
   * Optional parameters for #Azure::Storage::Queues::QueueClient::SetMetadata.
   */
  struct SetQueueMetadataOptions final
  {
  };

  /**
   * Optional parameters for #Azure::Storage::Queues::QueueClient::GetAccessPolicy.
   */
  struct GetQueueAccessPolicyOptions final
  {
  };

  /**
   * Optional parameters for #Azure::Storage::Queues::QueueClient::SetAccessPolicy.
   */
  struct SetQueueAccessPolicyOptions final
  {
  };

  /**
   * Optional parameters for #Azure::Storage::Queues::QueueClient::EnqueueMessage.
   */
  struct EnqueueMessageOptions final
  {
    /**
     * Specifies how long the message should be invisible to dequeue and peek operations.
     */
    Azure::Nullable<std::chrono::seconds> VisibilityTimeout;

    /**
     * Specifies the time-to-live interval for the message. The maximum time-to-live can be any
     * positive number, as well as MessageNeverExpires indicating that the message does not expire
     */
    Azure::Nullable<std::chrono::seconds> TimeToLive;

    /**
     * @brief A TTL value representing the queue message does not expire.
     */
    AZ_STORAGE_QUEUES_DLLEXPORT const static std::chrono::seconds MessageNeverExpires;
  };

  /**
   * Optional parameters for #Azure::Storage::Queues::QueueClient::ReceiveMessages.
   */
  struct ReceiveMessagesOptions final
  {
    /**
     * Specifies the number of messages to retrieve from the queue.
     */
    Azure::Nullable<int64_t> MaxMessages;
    /**
     * After the messages have been retrieved, they are not visible to other clients for the time
     * interval specified by this parameter.
     */
    Azure::Nullable<std::chrono::seconds> VisibilityTimeout;
  };

  /**
   * Optional parameters for #Azure::Storage::Queues::QueueClient::PeekMessages.
   */
  struct PeekMessagesOptions final
  {
    /**
     * Specifies the number of messages to peek from the queue.
     */
    Azure::Nullable<int64_t> MaxMessages;
  };

  /**
   * Optional parameters for #Azure::Storage::Queues::QueueClient::UpdateMessage.
   */
  struct UpdateMessageOptions final
  {
    /**
     * Optionally update the queue message.
     */
    Azure::Nullable<std::string> MessageText;
  };

  /**
   * Optional parameters for #Azure::Storage::Queues::QueueClient::DeleteMessage.
   */
  struct DeleteMessageOptions final
  {
  };

  /**
   * Optional parameters for #Azure::Storage::Queues::QueueClient::ClearMessages.
   */
  struct ClearMessagesOptions final
  {
  };

}}} // namespace Azure::Storage::Queues
