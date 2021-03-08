// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/share_service_client.hpp"

#include <azure/core/credentials.hpp>
#include <azure/core/http/policy.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/shared_key_policy.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_credential.hpp>
#include <azure/storage/common/storage_per_retry_policy.hpp>

#include "azure/storage/files/shares/share_client.hpp"
#include "azure/storage/files/shares/version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {
  ShareServiceClient ShareServiceClient::CreateFromConnectionString(
      const std::string& connectionString,
      const ShareClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
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
        std::make_unique<Storage::Details::SharedKeyPolicy>(credential));

    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
    {
      Azure::Core::Http::Internal::ValueOptions valueOptions;
      valueOptions.HeaderValues[Storage::Details::HttpHeaderXMsVersion] = newOptions.ApiVersion;
      perOperationPolicies.emplace_back(
          std::make_unique<Azure::Core::Http::Internal::ValuePolicy>(valueOptions));
    }
    m_pipeline = std::make_shared<Azure::Core::Internal::Http::HttpPipeline>(
        newOptions,
        Storage::Details::FileServicePackageName,
        Details::Version::VersionString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  ShareServiceClient::ShareServiceClient(
      const std::string& serviceUrl,
      const ShareClientOptions& options)
      : m_serviceUrl(serviceUrl)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
    {
      Azure::Core::Http::Internal::ValueOptions valueOptions;
      valueOptions.HeaderValues[Storage::Details::HttpHeaderXMsVersion] = options.ApiVersion;
      perOperationPolicies.emplace_back(
          std::make_unique<Azure::Core::Http::Internal::ValuePolicy>(valueOptions));
    }
    m_pipeline = std::make_shared<Azure::Core::Internal::Http::HttpPipeline>(
        options,
        Storage::Details::FileServicePackageName,
        Details::Version::VersionString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  ShareClient ShareServiceClient::GetShareClient(const std::string& shareName) const
  {
    auto builder = m_serviceUrl;
    builder.AppendPath(Storage::Details::UrlEncodePath(shareName));
    return ShareClient(builder, m_pipeline);
  }

  Azure::Response<Models::ListSharesSinglePageResult> ShareServiceClient::ListSharesSinglePage(
      const ListSharesSinglePageOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Service::ListSharesSinglePageOptions();
    protocolLayerOptions.ListSharesInclude = options.ListSharesIncludeFlags;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.Prefix = options.Prefix;
    return Details::ShareRestClient::Service::ListSharesSinglePage(
        m_serviceUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Response<Models::SetServicePropertiesResult> ShareServiceClient::SetProperties(
      Models::FileServiceProperties properties,
      const SetServicePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = Details::ShareRestClient::Service::SetPropertiesOptions();
    protocolLayerOptions.ServiceProperties = std::move(properties);
    return Details::ShareRestClient::Service::SetProperties(
        m_serviceUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Response<Models::GetServicePropertiesResult> ShareServiceClient::GetProperties(
      const GetServicePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = Details::ShareRestClient::Service::GetPropertiesOptions();
    auto result = Details::ShareRestClient::Service::GetProperties(
        m_serviceUrl, *m_pipeline, context, protocolLayerOptions);
    Models::FileServiceProperties ret;
    ret.Cors = std::move(result->Cors);
    ret.HourMetrics = std::move(result->HourMetrics);
    ret.MinuteMetrics = std::move(result->MinuteMetrics);
    ret.Protocol = std::move(result->Protocol);
    return Azure::Response<Models::FileServiceProperties>(
        std::move(ret), result.ExtractRawResponse());
  }

}}}} // namespace Azure::Storage::Files::Shares
