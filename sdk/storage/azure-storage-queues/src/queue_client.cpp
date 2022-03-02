// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/queues/queue_client.hpp"

#include <azure/core/http/policies/policy.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>
#include <azure/storage/common/internal/storage_per_retry_policy.hpp>
#include <azure/storage/common/internal/storage_service_version_policy.hpp>
#include <azure/storage/common/internal/storage_switch_to_secondary_policy.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_exception.hpp>

#include "private/package_version.hpp"

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
    QueueClientOptions newOptions = options;
    newOptions.PerRetryPolicies.emplace_back(
        std::make_unique<_internal::SharedKeyPolicy>(credential));

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
        m_queueUrl.GetHost(), newOptions.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(newOptions.ApiVersion.ToString()));
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        newOptions,
        _internal::QueueServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  QueueClient::QueueClient(
      const std::string& queueUrl,
      std::shared_ptr<Core::Credentials::TokenCredential> credential,
      const QueueClientOptions& options)
      : QueueClient(queueUrl, options)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
        m_queueUrl.GetHost(), options.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    {
      Azure::Core::Credentials::TokenRequestContext tokenContext;
      tokenContext.Scopes.emplace_back(_internal::StorageScope);
      perRetryPolicies.emplace_back(
          std::make_unique<Azure::Core::Http::Policies::_internal::BearerTokenAuthenticationPolicy>(
              credential, tokenContext));
    }
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(options.ApiVersion.ToString()));
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        options,
        _internal::QueueServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  QueueClient::QueueClient(const std::string& queueUrl, const QueueClientOptions& options)
      : m_queueUrl(queueUrl)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
        m_queueUrl.GetHost(), options.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(options.ApiVersion.ToString()));
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        options,
        _internal::QueueServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  Azure::Response<Models::CreateQueueResult> QueueClient::Create(
      const CreateQueueOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      _detail::QueueClient::CreateQueueOptions protocolLayerOptions;
      protocolLayerOptions.Metadata
          = std::map<std::string, std::string>(options.Metadata.begin(), options.Metadata.end());
      auto response
          = _detail::QueueClient::Create(*m_pipeline, m_queueUrl, protocolLayerOptions, context);
      response.Value.Created
          = response.RawResponse->GetStatusCode() == Core::Http::HttpStatusCode::Created;
      return response;
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
      _detail::QueueClient::DeleteQueueOptions protocolLayerOptions;
      return _detail::QueueClient::Delete(*m_pipeline, m_queueUrl, protocolLayerOptions, context);
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
    _detail::QueueClient::GetQueuePropertiesOptions protocolLayerOptions;
    return _detail::QueueClient::GetProperties(
        *m_pipeline, m_queueUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::SetQueueMetadataResult> QueueClient::SetMetadata(
      Metadata metadata,
      const SetQueueMetadataOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    _detail::QueueClient::SetQueueMetadataOptions protocolLayerOptions;
    protocolLayerOptions.Metadata
        = std::map<std::string, std::string>(metadata.begin(), metadata.end());
    return _detail::QueueClient::SetMetadata(
        *m_pipeline, m_queueUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::QueueAccessPolicy> QueueClient::GetAccessPolicy(
      const GetQueueAccessPolicyOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    _detail::QueueClient::GetQueueAccessPolicyOptions protocolLayerOptions;
    return _detail::QueueClient::GetAccessPolicy(
        *m_pipeline, m_queueUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::SetQueueAccessPolicyResult> QueueClient::SetAccessPolicy(
      const Models::QueueAccessPolicy& accessPolicy,
      const SetQueueAccessPolicyOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    _detail::QueueClient::SetQueueAccessPolicyOptions protocolLayerOptions;
    protocolLayerOptions.QueueAcl = accessPolicy.SignedIdentifiers;
    return _detail::QueueClient::SetAccessPolicy(
        *m_pipeline, m_queueUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::EnqueueMessageResult> QueueClient::EnqueueMessage(
      std::string messageText,
      const EnqueueMessageOptions& options,
      const Azure::Core::Context& context) const
  {
    auto messagesUrl = m_queueUrl;
    messagesUrl.AppendPath("messages");
    _detail::QueueClient::EnqueueQueueMessageOptions protocolLayerOptions;
    protocolLayerOptions.QueueMessage.MessageText = std::move(messageText);
    if (options.TimeToLive.HasValue())
    {
      protocolLayerOptions.MessageTimeToLive
          = static_cast<int32_t>(options.TimeToLive.Value().count());
    }
    if (options.VisibilityTimeout.HasValue())
    {
      protocolLayerOptions.Visibilitytimeout
          = static_cast<int32_t>(options.VisibilityTimeout.Value().count());
    }
    return _detail::QueueClient::EnqueueMessage(
        *m_pipeline, messagesUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::ReceivedMessages> QueueClient::ReceiveMessages(
      const ReceiveMessagesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto messagesUrl = m_queueUrl;
    messagesUrl.AppendPath("messages");
    _detail::QueueClient::ReceiveQueueMessagesOptions protocolLayerOptions;
    protocolLayerOptions.NumberOfMessages = options.MaxMessages;
    if (options.VisibilityTimeout.HasValue())
    {
      protocolLayerOptions.Visibilitytimeout
          = static_cast<int32_t>(options.VisibilityTimeout.Value().count());
    }
    return _detail::QueueClient::ReceiveMessages(
        *m_pipeline, messagesUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::PeekedMessages> QueueClient::PeekMessages(
      const PeekMessagesOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto messagesUrl = m_queueUrl;
    messagesUrl.AppendPath("messages");
    _detail::QueueClient::PeekQueueMessagesOptions protocolLayerOptions;
    protocolLayerOptions.NumberOfMessages = options.MaxMessages;
    return _detail::QueueClient::PeekMessages(
        *m_pipeline, messagesUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::UpdateMessageResult> QueueClient::UpdateMessage(
      const std::string& messageId,
      const std::string& popReceipt,
      std::chrono::seconds visibilityTimeout,
      const UpdateMessageOptions& options,
      const Azure::Core::Context& context) const
  {
    auto messageUrl = m_queueUrl;
    messageUrl.AppendPath("messages");
    messageUrl.AppendPath(_internal::UrlEncodePath(messageId));
    if (options.MessageText.HasValue())
    {
      _detail::QueueClient::UpdateQueueMessageOptions protocolLayerOptions;
      protocolLayerOptions.QueueMessage.MessageText = options.MessageText.Value();
      protocolLayerOptions.PopReceipt = popReceipt;
      protocolLayerOptions.Visibilitytimeout = static_cast<int32_t>(visibilityTimeout.count());
      return _detail::QueueClient::UpdateMessage(
          *m_pipeline, messageUrl, protocolLayerOptions, context);
    }
    else
    {
      _detail::QueueClient::UpdateQueueMessageVisibilityOptions protocolLayerOptions;
      protocolLayerOptions.PopReceipt = popReceipt;
      protocolLayerOptions.Visibilitytimeout = static_cast<int32_t>(visibilityTimeout.count());
      return _detail::QueueClient::UpdateMessageVisibility(
          *m_pipeline, messageUrl, protocolLayerOptions, context);
    }
  }

  Azure::Response<Models::DeleteMessageResult> QueueClient::DeleteMessage(
      const std::string& messageId,
      const std::string& popReceipt,
      const DeleteMessageOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto messageUrl = m_queueUrl;
    messageUrl.AppendPath("messages");
    messageUrl.AppendPath(_internal::UrlEncodePath(messageId));
    _detail::QueueClient::DeleteQueueMessageOptions protocolLayerOptions;
    protocolLayerOptions.PopReceipt = popReceipt;
    return _detail::QueueClient::DeleteMessage(
        *m_pipeline, messageUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::ClearMessagesResult> QueueClient::ClearMessages(
      const ClearMessagesOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto messagesUrl = m_queueUrl;
    messagesUrl.AppendPath("messages");
    _detail::QueueClient::ClearQueueMessagesOptions protocolLayerOptions;
    return _detail::QueueClient::ClearMessages(
        *m_pipeline, messagesUrl, protocolLayerOptions, context);
  }

}}} // namespace Azure::Storage::Queues
