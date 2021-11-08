// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
/**
 * @file
 * @brief Defines Queue service client.
 *
 */

#pragma once

#include <memory>
#include <string>

#include <azure/core/credentials/credentials.hpp>
#include <azure/storage/common/storage_credential.hpp>

#include "azure/storage/queues/queue_client.hpp"
#include "azure/storage/queues/queue_options.hpp"
#include "azure/storage/queues/queue_responses.hpp"

namespace Azure { namespace Storage { namespace Queues {

  /**
   * The QueueServiceClient allows you to manipulate Azure Storage service resources and queues. The
   * storage account provides the top-level namespace for the Queue service.
   */
  class QueueServiceClient final {
  public:
    /**
     * @brief Initializes a new instance of QueueServiceClient.
     *
     * @param connectionString A connection string includes the authentication information required
     * for your application to access data in an Azure Storage account at runtime.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     * @return A new QueueServiceClient instance.
     */
    static QueueServiceClient CreateFromConnectionString(
        const std::string& connectionString,
        const QueueClientOptions& options = QueueClientOptions());

    /**
     * @brief Initializes a new instance of QueueServiceClient.
     *
     * @param serviceUrl A URL referencing the queue that includes the name of the account.
     * @param credential The shared key credential used to sign requests.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit QueueServiceClient(
        const std::string& serviceUrl,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        const QueueClientOptions& options = QueueClientOptions());

    /**
     * @brief Initializes a new instance of QueueServiceClient.
     *
     * @param serviceUrl A URL referencing the queue that includes the name of the account.
     * @param credential The token credential used to sign requests.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit QueueServiceClient(
        const std::string& serviceUrl,
        std::shared_ptr<Core::Credentials::TokenCredential> credential,
        const QueueClientOptions& options = QueueClientOptions());

    /**
     * @brief Initializes a new instance of QueueServiceClient.
     *
     * @param serviceUrl A URL referencing the queue that includes the name of the account.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit QueueServiceClient(
        const std::string& serviceUrl,
        const QueueClientOptions& options = QueueClientOptions());

    /**
     * @brief Creates a new QueueClient object with the same URL as this QueueServiceClient.
     * The new QueueClient uses the same request policy pipeline as this QueueServiceClient.
     *
     * @param queueName The name of the queue to reference.
     * @return A new QueueClient instance.
     */
    QueueClient GetQueueClient(const std::string& queueName) const;

    /**
     * @brief Gets the queue service's primary URL endpoint..
     *
     * @return The queue service's primary URL endpoint.
     */
    std::string GetUrl() const { return m_serviceUrl.GetAbsoluteUrl(); }

    /**
     * @brief Returns a sequence of queues in the storage account. Enumerating the queues may make
     * multiple requests to the service while fetching all the values. Queue names are returned in
     * lexicographic order.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A ListQueuesPagedResponse describing the queues in the storage account.
     */
    ListQueuesPagedResponse ListQueues(
        const ListQueuesOptions& options = ListQueuesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Sets the properties of the queue service.
     *
     * @param properties The queue service properties.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A SetServicePropertiesResult on successfully setting the properties.
     */
    Azure::Response<Models::SetServicePropertiesResult> SetProperties(
        Models::QueueServiceProperties properties,
        const SetServicePropertiesOptions& options = SetServicePropertiesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Gets the properties of a storage account's queue service.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A QueueServiceProperties describing the service properties.
     */
    Azure::Response<Models::QueueServiceProperties> GetProperties(
        const GetServicePropertiesOptions& options = GetServicePropertiesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Retrieves statistics related to replication for the Queue service. It is only
     * available on the secondary location endpoint when read-access geo-redundant replication is
     * enabled for the storage account.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A GetServiceStatisticsResult describing the service replication statistics.
     */
    Azure::Response<Models::ServiceStatistics> GetStatistics(
        const GetQueueServiceStatisticsOptions& options = GetQueueServiceStatisticsOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Creates a new queue under the specified account. If the queue with the same name
     * already exists and the metadata is identical to the one of the existing queue, the operation
     * is successful. If the metadata doesn't match the one of the existing queue, the operation
     * fails.
     *
     * @param queueName The name of the queue to create.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A QueueClient referencing the newly created container.
     */
    Azure::Response<QueueClient> CreateQueue(
        const std::string& queueName,
        const CreateQueueOptions& options = CreateQueueOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Marks the specified queue for deletion.
     *
     * @param queueName The name of the queue to delete.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A DeleteQueueResult if successful.
     */
    Azure::Response<Models::DeleteQueueResult> DeleteQueue(
        const std::string& queueName,
        const DeleteQueueOptions& options = DeleteQueueOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

  private:
    Azure::Core::Url m_serviceUrl;
    std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;
  };

}}} // namespace Azure::Storage::Queues
