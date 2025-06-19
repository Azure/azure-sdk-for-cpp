// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
/**
 * @file
 * @brief Defines Queue operation responses.
 *
 */

#pragma once

#include "azure/storage/queues/queue_options.hpp"
#include "azure/storage/queues/rest_client.hpp"

#include <azure/core/paged_response.hpp>

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace Queues {

  class QueueServiceClient;

  namespace Models {
    /**
     * @brief Response type for #Azure::Storage::Queues::QueueClient::GetProperties.
     */
    struct QueueProperties final
    {
      /**
       * A set of name-value pairs associated with this queue.
       */
      Core::CaseInsensitiveMap Metadata;
      /**
       * The approximate number of messages in the queue. This number is not lower than the actual
       * number of messages in the queue, but could be higher.
       *
       * This field is deprecated. The value is -1 if the value exceeds
       * INT32_MAX.Use ApproximateMessageCountLong instead.
       */
      std::int32_t ApproximateMessageCount = std::int32_t();
      /**
       * The approximate number of messages in the queue. This number is not lower than the actual
       * number of messages in the queue, but could be higher.
       */
      std::int64_t ApproximateMessageCountLong = std::int64_t();
    };
  } // namespace Models

  /**
   * @brief Response type for #Azure::Storage::Queues::QueueServiceClient::ListQueues.
   */
  class ListQueuesPagedResponse final : public Azure::Core::PagedResponse<ListQueuesPagedResponse> {
  public:
    /**
     * Service endpoint.
     */
    std::string ServiceEndpoint;

    /**
     * Container name prefix that's used to filter the result.
     */
    std::string Prefix;

    /**
     * Queue items.
     */
    std::vector<Models::QueueItem> Queues;

  private:
    void OnNextPage(const Azure::Core::Context& context);

    std::shared_ptr<QueueServiceClient> m_queueServiceClient;
    ListQueuesOptions m_operationOptions;

    friend class QueueServiceClient;
    friend class Azure::Core::PagedResponse<ListQueuesPagedResponse>;
  };

}}} // namespace Azure::Storage::Queues
