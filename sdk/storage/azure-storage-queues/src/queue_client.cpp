// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/queues/queue_client.hpp"

#include "private/package_version.hpp"
#include "private/queue_model_conversions.hpp"
#include "private/queue_protocol.hpp"

#include <azure/core/http/policies/policy.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>
#include <azure/storage/common/internal/storage_bearer_token_auth.hpp>
#include <azure/storage/common/internal/storage_pipeline.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_exception.hpp>

#include <stdexcept>

namespace Azure { namespace Storage { namespace Queues {

  QueueClient QueueClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& queueName,
      const QueueClientOptions& options)
  {
    auto parsedConnectionString = _internal::ParseConnectionString(connectionString);
    auto queueUrl = std::move(parsedConnectionString.QueueServiceUrl);
    queueUrl.AppendPath(_internal::UrlEncodePath(queueName));

    if (parsedConnectionString.KeyCredential)
    {
      return QueueClient(queueUrl.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return QueueClient(queueUrl.GetAbsoluteUrl(), options);
    }
  }

  QueueClient::QueueClient(
      const std::string& queueUrl,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const QueueClientOptions& options)
      : QueueClient(queueUrl, options)
  {
    _internal::BuildStoragePipelineOptions pipelineOptions;
    pipelineOptions.PackageName = _internal::QueueServicePackageName;
    pipelineOptions.PackageVersion = _detail::PackageVersion::ToString();
    pipelineOptions.PrimaryHost = m_queueUrl.GetHost();
    pipelineOptions.SecondaryHost = options.SecondaryHostForRetryReads;
    pipelineOptions.ApiVersion = options.ApiVersion.ToString();
    pipelineOptions.SharedKeyAuthPolicy = std::make_unique<_internal::SharedKeyPolicy>(credential);

    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        _detail::BuildQueuePipelinePolicies(options, std::move(pipelineOptions)));
  }

  QueueClient::QueueClient(
      const std::string& queueUrl,
      std::shared_ptr<const Core::Credentials::TokenCredential> credential,
      const QueueClientOptions& options)
      : QueueClient(queueUrl, options)
  {
    _internal::BuildStoragePipelineOptions pipelineOptions;
    pipelineOptions.PackageName = _internal::QueueServicePackageName;
    pipelineOptions.PackageVersion = _detail::PackageVersion::ToString();
    pipelineOptions.PrimaryHost = m_queueUrl.GetHost();
    pipelineOptions.SecondaryHost = options.SecondaryHostForRetryReads;
    pipelineOptions.ApiVersion = options.ApiVersion.ToString();
    {
      Azure::Core::Credentials::TokenRequestContext tokenContext;
      tokenContext.Scopes.emplace_back(
          options.Audience.HasValue()
              ? _internal::GetDefaultScopeForAudience(options.Audience.Value().ToString())
              : _internal::StorageScope);
      pipelineOptions.TokenAuthPolicy
          = std::make_unique<_internal::StorageBearerTokenAuthenticationPolicy>(
              credential, tokenContext, options.EnableTenantDiscovery);
    }

    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        _detail::BuildQueuePipelinePolicies(options, std::move(pipelineOptions)));
  }

  QueueClient::QueueClient(const std::string& queueUrl, const QueueClientOptions& options)
      : m_queueUrl(queueUrl)
  {
    _internal::BuildStoragePipelineOptions pipelineOptions;
    pipelineOptions.PackageName = _internal::QueueServicePackageName;
    pipelineOptions.PackageVersion = _detail::PackageVersion::ToString();
    pipelineOptions.PrimaryHost = m_queueUrl.GetHost();
    pipelineOptions.SecondaryHost = options.SecondaryHostForRetryReads;
    pipelineOptions.ApiVersion = options.ApiVersion.ToString();

    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        _detail::BuildQueuePipelinePolicies(options, std::move(pipelineOptions)));
  }

  Azure::Response<Models::CreateQueueResult> QueueClient::Create(
      const CreateQueueOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      _detail::QueuesClientCreateQueueOptions protocolLayerOptions;
      protocolLayerOptions.Metadata = options.Metadata;
      auto response = _detail::InvokeStorageProtocol([&] {
        return _detail::QueuesClient(m_queueUrl, m_pipeline)
            .CreateQueue(protocolLayerOptions, context);
      });
      Models::CreateQueueResult result;
      result.Created = response.RawResponse->GetStatusCode() == Core::Http::HttpStatusCode::Created;
      return Azure::Response<Models::CreateQueueResult>(
          std::move(result), std::move(response.RawResponse));
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == "QueueAlreadyExists")
      {
        Models::CreateQueueResult ret;
        ret.Created = false;
        return Azure::Response<Models::CreateQueueResult>(std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Response<Models::DeleteQueueResult> QueueClient::Delete(
      const DeleteQueueOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    try
    {
      auto response = _detail::InvokeStorageProtocol(
          [&] { return _detail::QueuesClient(m_queueUrl, m_pipeline).DeleteQueue({}, context); });
      return Azure::Response<Models::DeleteQueueResult>({}, std::move(response.RawResponse));
    }
    catch (StorageException& e)
    {
      if (e.StatusCode == Core::Http::HttpStatusCode::NotFound && e.ErrorCode == "QueueNotFound")
      {
        Models::DeleteQueueResult ret;
        ret.Deleted = false;
        return Azure::Response<Models::DeleteQueueResult>(std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Response<Models::QueueProperties> QueueClient::GetProperties(
      const GetQueuePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto ret = _detail::InvokeStorageProtocol([&] {
      return _detail::QueuesClient(m_queueUrl, m_pipeline).GetPropertiesQueue({}, context);
    });
    Models::QueueProperties queueProperties;
    if (ret.Value.Metadata.HasValue())
    {
      queueProperties.Metadata = std::move(ret.Value.Metadata.Value());
    }
    if (!ret.Value.ApproximateMessagesCount.HasValue())
    {
      throw std::out_of_range("Missing x-ms-approximate-messages-count response header.");
    }
    auto const count = ret.Value.ApproximateMessagesCount.Value();
    if (count > static_cast<int64_t>(INT32_MAX))
    {
      queueProperties.ApproximateMessageCount = -1;
    }
    else
    {
      queueProperties.ApproximateMessageCount = static_cast<std::int32_t>(count);
    }
    queueProperties.ApproximateMessageCountLong = count;
    return Azure::Response<Models::QueueProperties>(
        std::move(queueProperties), std::move(ret.RawResponse));
  }

  Azure::Response<Models::SetQueueMetadataResult> QueueClient::SetMetadata(
      Metadata metadata,
      const SetQueueMetadataOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    _detail::QueuesClientSetMetadataQueueOptions protocolLayerOptions;
    protocolLayerOptions.Metadata = std::move(metadata);
    auto response = _detail::InvokeStorageProtocol([&] {
      return _detail::QueuesClient(m_queueUrl, m_pipeline)
          .SetMetadataQueue(protocolLayerOptions, context);
    });
    return Azure::Response<Models::SetQueueMetadataResult>({}, std::move(response.RawResponse));
  }

  Azure::Response<Models::QueueAccessPolicy> QueueClient::GetAccessPolicy(
      const GetQueueAccessPolicyOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto response = _detail::InvokeStorageProtocol([&] {
      return _detail::QueuesClient(m_queueUrl, m_pipeline).GetAccessPolicyQueue({}, context);
    });
    return Azure::Response<Models::QueueAccessPolicy>(
        _detail::ToPublic(std::move(response.Value)), std::move(response.RawResponse));
  }

  Azure::Response<Models::SetQueueAccessPolicyResult> QueueClient::SetAccessPolicy(
      const Models::QueueAccessPolicy& accessPolicy,
      const SetQueueAccessPolicyOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    _detail::QueuesClientSetAccessPolicyQueueOptions protocolLayerOptions;
    protocolLayerOptions.QueueAcl = _detail::ToGenerated(accessPolicy);
    auto response = _detail::InvokeStorageProtocol([&] {
      return _detail::QueuesClient(m_queueUrl, m_pipeline)
          .SetAccessPolicyQueue(protocolLayerOptions, context);
    });
    return Azure::Response<Models::SetQueueAccessPolicyResult>({}, std::move(response.RawResponse));
  }

  Azure::Response<Models::EnqueueMessageResult> QueueClient::EnqueueMessage(
      std::string messageText,
      const EnqueueMessageOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::QueuesClientSendMessageQueueOptions protocolLayerOptions;
    _detail::Models::QueueMessage message;
    message.MessageText = std::move(messageText);
    if (options.TimeToLive.HasValue())
    {
      protocolLayerOptions.Messagettl = static_cast<int32_t>(options.TimeToLive.Value().count());
    }
    if (options.VisibilityTimeout.HasValue())
    {
      protocolLayerOptions.Visibilitytimeout
          = static_cast<int32_t>(options.VisibilityTimeout.Value().count());
    }
    auto response = _detail::InvokeStorageProtocol([&] {
      return _detail::QueuesClient(m_queueUrl, m_pipeline)
          .SendMessageQueue(message, protocolLayerOptions, context);
    });
    Models::EnqueueMessageResult result;
    if (!response.Value.Items.empty())
    {
      auto& item = response.Value.Items.front();
      result.MessageId = std::move(item.MessageId);
      result.InsertedOn = item.InsertionTime;
      result.ExpiresOn = item.ExpirationTime;
      result.PopReceipt = std::move(item.PopReceipt);
      result.NextVisibleOn = item.TimeNextVisible;
    }
    return Azure::Response<Models::EnqueueMessageResult>(
        std::move(result), std::move(response.RawResponse));
  }

  Azure::Response<Models::ReceivedMessages> QueueClient::ReceiveMessages(
      const ReceiveMessagesOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::QueuesClientReceiveMessagesQueueOptions protocolLayerOptions;
    protocolLayerOptions.Numofmessages = options.MaxMessages;
    if (options.VisibilityTimeout.HasValue())
    {
      protocolLayerOptions.Visibilitytimeout
          = static_cast<int32_t>(options.VisibilityTimeout.Value().count());
    }
    auto response = _detail::InvokeStorageProtocol([&] {
      return _detail::QueuesClient(m_queueUrl, m_pipeline)
          .ReceiveMessagesQueue(protocolLayerOptions, context);
    });
    Models::ReceivedMessages result;
    result.Messages.reserve(response.Value.Items.size());
    for (auto& item : response.Value.Items)
    {
      result.Messages.emplace_back(_detail::ToPublic(std::move(item)));
    }
    return Azure::Response<Models::ReceivedMessages>(
        std::move(result), std::move(response.RawResponse));
  }

  Azure::Response<Models::PeekedMessages> QueueClient::PeekMessages(
      const PeekMessagesOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::QueuesClientPeekMessagesQueueOptions protocolLayerOptions;
    protocolLayerOptions.Numofmessages = options.MaxMessages;
    auto response = _detail::InvokeStorageProtocol([&] {
      return _detail::QueuesClient(m_queueUrl, m_pipeline)
          .PeekMessagesQueue(protocolLayerOptions, context);
    });
    Models::PeekedMessages result;
    result.Messages.reserve(response.Value.Items.size());
    for (auto& item : response.Value.Items)
    {
      result.Messages.emplace_back(_detail::ToPublic(std::move(item)));
    }
    return Azure::Response<Models::PeekedMessages>(
        std::move(result), std::move(response.RawResponse));
  }

  Azure::Response<Models::UpdateMessageResult> QueueClient::UpdateMessage(
      const std::string& messageId,
      const std::string& popReceipt,
      std::chrono::seconds visibilityTimeout,
      const UpdateMessageOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::QueuesClientUpdateMessageQueueOptions protocolLayerOptions;
    if (options.MessageText.HasValue())
    {
      _detail::Models::QueueMessage message;
      message.MessageText = options.MessageText.Value();
      protocolLayerOptions.QueueMessage = std::move(message);
    }
    auto response = _detail::InvokeStorageProtocol([&] {
      return _detail::QueuesClient(m_queueUrl, m_pipeline)
          .UpdateMessageQueue(
              messageId,
              popReceipt,
              static_cast<int32_t>(visibilityTimeout.count()),
              protocolLayerOptions,
              context);
    });
    Models::UpdateMessageResult result;
    result.PopReceipt = std::move(response.Value.PopReceipt);
    result.NextVisibleOn = response.Value.TimeNextVisible;
    return Azure::Response<Models::UpdateMessageResult>(
        std::move(result), std::move(response.RawResponse));
  }

  Azure::Response<Models::DeleteMessageResult> QueueClient::DeleteMessage(
      const std::string& messageId,
      const std::string& popReceipt,
      const DeleteMessageOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto response = _detail::InvokeStorageProtocol([&] {
      return _detail::QueuesClient(m_queueUrl, m_pipeline)
          .DeleteMessageQueue(messageId, popReceipt, {}, context);
    });
    return Azure::Response<Models::DeleteMessageResult>({}, std::move(response.RawResponse));
  }

  Azure::Response<Models::ClearMessagesResult> QueueClient::ClearMessages(
      const ClearMessagesOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto response = _detail::InvokeStorageProtocol(
        [&] { return _detail::QueuesClient(m_queueUrl, m_pipeline).ClearQueue({}, context); });
    return Azure::Response<Models::ClearMessagesResult>({}, std::move(response.RawResponse));
  }

}}} // namespace Azure::Storage::Queues
