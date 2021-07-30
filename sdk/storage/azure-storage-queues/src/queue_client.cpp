// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/queues/queue_client.hpp"

#include <azure/core/http/policies/policy.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>
#include <azure/storage/common/internal/storage_per_retry_policy.hpp>
#include <azure/storage/common/internal/storage_service_version_policy.hpp>
#include <azure/storage/common/internal/storage_switch_to_secondary_policy.hpp>
#include <azure/storage/common/storage_common.hpp>

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
        std::make_unique<_internal::StorageServiceVersionPolicy>(newOptions.ApiVersion));
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
        std::make_unique<_internal::StorageServiceVersionPolicy>(options.ApiVersion));
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
        std::make_unique<_internal::StorageServiceVersionPolicy>(options.ApiVersion));
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        options,
        _internal::QueueServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

}}} // namespace Azure::Storage::Queues
