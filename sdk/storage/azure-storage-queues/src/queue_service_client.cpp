// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/queues/queue_service_client.hpp"

#include <azure/core/http/policies/policy.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>
#include <azure/storage/common/internal/storage_per_retry_policy.hpp>
#include <azure/storage/common/internal/storage_service_version_policy.hpp>
#include <azure/storage/common/internal/storage_switch_to_secondary_policy.hpp>
#include <azure/storage/common/storage_common.hpp>

#include "private/package_version.hpp"

namespace Azure { namespace Storage { namespace Queues {

  QueueServiceClient QueueServiceClient::CreateFromConnectionString(
      const std::string& connectionString,
      const QueueClientOptions& options)
  {
    auto parsedConnectionString = _internal::ParseConnectionString(connectionString);
    auto serviceUrl = std::move(parsedConnectionString.QueueServiceUrl);

    if (parsedConnectionString.KeyCredential)
    {
      return QueueServiceClient(
          serviceUrl.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return QueueServiceClient(serviceUrl.GetAbsoluteUrl(), options);
    }
  }

  QueueServiceClient::QueueServiceClient(
      const std::string& serviceUrl,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const QueueClientOptions& options)
      : QueueServiceClient(serviceUrl, options)
  {
    QueueClientOptions newOptions = options;
    newOptions.PerRetryPolicies.emplace_back(
        std::make_unique<_internal::SharedKeyPolicy>(credential));

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
        m_serviceUrl.GetHost(), newOptions.SecondaryHostForRetryReads));
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

  QueueServiceClient::QueueServiceClient(
      const std::string& serviceUrl,
      std::shared_ptr<Core::Credentials::TokenCredential> credential,
      const QueueClientOptions& options)
      : QueueServiceClient(serviceUrl, options)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
        m_serviceUrl.GetHost(), options.SecondaryHostForRetryReads));
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

  QueueServiceClient::QueueServiceClient(
      const std::string& serviceUrl,
      const QueueClientOptions& options)
      : m_serviceUrl(serviceUrl)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
        m_serviceUrl.GetHost(), options.SecondaryHostForRetryReads));
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

  QueueClient QueueServiceClient::GetQueueClient(const std::string& queueName) const
  {
    auto queueUrl = m_serviceUrl;
    queueUrl.AppendPath(_internal::UrlEncodePath(queueName));
    return QueueClient(std::move(queueUrl), m_pipeline);
  }

  ListQueuesPagedResponse QueueServiceClient::ListQueues(
      const ListQueuesOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::ServiceClient::ListServiceQueuesSegmentOptions protocolLayerOptions;
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.Marker = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.Include = options.Include;
    auto response = _detail::ServiceClient::ListQueuesSegment(
        *m_pipeline, m_serviceUrl, protocolLayerOptions, _internal::WithReplicaStatus(context));

    ListQueuesPagedResponse pagedResponse;
    pagedResponse.ServiceEndpoint = std::move(response.Value.ServiceEndpoint);
    pagedResponse.Prefix = std::move(response.Value.Prefix);
    pagedResponse.Queues = std::move(response.Value.Items);
    pagedResponse.m_queueServiceClient = std::make_shared<QueueServiceClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.CurrentPageToken = options.ContinuationToken.ValueOr(std::string());
    pagedResponse.NextPageToken = response.Value.ContinuationToken;
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

  Azure::Response<Models::SetServicePropertiesResult> QueueServiceClient::SetProperties(
      Models::QueueServiceProperties properties,
      const SetServicePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    _detail::ServiceClient::SetServicePropertiesOptions protocolLayerOptions;
    protocolLayerOptions.QueueServiceProperties = std::move(properties);
    return _detail::ServiceClient::SetProperties(
        *m_pipeline, m_serviceUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::QueueServiceProperties> QueueServiceClient::GetProperties(
      const GetServicePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    _detail::ServiceClient::GetServicePropertiesOptions protocolLayerOptions;
    return _detail::ServiceClient::GetProperties(
        *m_pipeline, m_serviceUrl, protocolLayerOptions, _internal::WithReplicaStatus(context));
  }

  Azure::Response<Models::ServiceStatistics> QueueServiceClient::GetStatistics(
      const GetQueueServiceStatisticsOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    _detail::ServiceClient::GetServiceStatisticsOptions protocolLayerOptions;
    return _detail::ServiceClient::GetStatistics(
        *m_pipeline, m_serviceUrl, protocolLayerOptions, context);
  }

  Azure::Response<QueueClient> QueueServiceClient::CreateQueue(
      const std::string& queueName,
      const CreateQueueOptions& options,
      const Azure::Core::Context& context) const
  {
    auto queueClient = GetQueueClient(queueName);
    auto response = queueClient.Create(options, context);
    return Azure::Response<QueueClient>(std::move(queueClient), std::move(response.RawResponse));
  }

  Azure::Response<Models::DeleteQueueResult> QueueServiceClient::DeleteQueue(
      const std::string& queueName,
      const DeleteQueueOptions& options,
      const Azure::Core::Context& context) const
  {
    auto queueClient = GetQueueClient(queueName);
    return queueClient.Delete(options, context);
  }

}}} // namespace Azure::Storage::Queues