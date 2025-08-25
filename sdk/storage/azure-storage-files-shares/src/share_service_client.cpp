// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/files/shares/share_service_client.hpp"

#include "azure/storage/files/shares/share_client.hpp"
#include "private/package_version.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>
#include <azure/storage/common/internal/storage_per_retry_policy.hpp>
#include <azure/storage/common/internal/storage_service_version_policy.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_credential.hpp>

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
      : m_serviceUrl(serviceUrl), m_allowTrailingDot(options.AllowTrailingDot),
        m_allowSourceTrailingDot(options.AllowSourceTrailingDot),
        m_shareTokenIntent(options.ShareTokenIntent)
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
      std::shared_ptr<const Core::Credentials::TokenCredential> credential,
      const ShareClientOptions& options)
      : m_serviceUrl(serviceUrl), m_allowTrailingDot(options.AllowTrailingDot),
        m_allowSourceTrailingDot(options.AllowSourceTrailingDot),
        m_shareTokenIntent(options.ShareTokenIntent)
  {
    ShareClientOptions newOptions = options;

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    {
      Azure::Core::Credentials::TokenRequestContext tokenContext;
      tokenContext.Scopes.emplace_back(
          options.Audience.HasValue()
              ? _internal::GetDefaultScopeForAudience(options.Audience.Value().ToString())
              : _internal::StorageScope);
      perRetryPolicies.emplace_back(
          std::make_unique<Azure::Core::Http::Policies::_internal::BearerTokenAuthenticationPolicy>(
              credential, tokenContext));
    }
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
      : m_serviceUrl(serviceUrl), m_allowTrailingDot(options.AllowTrailingDot),
        m_allowSourceTrailingDot(options.AllowSourceTrailingDot),
        m_shareTokenIntent(options.ShareTokenIntent)
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
    ShareClient shareClient(builder, m_pipeline);
    shareClient.m_allowTrailingDot = m_allowTrailingDot;
    shareClient.m_allowSourceTrailingDot = m_allowSourceTrailingDot;
    shareClient.m_shareTokenIntent = m_shareTokenIntent;
    return shareClient;
  }

  ListSharesPagedResponse ShareServiceClient::ListShares(
      const ListSharesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ServiceClient::ListServiceSharesSegmentOptions();
    protocolLayerOptions.Include = options.ListSharesIncludeFlags;
    protocolLayerOptions.Marker = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.FileRequestIntent = m_shareTokenIntent;
    auto response = _detail::ServiceClient::ListSharesSegment(
        *m_pipeline, m_serviceUrl, protocolLayerOptions, context);

    ListSharesPagedResponse pagedResponse;
    pagedResponse.ServiceEndpoint = std::move(response.Value.ServiceEndpoint);
    pagedResponse.Prefix = response.Value.Prefix.ValueOr(std::string());
    pagedResponse.Shares = std::move(response.Value.ShareItems);
    pagedResponse.m_shareServiceClient = std::make_shared<ShareServiceClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.CurrentPageToken = options.ContinuationToken.ValueOr(std::string());
    if (!response.Value.NextMarker.empty())
    {
      pagedResponse.NextPageToken = response.Value.NextMarker;
    }
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

  Azure::Response<Models::SetServicePropertiesResult> ShareServiceClient::SetProperties(
      Models::ShareServiceProperties properties,
      const SetServicePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::ServiceClient::SetServicePropertiesOptions();
    protocolLayerOptions.ShareServiceProperties.Cors = std::move(properties.Cors);
    protocolLayerOptions.ShareServiceProperties.HourMetrics = std::move(properties.HourMetrics);
    protocolLayerOptions.ShareServiceProperties.MinuteMetrics = std::move(properties.MinuteMetrics);
    if (properties.Protocol.HasValue())
    {
      protocolLayerOptions.ShareServiceProperties.Protocol
          = Files::Shares::Models::_detail::ProtocolSettings();
      if (properties.Protocol.Value().SmbSettings.HasValue()
          || properties.Protocol.Value().NfsSettings.HasValue())
      {
        protocolLayerOptions.ShareServiceProperties.Protocol.Value().SmbSettings
            = std::move(properties.Protocol.Value().SmbSettings);
        protocolLayerOptions.ShareServiceProperties.Protocol.Value().NfsSettings
            = std::move(properties.Protocol.Value().NfsSettings);
      }
      else
      {
        protocolLayerOptions.ShareServiceProperties.Protocol.Value().SmbSettings
            = Models::NewSmbSettings();
        protocolLayerOptions.ShareServiceProperties.Protocol.Value()
            .SmbSettings.Value()
            .Multichannel
            = properties.Protocol.Value().Settings.Multichannel;
      }
    }
    protocolLayerOptions.FileRequestIntent = m_shareTokenIntent;
    return _detail::ServiceClient::SetProperties(
        *m_pipeline, m_serviceUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::ShareServiceProperties> ShareServiceClient::GetProperties(
      const GetServicePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::ServiceClient::GetServicePropertiesOptions();
    protocolLayerOptions.FileRequestIntent = m_shareTokenIntent;
    auto result = _detail::ServiceClient::GetProperties(
        *m_pipeline, m_serviceUrl, protocolLayerOptions, context);
    Models::ShareServiceProperties ret;
    ret.Cors = std::move(result.Value.Cors);
    ret.HourMetrics = std::move(result.Value.HourMetrics);
    ret.MinuteMetrics = std::move(result.Value.MinuteMetrics);
    if (result.Value.Protocol.HasValue())
    {
      ret.Protocol = Models::ProtocolSettings();
      ret.Protocol.Value().SmbSettings = std::move(result.Value.Protocol.Value().SmbSettings);
      ret.Protocol.Value().NfsSettings = std::move(result.Value.Protocol.Value().NfsSettings);
      if (result.Value.Protocol.Value().SmbSettings.HasValue()
          && result.Value.Protocol.Value().SmbSettings.Value().Multichannel.HasValue())
      {
        ret.Protocol.Value().Settings.Multichannel
            = result.Value.Protocol.Value().SmbSettings.Value().Multichannel.Value();
      }
    }
    return Azure::Response<Models::ShareServiceProperties>(
        std::move(ret), std::move(result.RawResponse));
  }

  Azure::Response<Models::UserDelegationKey> ShareServiceClient::GetUserDelegationKey(
      const Azure::DateTime& expiresOn,
      const GetUserDelegationKeyOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::ServiceClient::GetServiceUserDelegationKeyOptions protocolLayerOptions;
    protocolLayerOptions.KeyInfo.Start = options.StartsOn.ToString(
        Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate);
    protocolLayerOptions.KeyInfo.Expiry = expiresOn.ToString(
        Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate);
    return _detail::ServiceClient::GetUserDelegationKey(
        *m_pipeline, m_serviceUrl, protocolLayerOptions, context);
  }

}}}} // namespace Azure::Storage::Files::Shares
