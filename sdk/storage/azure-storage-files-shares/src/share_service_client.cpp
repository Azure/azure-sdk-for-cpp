// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/share_service_client.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>
#include <azure/storage/common/internal/storage_per_retry_policy.hpp>
#include <azure/storage/common/internal/storage_service_version_policy.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_credential.hpp>

#include "azure/storage/files/shares/share_client.hpp"

#include "private/package_version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {
  ShareServiceClient ShareServiceClient::CreateFromConnectionString(
      const std::string& connectionString,
      const ShareClientOptions& options)
  {
    auto parsedConnectionString = _internal::ParseConnectionString(connectionString);
    auto serviceUrl = std::move(parsedConnectionString.FileServiceUrl);

    if (parsedConnectionString.KeyCredential)
    {
      return ShareServiceClient(
          serviceUrl.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return ShareServiceClient(serviceUrl.GetAbsoluteUrl(), options);
    }
  }

  ShareServiceClient::ShareServiceClient(
      const std::string& serviceUrl,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const ShareClientOptions& options)
      : m_serviceUrl(serviceUrl)
  {
    ShareClientOptions newOptions = options;
    newOptions.PerRetryPolicies.emplace_back(
        std::make_unique<_internal::SharedKeyPolicy>(credential));

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(newOptions.ApiVersion));
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        newOptions,
        _internal::FileServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  ShareServiceClient::ShareServiceClient(
      const std::string& serviceUrl,
      const ShareClientOptions& options)
      : m_serviceUrl(serviceUrl)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(options.ApiVersion));
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        options,
        _internal::FileServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  ShareClient ShareServiceClient::GetShareClient(const std::string& shareName) const
  {
    auto builder = m_serviceUrl;
    builder.AppendPath(_internal::UrlEncodePath(shareName));
    return ShareClient(builder, m_pipeline);
  }

  ListSharesPagedResponse ShareServiceClient::ListShares(
      const ListSharesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareRestClient::Service::ListSharesSinglePageOptions();
    protocolLayerOptions.IncludeFlags = options.ListSharesIncludeFlags;
    if (options.ContinuationToken.HasValue() && !options.ContinuationToken.Value().empty())
    {
      protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    }
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.Prefix = options.Prefix;
    auto response = _detail::ShareRestClient::Service::ListSharesSinglePage(
        m_serviceUrl, *m_pipeline, context, protocolLayerOptions);

    ListSharesPagedResponse pagedResponse;
    pagedResponse.ServiceEndpoint = std::move(response.Value.ServiceEndpoint);
    pagedResponse.Prefix = std::move(response.Value.Prefix);
    pagedResponse.Shares = std::move(response.Value.Items);
    pagedResponse.m_shareServiceClient = std::make_shared<ShareServiceClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.CurrentPageToken = options.ContinuationToken.ValueOr(std::string());
    pagedResponse.NextPageToken = response.Value.ContinuationToken;
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

  Azure::Response<Models::SetServicePropertiesResult> ShareServiceClient::SetProperties(
      Models::ShareServiceProperties properties,
      const SetServicePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::ShareRestClient::Service::SetPropertiesOptions();
    protocolLayerOptions.ServiceProperties = std::move(properties);
    return _detail::ShareRestClient::Service::SetProperties(
        m_serviceUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Response<Models::ShareServiceProperties> ShareServiceClient::GetProperties(
      const GetServicePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::ShareRestClient::Service::GetPropertiesOptions();
    auto result = _detail::ShareRestClient::Service::GetProperties(
        m_serviceUrl, *m_pipeline, context, protocolLayerOptions);
    Models::ShareServiceProperties ret;
    ret.Cors = std::move(result.Value.Cors);
    ret.HourMetrics = std::move(result.Value.HourMetrics);
    ret.MinuteMetrics = std::move(result.Value.MinuteMetrics);
    ret.Protocol = std::move(result.Value.Protocol);
    return Azure::Response<Models::ShareServiceProperties>(
        std::move(ret), std::move(result.RawResponse));
  }

}}}} // namespace Azure::Storage::Files::Shares
