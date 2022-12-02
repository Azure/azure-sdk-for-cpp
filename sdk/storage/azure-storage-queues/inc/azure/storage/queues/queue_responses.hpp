// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
/**
 * @file
 * @brief Defines Queue operation responses.
 *
 */

#pragma once

#include <memory>
#include <string>

#include <azure/core/paged_response.hpp>

#include "azure/storage/queues/queue_options.hpp"
#include "azure/storage/queues/rest_client.hpp"

namespace Azure { namespace Storage { namespace Queues {

  class QueueServiceClient;

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