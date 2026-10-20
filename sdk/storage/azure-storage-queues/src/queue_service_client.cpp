// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/queues/queue_service_client.hpp"

#include "private/package_version.hpp"
#include "private/queue_model_conversions.hpp"
#include "private/queue_protocol.hpp"

#include <azure/core/http/policies/policy.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>
#include <azure/storage/common/internal/storage_bearer_token_auth.hpp>
#include <azure/storage/common/internal/storage_pipeline.hpp>
#include <azure/storage/common/internal/storage_switch_to_secondary_policy.hpp>
#include <azure/storage/common/storage_common.hpp>

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
    _internal::BuildStoragePipelineOptions pipelineOptions;
    pipelineOptions.PackageName = _internal::QueueServicePackageName;
    pipelineOptions.PackageVersion = _detail::PackageVersion::ToString();
    pipelineOptions.PrimaryHost = m_serviceUrl.GetHost();
    pipelineOptions.SecondaryHost = options.SecondaryHostForRetryReads;
    pipelineOptions.ApiVersion = options.ApiVersion.ToString();
    pipelineOptions.SharedKeyAuthPolicy = std::make_unique<_internal::SharedKeyPolicy>(credential);

    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        _detail::BuildQueuePipelinePolicies(options, std::move(pipelineOptions)));
  }

  QueueServiceClient::QueueServiceClient(
      const std::string& serviceUrl,
      std::shared_ptr<const Core::Credentials::TokenCredential> credential,
      const QueueClientOptions& options)
      : QueueServiceClient(serviceUrl, options)
  {
    _internal::BuildStoragePipelineOptions pipelineOptions;
    pipelineOptions.PackageName = _internal::QueueServicePackageName;
    pipelineOptions.PackageVersion = _detail::PackageVersion::ToString();
    pipelineOptions.PrimaryHost = m_serviceUrl.GetHost();
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

  QueueServiceClient::QueueServiceClient(
      const std::string& serviceUrl,
      const QueueClientOptions& options)
      : m_serviceUrl(serviceUrl)
  {
    _internal::BuildStoragePipelineOptions pipelineOptions;
    pipelineOptions.PackageName = _internal::QueueServicePackageName;
    pipelineOptions.PackageVersion = _detail::PackageVersion::ToString();
    pipelineOptions.PrimaryHost = m_serviceUrl.GetHost();
    pipelineOptions.SecondaryHost = options.SecondaryHostForRetryReads;
    pipelineOptions.ApiVersion = options.ApiVersion.ToString();

    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        _detail::BuildQueuePipelinePolicies(options, std::move(pipelineOptions)));
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
    _detail::QueuesClientGetQueuesServiceOptions protocolLayerOptions;
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.Marker = options.ContinuationToken;
    protocolLayerOptions.Maxresults = options.PageSizeHint;
    if ((options.Include & Models::ListQueuesIncludeFlags::Metadata)
        == Models::ListQueuesIncludeFlags::Metadata)
    {
      protocolLayerOptions.Include = std::vector<_detail::Models::ListQueuesIncludeType>{
          _detail::Models::ListQueuesIncludeType::Metadata};
    }
    auto response = _detail::InvokeStorageProtocol([&] {
      return _detail::QueuesClient(m_serviceUrl, m_pipeline)
          .GetQueuesService(protocolLayerOptions, _internal::WithReplicaStatus(context));
    });

    ListQueuesPagedResponse pagedResponse;
    pagedResponse.ServiceEndpoint = std::move(response.Value.ServiceEndpoint);
    pagedResponse.Prefix = std::move(response.Value.Prefix);
    if (response.Value.QueueItems.HasValue())
    {
      pagedResponse.Queues.reserve(response.Value.QueueItems.Value().size());
      for (auto& item : response.Value.QueueItems.Value())
      {
        Models::QueueItem queue;
        queue.Name = std::move(item.Name);
        if (item.Metadata.HasValue())
        {
          for (auto& entry : item.Metadata.Value())
          {
            queue.Metadata[entry.first] = std::move(entry.second);
          }
        }
        pagedResponse.Queues.emplace_back(std::move(queue));
      }
    }
    pagedResponse.m_queueServiceClient = std::make_shared<QueueServiceClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.CurrentPageToken = options.ContinuationToken.ValueOr(std::string());
    pagedResponse.NextPageToken = response.Value.NextMarker.empty()
        ? Azure::Nullable<std::string>()
        : Azure::Nullable<std::string>(std::move(response.Value.NextMarker));
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

  Azure::Response<Models::SetServicePropertiesResult> QueueServiceClient::SetProperties(
      Models::QueueServiceProperties properties,
      const SetServicePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolProperties = _detail::ToGenerated(std::move(properties));
    auto response = _detail::InvokeStorageProtocol([&] {
      return _detail::QueuesClient(m_serviceUrl, m_pipeline)
          .SetPropertiesService(protocolProperties, {}, context);
    });
    return Azure::Response<Models::SetServicePropertiesResult>({}, std::move(response.RawResponse));
  }

  Azure::Response<Models::QueueServiceProperties> QueueServiceClient::GetProperties(
      const GetServicePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto response = _detail::InvokeStorageProtocol([&] {
      return _detail::QueuesClient(m_serviceUrl, m_pipeline)
          .GetPropertiesService({}, _internal::WithReplicaStatus(context));
    });
    return Azure::Response<Models::QueueServiceProperties>(
        _detail::ToPublic(std::move(response.Value)), std::move(response.RawResponse));
  }

  Azure::Response<Models::ServiceStatistics> QueueServiceClient::GetStatistics(
      const GetQueueServiceStatisticsOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto response = _detail::InvokeStorageProtocol([&] {
      return _detail::QueuesClient(m_serviceUrl, m_pipeline).GetStatisticsService({}, context);
    });
    Models::ServiceStatistics result;
    if (response.Value.GeoReplication.HasValue())
    {
      auto& replication = response.Value.GeoReplication.Value();
      result.GeoReplication.Status = Models::GeoReplicationStatus(replication.Status.ToString());
      result.GeoReplication.LastSyncedOn = replication.LastSyncTime;
    }
    return Azure::Response<Models::ServiceStatistics>(
        std::move(result), std::move(response.RawResponse));
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

  Azure::Response<Models::UserDelegationKey> QueueServiceClient::GetUserDelegationKey(
      const Azure::DateTime& expiresOn,
      const GetUserDelegationKeyOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::Models::KeyInfo keyInfo;
    keyInfo.Start = options.StartsOn.ToString(
        DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::Truncate);
    keyInfo.Expiry
        = expiresOn.ToString(DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::Truncate);
    keyInfo.DelegatedUserTid = options.DelegatedUserTid;
    auto response = _detail::InvokeStorageProtocol([&] {
      return _detail::QueuesClient(m_serviceUrl, m_pipeline)
          .GetUserDelegationKeyService(keyInfo, {}, context);
    });
    Models::UserDelegationKey result;
    result.SignedObjectId = std::move(response.Value.SignedOid);
    result.SignedTenantId = std::move(response.Value.SignedTid);
    result.SignedStartsOn = response.Value.SignedStart;
    result.SignedExpiresOn = response.Value.SignedExpiry;
    result.SignedService = std::move(response.Value.SignedService);
    result.SignedVersion = std::move(response.Value.SignedVersion);
    result.SignedDelegatedUserTid = std::move(response.Value.SignedDelegatedUserTid);
    result.Value = std::move(response.Value.Value);
    return Azure::Response<Models::UserDelegationKey>(
        std::move(result), std::move(response.RawResponse));
  }

}}} // namespace Azure::Storage::Queues
