// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include <azure/core/internal/client_options.hpp>

#include "azure/storage/queues/protocol/queue_rest_client.hpp"

namespace Azure { namespace Storage { namespace Queues {

  struct QueueClientOptions final : Azure::Core::_internal::ClientOptions
  {
    /**
     * SecondaryHostForRetryReads specifies whether the retry policy should retry a read
     * operation against another host. If SecondaryHostForRetryReads is "" (the default) then
     * operations are not retried against another host. NOTE: Before setting this field, make sure
     * you understand the issues around reading stale & potentially-inconsistent data at this
     * webpage: https://docs.microsoft.com/en-us/azure/storage/common/geo-redundant-design.
     */
    std::string SecondaryHostForRetryReads;

    /**
     * API version used by this client.
     */
    std::string ApiVersion = _detail::ApiVersion;
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
   * Optional parameters for #Azure::Storage::Queues::QueueClient::SendMessages.
   */
  struct SendMessageOptions final
  {
    /**
     * Specifies how long the message should be invisible to dequeue and peek operations.
     */
    Azure::Nullable<int32_t> VisibilityTimeout;

    /**
     * Specifies the time-to-live interval for the message. The maximum time-to-live can be any
     * positive number, as well as -1 indicating that the message does not expire
     */
    Azure::Nullable<int32_t> TimeToLive;
  };

  /**
   * Optional parameters for #Azure::Storage::Queues::QueueClient::ReceiveMessage.
   */
  struct ReceiveMessageOptions final
  {
    /**
     * After the message has been retrieved, it is not visible to other clients for the time
     * interval specified by this parameter.
     */
    Azure::Nullable<int32_t> VisibilityTimeout;
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
    Azure::Nullable<int32_t> VisibilityTimeout;
  };

  /**
   * Optional parameters for #Azure::Storage::Queues::QueueClient::PeekMessage.
   */
  struct PeekMessageOptions final
  {
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
