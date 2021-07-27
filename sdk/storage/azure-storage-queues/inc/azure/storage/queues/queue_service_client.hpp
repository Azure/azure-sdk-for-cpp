// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

#include <azure/core/credentials/credentials.hpp>
#include <azure/storage/common/storage_credential.hpp>

#include "azure/storage/queues/queue_client.hpp"
#include "azure/storage/queues/queue_options.hpp"
#include "azure/storage/queues/queue_responses.hpp"

namespace Azure { namespace Storage { namespace Queues {

  class QueueServiceClient final {
  public:
    static QueueServiceClient CreateFromConnectionString(
        const std::string& connectionString,
        const QueueClientOptions& options = QueueClientOptions());

    explicit QueueServiceClient(
        const std::string& serviceUrl,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        const QueueClientOptions& options = QueueClientOptions());

    explicit QueueServiceClient(
        const std::string& serviceUrl,
        std::shared_ptr<Core::Credentials::TokenCredential> credential,
        const QueueClientOptions& options = QueueClientOptions());

    explicit QueueServiceClient(
        const std::string& serviceUrl,
        const QueueClientOptions& options = QueueClientOptions());

    QueueClient GetQueueClient(const std::string& queueName) const;

    std::string GetUrl() const { return m_serviceUrl.GetAbsoluteUrl(); }

    ListQueuesPagedResponse ListQueues(
        const ListQueuesOptions& options = ListQueuesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    Azure::Response<Models::SetServicePropertiesResult> SetProperties(
        Models::QueueServiceProperties properties,
        const SetServicePropertiesOptions& options = SetServicePropertiesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    Azure::Response<Models::QueueServiceProperties> GetProperties(
        const GetServicePropertiesOptions& options = GetServicePropertiesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    Azure::Response<Models::ServiceStatistics> GetStatistics(
        const GetQueueServiceStatisticsOptions& options = GetQueueServiceStatisticsOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    Azure::Response<QueueClient> CreateQueue(
        const std::string& queueName,
        const CreateQueueOptions& options = CreateQueueOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    Azure::Response<Models::DeleteQueueResult> DeleteQueue(
        const std::string& queueName,
        const DeleteQueueOptions& options = DeleteQueueOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

  private:
    Azure::Core::Url m_serviceUrl;
    std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;
  };
}}} // namespace Azure::Storage::Queues
