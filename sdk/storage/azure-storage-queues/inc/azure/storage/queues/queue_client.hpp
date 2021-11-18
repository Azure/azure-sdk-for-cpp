// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
/**
 * @file
 * @brief Defines Queue client.
 *
 */

#pragma once

#include <memory>
#include <string>

#include <azure/core/credentials/credentials.hpp>
#include <azure/storage/common/storage_credential.hpp>

#include "azure/storage/queues/queue_options.hpp"

namespace Azure { namespace Storage { namespace Queues {

  /**
   * The QueueClient allows you to manipulate Azure Storage queues and their messages.
   */
  class QueueClient final {
  public:
    /**
     * @brief Initializes a new instance of QueueClient.
     *
     * @param connectionString A connection string includes the authentication information required
     * for your application to access data in an Azure Storage account at runtime.
     * @param queueName The name of the queue.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     * @return A new QueueClient instance.
     */
    static QueueClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& queueName,
        const QueueClientOptions& options = QueueClientOptions());

    /**
     * @brief Initializes a new instance of QueueClient.
     *
     * @param queueUrl A url referencing the queue that includes the name of the account and the
     * name of the queue.
     * @param credential The shared key credential used to sign requests.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit QueueClient(
        const std::string& queueUrl,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        const QueueClientOptions& options = QueueClientOptions());

    /**
     * @brief Initializes a new instance of QueueClient.
     *
     * @param queueUrl A url referencing the queue that includes the name of the account and the
     * name of the queue.
     * @param credential The token credential used to sign requests.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit QueueClient(
        const std::string& queueUrl,
        std::shared_ptr<Core::Credentials::TokenCredential> credential,
        const QueueClientOptions& options = QueueClientOptions());

    /**
     * @brief Initializes a new instance of QueueClient.
     *
     * @param queueUrl A url referencing the queue that includes the name of the account and the
     * name of the queue.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit QueueClient(
        const std::string& queueUrl,
        const QueueClientOptions& options = QueueClientOptions());

    /**
     * @brief Gets the queue's primary URL endpoint.
     *
     * @return The queue's primary URL endpoint.
     */
    std::string GetUrl() const { return m_queueUrl.GetAbsoluteUrl(); }

    /**
     * @brief Creates a new queue under the specified account. If the queue with the same name
     * already exists, it is not changed.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A CreateQueueResult describing the newly created queue if the queue doesn't exist.
     * CreateQueueResult.Created is false if the queue already exists.
     */
    Azure::Response<Models::CreateQueueResult> Create(
        const CreateQueueOptions& options = CreateQueueOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Marks the specified queue for deletion if it exists.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return DeleteQueueResult.Deleted will be true if successful, false if the queue doesn't
     * exist.
     */
    Azure::Response<Models::DeleteQueueResult> Delete(
        const DeleteQueueOptions& options = DeleteQueueOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Returns all user-defined metadata and system properties for the specified queue.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A QueueProperties describing the queue and its properties.
     */
    Azure::Response<Models::QueueProperties> GetProperties(
        const GetQueuePropertiesOptions& options = GetQueuePropertiesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Sets one or more user-defined name-value pairs for the specified queue.
     *
     * @param metadata Custom metadata to set for this queue.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A SetQueueMetadataResult if successful.
     */
    Azure::Response<Models::SetQueueMetadataResult> SetMetadata(
        Metadata metadata,
        const SetQueueMetadataOptions& options = SetQueueMetadataOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Gets details about any stored access policies specified on the queue that may be used
     * with SAS.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A QueueAccessPolicy describing the queue's access policy.
     */
    Azure::Response<Models::QueueAccessPolicy> GetAccessPolicy(
        const GetQueueAccessPolicyOptions& options = GetQueueAccessPolicyOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Sets stored access policies for the queue that may be used with SAS.
     *
     * @param accessPolicy Stored access policies that can be used to provide fine grained control
     * over queue permissions.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A SetQueueAccessPolicyResult describing the updated queue.
     */
    Azure::Response<Models::SetQueueAccessPolicyResult> SetAccessPolicy(
        const Models::QueueAccessPolicy& accessPolicy,
        const SetQueueAccessPolicyOptions& options = SetQueueAccessPolicyOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Adds a new message to the back of a queue. The visibility timeout specifies how long
     * the message should be invisible to dequeue and peek operations.
     *
     * @param messageText Message in plain text.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A EnqueueMessageResult describing the result.
     */
    Azure::Response<Models::EnqueueMessageResult> EnqueueMessage(
        std::string messageText,
        const EnqueueMessageOptions& options = EnqueueMessageOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Receives one or more messages from the front of the queue. Returns empty collection if
     * there's not message available.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A ReceivedMessages that contains a vector of queue messages.
     */
    Azure::Response<Models::ReceivedMessages> ReceiveMessages(
        const ReceiveMessagesOptions& options = ReceiveMessagesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Retrieves one or more messages from the front of the queue but does not alter the
     * visibility of the message. Returns empty collection if there's not message available.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A PeekedMessages that contains a vector of queue messages.
     */
    Azure::Response<Models::PeekedMessages> PeekMessages(
        const PeekMessagesOptions& options = PeekMessagesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Changes a message's visibility timeout and/or contents.
     *
     * @param messageId ID of the message to update.
     * @param popReceipt Specifies the valid pop receipt value returned from an earlier call.
     * @param visibilityTimeout Specifies the new visibility timeout value, in seconds, relative to
     * server time. The new value must be larger than or equal to 0, and cannot be larger than 7
     * days. The visibility timeout of a message cannot be set to a value later than the expiry
     * time. A message can be updated until it has been deleted or has expired.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return An UpdateMessageResult describing the updated message.
     */
    Azure::Response<Models::UpdateMessageResult> UpdateMessage(
        const std::string& messageId,
        const std::string& popReceipt,
        std::chrono::seconds visibilityTimeout,
        const UpdateMessageOptions& options = UpdateMessageOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Permanently removes the specified message from the queue.
     *
     * @param messageId ID of the message to delete.
     * @param popReceipt A valid pop receipt value returned from an earlier call.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A DeleteMessageResult describing the result.
     */
    Azure::Response<Models::DeleteMessageResult> DeleteMessage(
        const std::string& messageId,
        const std::string& popReceipt,
        const DeleteMessageOptions& options = DeleteMessageOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Deletes all messages from the queue.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A ClearMessagesResult describing the result.
     */
    Azure::Response<Models::ClearMessagesResult> ClearMessages(
        const ClearMessagesOptions& options = ClearMessagesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

  private:
    explicit QueueClient(
        Azure::Core::Url queueUrl,
        std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> pipeline)
        : m_queueUrl(std::move(queueUrl)), m_pipeline(std::move(pipeline))
    {
    }

  private:
    Azure::Core::Url m_queueUrl;
    std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;

    friend class QueueServiceClient;
  };

}}} // namespace Azure::Storage::Queues
