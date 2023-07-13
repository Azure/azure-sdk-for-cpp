// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/queues/queue_responses.hpp"

#include "azure/storage/queues/queue_service_client.hpp"

namespace Azure { namespace Storage { namespace Queues {

  void ListQueuesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    m_operationOptions.ContinuationToken = NextPageToken;
    *this = m_queueServiceClient->ListQueues(m_operationOptions, context);
  }

}}} // namespace Azure::Storage::Queues
