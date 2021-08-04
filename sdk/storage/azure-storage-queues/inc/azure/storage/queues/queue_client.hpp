// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

#include <azure/core/credentials/credentials.hpp>
#include <azure/storage/common/storage_credential.hpp>

#include "azure/storage/queues/queue_options.hpp"

namespace Azure { namespace Storage { namespace Queues {

  class QueueClient final {
  public:
    static QueueClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& queueName,
        const QueueClientOptions& options = QueueClientOptions());

    explicit QueueClient(
        const std::string& queueUrl,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        const QueueClientOptions& options = QueueClientOptions());

    explicit QueueClient(
        const std::string& queueUrl,
        std::shared_ptr<Core::Credentials::TokenCredential> credential,
        const QueueClientOptions& options = QueueClientOptions());

    explicit QueueClient(
        const std::string& queueUrl,
        const QueueClientOptions& options = QueueClientOptions());

    std::string GetUrl() const { return m_queueUrl.GetAbsoluteUrl(); }

    Azure::Response<Models::CreateQueueResult> Create(
        const CreateQueueOptions& options = CreateQueueOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    Azure::Response<Models::CreateQueueResult> CreateIfNotExists(
        const CreateQueueOptions& options = CreateQueueOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    Azure::Response<Models::DeleteQueueResult> Delete(
        const DeleteQueueOptions& options = DeleteQueueOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    Azure::Response<Models::DeleteQueueResult> DeleteIfExists(
        const DeleteQueueOptions& options = DeleteQueueOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    Azure::Response<Models::QueueProperties> GetProperties(
        const GetQueuePropertiesOptions& options = GetQueuePropertiesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    Azure::Response<Models::SetQueueMetadataResult> SetMetadata(
        Metadata metadata,
        const SetQueueMetadataOptions& options = SetQueueMetadataOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    Azure::Response<Models::QueueAccessPolicy> GetAccessPolicy(
        const GetQueueAccessPolicyOptions& options = GetQueueAccessPolicyOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    Azure::Response<Models::SetQueueAccessPolicyResult> SetAccessPolicy(
        std::vector<Models::SignedIdentifier> signedIdentifiers,
        const SetQueueAccessPolicyOptions& options = SetQueueAccessPolicyOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    Azure::Response<Models::SendMessageResult> SendMessage(
        std::string messageText,
        const SendMessageOptions& options = SendMessageOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    Azure::Response<Models::QueueMessage> ReceiveMessage(
        const ReceiveMessageOptions& options = ReceiveMessageOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    Azure::Response<std::vector<Models::QueueMessage>> ReceiveMessages(
        const ReceiveMessagesOptions& options = ReceiveMessagesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    Azure::Response<Models::PeekedQueueMessage> PeekMessage(
        const PeekMessageOptions& options = PeekMessageOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    Azure::Response<std::vector<Models::PeekedQueueMessage>> PeekMessages(
        const PeekMessagesOptions& options = PeekMessagesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    Azure::Response<Models::UpdateMessageResult> UpdateMessage(
        const std::string& messageId,
        const std::string& popReceipt,
        int32_t visibilityTimeout,
        const UpdateMessageOptions& options = UpdateMessageOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    Azure::Response<Models::DeleteMessageResult> DeleteMessage(
        const std::string& messageId,
        const std::string& popReceipt,
        const DeleteMessageOptions& options = DeleteMessageOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    Azure::Response<Models::ClearMessagesResult> ClearMessages(
        const ClearMessagesOptions& options = ClearMessagesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

  private:
    explicit QueueClient(
        Azure::Core::Url queueUrl,
        std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> pipeline,
        QueueMessageEncoding messageEncoding)
        : m_queueUrl(std::move(queueUrl)), m_pipeline(std::move(pipeline)),
          m_messageEncoding(messageEncoding)
    {
    }

  private:
    Azure::Core::Url m_queueUrl;
    std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;
    QueueMessageEncoding m_messageEncoding;

    friend class QueueServiceClient;
  };

}}} // namespace Azure::Storage::Queues
