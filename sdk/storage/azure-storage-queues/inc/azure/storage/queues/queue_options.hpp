// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include <azure/core/internal/client_options.hpp>

#include "azure/storage/queues/protocol/queue_rest_client.hpp"

namespace Azure { namespace Storage { namespace Queues {

  enum class QueueMessageEncoding
  {
    None,
    Base64,
  };

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

    /**
     * This determines how queue message body is represented in HTTP requests and responses.
     */
    QueueMessageEncoding MessageEncoding = QueueMessageEncoding::None;
  };

  struct ListQueuesOptions final
  {
    Azure::Nullable<std::string> Prefix;

    Azure::Nullable<std::string> ContinuationToken;

    Azure::Nullable<int32_t> PageSizeHint;

    Models::ListQueuesIncludeFlags Include = Models::ListQueuesIncludeFlags::None;
  };

  struct SetServicePropertiesOptions final
  {
  };

  struct GetServicePropertiesOptions final
  {
  };

  struct GetQueueServiceStatisticsOptions final
  {
  };

  struct CreateQueueOptions final
  {
    /**
     * @brief Name-value pairs to associate with the container as metadata.
     */
    Storage::Metadata Metadata;
  };

  struct DeleteQueueOptions final
  {
  };

  struct GetQueuePropertiesOptions final
  {
  };

  struct SetQueueMetadataOptions final
  {
  };

  struct GetQueueAccessPolicyOptions final
  {
  };

  struct SetQueueAccessPolicyOptions final
  {
  };

  struct SendMessageOptions final
  {
    Azure::Nullable<int32_t> VisibilityTimeout;
    Azure::Nullable<int32_t> TimeToLive;
  };

  struct ReceiveMessageOptions final
  {
    Azure::Nullable<int32_t> VisibilityTimeout;
  };

  struct ReceiveMessagesOptions final
  {
    Azure::Nullable<int64_t> MaxMessages;
    Azure::Nullable<int32_t> VisibilityTimeout;
  };

  struct PeekMessageOptions final
  {
  };

  struct PeekMessagesOptions final
  {
    Azure::Nullable<int64_t> MaxMessages;
  };

  struct UpdateMessageOptions final
  {
    Azure::Nullable<std::string> messageText;
  };

  struct DeleteMessageOptions final
  {
  };

  struct ClearMessagesOptions final
  {
  };

}}} // namespace Azure::Storage::Queues
